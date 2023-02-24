#pragma once
#include "../unit_base.h"
#include "utils/editor/editors_modules_list.h"

// custom "any" struct
struct my_any
{
  EditorArgsTypes Type;
  private:
    std::string arg_string;
    float arg_float;
    mth::vec2<float> arg_float2;
    mth::vec3<float> arg_float3;
    mth::vec4<float> arg_float4;
    mth::matr4f arg_matr;
    gdr_index arg_gdr_index;

    bool converted[(int)EditorArgsTypes::editor_arg_count];

    // for string delimiter
    std::vector<std::string> split(std::string s, std::string delimiter) {
      size_t pos_start = 0, pos_end, delim_len = delimiter.length();
      std::string token;
      std::vector<std::string> res;

      while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
      }

      res.push_back(s.substr(pos_start));
      return res;
    }
  public:
    template<typename T>
    T Get(void)
    {
      GDR_FAILED("UNKNOWN TYPE");
      return T();
    }

    template<>
    std::string Get<std::string>(void)
    {
      if (!converted[(int)EditorArgsTypes::editor_arg_string])
        GDR_FAILED("cannot convert to string");
      return arg_string;
    }

    template<>
    float Get<float>(void)
    {
      if (!converted[(int)EditorArgsTypes::editor_arg_float])
        GDR_FAILED("cannot convert to float");
      return arg_float;
    }

    template<>
    mth::vec2<float> Get<mth::vec2<float>>(void)
    {
      if (!converted[(int)EditorArgsTypes::editor_arg_float2])
        GDR_FAILED("cannot convert to float2");
      return arg_float2;
    }

    template<>
    mth::vec3<float> Get<mth::vec3<float>>(void)
    {
      if (!converted[(int)EditorArgsTypes::editor_arg_float3])
        GDR_FAILED("cannot convert to float3");
      return arg_float3;
    }

    template<>
    mth::vec4<float> Get<mth::vec4<float>>(void)
    {
      if (!converted[(int)EditorArgsTypes::editor_arg_float4])
        GDR_FAILED("cannot convert to float4");
      return arg_float4;
    }

    template<>
    mth::matr4f Get<mth::matr4f>(void)
    {
      if (!converted[(int)EditorArgsTypes::editor_arg_matr])
        GDR_FAILED("cannot convert to matr");
      return arg_matr;
    }

    template<>
    gdr_index Get<gdr_index>(void)
    {
      if (!converted[(int)EditorArgsTypes::editor_arg_gdr_index])
        GDR_FAILED("cannot convert to matr");
      return arg_gdr_index;
    }

    void Set(std::string t)
    {
      arg_string = t;

      if (strncmp(arg_string.c_str(), "float{", strlen("float{")) == 0)
      {
        std::string inner_part = arg_string.substr(strlen("float{"));
        inner_part.pop_back();

        std::vector<std::string> splitted = split(inner_part, ", ");

        Set((float)std::atof(splitted[0].c_str()));
      }
      else if (strncmp(arg_string.c_str(), "float2{", strlen("float2{")) == 0)
      {
        std::string inner_part = arg_string.substr(strlen("float2{"));
        inner_part.pop_back();

        std::vector<std::string> splitted = split(inner_part, ", ");

        Set(mth::vec2f{ (float)std::atof(splitted[0].c_str()), (float)std::atof(splitted[1].c_str())});
      }
      else if (strncmp(arg_string.c_str(), "float3{", strlen("float3{")) == 0)
      {
        std::string inner_part = arg_string.substr(strlen("float3{"));
        inner_part.pop_back();

        std::vector<std::string> splitted = split(inner_part, ", ");

        Set(mth::vec3f{ (float)std::atof(splitted[0].c_str()), (float)std::atof(splitted[1].c_str()), (float)std::atof(splitted[2].c_str()) });
      }
      else if (strncmp(arg_string.c_str(), "float4{", strlen("float4{")) == 0)
      {
        std::string inner_part = arg_string.substr(strlen("float4{"));
        inner_part.pop_back();

        std::vector<std::string> splitted = split(inner_part, ", ");

        Set(mth::vec4f{ (float)std::atof(splitted[0].c_str()), (float)std::atof(splitted[1].c_str()), (float)std::atof(splitted[2].c_str()), (float)std::atof(splitted[3].c_str()) });
      }
      else if (strncmp(arg_string.c_str(), "matr{", strlen("matr{")) == 0)
      {
        std::string inner_part = arg_string.substr(strlen("matr{"));
        inner_part.pop_back();

        std::vector<std::string> splitted = split(inner_part, ", ");

        Set(mth::matr{ 
          (float)std::atof(splitted[0].c_str()),  (float)std::atof(splitted[1].c_str()),  (float)std::atof(splitted[2].c_str()),  (float)std::atof(splitted[3].c_str()),
          (float)std::atof(splitted[4].c_str()),  (float)std::atof(splitted[5].c_str()),  (float)std::atof(splitted[6].c_str()),  (float)std::atof(splitted[7].c_str()),
          (float)std::atof(splitted[8].c_str()),  (float)std::atof(splitted[9].c_str()),  (float)std::atof(splitted[10].c_str()), (float)std::atof(splitted[11].c_str()),
          (float)std::atof(splitted[12].c_str()), (float)std::atof(splitted[13].c_str()), (float)std::atof(splitted[14].c_str()), (float)std::atof(splitted[15].c_str())});
      }
      else if (strncmp(arg_string.c_str(), "index{", strlen("index{")) == 0)
      {
        std::string inner_part = arg_string.substr(strlen("index{"));
        inner_part.pop_back();

        std::vector<std::string> splitted = split(inner_part, ", ");

        Set((gdr_index)std::atoll(splitted[0].c_str()));
      }
      else
      {
        for (int i = 0; i < (int)EditorArgsTypes::editor_arg_count; i++)
          converted[i] = false;
        converted[(int)EditorArgsTypes::editor_arg_string] = true;
      }
    }

    void Set(float t)
    {
      arg_float = t;
      arg_gdr_index = t;
      arg_string = std::string("float{") + std::to_string(t) + "}";

      for (int i = 0; i < (int)EditorArgsTypes::editor_arg_count; i++)
        converted[i] = false;
      converted[(int)EditorArgsTypes::editor_arg_float] = true;
      converted[(int)EditorArgsTypes::editor_arg_string] = true;
      converted[(int)EditorArgsTypes::editor_arg_gdr_index] = true;
    }

    void Set(mth::vec2<float> t)
    {
      arg_float2 = t;
      arg_string = std::string("float2{") + std::to_string(t[0]) + ", " + std::to_string(t[1]) + "}";

      for (int i = 0; i < (int)EditorArgsTypes::editor_arg_count; i++)
        converted[i] = false;
      converted[(int)EditorArgsTypes::editor_arg_string] = true;
      converted[(int)EditorArgsTypes::editor_arg_float2] = true;
    }

    void Set(mth::vec3<float> t)
    {
      arg_float3 = t;
      arg_string = std::string("float3{") + std::to_string(t[0]) + ", " + std::to_string(t[1]) + ", " + std::to_string(t[2]) + "}";

      for (int i = 0; i < (int)EditorArgsTypes::editor_arg_count; i++)
        converted[i] = false;
      converted[(int)EditorArgsTypes::editor_arg_string] = true;
      converted[(int)EditorArgsTypes::editor_arg_float3] = true;
    }

    void Set(mth::vec4<float> t)
    {
      arg_float4 = t;
      arg_string = std::string("float4{") + std::to_string(t[0]) + ", " + std::to_string(t[1]) + ", " + std::to_string(t[2]) + ", " + std::to_string(t[3]) + "}";

      for (int i = 0; i < (int)EditorArgsTypes::editor_arg_count; i++)
        converted[i] = false;
      converted[(int)EditorArgsTypes::editor_arg_string] = true;
      converted[(int)EditorArgsTypes::editor_arg_float4] = true;
    }

    void Set(mth::matr4<float> t)
    {
      arg_matr = t;
      arg_string = std::string("matr{");
      for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
          arg_string += std::to_string(t[i][j]) + ((i == 3 && j == 3) ? "}" : ", ");


      for (int i = 0; i < (int)EditorArgsTypes::editor_arg_count; i++)
        converted[i] = false;
      converted[(int)EditorArgsTypes::editor_arg_string] = true;
      converted[(int)EditorArgsTypes::editor_arg_matr] = true;
    }

    void Set(gdr_index t)
    {
      arg_gdr_index = t;
      arg_float = t;
      arg_string = std::string("index{") + std::to_string(t) + "}";

      for (int i = 0; i < (int)EditorArgsTypes::editor_arg_count; i++)
        converted[i] = false;
      converted[(int)EditorArgsTypes::editor_arg_string] = true;
      converted[(int)EditorArgsTypes::editor_arg_float] = true;
      converted[(int)EditorArgsTypes::editor_arg_gdr_index] = true;
    }
};

class unit_scripted : public gdr::unit_base
{
private:
  // forward declarations
  struct BlueprintScriptNode;
 
  /***********
   * DATA FROM ENGINE
   **********/

  // Libarary with all blueprint nodes. Gathered from mega-macro
  struct BlueprintLibraryNode
  {
    EditorNodeTypes NodeType;                     // Type of Node
    std::string Filter;                           // Filter
    std::string Name;                             // Name of node
    std::vector<EditorArgsTypes> InputArguments;  // vector with input arguments types 
    std::vector<EditorArgsTypes> OutputArguments; // vector with output arguments types
    std::function<int(unit_scripted::BlueprintScriptNode, std::vector<my_any>)> Function;
  };
  static std::vector<BlueprintLibraryNode> BlueprintLibrary;

  static std::unordered_map<std::string, std::unordered_map<std::string, int>> BlueprintLibraryMapping;

  /***********
   * DATA FROM SCRIPT
   **********/
  struct BlueprintScriptArgument
  {
    EditorArgsTypes ArgumentType; // type of argument
    gdr_index VariableSlot;       // from 0 to NONE_INDEX-1 if we should gather variable from LocalScope. NONE_INDEX if we want to place value directly
    my_any ConstantValue;         // constant variable if needed
  };

  struct BlueprintScriptNode
  {
    gdr_index LibraryNodeIndex;                          // index of node in Library
    std::vector<BlueprintScriptArgument> InputArguments; // Script input arguments
    std::vector<gdr_index> NextNode;                     // Next nodes to run. Array, because we might want to do "if" or "switch" statements
  };

  // Script we loaded
  std::vector<BlueprintScriptNode> LoadedScript;
  // Name of the unit
  std::string UnitName;
  // Local Scope. We put every output argument of function to this LocalScope so we can gather them from everythere.
  std::vector<my_any> LocalScope;
  std::vector<my_any> InitedLocalScope;

  gdr_index FindScriptNode(gdr_index LibraryNodeIndex)
  {
    gdr_index ScriptNodeIndex = NONE_INDEX;
    for (gdr_index i = 0; i < LoadedScript.size() && ScriptNodeIndex == NONE_INDEX; i++)
      if (LoadedScript[i].LibraryNodeIndex == LibraryNodeIndex)
        ScriptNodeIndex = i;
    return ScriptNodeIndex;
  }

  void RunFromScriptNode(gdr_index ScriptNodeIndex)
  {
    while (ScriptNodeIndex != NONE_INDEX)
    {
      // prepare input
      std::vector<my_any> input;
      for (int i = 0; i < LoadedScript[ScriptNodeIndex].InputArguments.size(); i++)
      {
        if (LoadedScript[ScriptNodeIndex].InputArguments[i].VariableSlot == NONE_INDEX)
          input.push_back(LoadedScript[ScriptNodeIndex].InputArguments[i].ConstantValue);
        else
          input.push_back(LocalScope[LoadedScript[ScriptNodeIndex].InputArguments[i].VariableSlot]);
      }

      // call function
      PROFILE_CPU_BEGIN((std::string("Node ") + BlueprintLibrary[LoadedScript[ScriptNodeIndex].LibraryNodeIndex].Filter + ":" +BlueprintLibrary[LoadedScript[ScriptNodeIndex].LibraryNodeIndex].Name).c_str());
      int result = BlueprintLibrary[LoadedScript[ScriptNodeIndex].LibraryNodeIndex].Function(LoadedScript[ScriptNodeIndex], input);
      PROFILE_CPU_END();

      // go to next node
      ScriptNodeIndex = (result >= 0 && result < LoadedScript[ScriptNodeIndex].NextNode.size()) ? LoadedScript[ScriptNodeIndex].NextNode[result] : NONE_INDEX;
    }
  }

  void FillBlueprintFunctions();

public:
  unit_scripted(std::string Name);

  void Initialize(void)
  {
    PROFILE_CPU_BEGIN("Initialize");
    gdr_index InitSciptNode = FindScriptNode(GDR_EDITOR_EVENT_INIT_LIBRARY_INDEX);
    RunFromScriptNode(InitSciptNode);
    PROFILE_CPU_END();
    InitedLocalScope = LocalScope;
  }

  void Response(void)
  {
    LocalScope = InitedLocalScope;
    PROFILE_CPU_BEGIN("Response");
    gdr_index ResponseSciptNode = FindScriptNode(GDR_EDITOR_EVENT_RESPONSE_LIBRARY_INDEX);
    RunFromScriptNode(ResponseSciptNode);
    PROFILE_CPU_END();
  }

  void ResponsePhys(void)
  {
    LocalScope = InitedLocalScope;
    PROFILE_CPU_BEGIN("Response physics");
    gdr_index ResponsePhysSciptNode = FindScriptNode(GDR_EDITOR_EVENT_RESPONSE_PHYS_LIBRARY_INDEX);
    RunFromScriptNode(ResponsePhysSciptNode);
    PROFILE_CPU_END();
  }

  std::string GetName(void)
  {
    return UnitName;
  }

  ~unit_scripted(void)
  {
    PROFILE_CPU_BEGIN("Deinitialisation");
    gdr_index DeinitSciptNode = FindScriptNode(GDR_EDITOR_EVENT_DEINIT_LIBRARY_INDEX);
    RunFromScriptNode(DeinitSciptNode);
    PROFILE_CPU_END();
  }
};
