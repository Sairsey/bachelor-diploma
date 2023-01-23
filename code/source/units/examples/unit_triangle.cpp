#include "p_header.h"

#include "unit_triangle.h"

void unit_triangle::Initialize(void)
{
  static const GDRVertex vertices[3] = {
  {{-0.5f, -0.5f, 0}, {0,0,1}, {0, 0}, {0, 0, 0}, {0, NONE_INDEX, NONE_INDEX, NONE_INDEX}, {1, 0, 0, 0}},
  {{ 0, 0.5f, 0}, {0,0,1} , {0.5, 1}, {0, 0, 0}, {0, NONE_INDEX, NONE_INDEX, NONE_INDEX}, {1, 0, 0, 0}},
  {{ 0.5f, -0.5f, 0}, {0,0,1} , {1, 0}, {0, 0, 0}, {0, NONE_INDEX, NONE_INDEX, NONE_INDEX}, {1, 0, 0, 0}}
  };
  static const UINT32 indices[3] = { 0, 1, 2 };
  ID3D12GraphicsCommandList* commandList;
  Engine->GetDevice().BeginUploadCommandList(&commandList);
  PROFILE_BEGIN(commandList, "unit_triangle Init");
  {
	  TriangleGeometry = Engine->GeometrySystem->Add(vertices, 3, indices, 3);
  }
  {
	  TriangleTransform = Engine->ObjectTransformsSystem->Add();
    Engine->ObjectTransformsSystem->GetEditable(TriangleTransform).Transform = mth::matr::Identity();
    Engine->ObjectTransformsSystem->GetEditable(TriangleTransform).minAABB = {-0.5, -0.5, 0};
    Engine->ObjectTransformsSystem->GetEditable(TriangleTransform).maxAABB = { 0.5, 0.5, 0 };
  }
  {
	  TriangleMaterial = Engine->MaterialsSystem->Add();
	  Engine->MaterialsSystem->GetEditable(TriangleMaterial).ShadeType = MATERIAL_SHADER_COLOR;
	  GDRGPUMaterialColorGetColor(Engine->MaterialsSystem->GetEditable(TriangleMaterial)) = mth::vec3f(1, 0, 0);
  }
  {
	  TriangleDrawCall = Engine->DrawCommandsSystem->Add(TriangleGeometry);
	  Engine->DrawCommandsSystem->GetEditable(TriangleDrawCall).Indices.ObjectTransformIndex = TriangleTransform;
	  Engine->DrawCommandsSystem->GetEditable(TriangleDrawCall).Indices.ObjectMaterialIndex = TriangleMaterial;
  }
  PROFILE_END(commandList);
  Engine->GetDevice().CloseUploadCommandList();
}

void unit_triangle::Response(void)
{
  Engine->ObjectTransformsSystem->GetEditable(TriangleTransform).Transform = mth::matr::Translate(mth::vec3f(0, 0, sin(Engine->GetTime()) / 2.0f));
}