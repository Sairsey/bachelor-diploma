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
  class materials_subsystem;
  class textures_subsystem;
  class cube_textures_subsystem;
  class lights_subsystem;
  class luminance_subsystem;
  class enviroment_subsystem;
  struct render_runtime_params
  {
    bool IsIndirect = true;        // Enables indirect draw

    bool IsUploadEveryFrame = true; // If true, each frame contains 2 cmd lists, 1-upload and 1-draw. They are trying to sync one to another.
    
    bool IsFrustumCulling = true;  // Enables Frustum Culling
    bool IsOccusionCulling = true; // Enables Occlusion Culling (avalible only with Frustum Culling and only in Indirect mode)
    bool IsViewLocked = false;     // Lock View for culling debugging (avalible only with Frustum Culling or Frustum + Occlusion)
    
    bool IsShowAABB = false;       // Show AABBs of objects
    bool IsShowHier = false;       // Show Hierarchy of objects

    bool IsTonemapping = true;    // Enable tonemapping or not
    float SceneExposure = 8.0;     // Exposure of scene

    bool IsIBL = true;            // Enable image based lighting
  };
  struct render_creation_params
  {
    size_t MaxTextureAmount = 256;   // maximum amount of textures allocated
    size_t MaxCubeTextureAmount = 6; // maximum amount of cube textures allocated
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

      void EnableFullscreen();

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
      const render_creation_params CreationParams;

      // Current player camera
      mth::cam PlayerCamera;
      // Depth buffer resource
      GPUResource DepthBuffer;

      // All supported passes
      std::vector<base_pass*> Passes;

      // Subsystems
      globals_subsystem* GlobalsSystem;                    // Store camera info and other important stuff, which is relevant per frame
      render_targets_subsystem* RenderTargetsSystem;       // System to change and use different render targets
      object_transforms_subsystem* ObjectTransformsSystem; // System to store Root transforms of objects and AABB-s for culling
      node_transforms_subsystem* NodeTransformsSystem;     // System to store hierarchical transform data.
      draw_commands_subsystem* DrawCommandsSystem;         // support of SRVs and UAVs for indirect draw
      geometry_subsystem* GeometrySystem;                  // support of geometry creation
      materials_subsystem* MaterialsSystem;                // system to store info about materials
      textures_subsystem* TexturesSystem;                  // System to store info about textures
      lights_subsystem* LightsSystem;                      // System to store info about light sources
      cube_textures_subsystem* CubeTexturesSystem;         // System to store info about cube textures
      luminance_subsystem* LuminanceSystem;                // System to store info about Luminance
      enviroment_subsystem* EnviromentSystem;              // System to store info about Enviroment textures (Skybox for example)
#if 0
      screenshot_support* ScreenshotsSystem; //System to store and generate Hierarhical Depth Texture
#endif
      long long UpdateBuffersTime;
      long long CPUDrawFrameTime;
      device_time_query DeviceFrameCounter;
  };
}