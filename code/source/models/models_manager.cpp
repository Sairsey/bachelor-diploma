#include "p_header.h"

void gdr::models_manager::CloneModel(gdr_index SrcModel, gdr_index DstModel)
{
	model& OldModel = ModelsPool[SrcModel];
	model& NewModel = ModelsPool[DstModel];

	NewModel.Name == OldModel.Name;

	// Clone Rnd
	{
		// Root Transform
		NewModel.Rnd.RootTransform = Eng->ObjectTransformsSystem->Add();
		Eng->ObjectTransformsSystem->GetEditable(NewModel.Rnd.RootTransform) = Eng->ObjectTransformsSystem->Get(OldModel.Rnd.RootTransform);

		NewModel.Rnd.Hierarchy.resize(OldModel.Rnd.Hierarchy.size());
		// Hierarchy 1-st pass
		for (int i = 0; i < NewModel.Rnd.Hierarchy.size(); i++)
		{
			render_mesh_node& NewNode = NewModel.Rnd.Hierarchy[i];
			render_mesh_node& OldNode = OldModel.Rnd.Hierarchy[i];

			NewNode.Name = OldNode.Name;
			NewNode.Type = OldNode.Type;
			NewNode.ParentIndex = OldNode.ParentIndex;
			NewNode.ChildIndex = OldNode.ChildIndex;
			NewNode.NextIndex = OldNode.NextIndex;
			if (NewModel.Rnd.Hierarchy[i].Type == gdr_hier_node_type::node)
			{
				NewModel.Rnd.Hierarchy[i].NodeTransform = Eng->NodeTransformsSystem->Add();
			}
		}

		// Hierarchy 2-nd pass
		for (int i = 0; i < NewModel.Rnd.Hierarchy.size(); i++)
		{
			render_mesh_node& NewNode = NewModel.Rnd.Hierarchy[i];
			render_mesh_node& OldNode = OldModel.Rnd.Hierarchy[i];
			if (NewModel.Rnd.Hierarchy[i].Type == gdr_hier_node_type::node)
			{
				GDRGPUNodeTransform& OldTransform = Eng->NodeTransformsSystem->GetEditable(OldNode.NodeTransform);
				GDRGPUNodeTransform& NewTransform = Eng->NodeTransformsSystem->GetEditable(NewNode.NodeTransform);
				NewTransform.LocalTransform = OldTransform.LocalTransform;
				NewTransform.BoneOffset = OldTransform.BoneOffset;
				NewTransform.GlobalTransform = OldTransform.GlobalTransform;
				NewTransform.IsNeedRecalc = OldTransform.IsNeedRecalc;

				//find parent child and next indices
				NewTransform.ParentIndex = OldTransform.ParentIndex == NONE_INDEX ? NONE_INDEX : NewModel.Rnd.Hierarchy[NewNode.ParentIndex].NodeTransform;
				NewTransform.ChildIndex = OldTransform.ChildIndex == NONE_INDEX ? NONE_INDEX : NewModel.Rnd.Hierarchy[NewNode.ChildIndex].NodeTransform;
				NewTransform.NextIndex = OldTransform.NextIndex == NONE_INDEX ? NONE_INDEX : NewModel.Rnd.Hierarchy[NewNode.NextIndex].NodeTransform;
			}
			else
			{
				// we need new BoneMapping
				gdr_index BoneMapping = Eng->BoneMappingSystem->Add();

				for (int j = 0; j < MAX_BONE_PER_MODEL; j++)
				{
					// old index 
					UINT OldMap = Eng->BoneMappingSystem->Get(Eng->DrawCommandsSystem->Get(OldNode.DrawCommand).Indices.BoneMappingIndex).BoneMapping[j];
					UINT &NewMap = Eng->BoneMappingSystem->GetEditable(BoneMapping).BoneMapping[j];

					if (OldMap == NONE_INDEX)
						continue;

					// find index
					int index_to_find;
					for (index_to_find = 0; index_to_find < OldModel.Rnd.Hierarchy.size(); index_to_find++)
						if (OldModel.Rnd.Hierarchy[index_to_find].Type == gdr_hier_node_type::node &&
							OldModel.Rnd.Hierarchy[index_to_find].NodeTransform == OldMap)
							break;

					NewMap = NewModel.Rnd.Hierarchy[index_to_find].NodeTransform;
				}

				NewNode.DrawCommand = Eng->DrawCommandsSystem->Add(
					Eng->DrawCommandsSystem->Get(OldNode.DrawCommand).Indices.ObjectIndex,
					NewModel.Rnd.RootTransform,
					Eng->DrawCommandsSystem->Get(OldNode.DrawCommand).Indices.ObjectMaterialIndex,
					BoneMapping);
				Eng->DrawCommandsSystem->GetEditable(NewNode.DrawCommand).Indices.ObjectParamsMask =
					Eng->DrawCommandsSystem->GetEditable(OldNode.DrawCommand).Indices.ObjectParamsMask;
			}
		}
	}
}

gdr_index gdr::models_manager::AddModel(mesh_import_data ImportData)
{
	ModelsPool.emplace_back();
	model& NewModel = ModelsPool[ModelsPool.size() - 1];

	for (int i = 0; i < ModelsPool.size() - 1; i++)
	{
		if (ModelsPool[i].Name == ImportData.FileName)
		{
			CloneModel(i, ModelsPool.size() - 1);
			return (gdr_index)(ModelsPool.size() - 1);
		}
	}

	NewModel.Name = ImportData.FileName;

	// Render Model creation
	{
		// At first -> create Root transform for our model
		{
			NewModel.Rnd.RootTransform = Eng->ObjectTransformsSystem->Add();
			Eng->ObjectTransformsSystem->GetEditable(NewModel.Rnd.RootTransform) = ImportData.RootTransform;
		}

		// Second -> Copy all materials
		std::vector<gdr_index> MaterialsIndices;
		{
			for (int i = 0; i < ImportData.Materials.size(); i++)
			{
				MaterialsIndices.push_back(Eng->MaterialsSystem->Add());
				Eng->MaterialsSystem->GetEditable(MaterialsIndices[i]) = ImportData.Materials[i];
				//check for textures
				switch (ImportData.Materials[i].ShadeType)
				{
					case MATERIAL_SHADER_COLOR:
						if (GDRGPUMaterialColorGetColorMapIndex(ImportData.Materials[i]) != NONE_INDEX)
						{
							std::string &textureName = ImportData.TexturesPaths[GDRGPUMaterialColorGetColorMapIndex(ImportData.Materials[i])];
							GDRGPUMaterialColorGetColorMapIndex(Eng->MaterialsSystem->GetEditable(MaterialsIndices[i])) = Eng->TexturesSystem->Add(textureName, true);
						}
						break;
					case MATERIAL_SHADER_PHONG:
						if (GDRGPUMaterialPhongGetAmbientMapIndex(ImportData.Materials[i]) != NONE_INDEX)
						{
							std::string& textureName = ImportData.TexturesPaths[GDRGPUMaterialPhongGetAmbientMapIndex(ImportData.Materials[i])];
							GDRGPUMaterialPhongGetAmbientMapIndex(Eng->MaterialsSystem->GetEditable(MaterialsIndices[i])) = Eng->TexturesSystem->Add(textureName, false);
						}
						if (GDRGPUMaterialPhongGetDiffuseMapIndex(ImportData.Materials[i]) != NONE_INDEX)
						{
							std::string& textureName = ImportData.TexturesPaths[GDRGPUMaterialPhongGetDiffuseMapIndex(ImportData.Materials[i])];
							GDRGPUMaterialPhongGetDiffuseMapIndex(Eng->MaterialsSystem->GetEditable(MaterialsIndices[i])) = Eng->TexturesSystem->Add(textureName, true);
						}
						if (GDRGPUMaterialPhongGetSpecularMapIndex(ImportData.Materials[i]) != NONE_INDEX)
						{
							std::string& textureName = ImportData.TexturesPaths[GDRGPUMaterialPhongGetSpecularMapIndex(ImportData.Materials[i])];
							GDRGPUMaterialPhongGetSpecularMapIndex(Eng->MaterialsSystem->GetEditable(MaterialsIndices[i])) = Eng->TexturesSystem->Add(textureName, false);
						}
						if (GDRGPUMaterialPhongGetNormalMapIndex(ImportData.Materials[i]) != NONE_INDEX)
						{
							std::string& textureName = ImportData.TexturesPaths[GDRGPUMaterialPhongGetNormalMapIndex(ImportData.Materials[i])];
							GDRGPUMaterialPhongGetNormalMapIndex(Eng->MaterialsSystem->GetEditable(MaterialsIndices[i])) = Eng->TexturesSystem->Add(textureName, false);
						}
						break;
					case MATERIAL_SHADER_COOKTORRANCE_METALNESS:
						if (GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(ImportData.Materials[i]) != NONE_INDEX)
						{
							std::string& textureName = ImportData.TexturesPaths[GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(ImportData.Materials[i])];
							GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(Eng->MaterialsSystem->GetEditable(MaterialsIndices[i])) = Eng->TexturesSystem->Add(textureName, false);
						}
						if (GDRGPUMaterialCookTorranceGetAlbedoMapIndex(ImportData.Materials[i]) != NONE_INDEX)
						{
							std::string& textureName = ImportData.TexturesPaths[GDRGPUMaterialCookTorranceGetAlbedoMapIndex(ImportData.Materials[i])];
							GDRGPUMaterialCookTorranceGetAlbedoMapIndex(Eng->MaterialsSystem->GetEditable(MaterialsIndices[i])) = Eng->TexturesSystem->Add(textureName, true);
						}
						if (GDRGPUMaterialCookTorranceGetNormalMapIndex(ImportData.Materials[i]) != NONE_INDEX)
						{
							std::string& textureName = ImportData.TexturesPaths[GDRGPUMaterialCookTorranceGetNormalMapIndex(ImportData.Materials[i])];
							GDRGPUMaterialCookTorranceGetNormalMapIndex(Eng->MaterialsSystem->GetEditable(MaterialsIndices[i])) = Eng->TexturesSystem->Add(textureName, false);
						}
						if (GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(ImportData.Materials[i]) != NONE_INDEX)
						{
							std::string& textureName = ImportData.TexturesPaths[GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(ImportData.Materials[i])];
							GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(Eng->MaterialsSystem->GetEditable(MaterialsIndices[i])) = Eng->TexturesSystem->Add(textureName, false);
						}
						break;
					case MATERIAL_SHADER_COOKTORRANCE_SPECULAR:
						if (GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(ImportData.Materials[i]) != NONE_INDEX)
						{
							std::string& textureName = ImportData.TexturesPaths[GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(ImportData.Materials[i])];
							GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(Eng->MaterialsSystem->GetEditable(MaterialsIndices[i])) = Eng->TexturesSystem->Add(textureName, false);
						}
						if (GDRGPUMaterialCookTorranceGetAlbedoMapIndex(ImportData.Materials[i]) != NONE_INDEX)
						{
							std::string& textureName = ImportData.TexturesPaths[GDRGPUMaterialCookTorranceGetAlbedoMapIndex(ImportData.Materials[i])];
							GDRGPUMaterialCookTorranceGetAlbedoMapIndex(Eng->MaterialsSystem->GetEditable(MaterialsIndices[i])) = Eng->TexturesSystem->Add(textureName, true);
						}
						if (GDRGPUMaterialCookTorranceGetNormalMapIndex(ImportData.Materials[i]) != NONE_INDEX)
						{
							std::string& textureName = ImportData.TexturesPaths[GDRGPUMaterialCookTorranceGetNormalMapIndex(ImportData.Materials[i])];
							GDRGPUMaterialCookTorranceGetNormalMapIndex(Eng->MaterialsSystem->GetEditable(MaterialsIndices[i])) = Eng->TexturesSystem->Add(textureName, false);
						}
						if (GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(ImportData.Materials[i]) != NONE_INDEX)
						{
							std::string& textureName = ImportData.TexturesPaths[GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(ImportData.Materials[i])];
							GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(Eng->MaterialsSystem->GetEditable(MaterialsIndices[i])) = Eng->TexturesSystem->Add(textureName, true);
						}
						break;
					default:
						GDR_FAILED("Unsupported shader type.");
						break;
				}
			}
		}

		// Third -> Copy hierarchy in 2 phases
		{
			//First pass 
			// -> For all nodes init just gdr_indices of transforms
			for (int i = 0; i < ImportData.HierarchyNodes.size(); i++)
			{
				NewModel.Rnd.Hierarchy.emplace_back();
				NewModel.Rnd.Hierarchy[i].Name = ImportData.HierarchyNodes[i].Name;
				NewModel.Rnd.Hierarchy[i].Type = ImportData.HierarchyNodes[i].Type;
				NewModel.Rnd.Hierarchy[i].ParentIndex = ImportData.HierarchyNodes[i].ParentIndex;
				NewModel.Rnd.Hierarchy[i].ChildIndex = ImportData.HierarchyNodes[i].ChildIndex;
				NewModel.Rnd.Hierarchy[i].NextIndex = ImportData.HierarchyNodes[i].NextIndex;
				if (NewModel.Rnd.Hierarchy[i].Type == gdr_hier_node_type::node)
				{
					NewModel.Rnd.Hierarchy[i].NodeTransform = Eng->NodeTransformsSystem->Add();
				}
			}
			//Second pass 
			// -> Fix hierarchy for nodes
			// -> For all meshes init everything
			for (int i = 0; i < ImportData.HierarchyNodes.size(); i++)
			{
				if (NewModel.Rnd.Hierarchy[i].Type == gdr_hier_node_type::node)
				{
					Eng->NodeTransformsSystem->GetEditable(NewModel.Rnd.Hierarchy[i].NodeTransform).LocalTransform = ImportData.HierarchyNodes[i].LocalTransform;
					Eng->NodeTransformsSystem->GetEditable(NewModel.Rnd.Hierarchy[i].NodeTransform).BoneOffset = ImportData.HierarchyNodes[i].BoneOffset;
					if (NewModel.Rnd.Hierarchy[i].ParentIndex != NONE_INDEX)
						Eng->NodeTransformsSystem->GetEditable(NewModel.Rnd.Hierarchy[i].NodeTransform).ParentIndex = NewModel.Rnd.Hierarchy[NewModel.Rnd.Hierarchy[i].ParentIndex].NodeTransform;
					else
						Eng->NodeTransformsSystem->GetEditable(NewModel.Rnd.Hierarchy[i].NodeTransform).ParentIndex = NONE_INDEX;

					if (NewModel.Rnd.Hierarchy[i].NextIndex != NONE_INDEX && NewModel.Rnd.Hierarchy[NewModel.Rnd.Hierarchy[i].NextIndex].Type == gdr_hier_node_type::node)
						Eng->NodeTransformsSystem->GetEditable(NewModel.Rnd.Hierarchy[i].NodeTransform).NextIndex = NewModel.Rnd.Hierarchy[NewModel.Rnd.Hierarchy[i].NextIndex].NodeTransform;
					else
						Eng->NodeTransformsSystem->GetEditable(NewModel.Rnd.Hierarchy[i].NodeTransform).NextIndex = NONE_INDEX;

					if (NewModel.Rnd.Hierarchy[i].ChildIndex != NONE_INDEX && NewModel.Rnd.Hierarchy[NewModel.Rnd.Hierarchy[i].ChildIndex].Type == gdr_hier_node_type::node)
						Eng->NodeTransformsSystem->GetEditable(NewModel.Rnd.Hierarchy[i].NodeTransform).ChildIndex = NewModel.Rnd.Hierarchy[NewModel.Rnd.Hierarchy[i].ChildIndex].NodeTransform;
					else
						Eng->NodeTransformsSystem->GetEditable(NewModel.Rnd.Hierarchy[i].NodeTransform).ChildIndex = NONE_INDEX;
				}
				else if (NewModel.Rnd.Hierarchy[i].Type == gdr_hier_node_type::mesh)
				{
					std::vector<GDRVertex> vertices = ImportData.HierarchyNodes[i].vertices;
					std::vector<UINT> indices = ImportData.HierarchyNodes[i].indices;

					// create BoneMapping!!!
					gdr_index BoneMapping = Eng->BoneMappingSystem->Add();

					// fix bone indices for skinning
					for (int vertex_index = 0; vertex_index < vertices.size(); vertex_index++)
					{
						for (int vertex_bone_index = 0; vertex_bone_index < 4; vertex_bone_index++)
						{
							UINT in_model_bone_index = vertices[vertex_index].BonesIndices[vertex_bone_index];
							if (in_model_bone_index == NONE_INDEX)
								continue;
							gdr_index in_engine_node_transform = NewModel.Rnd.Hierarchy[in_model_bone_index].NodeTransform;
							UINT in_bone_mapping_index = MAX_BONE_PER_MODEL;
							
							for (int i = 0; i < MAX_BONE_PER_MODEL && in_bone_mapping_index == MAX_BONE_PER_MODEL; i++)
								// if we found same or place for new
								if (Eng->BoneMappingSystem->Get(BoneMapping).BoneMapping[i] == in_engine_node_transform || 
									Eng->BoneMappingSystem->Get(BoneMapping).BoneMapping[i] == NONE_INDEX)
									in_bone_mapping_index = i;
							
							GDR_ASSERT(in_bone_mapping_index < MAX_BONE_PER_MODEL);
							vertices[vertex_index].BonesIndices[vertex_bone_index] = in_bone_mapping_index;
							if (Eng->BoneMappingSystem->Get(BoneMapping).BoneMapping[in_bone_mapping_index] == NONE_INDEX)
								Eng->BoneMappingSystem->GetEditable(BoneMapping).BoneMapping[in_bone_mapping_index] = in_engine_node_transform;
						}
					}

					gdr_index geometry = Eng->GeometrySystem->Add(vertices.data(), vertices.size(), indices.data(), indices.size());
					NewModel.Rnd.Hierarchy[i].DrawCommand = Eng->DrawCommandsSystem->Add(
					geometry,
					NewModel.Rnd.RootTransform,
					MaterialsIndices[ImportData.HierarchyNodes[i].MaterialIndex],
					BoneMapping);
					Eng->DrawCommandsSystem->GetEditable(NewModel.Rnd.Hierarchy[i].DrawCommand).Indices.ObjectParamsMask = ImportData.HierarchyNodes[i].Params;
				}
			}
		}
	}
	return (gdr_index)(ModelsPool.size() - 1);
}


void gdr::models_manager::DeleteModel(gdr_index index)
{
	model& ModelToDelete = ModelsPool[index];
	ModelToDelete.Name = "GDR_MODEL_DELETED";

	for (int i = 0; i < ModelToDelete.Rnd.Hierarchy.size(); i++)
	{
		gdr::render_mesh_node &node = ModelToDelete.Rnd.Hierarchy[i];
		if (node.Type == gdr_hier_node_type::mesh)
		{
			Eng->DrawCommandsSystem->Remove(node.DrawCommand);
		}
		if (node.Type == gdr_hier_node_type::node)
		{
			Eng->NodeTransformsSystem->Remove(node.NodeTransform);
		}
	}

	// Little memory defrag
	while (ModelsPool[ModelsPool.size() - 1].Name == "GDR_MODEL_DELETED")
		ModelsPool.pop_back();
}
