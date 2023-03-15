#pragma once
#include "../unit_base.h"

class unit_funk : public gdr::unit_base
{
private:
    std::vector<gdr_index> SceneModels;
    gdr_index Animation;
    std::vector<gdr_index> Motherbase;
    const int TroopersAmount = 1000;
    const int LineWidth = 10;
    float Offset = 0;

    int FramesToCalc = 60;
    double SumEngTime = 0;
    double SumCPUTime = 0;

    int state = 0;
public:
    void Initialize(void)
    {
        auto stormtrooper = gdr::ImportModelFromAssimp("bin\\models\\stormtrooper\\stormtrooper.glb");
        auto base = gdr::ImportSplittedModelFromAssimp("bin\\models\\motherbase\\motherbase.glb");

        // Load models
        ID3D12GraphicsCommandList* commandList;
        Engine->GetDevice().BeginUploadCommandList(&commandList);
        PROFILE_BEGIN(commandList, "unit_thriller_stage Load stage");
        for (int i = 0; i < TroopersAmount; i++)
            SceneModels.push_back(Engine->ModelsManager->Add(stormtrooper, false));
        for (int i = 0; i < base.size(); i++)
            Motherbase.push_back(Engine->ModelsManager->Add(base[i]));
        PROFILE_END(commandList);
        Engine->GetDevice().CloseUploadCommandList();

        Animation = Engine->AnimationManager->Add(stormtrooper);
        Engine->PlayerCamera.SetPos({ -265.5, 3.766, 838.552 });
        Engine->PlayerCamera.SetDir({ -0.011, -0.603, -0.865 });
    }

    float GetMedian(std::queue<float> q)
    {
        int size = q.size();
        float val = 0;
        int skip = 10;

        for (int i = 0; i < skip; i++)
            q.pop();
        while (!q.empty())
        {
            val += q.front();
            q.pop();
        }

        return val / (size - skip);
    }

    void Response(void)
    {
        static size_t frameCount = 0;
        frameCount++;

        if (state == 0)
        {
            Engine->ToggleFullScreen();
            state++;
        }

        if (state == 2)
        {
            Offset = Engine->GetTime();
            PlaySound(TEXT("bin/sounds/funk.wav"), NULL, SND_FILENAME | SND_ASYNC);
            state++;
        }

        Engine->AddLambdaForIMGUI([&]() {
            static std::queue<float> queue_fps;
            static std::queue<float> queue_cpu;
            static std::queue<float> queue_gpu;

            static float fps = 100;
            static float cpu = 1;
            static float gpu = 1;

            ImGui::GetIO().FontAllowUserScaling = true;
            ImGui::GetIO().FontGlobalScale = 2;

            ImGui::Begin("FUNK EXAMPLE");
            ImGui::Text("FPS: %f", fps);
            ImGui::Text("CPU TIME: %f ms", cpu);
            ImGui::Text("GPU TIME: %f ms", gpu);
            ImGui::Checkbox("Indirect", &Engine->Params.IsIndirect);
            ImGui::Checkbox("Frustum culling", &Engine->Params.IsFrustumCulling);
            
            if (state == 1 && ImGui::Button("Start"))
            {
                state = 2;
            }

            if (frameCount % FramesToCalc == 0)
            {
                // grab median 
                fps = GetMedian(queue_fps);
                cpu = GetMedian(queue_cpu);
                gpu = GetMedian(queue_gpu);
            }
            else
            {
                while (queue_fps.size() >= FramesToCalc)
                    queue_fps.pop();
                while (queue_cpu.size() >= FramesToCalc)
                    queue_cpu.pop();
                while (queue_gpu.size() >= FramesToCalc)
                    queue_gpu.pop();

                queue_fps.push(1000000000.0 / Engine->EngineClock);
                queue_cpu.push(Engine->CPUDrawFrameTime / 1000000.0);
                queue_gpu.push(Engine->DeviceFrameCounter.GetUSec() / 1000.0);
            }

            ImGui::End();
            });

        if (state == 3)
        {
            float alpha = (Engine->GetTime() - Offset) * 1000.0;

            for (int i = 0; i < SceneModels.size(); i++)
            {
                Engine->AnimationManager->SetAnimationTime(SceneModels[i], Animation, alpha);

                float pos = 795.0f + float(i / LineWidth) + (Engine->GetTime() - Offset);

                while (pos > 840)
                {
                    pos -= (TroopersAmount / LineWidth);
                }

                Engine->ObjectTransformsSystem->GetEditable(Engine->ModelsManager->Get(SceneModels[i]).Render.RootTransform).Transform = mth::matr4f::Translate({ -265.0f + float(i % LineWidth - LineWidth / 2.0), 0.1, pos });
            }
        }
    }

    std::string GetName(void)
    {
        return "unit_funk";
    }

    ~unit_funk(void)
    {
        for (int i = 0; i < SceneModels.size(); i++)
            Engine->ModelsManager->Remove(SceneModels[i]);
        for (int i = 0; i < Motherbase.size(); i++)
            Engine->ModelsManager->Remove(Motherbase[i]);
        Engine->AnimationManager->Remove(Animation);
    }
};