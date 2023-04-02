#include "p_header.h"
#include "unit_scripted.h"
#include <fstream>
#include "json.hpp"

// static field initialisation
std::vector<unit_scripted::BlueprintLibraryNode> unit_scripted::BlueprintLibrary;
std::unordered_map<std::string, std::unordered_map<std::string, int>> unit_scripted::BlueprintLibraryMapping;

// unit constructor with initialisation of everything
unit_scripted::unit_scripted(std::string Name) : UnitName(Name)
{
  // if we have no library - init it and functions
  if (BlueprintLibrary.size() == 0)
  {
    // add default function
    auto DefaultFunction = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any> &output) -> int {
      PROFILE_CPU_BEGIN("DEFAULT FUNCTION");

      // add all output members to local scope so nothing will break.
      for (int i = 0; i < BlueprintLibrary[me.LibraryNodeIndex].OutputArguments.size(); i++)
      {
        output.push_back(my_any());

        switch (BlueprintLibrary[me.LibraryNodeIndex].OutputArguments[i])
        {
        case EditorArgsTypes::editor_arg_string:
          output[output.size() - 1].Set("");
          break;
        case EditorArgsTypes::editor_arg_float:
          output[output.size() - 1].Set(0.0f);
          break;
        case EditorArgsTypes::editor_arg_float2:
          output[output.size() - 1].Set(mth::vec2f());
          break;
        case EditorArgsTypes::editor_arg_float3:
          output[output.size() - 1].Set(mth::vec3f());
          break;
        case EditorArgsTypes::editor_arg_float4:
          output[output.size() - 1].Set(mth::vec4f());
          break;
        case EditorArgsTypes::editor_arg_matr:
          output[output.size() - 1].Set(mth::matr4f::Identity());
          break;
        case EditorArgsTypes::editor_arg_gdr_index:
          output[output.size() - 1].Set(0u);
          break;
        default:
          break;
        }
      }

      PROFILE_CPU_END();
      return 0;
    };

    // init library
    {
        #define GDR_BLUEPRINT_NODE(type, filter, name, number_of_input_args, number_of_output_args, input_args_types, output_args_types, input_args_names, output_args_names) \
        { \
          BlueprintLibraryNode newNode;\
          newNode.NodeType = type;\
          newNode.Filter = filter;\
          newNode.Name = name;\
          EditorArgsTypes input_args[max(number_of_input_args, 1)] = input_args_types;\
          EditorArgsTypes output_args[max(number_of_output_args, 1)] = output_args_types;\
          for (int i = 0; i < number_of_input_args; i++) \
            newNode.InputArguments.push_back(input_args[i]);\
          for (int i = 0; i < number_of_output_args; i++) \
            newNode.OutputArguments.push_back(output_args[i]);\
          newNode.Function = DefaultFunction; \
          BlueprintLibrary.push_back(newNode); \
          BlueprintLibraryMapping[filter][name] = (int)(BlueprintLibrary.size() - 1); \
        };

      GDR_BLUEPRINT_LIST

      #undef GDR_BLUEPRINT_NODE
    }

    // init all implemented functions
    FillBlueprintFunctions();
  }

  // Load JSON
  {
    std::ifstream i(UnitName);
    
    if (i.is_open())
    {
      nlohmann::json data;
      bool result = true;
      try
      {
        i >> data;
      
        // aquire version
        if (MIN_SUPPORTED_VERSION > data["version"] || MAX_SUPPORTED_VERSION < data["version"])
        {
          OutputDebugStringA("Incorrect version!");
          result = false;
        }
 
        // aquire name
        if (result)
          UnitName = data["name"];

        // aquire commands
        for (int i = 0; result && i < data["commands"].size(); i++)
        {
          auto command = data["commands"][i];

          BlueprintScriptNode n;
          n.LibraryNodeIndex = command["library_func_index"];
          
          for (int j = 0; j < command["next_nodes"].size(); j++)
            n.NextNode.push_back(command["next_nodes"][j]);

          
          if (command["input_argument"].size() != BlueprintLibrary[n.LibraryNodeIndex].InputArguments.size())
          {
            OutputDebugStringA("some node has invalid amount of input arguments!\n");
            result = false;
          }

          if (command["output_argument"].size() != BlueprintLibrary[n.LibraryNodeIndex].OutputArguments.size())
          {
            OutputDebugStringA("some node has invalid amount of output arguments!\n");
            result = false;
          }

          for (int j = 0; result && j < command["output_argument"].size(); j++)
          {
            n.OutputArguments.push_back(command["output_argument"][j]);
          }

          for (int j = 0; result && j < command["input_argument"].size(); j++)
          {
            auto argument = command["input_argument"][j];
            BlueprintScriptArgument arg;

            arg.ArgumentType = argument["type"];
            for (int k = 0; k < argument["slots"].size(); k++)
              arg.VariableSlot.push_back(argument["slots"][k]);
            arg.ConstantValue.Set(argument["value"].get<std::string>());
            n.InputArguments.push_back(arg);
          }

          LoadedScript.push_back(n);
        }
      }
      catch (nlohmann::json::parse_error& ex)
      {
        OutputDebugStringA(ex.what());
        OutputDebugStringA("\n");
        result = false;
      }

      if (!result)
      {
        OutputDebugStringA("ERROR: incorrect json\n");
        LoadedScript.clear();
      }
    }
  }
};

// Big function for every node function
void unit_scripted::FillBlueprintFunctions()
{
  /**********
   * EVENT
   ***********/
  // Events are no need to do anything

  /**********
   * TIME
   ***********/
  {
    BlueprintLibrary[BlueprintLibraryMapping["Time"]["Get time"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any> &output) -> int
    {
      my_any Result;
      Result.Set(Engine->GetTime());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Time"]["Get delta time"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(Engine->GetDeltaTime());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Time"]["Set pause"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      Engine->SetPause((BOOL)input[0].Get<float>());
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Time"]["Get pause"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set((float)Engine->GetPause());
      output.push_back(Result);
      return 0;
    };
  };

  /**********
   * MODEL
   ***********/
  {
    BlueprintLibrary[BlueprintLibraryMapping["Model"]["Load"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      gdr::model_import_data importData = gdr::ImportModelFromAssimp(input[0].Get<std::string>());

      ID3D12GraphicsCommandList* commandList;
      Engine->GetDevice().BeginUploadCommandList(&commandList);
      gdr_index model = Engine->ModelsManager->Add(importData);
      Engine->GetDevice().CloseUploadCommandList();

      my_any Result;
      Result.Set(model);
      output.push_back(Result);
      return 0;
    };

    //TODO BlueprintLibrary[BlueprintLibraryMapping["Model"]["Clone"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input) -> int
    
    BlueprintLibrary[BlueprintLibraryMapping["Model"]["Delete"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      Engine->ModelsManager->Remove(input[0].Get<gdr_index>());
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Model"]["Get root transform"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(Engine->ModelsManager->Get(input[0].Get<gdr_index>()).Render.RootTransform);
      output.push_back(Result);
      return 0;
    };
  };

  /**********
   * ANIMATIONS
   ***********/
  {
    BlueprintLibrary[BlueprintLibraryMapping["Animation"]["Load"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      gdr::model_import_data importData = gdr::ImportModelFromAssimp(input[0].Get<std::string>());

      gdr_index anim = Engine->AnimationManager->Add(importData);

      my_any Result;
      Result.Set(anim);
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Animation"]["Delete"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      Engine->AnimationManager->Remove(input[0].Get<gdr_index>());
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Animation"]["Get Duration"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(Engine->AnimationManager->Get(input[0].Get<gdr_index>()).Duration);
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Animation"]["Set animation frame"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      //"Animation index", "Model index", "Time", "Offset", "Duration"
      // gdr_index ModelIndex, gdr_index AnimationIndex, float time, float offset, float duration
      //Engine->AnimationManager->
      Engine->AnimationManager->SetAnimationTime(input[1].Get<gdr_index>(), input[0].Get<gdr_index>(), input[2].Get<float>(), input[3].Get<float>(), input[4].Get<float>());
      return 0;
    };
  }

  /**********
   * TEXTURE
   ***********/
  {
    BlueprintLibrary[BlueprintLibraryMapping["Texture"]["Load"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      ID3D12GraphicsCommandList* commandList;
      Engine->GetDevice().BeginUploadCommandList(&commandList);
      gdr_index texture = Engine->TexturesSystem->Add(input[0].Get<std::string>());
      Engine->GetDevice().CloseUploadCommandList();

      my_any Result;
      Result.Set(texture);
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Texture"]["Delete"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      Engine->TexturesSystem->Remove(input[0].Get<gdr_index>());
      return 0;
    };
  }

  /**********
   * OBJECT TRANSFORM
   ***********/
  {
    BlueprintLibrary[BlueprintLibraryMapping["Object Transform"]["Create"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      gdr_index transform = Engine->ObjectTransformsSystem->Add();
      
      my_any Result;
      Result.Set(transform);
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Object Transform"]["Delete"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      Engine->ObjectTransformsSystem->Remove(input[0].Get<gdr_index>());
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Object Transform"]["Set"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      Engine->ObjectTransformsSystem->GetEditable(input[0].Get<gdr_index>()).Transform = input[0].Get<mth::matr4f>();
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Object Transform"]["Get"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      mth::matr4f transform = Engine->ObjectTransformsSystem->Get(input[0].Get<gdr_index>()).Transform;
      
      my_any Result;
      Result.Set(transform);
      output.push_back(Result);
      return 0;
    };
  }

  /**********
   * MATH
   ***********/
  {
    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Compose float2"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(mth::vec2f{input[0].Get<float>(), input[1].Get<float>() });
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Decompose float2"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      mth::vec2f data = input[0].Get<mth::vec2f>();
      my_any Result;
      Result.Set(data[0]);
      output.push_back(Result);
      Result.Set(data[1]);
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Compose float3"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(mth::vec3f{ input[0].Get<float>(), input[1].Get<float>(), input[2].Get<float>() });
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Decompose float3"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      mth::vec3f data = input[0].Get<mth::vec3f>();
      my_any Result;
      Result.Set(data[0]);
      output.push_back(Result);
      Result.Set(data[1]);
      output.push_back(Result);
      Result.Set(data[2]);
      output.push_back(Result);
      return 0;
    };
    
    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Compose float4"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(mth::vec4f{ input[0].Get<float>(), input[1].Get<float>(), input[2].Get<float>(), input[3].Get<float>() });
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Decompose float4"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      mth::vec4f data = input[0].Get<mth::vec4f>();
      my_any Result;
      Result.Set(data[0]);
      output.push_back(Result);
      Result.Set(data[1]);
      output.push_back(Result);
      Result.Set(data[2]);
      output.push_back(Result);
      Result.Set(data[3]);
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Compose matr"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(mth::matr4f::BuildTransform(input[0].Get<mth::vec3f>(), input[1].Get<mth::vec4f>(), input[2].Get<mth::vec3f>()));
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Decompose matr"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      mth::matr4f data = input[0].Get<mth::matr4f >();
      mth::vec3f position, scale;
      mth::vec4f rot;

      data.Decompose(position, rot, scale);

      my_any Result;
      Result.Set(position);
      output.push_back(Result);
      Result.Set(rot);
      output.push_back(Result);
      Result.Set(scale);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Add float"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<float>() + input[1].Get<float>());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Add float2"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec2f>() + input[1].Get<mth::vec2f>());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Add float3"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec3f>() + input[1].Get<mth::vec3f>());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Add float4"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec4f>() + input[1].Get<mth::vec4f>());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Substract float"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<float>() - input[1].Get<float>());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Substract float2"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec2f>() - input[1].Get<mth::vec2f>());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Substract float3"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec3f>() - input[1].Get<mth::vec3f>());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Substract float4"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec4f>() - input[1].Get<mth::vec4f>());
      output.push_back(Result);
      return 0;
    };
    
    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Multiply float"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<float>() * input[1].Get<float>());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Multiply per component float2"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec2f>() * input[1].Get<mth::vec2f>());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Multiply per component float3"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec3f>() * input[1].Get<mth::vec3f>());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Multiply per component float4"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec4f>() * input[1].Get<mth::vec4f>());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Divide float"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<float>() / input[1].Get<float>());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Multiply float2 per float"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec2f>() * input[1].Get<float>());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Multiply float3 per float"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec3f>() * input[1].Get<float>());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Multiply float4 per float"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec4f>() * input[1].Get<float>());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Multiply matr per matr"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::matr4f>() * input[1].Get<mth::matr4f>());
      output.push_back(Result);
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Dot product float2"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec2f>() dot input[1].Get<mth::vec2f>());
      output.push_back(Result);
      return 0;
    };
    
    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Dot product float3"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec3f>() dot input[1].Get<mth::vec3f>());
      output.push_back(Result);
      return 0;
    };
    
    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Dot product float4"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec4f>() dot input[1].Get<mth::vec4f>());
      output.push_back(Result);
      return 0;
    };
    
    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Cross product float3"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec3f>() cross input[1].Get<mth::vec3f>());
      output.push_back(Result);
      return 0;
    };
    
    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Normalize float2"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec2f>().Normalized());
      output.push_back(Result);
      return 0;
    };
    
    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Normalize float3"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec3f>().Normalized());
      output.push_back(Result);
      return 0;
    };
    
    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Normalize float4"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(input[0].Get<mth::vec4f>().Normalized());
      output.push_back(Result);
      return 0;
    };
    
    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Matr rotate X"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(mth::matr4f::RotateX(input[0].Get<float>()));
      output.push_back(Result);
      return 0;
    };
    
    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Matr rotate Y"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(mth::matr4f::RotateY(input[0].Get<float>()));
      output.push_back(Result);
      return 0;
    };
    
    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Matr rotate Z"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(mth::matr4f::RotateZ(input[0].Get<float>()));
      output.push_back(Result);
      return 0;
    };
    
    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Matr rotate"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(mth::matr4f::Rotate(input[0].Get<float>(), input[1].Get<mth::vec3f>()));
      output.push_back(Result);
      return 0;
    };
    
    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Matr translate"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(mth::matr4f::Translate(input[0].Get<mth::vec3f>()));
      output.push_back(Result);
      return 0;
    };
    
    BlueprintLibrary[BlueprintLibraryMapping["Math"]["Matr scale"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      my_any Result;
      Result.Set(mth::matr4f::Translate(input[0].Get<mth::vec3f>()));
      output.push_back(Result);
      return 0;
    };
  };

  /**********
   * WORKFLOW
   ***********/
  {
    BlueprintLibrary[BlueprintLibraryMapping["Workflow"]["If Key pressed"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      std::string StringKey = input[0].Get<std::string>();
      GDR_ASSERT(StringKey.size() != 0);

      my_any Result;
      Result.Set((float)Engine->Keys[StringKey[0]]);
      output.push_back(Result);

      if (Engine->Keys[StringKey[0]])
        return 0;
      else
        return 1;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Workflow"]["If Key clicked"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      std::string StringKey = input[0].Get<std::string>();
      GDR_ASSERT(StringKey.size() != 0);

      my_any Result;
      Result.Set((float)Engine->KeysClick[StringKey[0]]);
      output.push_back(Result);

      if (Engine->KeysClick[StringKey[0]])
        return 0;
      else
        return 1;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Workflow"]["If float equal"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      float first = input[0].Get<float>();
      float second = input[1].Get<float>();
      bool op = (first == second);

      my_any Result;
      Result.Set((float)op);
      output.push_back(Result);

      if (op)
        return 0;
      else
        return 1;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Workflow"]["If float less"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      float first = input[0].Get<float>();
      float second = input[1].Get<float>();
      bool op = (first < second);

      my_any Result;
      Result.Set((float)op);
      output.push_back(Result);

      if (op)
        return 0;
      else
        return 1;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Workflow"]["If float less equal"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      float first = input[0].Get<float>();
      float second = input[1].Get<float>();
      bool op = (first <= second);

      my_any Result;
      Result.Set((float)op);
      output.push_back(Result);

      if (op)
        return 0;
      else
        return 1;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Workflow"]["If float greater"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      float first = input[0].Get<float>();
      float second = input[1].Get<float>();
      bool op = (first > second);

      my_any Result;
      Result.Set((float)op);
      output.push_back(Result);

      if (op)
        return 0;
      else
        return 1;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Workflow"]["If float greater equal"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      float first = input[0].Get<float>();
      float second = input[1].Get<float>();
      bool op = (first >= second);

      my_any Result;
      Result.Set((float)op);
      output.push_back(Result);

      if (op)
        return 0;
      else
        return 1;
    };
  };

  /**********
   * Vars
   ***********/
  {
    BlueprintLibrary[BlueprintLibraryMapping["Var"]["Set float"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      std::string StringKey = input[0].Get<std::string>();
      float Value = input[1].Get<float>();
      FloatVars[StringKey] = Value;
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Var"]["Get float"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      std::string StringKey = input[0].Get<std::string>();

      my_any Result;
      Result.Set(FloatVars[StringKey]);
      output.push_back(Result);

      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Var"]["Set float2"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      std::string StringKey = input[0].Get<std::string>();
      mth::vec2f Value = input[1].Get<mth::vec2f>();
      Float2Vars[StringKey] = Value;
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Var"]["Get float2"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      std::string StringKey = input[0].Get<std::string>();

      my_any Result;
      Result.Set(Float2Vars[StringKey]);
      output.push_back(Result);

      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Var"]["Set float3"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      std::string StringKey = input[0].Get<std::string>();
      mth::vec3f Value = input[1].Get<mth::vec3f>();
      Float3Vars[StringKey] = Value;
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Var"]["Get float3"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      std::string StringKey = input[0].Get<std::string>();

      my_any Result;
      Result.Set(Float3Vars[StringKey]);
      output.push_back(Result);

      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Var"]["Set float4"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      std::string StringKey = input[0].Get<std::string>();
      mth::vec4f Value = input[1].Get<mth::vec4f>();
      Float4Vars[StringKey] = Value;
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Var"]["Get float4"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      std::string StringKey = input[0].Get<std::string>();

      my_any Result;
      Result.Set(Float4Vars[StringKey]);
      output.push_back(Result);

      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Var"]["Set matr"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      std::string StringKey = input[0].Get<std::string>();
      mth::matr4f Value = input[1].Get<mth::matr4f>();
      MatrVars[StringKey] = Value;
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Var"]["Get matr"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      std::string StringKey = input[0].Get<std::string>();

      my_any Result;
      Result.Set(MatrVars[StringKey]);
      output.push_back(Result);

      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Var"]["Set string"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      std::string StringKey = input[0].Get<std::string>();
      std::string Value = input[1].Get<std::string>();
      StringVars[StringKey] = Value;
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Var"]["Get string"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      std::string StringKey = input[0].Get<std::string>();

      my_any Result;
      Result.Set(StringVars[StringKey]);
      output.push_back(Result);

      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Var"]["Set index"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      std::string StringKey = input[0].Get<std::string>();
      gdr_index Value = input[1].Get<gdr_index>();
      StringVars[StringKey] = Value;
      return 0;
    };

    BlueprintLibrary[BlueprintLibraryMapping["Var"]["Get index"]].Function = [&](BlueprintScriptNode me, std::vector<my_any> input, std::vector<my_any>& output) -> int
    {
      std::string StringKey = input[0].Get<std::string>();

      my_any Result;
      Result.Set(IndicesVars[StringKey]);
      output.push_back(Result);

      return 0;
    };
  }
}
