#pragma once
#include "../unit_base.h"

class unit_choose : public gdr::unit_base
{
private:
  gdr_index ChooseDrawCall;
  gdr_index ChooseMaterial;
  gdr_index ChooseTransform;
  gdr_index ChoosedModel = NONE_INDEX;
public:
    void Initialize(void)
    {
        ChooseDrawCall = Engine->DrawCommandsSystem->Add(NONE_INDEX);

        ChooseMaterial = Engine->MaterialsSystem->Add();
        Engine->MaterialsSystem->GetEditable(ChooseMaterial).ShadeType = MATERIAL_SHADER_COLOR;
        GDRGPUMaterialColorGetColor(Engine->MaterialsSystem->GetEditable(ChooseMaterial)) = { 1, 0, 0 };
        GDRGPUMaterialColorGetColorMapIndex(Engine->MaterialsSystem->GetEditable(ChooseMaterial)) = NONE_INDEX;

        ChooseTransform = Engine->ObjectTransformsSystem->Add();

        Engine->DrawCommandsSystem->GetEditable(ChooseDrawCall).Indices.ObjectMaterialIndex = ChooseMaterial;
        Engine->DrawCommandsSystem->GetEditable(ChooseDrawCall).Indices.ObjectTransformIndex = ChooseTransform;
        Engine->DrawCommandsSystem->GetEditable(ChooseDrawCall).Indices.ObjectParamsMask = OBJECT_PARAMETER_BACK_FACE_CULL;
    }

    void Response(void)
    {
        if (Engine->KeysClick[VK_LBUTTON])
        {
            // get coordinates
            mth::vec3f ScreenDir = { 2.0f * Engine->Mx / Engine->Width - 1, 1.0f - 2.0f * Engine->My / Engine->Height, 1};
            mth::matr4f VP = Engine->PlayerCamera.GetVP();
            mth::matr4f VPInverse = VP.Inversed();
            mth::vec3f WorldOrg = Engine->PlayerCamera.GetPos();
            mth::vec3f WorldDir = VPInverse * ScreenDir - WorldOrg;

            WorldDir.Normalize();

            std::vector<gdr::ray_intersect> Intersects;
            if (Engine->PhysicsManager->Raycast(WorldOrg, WorldDir, 10000, Intersects))
            {
                for (int i = 0; i < Intersects.size(); i++)
                    if (Engine->PhysicsManager->IsExist(Intersects[i].Index))
                    {
                        gdr_index ParentIndex = Engine->PhysicsManager->Get(Intersects[i].Index).GetParent();
                        if (Engine->ModelsManager->IsExist(ParentIndex))
                        {
                            ChoosedModel = ParentIndex;
                            break;
                        }
                    }
            }
        }

        if (Engine->ModelsManager->IsExist(ChoosedModel))
        {
            const gdr::model& Model = Engine->ModelsManager->Get(ChoosedModel);

            if (Model.Render.Meshes.size() != 0)
            {
                const gdr::render_model_node& MeshNode = Model.Render.Hierarchy[Model.Render.Meshes[0]];

                Engine->DrawCommandsSystem->GetEditable(ChooseDrawCall).Indices.BoneMappingIndex = Engine->DrawCommandsSystem->Get(MeshNode.DrawCommand).Indices.BoneMappingIndex;
                Engine->DrawCommandsSystem->GetEditable(ChooseDrawCall).Indices.ObjectIndex = Engine->DrawCommandsSystem->Get(MeshNode.DrawCommand).Indices.ObjectIndex;
                Engine->DrawCommandsSystem->GetEditable(ChooseDrawCall).DrawArguments = Engine->DrawCommandsSystem->Get(MeshNode.DrawCommand).DrawArguments;
                Engine->DrawCommandsSystem->GetEditable(ChooseDrawCall).VertexBuffer = Engine->DrawCommandsSystem->Get(MeshNode.DrawCommand).VertexBuffer;
                Engine->DrawCommandsSystem->GetEditable(ChooseDrawCall).IndexBuffer = Engine->DrawCommandsSystem->Get(MeshNode.DrawCommand).IndexBuffer;
                Engine->DrawCommandsSystem->GetEditable(ChooseDrawCall).IsExist = true;

                gdr_index OriginalMeshTransform = Engine->DrawCommandsSystem->Get(MeshNode.DrawCommand).Indices.ObjectTransformIndex;

                Engine->ObjectTransformsSystem->GetEditable(ChooseTransform).minAABB = Engine->ObjectTransformsSystem->Get(OriginalMeshTransform).minAABB;
                Engine->ObjectTransformsSystem->GetEditable(ChooseTransform).maxAABB = Engine->ObjectTransformsSystem->Get(OriginalMeshTransform).maxAABB;
                Engine->ObjectTransformsSystem->GetEditable(ChooseTransform).Transform = Engine->ObjectTransformsSystem->Get(OriginalMeshTransform).Transform;

                mth::matr4f WVP = Engine->ObjectTransformsSystem->Get(OriginalMeshTransform).Transform * Engine->PlayerCamera.GetVP();
                mth::matr4f VP = Engine->PlayerCamera.GetVP();
                mth::matr4f VPInverse = VP.Inversed();

                mth::vec3f ObjectScreenMax = WVP * Engine->ObjectTransformsSystem->Get(OriginalMeshTransform).maxAABB;
                mth::vec3f ObjectScreenMin = WVP * Engine->ObjectTransformsSystem->Get(OriginalMeshTransform).minAABB;
                mth::vec3f ObjectScreenSize;

                ObjectScreenSize.X = max(ObjectScreenMax.X, ObjectScreenMin.X) - min(ObjectScreenMax.X, ObjectScreenMin.X);
                ObjectScreenSize.Y = max(ObjectScreenMax.Y, ObjectScreenMin.Y) - min(ObjectScreenMax.Y, ObjectScreenMin.Y);
                ObjectScreenSize.Z = max(ObjectScreenMax.Z, ObjectScreenMin.Z) - min(ObjectScreenMax.Z, ObjectScreenMin.Z);

                mth::vec3f ObjectScreenPos = (ObjectScreenMax + ObjectScreenMin) / 2.0;

                // remove node transform

                mth::vec3f Delta = mth::vec3f({ ObjectScreenSize.X , ObjectScreenSize.Y, ObjectScreenSize.Z}) * 0.05;
                Delta.Z += max(ObjectScreenMax.Z, ObjectScreenMin.Z);
                Engine->ObjectTransformsSystem->GetEditable(ChooseTransform).Transform = 
                    Engine->ObjectTransformsSystem->GetEditable(ChooseTransform).Transform * 
                    VP *
                    mth::matr4f::Scale({ 1, 1, 0 }) *
                    mth::matr4f::Translate(Delta) *
                    VPInverse;
            }
            else
            {
                Engine->DrawCommandsSystem->GetEditable(ChooseDrawCall).IsExist = false;
            }
        }
        else
        {
            Engine->DrawCommandsSystem->GetEditable(ChooseDrawCall).IsExist = false;
        }
    }
    
  std::string GetName(void)
  {
    return "unit_choose";
  }

  ~unit_choose(void)
  {
  }
};