#pragma once
#include "def.h"

#include "utils/utils.h"
#include "window/win.h"

#include "render/device/command_queue.h"
#include "render/device/gpu_ring_buffer.h"
#include "render/device/d3dinclude.h"
#include "render/device/device.h"
#include "render/device/device_time_query.h"

#include "render/render.h"

#include "utils/resource_pool_subsystem.h"

#include "render/subsystems/globals_subsystem.h"
#include "render/subsystems/geometry_subsystem.h"
#include "render/subsystems/object_transforms_subsystem.h"
#include "render/subsystems/node_transforms_subsystem.h"
#include "render/subsystems/render_target_subsystem.h"
#include "render/subsystems/draw_commands_subsystem.h"
#include "render/subsystems/texture_subsystem.h"
#include "render/subsystems/cube_texture_subsystem.h"
#include "render/subsystems/materials_subsystem.h"
#include "render/subsystems/shadow_maps_subsystem.h"
#include "render/subsystems/lights_subsystem.h"
#include "render/subsystems/luminance_subsystem.h"
#include "render/subsystems/enviroment_subsystem.h"
#include "render/subsystems/bone_mapping_subsystem.h"
#include "render/subsystems/oit_transparency_subsystem.h"

#if 0
#include "render/subsystems/hier_depth_support.h"
#include "render/subsystems/screenshot_subsystem.h"
#endif

#include "render/passes/base_pass.h"
/// Preprocess
  #include "render/passes/visibility_frustum_pass.h"
  #include "render/passes/visibility_hier_depth_pass.h"
  #include "render/passes/visibility_occlusion_pass.h"
  #include "render/passes/shadow_map_pass.h"

/// Main Pass
  #include "render/passes/albedo_pass.h"
  #include "render/passes/skybox_pass.h"
  #include "render/passes/oit_color_pass.h"
  #include "render/passes/oit_compose_pass.h"

/// Postprocess
  #include "render/passes/luminance_pass.h"
  #include "render/passes/tonemap_pass.h"
  #include "render/passes/fxaa_pass.h"
  #include "render/passes/copy_pass.h"
  // ui
  #include "render/passes/debug_hier_pass.h"
  #include "render/passes/debug_aabb_pass.h"
  #include "render/passes/imgui_pass.h"

#if 0
#include "render/passes/hier_depth_pass.h"
#include "render/passes/occlusion_pass.h"


#include "render/passes/order_transparent_pass.h"
#include "render/passes/oit_transparent_pass.h"

#include "render/passes/debug_pass.h"

#include "render/passes/skybox_pass.h"

#include "render/passes/hdr_copy_pass.h"
#include "render/passes/tonemap_pass.h"

#endif

#include "animations/animations_manager.h"

#include "models/render_model.h"
#include "models/model_import_data.h"

#include "physics/physics_manager.h"
#include "models/models_manager.h"

#include "raycast/raycast_manager.h"

#include "timer/timer.h"
#include "input/input.h"

#include "units/unit_base.h"
#include "engine/eng.h"