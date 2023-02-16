// Max supported and min supported versions of JSONS

#define MAX_SUPPORTED_VERSION 1
#define MIN_SUPPORTED_VERSION 1

enum EditorArgsTypes
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

enum EditorNodeTypes
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
// GDR_BLUEPRINT_NODE(editor_node_function, "Model load", 1, 1, ModelLoad, {editor_arg_string}, {editor_arg_gdr_index})

#define GDR_BLUEPRINT_LIST \
GDR_BLUEPRINT_NODE(editor_node_event, "Event", "Initialize", 0, 0, {}, {}) \
GDR_BLUEPRINT_NODE(editor_node_event, "Event", "Response", 0, 0, {}, {}) \
GDR_BLUEPRINT_NODE(editor_node_event, "Event", "Physics Response", 0, 0, {}, {}) \
GDR_BLUEPRINT_NODE(editor_node_event, "Event", "Deinitialize", 0, 0, {}, {}) \
\
GDR_BLUEPRINT_NODE(editor_node_function, "Time", "Get time", 0, 1, {}, { editor_arg_float }, {}, { "Time in seconds" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Time", "Get delta time", 0, 1, {}, { editor_arg_float }, {}, { "Delta time in seconds" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Time", "Set pause", 1, 0, { editor_arg_float }, {}, {"Is pause"}, {}) \
GDR_BLUEPRINT_NODE(editor_node_function, "Time", "Get pause", 0, 1, {}, { editor_arg_float }, {}, { "Is pause" }) \
\
GDR_BLUEPRINT_NODE(editor_node_function, "Model", "Load", 1, 1, {editor_arg_string}, {editor_arg_gdr_index}, {"Path"}, {"Model index"}) \
GDR_BLUEPRINT_NODE(editor_node_function, "Model", "Clone", 1, 1, { editor_arg_gdr_index }, { editor_arg_gdr_index }, { "Model index" }, { "Cloned model index" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Model", "Delete", 1, 0, { editor_arg_gdr_index }, {}, { "Model index" }, {}) \
GDR_BLUEPRINT_NODE(editor_node_function, "Model", "Get root transform", 1, 1, {editor_arg_gdr_index}, {editor_arg_gdr_index}, {"Model index"}, {"Object transform index"}) \
\
GDR_BLUEPRINT_NODE(editor_node_function, "Animation", "Load", 1, 1, { editor_arg_string }, { editor_arg_gdr_index }, { "Path" }, { "Animation index" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Animation", "Delete", 1, 0, { editor_arg_gdr_index }, {}, { "Animation index" }, {}) \
GDR_BLUEPRINT_NODE(editor_node_function, "Animation", "Get Duration", 1, 1, { editor_arg_gdr_index }, { editor_arg_float }, { "Animation index" }, { "Animation duration" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Animation", "Set animation frame", 2, 0, { editor_arg_gdr_index, editor_arg_gdr_index, editor_arg_float, editor_arg_float, editor_arg_float }, {}, { "Animation index", "Model index", "Time", "Offset", "Duration" }, {}) \
\
GDR_BLUEPRINT_NODE(editor_node_function, "Texture", "Load", 1, 1, { editor_arg_string }, { editor_arg_gdr_index }, { "Path" }, { "Texture index" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Texture", "Delete", 1, 0, { editor_arg_gdr_index }, {}, { "Texture index" }, {}) \
\
GDR_BLUEPRINT_NODE(editor_node_function, "Object Transform", "Create", 0, 1, {}, { editor_arg_gdr_index }, { "" }, { "Object transform index" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Object Transform", "Delete", 1, 0, { editor_arg_gdr_index }, {}, { "Object transform index" }, {}) \
GDR_BLUEPRINT_NODE(editor_node_function, "Object Transform", "Set", 2, 0, { editor_arg_gdr_index, editor_arg_matr }, {}, { "Object transform index", "Transformation Matrix"}, {}) \
GDR_BLUEPRINT_NODE(editor_node_function, "Object Transform", "Get", 1, 1, { editor_arg_gdr_index }, { editor_arg_matr }, { "Object transform index"}, { "Transformation Matrix" }) \
\
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Compose matr", 3, 1, { editor_arg_float3, editor_arg_float4, editor_arg_float3 }, { editor_arg_matr }, { "Position", "Rotation", "Scale"}, {"Result matrix"}) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Decompose matr", 1, 3, { editor_arg_matr }, { editor_arg_float3, editor_arg_float4, editor_arg_float3 }, { "Result matrix" }, { "Position", "Rotation", "Scale" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Add float", 2, 1, { editor_arg_float, editor_arg_float }, { editor_arg_float }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Add float2", 2, 1, { editor_arg_float2, editor_arg_float2 }, { editor_arg_float2 }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Add float3", 2, 1, { editor_arg_float3, editor_arg_float3 }, { editor_arg_float3 }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Add float4", 2, 1, { editor_arg_float4, editor_arg_float4 }, { editor_arg_float4 }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Substract float", 2, 1, { editor_arg_float, editor_arg_float }, { editor_arg_float }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Substract float2", 2, 1, { editor_arg_float2, editor_arg_float2 }, { editor_arg_float2 }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Substract float3", 2, 1, { editor_arg_float3, editor_arg_float3 }, { editor_arg_float3 }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Substract float4", 2, 1, { editor_arg_float4, editor_arg_float4 }, { editor_arg_float4 }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Multiply float", 2, 1, { editor_arg_float, editor_arg_float }, { editor_arg_float }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Multiply per component float2", 2, 1, { editor_arg_float2, editor_arg_float2 }, { editor_arg_float2 }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Multiply per component float3", 2, 1, { editor_arg_float3, editor_arg_float3 }, { editor_arg_float3 }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Multiply per component float4", 2, 1, { editor_arg_float4, editor_arg_float4 }, { editor_arg_float4 }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Divide float", 2, 1, { editor_arg_float, editor_arg_float }, { editor_arg_float }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Multiply float2 per float", 2, 1, { editor_arg_float2, editor_arg_float }, { editor_arg_float2 }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Multiply float3 per float", 2, 1, { editor_arg_float3, editor_arg_float }, { editor_arg_float3 }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Multiply float4 per float", 2, 1, { editor_arg_float4, editor_arg_float }, { editor_arg_float4 }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Multiply matr per matr", 2, 1, { editor_arg_matr, editor_arg_matr }, { editor_arg_matr }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Dot product float2", 2, 1, { editor_arg_float2, editor_arg_float2 }, { editor_arg_float }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Dot product float3", 2, 1, { editor_arg_float3, editor_arg_float3 }, { editor_arg_float }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Dot product float4", 2, 1, { editor_arg_float4, editor_arg_float4 }, { editor_arg_float }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Cross product float3", 2, 1, { editor_arg_float3, editor_arg_float3 }, { editor_arg_float3 }, { "First", "Second" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Normalize float2", 1, 1, { editor_arg_float2 }, { editor_arg_float2 }, { "Input" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Normalize float3", 1, 1, { editor_arg_float3 }, { editor_arg_float3 }, { "Input" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Normalize float4", 1, 1, { editor_arg_float4 }, { editor_arg_float4 }, { "Input" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Matr rotate X", 1, 1, { editor_arg_float }, { editor_arg_matr }, { "Angle in degrees" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Matr rotate Y", 1, 1, { editor_arg_float }, { editor_arg_matr }, { "Angle in degrees" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Matr rotate Z", 1, 1, { editor_arg_float }, { editor_arg_matr }, { "Angle in degrees" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Matr rotate", 1, 1, { editor_arg_float, editor_arg_float3 }, { editor_arg_matr }, { "Angle in degrees", "Axis"}, {"Result"}) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Matr translate", 1, 1, { editor_arg_float3 }, { editor_arg_matr }, { "Translation vector" }, { "Result" }) \
GDR_BLUEPRINT_NODE(editor_node_function, "Math", "Matr scale", 1, 1, { editor_arg_float3 }, { editor_arg_matr }, { "Scale vector" }, { "Result" }) \
\
GDR_BLUEPRINT_NODE(editor_node_workflow, "Workflow", "If Key pressed", 1, 1, {editor_arg_string}, {editor_arg_float}, {"Key"}, {"Flag"}) \
GDR_BLUEPRINT_NODE(editor_node_workflow, "Workflow", "If Key released", 1, 1, {editor_arg_string}, {editor_arg_float}, {"Key"}, {"Flag"}) \
GDR_BLUEPRINT_NODE(editor_node_workflow, "Workflow", "If Key clicked", 1, 1, {editor_arg_string}, {editor_arg_float}, {"Key"}, {"Flag"})
