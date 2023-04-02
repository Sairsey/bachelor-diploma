// Max supported and min supported versions of JSONS
#pragma once

#define MAX_SUPPORTED_VERSION 2
#define MIN_SUPPORTED_VERSION 1

enum struct EditorArgsTypes
{
  editor_arg_none = 0,
  editor_arg_string,
  editor_arg_float,
  editor_arg_float2,
  editor_arg_float3,
  editor_arg_float4,
  editor_arg_matr,
  editor_arg_gdr_index,

  editor_arg_count
};

enum struct EditorNodeTypes
{
  editor_node_none = 0,
  editor_node_function, // like "Add two numbers" or "Update transform"
  editor_node_workflow, // "if" statements
  editor_node_event,    // "Response", "Init" and other events

  editor_node_count
};

// All nodes are written to be used with one of this defines
// #define GDR_BLUEPRINT_NODE(type, filter, name, number_of_input_args, number_of_output_args, input_args_types, output_args_types, input_args_names, output_args_names)
// For example
// GDR_BLUEPRINT_NODE(editor_node_function, "Model load", 1, 1, ModelLoad, P99_PROTECT({editor_arg_string}), P99_PROTECT({editor_arg_gdr_index}))

#define P99_PROTECT(...) __VA_ARGS__

// reserved indices
#define GDR_EDITOR_EVENT_INIT_LIBRARY_INDEX 0
#define GDR_EDITOR_EVENT_RESPONSE_LIBRARY_INDEX 1
#define GDR_EDITOR_EVENT_RESPONSE_PHYS_LIBRARY_INDEX 2
#define GDR_EDITOR_EVENT_DEINIT_LIBRARY_INDEX 3

#define GDR_BLUEPRINT_LIST \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_event, "Event", "Initialize", 0, 0, P99_PROTECT({}), P99_PROTECT({}), P99_PROTECT({}), P99_PROTECT({})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_event, "Event", "Response", 0, 0, P99_PROTECT({}), P99_PROTECT({}), P99_PROTECT({}), P99_PROTECT({})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_event, "Event", "Physics Response", 0, 0, P99_PROTECT({}), P99_PROTECT({}), P99_PROTECT({}), P99_PROTECT({})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_event, "Event", "Deinitialize", 0, 0, P99_PROTECT({}), P99_PROTECT({}), P99_PROTECT({}), P99_PROTECT({})) \
\
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Time", "Get time", 0, 1, P99_PROTECT({}), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({}), P99_PROTECT({ "Time in seconds" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Time", "Get delta time", 0, 1, P99_PROTECT({}), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({}), P99_PROTECT({ "Delta time in seconds" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Time", "Set pause", 1, 0, P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({}), P99_PROTECT({ "Is pause" }), P99_PROTECT({})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Time", "Get pause", 0, 1, P99_PROTECT({}), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({}), P99_PROTECT({ "Is pause" })) \
\
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Model", "Load", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_string }), P99_PROTECT({ EditorArgsTypes::editor_arg_gdr_index }), P99_PROTECT({ "Path" }), P99_PROTECT({ "Model index" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Model", "Clone", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_gdr_index }), P99_PROTECT({ EditorArgsTypes::editor_arg_gdr_index }), P99_PROTECT({ "Model index" }), P99_PROTECT({ "Cloned model index" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Model", "Delete", 1, 0, P99_PROTECT({ EditorArgsTypes::editor_arg_gdr_index }), P99_PROTECT({}), P99_PROTECT({ "Model index" }), P99_PROTECT({})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Model", "Get root transform", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_gdr_index }), P99_PROTECT({ EditorArgsTypes::editor_arg_gdr_index }), P99_PROTECT({"Model index"}), P99_PROTECT({"Object transform index"})) \
\
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Animation", "Load", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_string }), P99_PROTECT({ EditorArgsTypes::editor_arg_gdr_index }), P99_PROTECT({ "Path" }), P99_PROTECT({ "Animation index" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Animation", "Delete", 1, 0, P99_PROTECT({ EditorArgsTypes::editor_arg_gdr_index }), P99_PROTECT({}), P99_PROTECT({ "Animation index" }), P99_PROTECT({})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Animation", "Get Duration", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_gdr_index }), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({ "Animation index" }), P99_PROTECT({ "Animation duration" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Animation", "Set animation frame", 5, 0, P99_PROTECT({ EditorArgsTypes::editor_arg_gdr_index, EditorArgsTypes::editor_arg_gdr_index, EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float }), P99_PROTECT({}), P99_PROTECT({ "Animation index", "Model index", "Time", "Offset", "Duration" }), P99_PROTECT({})) \
\
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Texture", "Load", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_string }), P99_PROTECT({ EditorArgsTypes::editor_arg_gdr_index }), P99_PROTECT({ "Path" }), P99_PROTECT({ "Texture index" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Texture", "Delete", 1, 0, P99_PROTECT({ EditorArgsTypes::editor_arg_gdr_index }), P99_PROTECT({}), P99_PROTECT({ "Texture index" }), P99_PROTECT({})) \
\
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Object Transform", "Create", 0, 1, P99_PROTECT({}), P99_PROTECT({ EditorArgsTypes::editor_arg_gdr_index }), P99_PROTECT({ }), P99_PROTECT({ "Object transform index" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Object Transform", "Delete", 1, 0, P99_PROTECT({ EditorArgsTypes::editor_arg_gdr_index }), P99_PROTECT({}), P99_PROTECT({ "Object transform index" }), P99_PROTECT({})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Object Transform", "Set", 2, 0, P99_PROTECT({ EditorArgsTypes::editor_arg_gdr_index, EditorArgsTypes::editor_arg_matr }), P99_PROTECT({}), P99_PROTECT({ "Object transform index", "Transformation Matrix"}), P99_PROTECT({})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Object Transform", "Get", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_gdr_index }), P99_PROTECT({ EditorArgsTypes::editor_arg_matr }), P99_PROTECT({ "Object transform index"}), P99_PROTECT({ "Transformation Matrix" })) \
\
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Compose float2", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float}), P99_PROTECT({ EditorArgsTypes::editor_arg_float2 }), P99_PROTECT({ "X", "Y" }), P99_PROTECT({"Result"})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Decompose float2", 1, 2, P99_PROTECT({ EditorArgsTypes::editor_arg_float2 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float }), P99_PROTECT({ "Input" }), P99_PROTECT({"X", "Y"})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Compose float3", 3, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float}), P99_PROTECT({ EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ "X", "Y", "Z" }), P99_PROTECT({"Result"})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Decompose float3", 1, 3, P99_PROTECT({ EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float}), P99_PROTECT({"Input"}), P99_PROTECT({ "X", "Y", "Z" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Compose float4", 4, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float}), P99_PROTECT({ EditorArgsTypes::editor_arg_float4 }), P99_PROTECT({ "X", "Y", "Z", "W" }), P99_PROTECT({"Result"})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Decompose float4", 1, 4, P99_PROTECT({ EditorArgsTypes::editor_arg_float4 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float}), P99_PROTECT({"Input"}), P99_PROTECT({ "X", "Y", "Z", "W" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Compose matr", 3, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float3, EditorArgsTypes::editor_arg_float4, EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ EditorArgsTypes::editor_arg_matr }), P99_PROTECT({ "Position", "Rotation", "Scale"}), P99_PROTECT({"Result matrix"})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Decompose matr", 1, 3, P99_PROTECT({ EditorArgsTypes::editor_arg_matr }), P99_PROTECT({ EditorArgsTypes::editor_arg_float3, EditorArgsTypes::editor_arg_float4, EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ "Result matrix" }), P99_PROTECT({ "Position", "Rotation", "Scale" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Add float", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float }), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Add float2", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float2, EditorArgsTypes::editor_arg_float2 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float2 }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Add float3", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float3, EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Add float4", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float4, EditorArgsTypes::editor_arg_float4 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float4 }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Substract float", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float }), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Substract float2", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float2, EditorArgsTypes::editor_arg_float2 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float2 }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Substract float3", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float3, EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Substract float4", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float4, EditorArgsTypes::editor_arg_float4 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float4 }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Multiply float", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float }), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Multiply per component float2", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float2, EditorArgsTypes::editor_arg_float2 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float2 }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Multiply per component float3", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float3, EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Multiply per component float4", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float4, EditorArgsTypes::editor_arg_float4 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float4 }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Divide float", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float,EditorArgsTypes::editor_arg_float }), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Multiply float2 per float", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float2, EditorArgsTypes::editor_arg_float }), P99_PROTECT({ EditorArgsTypes::editor_arg_float2 }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Multiply float3 per float", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float3, EditorArgsTypes::editor_arg_float }), P99_PROTECT({ EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Multiply float4 per float", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float4, EditorArgsTypes::editor_arg_float }), P99_PROTECT({ EditorArgsTypes::editor_arg_float4 }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Multiply matr per matr", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_matr, EditorArgsTypes::editor_arg_matr }), P99_PROTECT({ EditorArgsTypes::editor_arg_matr }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Dot product float2", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float2, EditorArgsTypes::editor_arg_float2 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Dot product float3", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float3, EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Dot product float4", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float4, EditorArgsTypes::editor_arg_float4 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Cross product float3", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float3, EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ "First", "Second" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Normalize float2", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float2 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float2 }), P99_PROTECT({ "Input" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Normalize float3", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ "Input" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Normalize float4", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float4 }), P99_PROTECT({ EditorArgsTypes::editor_arg_float4 }), P99_PROTECT({ "Input" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Matr rotate X", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({ EditorArgsTypes::editor_arg_matr }), P99_PROTECT({ "Angle in degrees" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Matr rotate Y", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({ EditorArgsTypes::editor_arg_matr }), P99_PROTECT({ "Angle in degrees" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Matr rotate Z", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({ EditorArgsTypes::editor_arg_matr }), P99_PROTECT({ "Angle in degrees" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Matr rotate", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ EditorArgsTypes::editor_arg_matr }), P99_PROTECT({ "Angle in degrees", "Axis"}), P99_PROTECT({"Result"})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Matr translate", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ EditorArgsTypes::editor_arg_matr }), P99_PROTECT({ "Translation vector" }), P99_PROTECT({ "Result" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Math", "Matr scale", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({ EditorArgsTypes::editor_arg_matr }), P99_PROTECT({ "Scale vector" }), P99_PROTECT({ "Result" })) \
\
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_workflow, "Workflow", "If Key pressed", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_string }), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({"Key"}), P99_PROTECT({"Flag"})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_workflow, "Workflow", "If Key clicked", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_string }), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({"Key"}), P99_PROTECT({"Flag"})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_workflow, "Workflow", "If float equal", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float }), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({"First", "Second"}), P99_PROTECT({"Flag"})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_workflow, "Workflow", "If float less", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float }), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({"First", "Second"}), P99_PROTECT({"Flag"})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_workflow, "Workflow", "If float less equal", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float }), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({"First", "Second"}), P99_PROTECT({"Flag"})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_workflow, "Workflow", "If float greater", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float }), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({"First", "Second"}), P99_PROTECT({"Flag"})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_workflow, "Workflow", "If float greater equal", 2, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_float, EditorArgsTypes::editor_arg_float }), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({"First", "Second"}), P99_PROTECT({"Flag"})) \
\
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Var", "Set float", 2, 0, P99_PROTECT({ EditorArgsTypes::editor_arg_string, EditorArgsTypes::editor_arg_float }), P99_PROTECT({}), P99_PROTECT({"Key", "Value"}), P99_PROTECT({})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Var", "Get float", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_string }), P99_PROTECT({ EditorArgsTypes::editor_arg_float }), P99_PROTECT({"Key"}), P99_PROTECT({ "Value" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Var", "Set float2", 2, 0, P99_PROTECT({ EditorArgsTypes::editor_arg_string, EditorArgsTypes::editor_arg_float2 }), P99_PROTECT({}), P99_PROTECT({"Key", "Value"}), P99_PROTECT({})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Var", "Get float2", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_string }), P99_PROTECT({ EditorArgsTypes::editor_arg_float2 }), P99_PROTECT({"Key"}), P99_PROTECT({ "Value" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Var", "Set float3", 2, 0, P99_PROTECT({ EditorArgsTypes::editor_arg_string, EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({}), P99_PROTECT({"Key", "Value"}), P99_PROTECT({})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Var", "Get float3", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_string }), P99_PROTECT({ EditorArgsTypes::editor_arg_float3 }), P99_PROTECT({"Key"}), P99_PROTECT({ "Value" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Var", "Set float4", 2, 0, P99_PROTECT({ EditorArgsTypes::editor_arg_string, EditorArgsTypes::editor_arg_float4 }), P99_PROTECT({}), P99_PROTECT({"Key", "Value"}), P99_PROTECT({})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Var", "Get float4", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_string }), P99_PROTECT({ EditorArgsTypes::editor_arg_float4 }), P99_PROTECT({"Key"}), P99_PROTECT({ "Value" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Var", "Set matr", 2, 0, P99_PROTECT({ EditorArgsTypes::editor_arg_string, EditorArgsTypes::editor_arg_matr }), P99_PROTECT({}), P99_PROTECT({"Key", "Value"}), P99_PROTECT({})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Var", "Get matr", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_string }), P99_PROTECT({ EditorArgsTypes::editor_arg_matr }), P99_PROTECT({"Key"}), P99_PROTECT({ "Value" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Var", "Set string", 2, 0, P99_PROTECT({ EditorArgsTypes::editor_arg_string, EditorArgsTypes::editor_arg_string }), P99_PROTECT({}), P99_PROTECT({"Key", "Value"}), P99_PROTECT({})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Var", "Get string", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_string }), P99_PROTECT({ EditorArgsTypes::editor_arg_string }), P99_PROTECT({"Key"}), P99_PROTECT({ "Value" })) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Var", "Set index", 2, 0, P99_PROTECT({ EditorArgsTypes::editor_arg_string, EditorArgsTypes::editor_arg_gdr_index }), P99_PROTECT({}), P99_PROTECT({"Key", "Value"}), P99_PROTECT({})) \
GDR_BLUEPRINT_NODE(EditorNodeTypes::editor_node_function, "Var", "Get index", 1, 1, P99_PROTECT({ EditorArgsTypes::editor_arg_string }), P99_PROTECT({ EditorArgsTypes::editor_arg_gdr_index }), P99_PROTECT({"Key"}), P99_PROTECT({ "Value" }))


