#pragma once
#pragma once
#include "../unit_base.h"

class unit_frame_times : public gdr::unit_base
{
public:
    void Initialize()
    {
    }

    void Response(void)
    {
        Engine->AddLambdaForIMGUI(
            [&]()
            {
                ImGui::Begin("Frame Times", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if (ImGui::TreeNode("Root", "Device Frame Time %f ms(%f FPS)", Engine->DeviceFrameCounter.GetMSec(), 1000 / Engine->DeviceFrameCounter.GetMSec()))
                {
                    for (int i = 0; i < Engine->Passes.size(); i++)
                    {
                        ImGui::SetNextItemOpen(true, ImGuiCond_Once);

                        std::string label = Engine->Passes[i]->GetName() + " " + std::to_string(Engine->Passes[i]->DeviceTimeCounter.GetMSec()) + " ms";
                        if (ImGui::TreeNode((void*)(intptr_t)i, label.c_str(), i))
                        {
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                ImGui::End();
            }
        );
    }

    std::string GetName(void)
    {
        return "unit_frame_times";
    }

    ~unit_frame_times(void)
    {
    }
};