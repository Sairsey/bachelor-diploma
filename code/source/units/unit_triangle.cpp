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
  GDRGPUMaterialColorGetColor(mat) = mth::vec3f(1, 1, 1);
  GDRGPUMaterialColorGetColorMapIndex(mat) = Engine->TexturesSystem->AddElementInPool("bin\\models\\crazy_frog\\Poo_poo_pee_pee.png", true);
  
  gdr::mesh_import_node bone_node;
  bone_node.Name = "bone";
  bone_node.Type = gdr_hier_node_type::node;
  bone_node.ParentIndex = NONE_INDEX;
  bone_node.ChildIndex = 1;
  bone_node.NextIndex = NONE_INDEX;
  bone_node.LocalTransform = mth::matr::Scale({ 1, 1, 0 });
  bone_node.BoneOffset = mth::matr::Identity();

  gdr::mesh_import_node node;
  node.Name = "mesh";
  node.Type = gdr_hier_node_type::mesh;
  node.ParentIndex = 0;
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

  tmp_import_data.FileName = "Triangle";
  tmp_import_data.Materials.push_back(mat);
  tmp_import_data.RootTransform.Transform = mth::matr::Identity();

  tmp_import_data.HierarchyNodes.push_back(bone_node);
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