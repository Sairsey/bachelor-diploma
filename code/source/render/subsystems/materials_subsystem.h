#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

/* Project namespace */
namespace gdr
{
  // Material pool representation class
  class materials_subsystem
  {
  private:
    // pointer to Render
    render* Render;
    size_t StoredSize; // stored copy to check if data has changed

    D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
    D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor;

    // transform chunk for update
    const int CHUNK_SIZE = 128 * sizeof(GDRGPUMaterial);

    std::vector<bool> ChunkMarkings;
  public:
    // default constructor
    materials_subsystem(render* Rnd);

    void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

    gdr_index AddElementInPool()
    {
        GDRGPUMaterial mat;
        mat.ShadeType = MATERIAL_SHADER_PHONG;
        GDRGPUMaterialPhongGetAmbient(mat) = mth::vec3f(0.f, 0.f, 0.f);
        GDRGPUMaterialPhongGetDiffuse(mat) = mth::vec3f(0.7f, 0.7f, 0.7f);
        GDRGPUMaterialPhongGetSpecular(mat) = mth::vec3f(0.1f, 0.1f, 0.1f);
        GDRGPUMaterialPhongGetShiness(mat) = 30.0f;
        GDRGPUMaterialPhongGetAmbientMapIndex(mat) = NONE_INDEX;
        GDRGPUMaterialPhongGetDiffuseMapIndex(mat) = NONE_INDEX;
        GDRGPUMaterialPhongGetSpecularMapIndex(mat) = NONE_INDEX;
        GDRGPUMaterialPhongGetNormalMapIndex(mat) = NONE_INDEX;
        CPUData.push_back(mat);
        MarkChunkByMaterialIndex(CPUData.size() - 1);
        return CPUData.size() - 1;
    }

    std::vector<GDRGPUMaterial> CPUData;  // data stored in CPU
    GPUResource GPUData; // data stored in GPU

    void MarkChunkByMaterialIndex(gdr_index index);

    // desctructor
    ~materials_subsystem();
  };
}
