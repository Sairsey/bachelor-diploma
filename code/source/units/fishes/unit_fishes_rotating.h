#pragma once
#include "../unit_base.h"

class unit_fishes_rotating : public gdr::unit_base
{
private:
  struct Fish
  {
    int IsClockwise;
    float Duration;
    mth::vec3f Points[4];
    gdr_index Model;
  };

  std::vector<Fish> LeftTank;
  std::vector<gdr::model_import_data> Fishes;

  //fish space
  // from -2.5 to -8.5 by X
  // from -1.8 to 5 by Z
  // from 1 to 2.5

  float MaxDuration = 30;
  float BaseDuration = 30;

  bool SavePerf = false;
  bool IsVisualise = true;

  size_t StartFishesAmount = 10;
  size_t FishesStep = 0;
  int EditableFishesStep = 10;

  size_t MaxFishesAmount = 5000;
  int FramesToCalc = 400;
  double SumEngTime = 0;
  double SumCPUTime = 0;
  std::string Filename = "indirect_perf.txt";

public:
  void Initialize(void)
  {
    auto FishesPack1 = gdr::ImportSplittedModelFromAssimp("bin\\models\\ManyFish\\fish_pack_1\\pack.glb");
    auto FishesPack2 = std::vector<gdr::model_import_data>();// gdr::ImportSplittedModelFromAssimp("bin\\models\\ManyFish\\fish_pack_2\\pack.glb");
    auto FishesPack3 = std::vector<gdr::model_import_data>(); //gdr::ImportSplittedModelFromAssimp("bin\\models\\ManyFish\\fish_pack_3\\pack.glb");

    Fishes.resize(FishesPack1.size() + FishesPack2.size() + FishesPack3.size());
    for (int i = 0; i < Fishes.size(); i++)
      if (i < FishesPack1.size())
        Fishes[i] = FishesPack1[i];
      else if (i < FishesPack1.size() + FishesPack2.size())
        Fishes[i] = FishesPack2[i - FishesPack1.size()];
      else
        Fishes[i] = FishesPack3[i - FishesPack1.size() - FishesPack2.size()];
    AddFishes(StartFishesAmount);

    Engine->EnableFullscreen();
  }

  mth::vec3f getRandomPointInAuarium()
  {
    float min_x = -8.5;
    float max_x = -2.5;

    float min_y = 1;
    float max_y = 2.5;

    float max_z = 1.8;
    float min_z = -5;
  
    float x = min_x + (max_x - min_x) * rand() / RAND_MAX;
    float z = min_z + (max_z - min_z) * rand() / RAND_MAX;
    float y = min_y + (max_y - min_y) * rand() / RAND_MAX;

    return mth::vec3f{x, y, z};
  }

  mth::vec3f CatmulRomSpline(Fish f, float t)
  {
    t -= int(t); // leave only frac part
    int p1 = t * 4;
    int p0 = (p1 - 1 + 4) % 4;
    int p2 = (p1 + 1 + 4) % 4;
    int p3 = (p1 + 2 + 4) % 4;

    float t0 = 1.0 * p1 / 4;
    float dt = 1.0 / 4;

    t = (t - t0) / dt;

    float t_2 = t * t;
    float t_3 = t * t * t;

    mth::vec3f result =
      (f.Points[p1] * (2)) +
      (f.Points[p0] * (-1) + f.Points[p2] * (1)) * t +
      (f.Points[p0] * (2) + f.Points[p1] * (-5) + f.Points[p2] * (4) + f.Points[p3] * (-1)) * t_2 +
      (f.Points[p0] * (-1) + f.Points[p1] * (3) + f.Points[p2] * (-3) + f.Points[p3] * (1)) * t_3;
    result *= 0.5f;
    return result;
  }

  void AddFishes(int count)
  {
      // Load fishes
      ID3D12GraphicsCommandList* commandList;
      Engine->GetDevice().BeginUploadCommandList(&commandList);
      PROFILE_BEGIN(commandList, "unit_fishes_rotating Load");
      for (int i = 0; i < count; i++)
      {
          Fish fish;

          fish.IsClockwise = rand() % 2;
          fish.Duration = 1.0f * rand() / RAND_MAX * (MaxDuration - BaseDuration) + BaseDuration;
          fish.Points[0] = getRandomPointInAuarium();
          fish.Points[1] = getRandomPointInAuarium();
          fish.Points[2] = getRandomPointInAuarium();
          fish.Points[3] = getRandomPointInAuarium();

          fish.Model = Engine->ModelsManager->Add(Fishes[rand() % Fishes.size()]);
          GDRGPUMaterialCookTorranceGetAlbedo(Engine->MaterialsSystem->GetEditable(Engine->ModelsManager->Get(fish.Model).Render.Materials[0])) = { 1.0f * rand() / RAND_MAX, 1.0f * rand() / RAND_MAX, 1.0f * rand() / RAND_MAX };

          LeftTank.push_back(fish);
      }
      PROFILE_END(commandList);
      Engine->GetDevice().CloseUploadCommandList();
  }

  void Response(void)
  {
    static size_t frameCount = 0;
    frameCount++;
    if (SavePerf)
    {
        Engine->PlayerCamera.SetView({ 7, 2, -1.5 }, { 8, 2, -1.5 }, { 0, 1, 0 });
        if (frameCount % FramesToCalc == 0)
        {
            FILE* F;
            fopen_s(&F, Filename.c_str(), "a");
            // fishes, Frame time, CPU Render time, GPU Render time
            fprintf(F, "%d, %f, %f, %f\n", LeftTank.size(), SumEngTime / FramesToCalc * 4.0/3.0, SumCPUTime / FramesToCalc * 4.0 / 3.0, Engine->DeviceFrameCounter.GetUSec());
            fclose(F);
            SumEngTime = 0;
            SumCPUTime = 0;

            if (LeftTank.size() >= MaxFishesAmount)
            {
                exit(0);
            }

            AddFishes(FishesStep);
        }
        else if (frameCount % FramesToCalc >= FramesToCalc / 4)
        {
            SumEngTime += Engine->EngineClock;
            SumCPUTime += Engine->CPUDrawFrameTime;
        }
    }

    if (IsVisualise)
    {
        Engine->AddLambdaForIMGUI([&](){
            static float fps = 100;
            static float cpu = 1;
            static float gpu = 1;

            ImGui::GetIO().FontAllowUserScaling = true;
            ImGui::GetIO().FontGlobalScale = 2;

            ImGui::Begin("FISH EXAMPLE");
            ImGui::Text("FPS: %f", fps);
            ImGui::Text("CPU TIME: %f ms", cpu);
            ImGui::Text("GPU TIME: %f ms", gpu);
            ImGui::Text("FISH AMOUNT: %d", LeftTank.size());
            ImGui::DragInt("DELTA FISHES", &EditableFishesStep);

            if (FishesStep == 0 && ImGui::Button("Start"))
                FishesStep = EditableFishesStep;
            else
            {
                ImGui::Checkbox("Indirect", &Engine->Params.IsIndirect);
                ImGui::Checkbox("Frustum culling", &Engine->Params.IsFrustumCulling);
                if (ImGui::Button("Stop"))
                    FishesStep = 0;
            }

            if (frameCount % 20 == 0)
            {
                fps = 1000000000.0 / Engine->EngineClock;
                cpu = Engine->CPUDrawFrameTime / 1000000.0;
                gpu = Engine->DeviceFrameCounter.GetUSec() / 1000.0;
            }

            ImGui::End();
        });

        if (frameCount % FramesToCalc == 0)
        {
            AddFishes(FishesStep);
        }
    }



    for (int i = 0; i < LeftTank.size(); i++)
    {
      Fish& f = LeftTank[i];
      gdr_index ObjectTransform = Engine->ModelsManager->Get(f.Model).Render.RootTransform;
      mth::vec3f ModelCenter = (Engine->ObjectTransformsSystem->Get(ObjectTransform).minAABB + Engine->ObjectTransformsSystem->Get(ObjectTransform).maxAABB) / 2.0;

      mth::vec3f Position = CatmulRomSpline(f, Engine->GetTime() / f.Duration);
      mth::vec3f NextPosition = CatmulRomSpline(f, Engine->GetTime() / f.Duration + 0.1);
      mth::vec3f Dir = (NextPosition - Position).Normalized();
      mth::vec3f StartDir = {0, 0, 1};
      
      mth::vec3f Axis = StartDir cross Dir;
      float sin = Axis.Lenght();
      Axis /= sin;
      float cos = StartDir dot Dir;

      Engine->ObjectTransformsSystem->GetEditable(ObjectTransform).Transform =
        mth::matr4f::Translate(-ModelCenter) *
        mth::matr4f::Scale(0.03) * mth::matr4f::Rotate(std::atan2(sin, cos) * MTH_R2D, Axis) *
        mth::matr4f::Translate(Position);
    }
  }

  std::string GetName(void)
  {
    return "unit_fishes_rotating";
  }

  ~unit_fishes_rotating(void)
  {
    for (int i = 0; i < LeftTank.size(); i++)
    {
      Engine->ModelsManager->Remove(LeftTank[i].Model);
    }
  }
};