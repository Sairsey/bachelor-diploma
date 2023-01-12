#pragma once
#include "def.h"


/* Project namespace */
namespace gdr
{
  class engine;
  class base_pass;
  class globals_subsystem;
  class render_targets_subsystem;
  class object_transforms_subsystem;
  class node_transforms_subsystem;
  class draw_commands_subsystem;
  class geometry_subsystem;
  struct render_runtime_params
  {
    bool IsIndirect = false;     // Enables indirect draw
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
      // Depth-Stencil buffer support
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
      mth::cam PlayerCamera;
      // Depth buffer resource
      GPUResource DepthBuffer;

      // All supported passes
      std::vector<base_pass*> Passes;

      // Subsystems
      globals_subsystem* GlobalsSystem; // Store camera info and other important stuff, which is relevant per frame
      render_targets_subsystem* RenderTargetsSystem; //System to change and use different render targets
      object_transforms_subsystem* ObjectTransformsSystem; // System to store Root transforms of objects and AABB-s for culling
      node_transforms_subsystem* NodeTransformsSystem; // System to store hierarchical transform data.
      draw_commands_subsystem* DrawCommandsSystem; // support of SRVs and UAVs for indirect draw
      geometry_subsystem* GeometrySystem; // support of geometry creation
#if 0
      indirect_support* IndirectSystem; // support of SRVs and UAVs for indirect draw
      geometry_support *GeometrySystem; // support of geometry creation
      
      
      materials_support *MaterialsSystem; //System to store info about materials
      textures_support* TexturesSystem; //System to store info about textures
      cube_textures_support* CubeTexturesSystem; // System to store info about cube textures
      light_sources_support* LightsSystem; //System to store info about lights      
      hier_depth_support* HierDepth; //System to store and generate Hierarhical Depth Texture
      screenshot_support* ScreenshotsSystem; //System to store and generate Hierarhical Depth Texture
#endif

      long long UpdateBuffersTime;
      long long CPUDrawFrameTime;
      device_time_query DeviceFrameCounter;
  };
}