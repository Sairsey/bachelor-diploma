#pragma once
#pragma once
#include "../unit_base.h"

class unit_log_time : public gdr::unit_base
{
    FILE* F;
    bool IsLogging;
public:
    void Initialize()
    {
    }

    void Response(void)
    {
        Engine->AddLambdaForIMGUI(
            [&]()
            {
                ImGui::Begin("Time logger", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
                if (!IsLogging)
                {
                    if (ImGui::Button("Start"))
                    {
                        fopen_s(&F, "time_log.txt", "w");
                        IsLogging = true;
                    }
                }
                else
                {
                    if (ImGui::Button("Stop"))
                    {
                        fclose(F);
                        IsLogging = false;
                    }
                }
                ImGui::End();
            }
        );

        if (IsLogging)
            fprintf(F, "%f\n", Engine->DeviceFrameCounter.GetUSec());
    }

    std::string GetName(void)
    {
        return "unit_log_time";
    }

    ~unit_log_time(void)
    {
    }
};