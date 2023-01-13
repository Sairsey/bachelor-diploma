#include "p_header.h"

void gdr::albedo_pass::Initialize(void)
{
    // 1) Compile our shaders
    Render->GetDevice().CompileShader(_T("bin/shaders/albedo/albedo.hlsl"), {}, shader_stage::Vertex, &VertexShader);
    Render->GetDevice().CompileShader(_T("bin/shaders/albedo/albedo.hlsl"), {}, shader_stage::Pixel, &PixelShader);

    // 2) Create root signature 
    {
        std::vector<CD3DX12_ROOT_PARAMETER> params;
        params.resize((int)root_parameters_draw_indices::total_root_parameters);

        params[(int)root_parameters_draw_indices::globals_buffer_index].InitAsConstantBufferView(GDRGPUGlobalDataConstantBufferSlot);
        params[(int)root_parameters_draw_indices::index_buffer_index].InitAsConstants(sizeof(GDRGPUObjectIndices) / sizeof(int32_t), (int)GDRGPUObjectIndicesConstantBufferSlot);
        static_assert(GDRGPUObjectIndicesRecordRootIndex == (int)root_parameters_draw_indices::index_buffer_index, "Index buffer not in right root signature index");
        params[(int)root_parameters_draw_indices::object_transform_pool_index].InitAsShaderResourceView(GDRGPUObjectTransformPoolSlot);
        params[(int)root_parameters_draw_indices::node_transform_pool_index].InitAsShaderResourceView(GDRGPUNodeTransformPoolSlot);
        params[(int)root_parameters_draw_indices::material_pool_index].InitAsShaderResourceView(GDRGPUMaterialPoolSlot);

        {
            CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
            rootSignatureDesc.Init((UINT)params.size(), &params[0], 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
            Render->GetDevice().CreateRootSignature(rootSignatureDesc, &RootSignature);
        }
    }

    // 3) Create PSO
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { defaultInputElementLayout, _countof(defaultInputElementLayout) };
        psoDesc.pRootSignature = RootSignature;
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(VertexShader);
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(PixelShader);
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = TRUE;
        psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = Render->RenderTargetsSystem->TargetParams[(int)render_targets_enum::target_display].Format;
        psoDesc.DSVFormat = Render->DepthBuffer.Resource->GetDesc().Format;
        psoDesc.SampleDesc.Count = 1;

        Render->GetDevice().CreatePSO(psoDesc, &PSO);
    }
    // 4) Create Command signature
    Render->GetDevice().GetDXDevice()->CreateCommandSignature(&Render->DrawCommandsSystem->commandSignatureDesc, RootSignature, IID_PPV_ARGS(&CommandSignature));
}

void gdr::albedo_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
    static mth::matr4f VP = mth::matr::Identity();
    Render->RenderTargetsSystem->Set(currentCommandList, render_targets_enum::target_display);

    // set common params
    currentCommandList->SetPipelineState(PSO);
    currentCommandList->SetGraphicsRootSignature(RootSignature);
    currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // just iterate for every draw call
    for (auto& i : Render->DrawCommandsSystem->CPUData)
    {
        currentCommandList->IASetVertexBuffers(0, 1, &i.VertexBuffer);
        currentCommandList->IASetIndexBuffer(&i.IndexBuffer);
        {
            currentCommandList->SetGraphicsRootConstantBufferView(
                (int)root_parameters_draw_indices::globals_buffer_index,
                Render->GlobalsSystem->GPUData.Resource->GetGPUVirtualAddress());
            currentCommandList->SetGraphicsRoot32BitConstants(
                (int)root_parameters_draw_indices::index_buffer_index,
                sizeof(GDRGPUObjectIndices) / sizeof(int32_t),
                &i.Indices, 0);
            currentCommandList->SetGraphicsRootShaderResourceView(
                (int)root_parameters_draw_indices::object_transform_pool_index,
                Render->ObjectTransformsSystem->GPUData.Resource->GetGPUVirtualAddress());
            currentCommandList->SetGraphicsRootShaderResourceView(
                (int)root_parameters_draw_indices::node_transform_pool_index,
                Render->NodeTransformsSystem->GPUData.Resource->GetGPUVirtualAddress());
            currentCommandList->SetGraphicsRootShaderResourceView(
                (int)root_parameters_draw_indices::material_pool_index,
                Render->MaterialsSystem->GPUData.Resource->GetGPUVirtualAddress());
        }

        currentCommandList->DrawIndexedInstanced(i.DrawArguments.IndexCountPerInstance, 1, 0, 0, 0);
    }
}

gdr::albedo_pass::~albedo_pass(void)
{
    CommandSignature->Release();
    PSO->Release();
    RootSignature->Release();
    VertexShader->Release();
    PixelShader->Release();
}