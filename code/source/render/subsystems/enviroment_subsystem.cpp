#include "p_header.h"

gdr::enviroment_subsystem::enviroment_subsystem(render* Rnd)
{
    Render = Rnd;
    NeedUpdate = true;
    GPUData.Resource = nullptr;
    SetDefault();
}

void gdr::enviroment_subsystem::SetDefault(void)
{
    CPUData.BRDFLUTIndex = NONE_INDEX;
    CPUData.IrradianceCubemapIndex = NONE_INDEX;
    CPUData.PrefilteredCubemapIndex = NONE_INDEX;
    CPUData.SkyboxIndex = NONE_INDEX;
    CPUData.MaxReflectionLod = 0;
    NeedUpdate = true;
}

void gdr::enviroment_subsystem::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
    // if buffers are not the same
    if (NeedUpdate)
    {
        if (GPUData.Resource == nullptr)
        {
            size_t CBufferGPUSize = Align(sizeof(GDRGPUEnviromentData), (size_t)D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

            Render->GetDevice().CreateGPUResource(CD3DX12_RESOURCE_DESC::Buffer({ CBufferGPUSize }),
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                GPUData,
                &CPUData,
                sizeof(GDRGPUEnviromentData));

            GPUData.Resource->SetName(L"Enviroment buffer");

            Render->GetDevice().AllocateStaticDescriptors(1, CPUDescriptor, GPUDescriptor);
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
            cbvDesc.BufferLocation = GPUData.Resource->GetGPUVirtualAddress();
            cbvDesc.SizeInBytes = (UINT)CBufferGPUSize;
            Render->GetDevice().GetDXDevice()->CreateConstantBufferView(&cbvDesc, CPUDescriptor);
            Render->GetDevice().TransitResourceState(
                pCommandList,
                GPUData.Resource,
                D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
        }
        else
        {
            Render->GetDevice().TransitResourceState(
                pCommandList,
                GPUData.Resource,
                D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
            Render->GetDevice().UpdateBuffer(pCommandList, GPUData.Resource, &CPUData, sizeof(GDRGPUEnviromentData));
            Render->GetDevice().TransitResourceState(
                pCommandList,
                GPUData.Resource,
                D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
        }
        NeedUpdate = false;
    }
}

// Get element the way it can be edited
GDRGPUEnviromentData& gdr::enviroment_subsystem::GetEditable()
{
    NeedUpdate = true;
    return CPUData;
}

// Get element the way it cannot be edited
const GDRGPUEnviromentData& gdr::enviroment_subsystem::Get() const
{
    return CPUData;
}

gdr::enviroment_subsystem::~enviroment_subsystem()
{
    if (GPUData.Resource != nullptr)
    {
        Render->GetDevice().ReleaseGPUResource(GPUData);
    }
}
