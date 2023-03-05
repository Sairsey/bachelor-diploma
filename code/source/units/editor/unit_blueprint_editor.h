#pragma once
#include "../unit_base.h"
#include "imgui/imnodes.h"

class unit_blueprint_editor : public gdr::unit_base
{
private:
  struct Event
  {
    std::string name;
  };

  struct Func
  {
    std::string name;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
  };

  struct Edge
  {
    int from;
    int to;
  };
  
  std::vector<Event> events;
  std::vector<Func> functions;
  std::vector<Edge> edges;


public:
  void Initialize(void)
  {
    events.push_back({ "Initialize" });
    events.push_back({ "Deinitialize" });
    events.push_back({ "Response" });
    events.push_back({ "Response Phys" });

    functions.push_back({ "Model Loading", {"Path"}, {"Index"}});
    functions.push_back({ "Model Remove", {"Index"}, {} });
    functions.push_back({ "Model get Root transform", {"Model index"}, {"Object Transform index"} });
    functions.push_back({ "Set object transform", {"Object Transform index", "Matrix"}, {}});

    functions.push_back({ "Get Time", {}, {"Time in seconds"} });
    functions.push_back({ "Translation Matrix", {"delta X", "delta Y", "delta Z"}, {"matrix"}});    
    functions.push_back({ "Rotation around axis Y Matrix", {"angle in degree"}, {"matrix"} });
    functions.push_back({ "Matrix multiplication", {"first", "second"}, {"result"}});

  }

  void Response(void)
  {
    Engine->AddLambdaForIMGUI(
      [&]()
      {
    static bool isFirst = true;
    if (isFirst)
    {
      ImNodes::SetNodeGridSpacePos(1, ImVec2(200.0f, 200.0f));
      isFirst = false;
    }

    ImGui::Begin("simple node editor");

    ImNodes::BeginNodeEditor();

    int attributes = 1;

    for (int i = 0; i < events.size(); i++)
    {
      ImNodes::BeginNode(i + 1);
      ImNodes::BeginNodeTitleBar();
      ImGui::TextUnformatted(events[i].name.c_str());
      ImNodes::EndNodeTitleBar();

      ImNodes::BeginOutputAttribute(attributes++);
      ImGui::Indent(40);
      ImGui::Text("next node");
      ImNodes::EndOutputAttribute();

      ImNodes::EndNode();
    }

    for (int i = 0; i < functions.size(); i++)
    {
      ImNodes::BeginNode(i + 1 + events.size());
      ImNodes::BeginNodeTitleBar();
      ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(11, 109, 191, 255));
      ImGui::TextUnformatted(functions[i].name.c_str());
      ImNodes::EndNodeTitleBar();

      ImNodes::BeginInputAttribute(attributes++);
      ImGui::Text("prev node");
      ImNodes::EndOutputAttribute();

      for (int j = 0; j < functions[i].inputs.size(); j++)
      {
        ImNodes::BeginInputAttribute(attributes++);
        ImGui::Text(functions[i].inputs[j].c_str());
        ImNodes::EndOutputAttribute();
      }

      ImNodes::BeginOutputAttribute(attributes++);
      ImGui::Indent(40);
      ImGui::Text("next node");
      ImNodes::EndOutputAttribute();

      for (int j = 0; j < functions[i].outputs.size(); j++)
      {
        ImNodes::BeginOutputAttribute(attributes++);
        ImGui::Indent(40);
        ImGui::Text(functions[i].outputs[j].c_str());
        ImNodes::EndOutputAttribute();
      }
      
      ImNodes::PopColorStyle();
      ImNodes::EndNode();
    }

    for (auto& edge : edges)
    {
      ImNodes::Link(attributes++, edge.from, edge.to);
    }
    ImNodes::EndNodeEditor();

    int start_attr, end_attr;
    if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
    {
        edges.push_back({start_attr, end_attr});
    }


    ImGui::End();
    });
  }

  std::string GetName(void)
  {
    return "unit_blueprint_editor";
  }

  ~unit_blueprint_editor(void)
  {
  }
};