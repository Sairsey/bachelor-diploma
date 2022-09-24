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

#include "render/subsystems/globals_support.h"
#include "render/subsystems/geometry_support.h"
#include "render/subsystems/transforms_support.h"
#include "render/subsystems/object_support.h"
#include "render/subsystems/light_sources_support.h"
#include "render/subsystems/materials_support.h"
#include "render/subsystems/indirect_support.h"
#include "render/subsystems/texture_support.h"

#include "render/passes/base_pass.h"
#include "render/passes/albedo_pass.h"
#include "render/passes/order_transparent_pass.h"
#include "render/passes/oit_transparent_pass.h"
#include "render/passes/debug_pass.h"
#include "render/passes/imgui_pass.h"

#include "timer/timer.h"

#include "input/input.h"

#include "units/unit_base.h"
#include "engine/eng.h"