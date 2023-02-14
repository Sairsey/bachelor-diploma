#pragma once
#include "def.h"
#include "../bin/shaders/shared/cpu_gpu_shared.h"

/* Project namespace */
namespace gdr
{
    // Enviroment data representation class
    class enviroment_subsystem
    {
    private:
        render* Render; // pointer on Render
        D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
        D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor;
        GDRGPUEnviromentData CPUData;  // data stored in CPU
        GPUResource GPUData;  // data stored in GPU
        bool NeedUpdate = false;
    public:
        // Constructor
        enviroment_subsystem(render* Rnd);

        // Set globals to Default values
        void SetDefault(void);

        // Update data stored on GPU
        void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList);

        // Get element the way it can be edited
        GDRGPUEnviromentData& GetEditable();

        // Get element the way it cannot be edited
        const GDRGPUEnviromentData& Get() const;

        // Get GPU resource
        GPUResource& GetGPUResource() { return GPUData; };

        // Destructor 
        ~enviroment_subsystem(void);
    };
}