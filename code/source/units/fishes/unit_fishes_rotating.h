#pragma once
#include "../unit_base.h"

class unit_fishes_rotating : public gdr::unit_base
{
private:
  struct Fish
  {
    int IsClockwise;
    float Radius;
    float Height;
    float Speed;
    float Angle;
    gdr_index Model;
  };

  std::vector<Fish> LeftTank;
  std::vector<Fish> RightTank;
  std::vector<gdr::model_import_data> Fishes;

  mth::vec3f LeftTankBottom = { -5.5f, 1.f, -1.5f };
  mth::vec3f RightTankBottom = { 10.3f, 1.f, -1.5f };
  float MaxHeight = 1.5;
  float MaxRadius = 0.5;
  float MaxSpeed = 1;

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

    /*
    Engine->Params.IsOccusionCulling = false;
    Engine->Params.IsFrustumCulling = false;
    Engine->Params.IsIndirect = true;
    Engine->Params.IsIBL = false;
    Engine->Params.IsUploadEveryFrame = false;
    */
    Engine->EnableFullscreen();
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

          int Tank = 1;// rand() % 2;
          fish.IsClockwise = rand() % 2;
          fish.Height = 1.0f * rand() / RAND_MAX * MaxHeight;
          fish.Radius = 1.0f * rand() / RAND_MAX * MaxRadius;
          fish.Speed = 1.0f * rand() / RAND_MAX * MaxSpeed;
          fish.Angle = 1.0f * rand() / RAND_MAX * 360.0;

          fish.Model = Engine->ModelsManager->Add(Fishes[rand() % Fishes.size()]);
          GDRGPUMaterialCookTorranceGetAlbedo(Engine->MaterialsSystem->GetEditable(Engine->ModelsManager->Get(fish.Model).Render.Materials[0])) = { 1.0f * rand() / RAND_MAX, 1.0f * rand() / RAND_MAX, 1.0f * rand() / RAND_MAX };

          if (Tank == 0)
          {
              LeftTank.push_back(fish);
          }
          else
          {
              RightTank.push_back(fish);
          }
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
            fprintf(F, "%d, %f, %f, %f\n", RightTank.size(), SumEngTime / FramesToCalc * 4.0/3.0, SumCPUTime / FramesToCalc * 4.0 / 3.0, Engine->DeviceFrameCounter.GetUSec());
            fclose(F);
            SumEngTime = 0;
            SumCPUTime = 0;

            if (RightTank.size() >= MaxFishesAmount)
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

            ImGui::GetIO().FontGlobalScale = 2;

            ImGui::Begin("FISH EXAMPLE");
            ImGui::Text("FPS: %f", fps);
            ImGui::Text("CPU TIME: %f ms", cpu);
            ImGui::Text("GPU TIME: %f ms", gpu);
            ImGui::Text("FISH AMOUNT: %d", RightTank.size());
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

            ImGui::GetIO().FontGlobalScale = 1;
        });

        if (frameCount % FramesToCalc == 0)
        {
            AddFishes(FishesStep);
        }
    }



    for (int i = 0; i < LeftTank.size(); i++)
    {
      Fish& f = LeftTank[i];
      f.Angle += f.Speed * MTH_R2D * Engine->GetDeltaTime();
      gdr_index ObjectTransform = Engine->ModelsManager->Get(f.Model).Render.RootTransform;
      mth::vec3f ModelCenter = (Engine->ObjectTransformsSystem->Get(ObjectTransform).minAABB + Engine->ObjectTransformsSystem->Get(ObjectTransform).maxAABB) / 2.0;
      Engine->ObjectTransformsSystem->GetEditable(ObjectTransform).Transform =
        mth::matr4f::Translate(-ModelCenter) *
        mth::matr4f::Scale(0.03) *
        mth::matr4f::Translate({f.Radius * (f.IsClockwise ? -1 : 1), 0, 0}) *
        mth::matr4f::RotateY(f.Angle * (f.IsClockwise ? -1 : 1)) *
        mth::matr4f::Translate(LeftTankBottom + mth::vec3f{0, f.Height, 0});
    }
    for (int i = 0; i < RightTank.size(); i++)
    {
      Fish& f = RightTank[i];
      f.Angle += f.Speed * MTH_R2D * Engine->GetDeltaTime();
      gdr_index ObjectTransform = Engine->ModelsManager->Get(f.Model).Render.RootTransform;
      mth::vec3f ModelCenter = (Engine->ObjectTransformsSystem->Get(ObjectTransform).minAABB + Engine->ObjectTransformsSystem->Get(ObjectTransform).maxAABB) / 2.0;
      Engine->ObjectTransformsSystem->GetEditable(ObjectTransform).Transform =
        mth::matr4f::Translate(-ModelCenter) *
        mth::matr4f::Scale(0.03) *
        mth::matr4f::Translate({f.Radius * (f.IsClockwise ? -1 : 1), 0, 0}) *
        mth::matr4f::RotateY(f.Angle * (f.IsClockwise ? -1 : 1)) *
        mth::matr4f::Translate(RightTankBottom + mth::vec3f{ 0, f.Height, 0 });
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
    for (int i = 0; i < RightTank.size(); i++)
    {
      Engine->ModelsManager->Remove(RightTank[i].Model);
    }
  }
};