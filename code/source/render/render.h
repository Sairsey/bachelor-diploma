#pragma once
#include "def.h"


/* Project namespace */
namespace gdr
{
  class engine;
  class base_pass;
  class geometry_support;
  class globals_support;
  class object_support;
  class transforms_support;
  class materials_support;
  class indirect_support;
  class textures_support;
  class cube_textures_support;
  class light_sources_support;
  class render_targets_support;

  struct render_runtime_params
  {
    bool IsIndirect = true;     // Enables indirect draw
    bool IsCulling = false;      // Enables culling
    bool IsTransparent = false; // Enables Transparency support
  };

  /* Render representation class */
  class render
  {
    private:
      // Create Depth-stencil buffer
      bool CreateDepthStencil(void);

      // Current Engine 
      engine *Engine;
      // Init flag
      bool IsInited;
      // Device
      device Device;
      // All supported passes
      std::vector<base_pass *> Passes;

      // Depth-Stencil buffer support
      GPUResource DepthBuffer;
      ID3D12DescriptorHeap* DSVHeap;
    public:
      /* Default Contructor */
      render();

      /* Initialize function 
       * ARGUMENTS:
       *      - Engine to init
       *          (engine *) Eng
       * RETURNS: true if success, false otherwise
       */
      bool Init(engine *Eng);

      /* Draw frame function */
      void DrawFrame(void);

      /* Resize frame function
       * ARGUMENTS:
       *      - new frame width
       *          (UINT) w
       *      - new frame height
       *          (UINT) h
       * RETURNS: None
       */
      void Resize(UINT w, UINT h);

      /* Get Device function */
      device &GetDevice(void) {return Device;}

      /* Get Engine function */
      engine *GetEngine(void);

      /* Add IMGUI lambda*/
      void AddLambdaForIMGUI(std::function<void(void)> func);

      /* Terminate function
       * ARGUMENTS: None
       * RETURNS: None
       */
      void Term(void);

      // Default destructor
      ~render();

      // Current params
      render_runtime_params Params;
      // Current player camera
      mth::cam3<float> PlayerCamera;
      // Subsystems
      globals_support *GlobalsSystem; // Store camera info and other important stuff
      indirect_support* IndirectSystem; // support of SRVs and UAVs for indirect draw
      geometry_support *GeometrySystem; // support of geometry creation
      transforms_support* TransformsSystem; // support of object Transforms
      object_support *ObjectSystem; // Helper of every subsystem
      materials_support *MaterialsSystem; //System to store info about materials
      textures_support* TexturesSystem; //System to store info about textures
      cube_textures_support* CubeTexturesSystem; // System to store info about cube textures
      light_sources_support* LightsSystem; //System to store info about lights
      render_targets_support* RenderTargets; //System to store info about lights

      device_time_query DeviceFrameCounter;
  };
}