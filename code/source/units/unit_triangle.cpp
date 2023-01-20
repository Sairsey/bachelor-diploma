#include "p_header.h"

#include "unit_triangle.h"
void unit_triangle::Initialize(void)
{
  static const GDRVertex vertices[3] = {
  {{-0.5f, -0.5f, 0}, {0,0,1}, {0, 0}, {0, 0, 0}, {NONE_INDEX, NONE_INDEX, NONE_INDEX, NONE_INDEX}, {0, 0, 0, 0}},
  {{ 0, 0.5f, 0}, {0,0,1} , {0, 0}, {0, 0, 0}, {NONE_INDEX, NONE_INDEX, NONE_INDEX, NONE_INDEX}, {0, 0, 0, 0}},
  {{ 0.5f, -0.5f, 0}, {0,0,1} , {0, 0}, {0, 0, 0}, {NONE_INDEX, NONE_INDEX, NONE_INDEX, NONE_INDEX}, {0, 0, 0, 0}}
  };
  static const UINT32 indices[3] = { 0, 1, 2 };
  ID3D12GraphicsCommandList* commandList;
  Engine->GetDevice().BeginUploadCommandList(&commandList);
  PROFILE_BEGIN(commandList, "unit_triangle Init");
#if NO_MODEL_SYSTEM
  {
	  TriangleGeometry = Engine->GeometrySystem->CPUData.size();
	  Engine->GeometrySystem->CreateGeometry(vertices, 3, indices, 3);
  }
  {
	  TriangleTransform = Engine->ObjectTransformsSystem->AddElementInPool();
	  Engine->ObjectTransformsSystem->CPUData[TriangleTransform].Transform = mth::matr::Identity();
  }
  {
	  TriangleMaterial = Engine->MaterialsSystem->AddElementInPool();
	  Engine->MaterialsSystem->CPUData[TriangleMaterial].ShadeType = MATERIAL_SHADER_COLOR;
	  GDRGPUMaterialColorGetColor(Engine->MaterialsSystem->CPUData[TriangleMaterial]) = mth::vec3f(0, 1, 0);
  }
  {
	  TriangleDrawCall = Engine->DrawCommandsSystem->AddElementInPool(Engine->GeometrySystem->CPUData[TriangleGeometry]);
	  Engine->DrawCommandsSystem->CPUData[TriangleDrawCall].Indices.ObjectTransformIndex = TriangleTransform;
	  Engine->DrawCommandsSystem->CPUData[TriangleDrawCall].Indices.ObjectMaterialIndex = TriangleMaterial;
  }
#else
  gdr::mesh_import_data tmp_import_data;
  
  GDRGPUMaterial mat;
  mat.ShadeType = MATERIAL_SHADER_COLOR;
  
  gdr::mesh_import_node node;
  node.Name = "mesh";
  node.Type = gdr_hier_node_type::mesh;
  node.ParentIndex = NONE_INDEX;
  node.ChildIndex = NONE_INDEX;
  node.NextIndex = NONE_INDEX;
  node.vertices.push_back(vertices[0]);
  node.vertices.push_back(vertices[1]);
  node.vertices.push_back(vertices[2]);
  node.indices.push_back(indices[0]);
  node.indices.push_back(indices[1]);
  node.indices.push_back(indices[2]);
  node.MaterialIndex = 0;
  node.Params = 0;

  GDRGPUMaterialColorGetColor(mat) = mth::vec3f(0, 1, 0);
  tmp_import_data.FileName = "Triangle";
  tmp_import_data.Materials.push_back(mat);
  tmp_import_data.RootTransform.Transform = mth::matr::Identity();

  tmp_import_data.HierarchyNodes.push_back(node);
  TriangleModel = Engine->AddModel(tmp_import_data);
#endif
  PROFILE_END(commandList);
  Engine->GetDevice().CloseUploadCommandList();
}

void unit_triangle::Response(void)
{
#if NO_MODEL_SYSTEM
  Engine->ObjectTransformsSystem->CPUData[TriangleTransform].Transform = mth::matr::Translate(mth::vec3f(0, 0, sin(Engine->GetTime()) / 2.0f));
  Engine->ObjectTransformsSystem->MarkChunkByTransformIndex(TriangleTransform);
#else
	gdr::model& model = Engine->ModelsPool[TriangleModel];
	Engine->ObjectTransformsSystem->CPUData[model.Rnd.RootTransform].Transform = mth::matr::Translate(mth::vec3f(0, 0, sin(Engine->GetTime()) / 2.0f));
	Engine->ObjectTransformsSystem->MarkChunkByTransformIndex(model.Rnd.RootTransform);
#endif
}