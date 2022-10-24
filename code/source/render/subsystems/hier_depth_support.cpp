#include "p_header.h"

gdr::hier_depth_support::hier_depth_support(render* Rnd)
{
  Render = Rnd;
  Texture.Resource = nullptr;

  for (int i = 0; i < MAX_AMOUNT_OF_MIPS; i++)
  {
    Render->GetDevice().AllocateStaticDescriptors(1, CPUDescriptorHandles[2 * i], GPUDescriptorHandles[2 * i]);
    Render->GetDevice().AllocateStaticDescriptors(1, CPUDescriptorHandles[2 * i + 1], GPUDescriptorHandles[2 * i + 1]);
  }

  // 1) Compile our shaders
  Render->GetDevice().CompileShader(_T("bin/shaders/GenerateMips.hlsl"), {}, shader_stage::Compute, &ComputeShader);

  // 2) Generate Root Signature
  {
    std::vector<CD3DX12_ROOT_PARAMETER> params;
    CD3DX12_DESCRIPTOR_RANGE descr[2] = {};

    params.resize((int)root_parameters_indices::total_root_parameters);
   
    params[(int)root_parameters_indices::mip_params_index].InitAsConstants(4, (int)buffer_registers::mip_params_register);

    {
      descr[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, (int)texture_registers::input_srv_register);
      params[(int)root_parameters_indices::input_srv_index].InitAsDescriptorTable(1, &descr[0]);
    }

    {
      descr[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, (int)texture_registers::output_uav_register);
      params[(int)root_parameters_indices::output_uav_index].InitAsDescriptorTable(1, &descr[1]);
    }

    // Texture samplers
    CD3DX12_STATIC_SAMPLER_DESC samplerDescs[1];
    samplerDescs[0].Init(0, D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init((UINT)params.size(), &params[0], sizeof(samplerDescs) / sizeof(samplerDescs[0]), samplerDescs, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    Render->GetDevice().CreateRootSignature(rootSignatureDesc, &RootSignature);
  }

  // 3) Generate PSO
  {
    // Describe and create the compute pipeline state object (PSO).
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = RootSignature;
    computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(ComputeShader);

    Render->GetDevice().CreateComputePSO(computePsoDesc, &PSO);
  }
}

static int CalculateMipMapsAmount(int W, int H)
{
  int res = 0;
  for (res = 0; W > 1 && H > 1; res++)
  {
    W /= 2;
    H /= 2;
  }
  return res;
}

void gdr::hier_depth_support::Generate(ID3D12GraphicsCommandList* pCommandList)
{
  int MipsAmount = CalculateMipMapsAmount(Render->DepthBuffer.Resource->GetDesc().Width, Render->DepthBuffer.Resource->GetDesc().Height);

  // Create or resize object
  if (Texture.Resource == nullptr
    || Texture.Resource->GetDesc().Width != Render->DepthBuffer.Resource->GetDesc().Width
    || Texture.Resource->GetDesc().Height != Render->DepthBuffer.Resource->GetDesc().Height)
  {
    if (Texture.Resource != nullptr) Render->GetDevice().ReleaseGPUResource(Texture);

    D3D12_RESOURCE_DESC textureResource = Render->DepthBuffer.Resource->GetDesc();

    textureResource.Format = DXGI_FORMAT_R32_FLOAT;
    textureResource.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    textureResource.MipLevels = MipsAmount;

    Render->GetDevice().CreateGPUResource(
      textureResource,
      D3D12_RESOURCE_STATE_COMMON,
      0,
      Texture
    );
    Texture.Resource->SetName(L"Hierachical Depth texture");
    Render->GetDevice().TransitResourceState(
      pCommandList,
      Texture.Resource,
      D3D12_RESOURCE_STATE_COPY_DEST,
      D3D12_RESOURCE_STATE_COMMON);
  }

  // Transit all resource states for copy
  {
    Render->GetDevice().TransitResourceState(
      pCommandList, 
      Render->DepthBuffer.Resource,
      D3D12_RESOURCE_STATE_DEPTH_WRITE,
      D3D12_RESOURCE_STATE_COPY_SOURCE);
  
    Render->GetDevice().TransitResourceState(
      pCommandList,
      Texture.Resource,
      D3D12_RESOURCE_STATE_COMMON,
      D3D12_RESOURCE_STATE_COPY_DEST);
  
    D3D12_TEXTURE_COPY_LOCATION Dest;
    Dest.pResource = Texture.Resource;
    Dest.SubresourceIndex = 0;
    Dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  
    D3D12_TEXTURE_COPY_LOCATION Source;
    Source.pResource = Render->DepthBuffer.Resource;
    Source.SubresourceIndex = 0;
    Source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  
    pCommandList->CopyTextureRegion(&Dest, 0, 0, 0, &Source, NULL);
  
    // Transit all resource states back
    Render->GetDevice().TransitResourceState(
      pCommandList,
      Texture.Resource,
      D3D12_RESOURCE_STATE_COPY_DEST,
      D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
  
    Render->GetDevice().TransitResourceState(
      pCommandList,
      Render->DepthBuffer.Resource,
      D3D12_RESOURCE_STATE_COPY_SOURCE,
      D3D12_RESOURCE_STATE_DEPTH_WRITE);
  }

  pCommandList->SetPipelineState(PSO);
  pCommandList->SetComputeRootSignature(RootSignature);

  struct {
    float TexX;
    float TexY;
    int W;
    int H;
  } data;

  data.W = Render->DepthBuffer.Resource->GetDesc().Width / 2;
  data.H = Render->DepthBuffer.Resource->GetDesc().Height / 2;

  data.TexX = 1.0 / data.W;
  data.TexY = 1.0 / data.H;

  // Calculate Mips
  for (int i = 1; i < MipsAmount; i++)
  {
    // generate correct SRV Descriptor
    {
      D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
      srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      srvDesc.Format = Texture.Resource->GetDesc().Format;
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
      srvDesc.Texture2D.MipLevels = 1;
      srvDesc.Texture2D.MostDetailedMip = i - 1;

      Render->GetDevice().GetDXDevice()->CreateShaderResourceView(Texture.Resource, &srvDesc, CPUDescriptorHandles[2 * (i - 1)]);
    }

    // generate correct UAV Descriptor
    {
      D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
      uavDesc.Format = Texture.Resource->GetDesc().Format;
      uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
      uavDesc.Texture2D.MipSlice = i;
      uavDesc.Texture2D.PlaneSlice = 0;

      Render->GetDevice().GetDXDevice()->CreateUnorderedAccessView(Texture.Resource, NULL, &uavDesc, CPUDescriptorHandles[2 * (i - 1) + 1]);
    }


    pCommandList->SetComputeRoot32BitConstants((int)root_parameters_indices::mip_params_index, 4, &data, 0);
    pCommandList->SetComputeRootDescriptorTable((int)root_parameters_indices::input_srv_index, GPUDescriptorHandles[2 * (i - 1)]);
    pCommandList->SetComputeRootDescriptorTable((int)root_parameters_indices::output_uav_index, GPUDescriptorHandles[2 * (i - 1) + 1]);

    pCommandList->Dispatch(max(ceil(data.W / 32.0), 1u), max(ceil(data.H / 32.0), 1u), 1);

    data.W /= 2;
    data.H /= 2;

    data.TexX = 1.0 / data.W;
    data.TexY = 1.0 / data.H;

    CD3DX12_RESOURCE_BARRIER bar = CD3DX12_RESOURCE_BARRIER::UAV(Texture.Resource);
    pCommandList->ResourceBarrier(1, &bar);
  }

  // Transit all resource states back
  Render->GetDevice().TransitResourceState(
    pCommandList,
    Texture.Resource,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
    D3D12_RESOURCE_STATE_COMMON);
}

gdr::hier_depth_support::~hier_depth_support()
{
  ComputeShader->Release();
  RootSignature->Release();
  PSO->Release();
  if (Texture.Resource != nullptr) Render->GetDevice().ReleaseGPUResource(Texture);
  ComputeShader->Release();
}
