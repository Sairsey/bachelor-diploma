#include "p_header.h"

gdr::models_manager::models_manager(engine* Eng) : resource_pool_subsystem(Eng), Engine(Eng)
{
}

void gdr::models_manager::BeforeRemoveJob(gdr_index index)
{
	if (IsExist(index))
	{
		model& ModelToDelete = GetEditable(index);
		ModelToDelete.Name = "GDR_MODEL_DELETED";

		for (int i = 0; i < ModelToDelete.Render.Hierarchy.size(); i++)
		{
			render_model_node& node = ModelToDelete.Render.Hierarchy[i];
			if (node.Type == gdr_hier_node_type::mesh)
				Engine->DrawCommandsSystem->Remove(node.DrawCommand);
			if (node.Type == gdr_hier_node_type::node)
				Engine->NodeTransformsSystem->Remove(node.NodeTransform);
		}
	}
}

gdr_index gdr::models_manager::Add(const model_import_data& ImportData)
{
	gdr_index NewModelIndex = NONE_INDEX;
	NewModelIndex = resource_pool_subsystem::Add();
	for (gdr_index i = 0; i < AllocatedSize(); i++)
		if (IsExist(i) && Get(i).Name == ImportData.FileName)
		{
			Clone(i, NewModelIndex);
			return NewModelIndex;
		}

	model &NewModel = GetEditable(NewModelIndex);

	NewModel.Name = ImportData.FileName;

	// Render model import
	{
		render_model &NewModelRender = NewModel.Render;
		
		// Root Transform
		NewModelRender.RootTransform = Engine->ObjectTransformsSystem->Add();
		Engine->ObjectTransformsSystem->GetEditable(NewModelRender.RootTransform) = ImportData.RootTransform;

		// Materials
		for (int i = 0; i < ImportData.Materials.size(); i++)
		{
			NewModelRender.Materials.push_back(Engine->MaterialsSystem->Add());
			GDRGPUMaterial &NewMaterial = Engine->MaterialsSystem->GetEditable(NewModelRender.Materials[i]);

			NewMaterial = ImportData.Materials[i];
			//check for textures
			switch (ImportData.Materials[i].ShadeType)
			{
			case MATERIAL_SHADER_COLOR:
				if (GDRGPUMaterialColorGetColorMapIndex(ImportData.Materials[i]) != NONE_INDEX)
				{
					std::string textureName = ImportData.TexturesPaths[GDRGPUMaterialColorGetColorMapIndex(ImportData.Materials[i])];
					GDRGPUMaterialColorGetColorMapIndex(NewMaterial) = Engine->TexturesSystem->Add(textureName, true);
				}
				break;
			case MATERIAL_SHADER_PHONG:
				if (GDRGPUMaterialPhongGetAmbientMapIndex(ImportData.Materials[i]) != NONE_INDEX)
				{
					std::string textureName = ImportData.TexturesPaths[GDRGPUMaterialPhongGetAmbientMapIndex(ImportData.Materials[i])];
					GDRGPUMaterialPhongGetAmbientMapIndex(NewMaterial) = Engine->TexturesSystem->Add(textureName, false);
				}
				if (GDRGPUMaterialPhongGetDiffuseMapIndex(ImportData.Materials[i]) != NONE_INDEX)
				{
					std::string textureName = ImportData.TexturesPaths[GDRGPUMaterialPhongGetDiffuseMapIndex(ImportData.Materials[i])];
					GDRGPUMaterialPhongGetDiffuseMapIndex(NewMaterial) = Engine->TexturesSystem->Add(textureName, true);
				}
				if (GDRGPUMaterialPhongGetSpecularMapIndex(ImportData.Materials[i]) != NONE_INDEX)
				{
					std::string textureName = ImportData.TexturesPaths[GDRGPUMaterialPhongGetSpecularMapIndex(ImportData.Materials[i])];
					GDRGPUMaterialPhongGetSpecularMapIndex(NewMaterial) = Engine->TexturesSystem->Add(textureName, false);
				}
				if (GDRGPUMaterialPhongGetNormalMapIndex(ImportData.Materials[i]) != NONE_INDEX)
				{
					std::string textureName = ImportData.TexturesPaths[GDRGPUMaterialPhongGetNormalMapIndex(ImportData.Materials[i])];
					GDRGPUMaterialPhongGetNormalMapIndex(NewMaterial) = Engine->TexturesSystem->Add(textureName, false);
				}
				break;
			case MATERIAL_SHADER_COOKTORRANCE_METALNESS:
				if (GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(ImportData.Materials[i]) != NONE_INDEX)
				{
					std::string textureName = ImportData.TexturesPaths[GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(ImportData.Materials[i])];
					GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(NewMaterial) = Engine->TexturesSystem->Add(textureName, false);
				}
				if (GDRGPUMaterialCookTorranceGetAlbedoMapIndex(ImportData.Materials[i]) != NONE_INDEX)
				{
					std::string textureName = ImportData.TexturesPaths[GDRGPUMaterialCookTorranceGetAlbedoMapIndex(ImportData.Materials[i])];
					GDRGPUMaterialCookTorranceGetAlbedoMapIndex(NewMaterial) = Engine->TexturesSystem->Add(textureName, true);
				}
				if (GDRGPUMaterialCookTorranceGetNormalMapIndex(ImportData.Materials[i]) != NONE_INDEX)
				{
					std::string textureName = ImportData.TexturesPaths[GDRGPUMaterialCookTorranceGetNormalMapIndex(ImportData.Materials[i])];
					GDRGPUMaterialCookTorranceGetNormalMapIndex(NewMaterial) = Engine->TexturesSystem->Add(textureName, false);
				}
				if (GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(ImportData.Materials[i]) != NONE_INDEX)
				{
					std::string textureName = ImportData.TexturesPaths[GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(ImportData.Materials[i])];
					GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(NewMaterial) = Engine->TexturesSystem->Add(textureName, false);
				}
				break;
			case MATERIAL_SHADER_COOKTORRANCE_SPECULAR:
				if (GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(ImportData.Materials[i]) != NONE_INDEX)
				{
					std::string textureName = ImportData.TexturesPaths[GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(ImportData.Materials[i])];
					GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(NewMaterial) = Engine->TexturesSystem->Add(textureName, false);
				}
				if (GDRGPUMaterialCookTorranceGetAlbedoMapIndex(ImportData.Materials[i]) != NONE_INDEX)
				{
					std::string textureName = ImportData.TexturesPaths[GDRGPUMaterialCookTorranceGetAlbedoMapIndex(ImportData.Materials[i])];
					GDRGPUMaterialCookTorranceGetAlbedoMapIndex(NewMaterial) = Engine->TexturesSystem->Add(textureName, true);
				}
				if (GDRGPUMaterialCookTorranceGetNormalMapIndex(ImportData.Materials[i]) != NONE_INDEX)
				{
					std::string textureName = ImportData.TexturesPaths[GDRGPUMaterialCookTorranceGetNormalMapIndex(ImportData.Materials[i])];
					GDRGPUMaterialCookTorranceGetNormalMapIndex(NewMaterial) = Engine->TexturesSystem->Add(textureName, false);
				}
				if (GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(ImportData.Materials[i]) != NONE_INDEX)
				{
					std::string textureName = ImportData.TexturesPaths[GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(ImportData.Materials[i])];
					GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(NewMaterial) = Engine->TexturesSystem->Add(textureName, true);
				}
				break;
			default:
				GDR_FAILED("Unsupported shader type.");
				break;
			}
		}

		// Hierarchy -> first pass
		NewModelRender.Hierarchy.reserve(ImportData.HierarchyNodes.size());
		for (int i = 0; i < ImportData.HierarchyNodes.size(); i++)
		{
			NewModelRender.Hierarchy.emplace_back();
			render_model_node &NewNode = NewModelRender.Hierarchy[i];
			NewNode.Name = ImportData.HierarchyNodes[i].Name;
			NewNode.Type = ImportData.HierarchyNodes[i].Type;
			NewNode.ParentIndex = ImportData.HierarchyNodes[i].ParentIndex;
			NewNode.ChildIndex = ImportData.HierarchyNodes[i].ChildIndex;
			NewNode.NextIndex = ImportData.HierarchyNodes[i].NextIndex;
			NewNode.GlobalKeyframes = ImportData.HierarchyNodes[i].GlobalKeyframes;
			NewNode.LocalKeyframes = ImportData.HierarchyNodes[i].LocalKeyframes;
			if (NewNode.Type == gdr_hier_node_type::node)
			{
				NewNode.NodeTransform = Engine->NodeTransformsSystem->Add();
			}
			else if (NewNode.Type == gdr_hier_node_type::mesh)
			{
				NewModelRender.Meshes.push_back(i);
			}
			else
			{
				GDR_FAILED("UNKNWON NODE TYPE");
			}
		}
		// Hierarchy -> second pass
		for (int i = 0; i < ImportData.HierarchyNodes.size(); i++)
		{
			render_model_node& NewNode = NewModelRender.Hierarchy[i];
			if (NewNode.Type == gdr_hier_node_type::node)
			{
				GDRGPUNodeTransform &NewNodeTransform = Engine->NodeTransformsSystem->GetEditable(NewNode.NodeTransform);
				NewNodeTransform.LocalTransform = ImportData.HierarchyNodes[i].LocalTransform;
				NewNodeTransform.BoneOffset = ImportData.HierarchyNodes[i].BoneOffset;
				NewNodeTransform.IsNeedRecalc = true;
				NewNodeTransform.ParentIndex = NewNode.ParentIndex == NONE_INDEX ? NONE_INDEX : NewModelRender.Hierarchy[NewNode.ParentIndex].NodeTransform;
				NewNodeTransform.ChildIndex = NewNode.ChildIndex == NONE_INDEX ? NONE_INDEX : NewModelRender.Hierarchy[NewNode.ChildIndex].NodeTransform;
				NewNodeTransform.NextIndex = NewNode.NextIndex == NONE_INDEX ? NONE_INDEX : NewModelRender.Hierarchy[NewNode.NextIndex].NodeTransform;
			}
			else if(NewNode.Type == gdr_hier_node_type::mesh)
			{
				const std::vector<GDRVertex> &vertices = ImportData.HierarchyNodes[i].Vertices;
				const std::vector<UINT> &indices = ImportData.HierarchyNodes[i].Indices;

				gdr_index BoneMapping = NONE_INDEX;

				if (ImportData.HierarchyNodes[i].BonesMapping.size() > 0) // fill BoneMapping with correct values
				{
					BoneMapping = Engine->BoneMappingSystem->Add();
					for (int j = 0; j < ImportData.HierarchyNodes[i].BonesMapping.size(); j++)
						Engine->BoneMappingSystem->GetEditable(BoneMapping).BoneMapping[j] =
							NewModelRender.Hierarchy[ImportData.HierarchyNodes[i].BonesMapping[j]].NodeTransform;
				}

				gdr_index NewGeometry = Engine->GeometrySystem->Add(vertices.data(), vertices.size(), indices.data(), indices.size());
				NewNode.DrawCommand = Engine->DrawCommandsSystem->Add(
					NewGeometry,
					NewModelRender.RootTransform,
					NewModelRender.Materials[ImportData.HierarchyNodes[i].MaterialIndex],
					BoneMapping);
				Engine->DrawCommandsSystem->GetEditable(NewNode.DrawCommand).Indices.ObjectParamsMask = ImportData.HierarchyNodes[i].Params;
			}
			else
			{
				GDR_FAILED("UNKNWON NODE TYPE");
			}
		}
	}

	return NewModelIndex;
}

void gdr::models_manager::Clone(gdr_index SrcModel, gdr_index DstModel)
{
	const model& OldModel = Get(SrcModel);
	model& NewModel = GetEditable(DstModel);

	NewModel.Name = OldModel.Name;

	// Render Model Clone
	{
		const render_model& OldModelRender = OldModel.Render;
		render_model& NewModelRender = NewModel.Render;

		// Root Transform
		NewModelRender.RootTransform = Engine->ObjectTransformsSystem->Add();
		Engine->ObjectTransformsSystem->GetEditable(NewModelRender.RootTransform) 
			= Engine->ObjectTransformsSystem->Get(OldModelRender.RootTransform);

		// Materials
		NewModelRender.Materials.reserve(OldModelRender.Materials.size());
		for (int i = 0; i < OldModelRender.Materials.size(); i++)
			NewModelRender.Materials.push_back(OldModelRender.Materials[i]);

		// Hierarchy first pass
		NewModelRender.Hierarchy.reserve(OldModelRender.Hierarchy.size());
		for (int i = 0; i < OldModelRender.Hierarchy.size(); i++)
		{
			NewModelRender.Hierarchy.emplace_back();
			const render_model_node& OldNode = OldModelRender.Hierarchy[i];
			render_model_node& NewNode = NewModelRender.Hierarchy[i];
			NewNode.Name = OldNode.Name;
			NewNode.Type = OldNode.Type;
			NewNode.ParentIndex = OldNode.ParentIndex;
			NewNode.ChildIndex = OldNode.ChildIndex;
			NewNode.NextIndex = OldNode.NextIndex;
			NewNode.GlobalKeyframes = OldNode.GlobalKeyframes;
			NewNode.LocalKeyframes = OldNode.LocalKeyframes;
			if (NewNode.Type == gdr_hier_node_type::node)
			{
				NewNode.NodeTransform = Engine->NodeTransformsSystem->Add();
			}
			else if (NewNode.Type == gdr_hier_node_type::mesh)
			{
				NewModelRender.Meshes.push_back(i);
			}
			else
			{
				GDR_FAILED("UNKNWON NODE TYPE");
			}
		}

		// Hierarchy second pass
		for (int i = 0; i < OldModelRender.Hierarchy.size(); i++)
		{
			render_model_node& NewNode = NewModelRender.Hierarchy[i];
			const render_model_node& OldNode = OldModelRender.Hierarchy[i];

			if (NewNode.Type == gdr_hier_node_type::node)
			{
				const GDRGPUNodeTransform& OldNodeTransform = Engine->NodeTransformsSystem->Get(OldNode.NodeTransform);
				GDRGPUNodeTransform& NewNodeTransform = Engine->NodeTransformsSystem->GetEditable(NewNode.NodeTransform);

				NewNodeTransform.LocalTransform = OldNodeTransform.LocalTransform;
				NewNodeTransform.BoneOffset = OldNodeTransform.BoneOffset;
				NewNodeTransform.IsNeedRecalc = true;
				NewNodeTransform.ParentIndex = NewNode.ParentIndex == NONE_INDEX ? NONE_INDEX : NewModelRender.Hierarchy[NewNode.ParentIndex].NodeTransform;
				NewNodeTransform.ChildIndex = NewNode.ChildIndex == NONE_INDEX ? NONE_INDEX : NewModelRender.Hierarchy[NewNode.ChildIndex].NodeTransform;
				NewNodeTransform.NextIndex = NewNode.NextIndex == NONE_INDEX ? NONE_INDEX : NewModelRender.Hierarchy[NewNode.NextIndex].NodeTransform;
			}
			else if (NewNode.Type == gdr_hier_node_type::mesh)
			{
				const GDRGPUIndirectCommand &OldDrawCommand = Engine->DrawCommandsSystem->Get(OldNode.DrawCommand);
				gdr_index BoneMapping = OldDrawCommand.Indices.BoneMappingIndex;

				if (BoneMapping != NONE_INDEX) // fill BoneMapping with correct values
				{
					BoneMapping = Engine->BoneMappingSystem->Add();
					const GDRGPUBoneMapping& OldBoneMapping = Engine->BoneMappingSystem->Get(OldDrawCommand.Indices.BoneMappingIndex);
					GDRGPUBoneMapping& NewBoneMapping = Engine->BoneMappingSystem->GetEditable(BoneMapping);

					for (int j = 0; j < MAX_BONE_PER_MESH && OldBoneMapping.BoneMapping[j] != NONE_INDEX; j++)
					{
						gdr_index in_model_index = NONE_INDEX;
						for (int k = 0; k < OldModelRender.Hierarchy.size() && in_model_index == NONE_INDEX; k++)
							if (OldModelRender.Hierarchy[k].Type == gdr_hier_node_type::node &&
								OldModelRender.Hierarchy[k].NodeTransform == OldBoneMapping.BoneMapping[j])
								in_model_index = k;
						GDR_ASSERT(in_model_index != NONE_INDEX);
						NewBoneMapping.BoneMapping[j] = NewModelRender.Hierarchy[in_model_index].NodeTransform;
					}
				}

				NewNode.DrawCommand = Engine->DrawCommandsSystem->Add(
					OldDrawCommand.Indices.ObjectIndex,
					NewModelRender.RootTransform,
					OldDrawCommand.Indices.ObjectMaterialIndex,
					BoneMapping);
				// because after Add we may be lost our link
				const GDRGPUIndirectCommand& OldDrawCommand1 = Engine->DrawCommandsSystem->Get(OldNode.DrawCommand);
				Engine->DrawCommandsSystem->GetEditable(NewNode.DrawCommand).Indices.ObjectParamsMask = OldDrawCommand1.Indices.ObjectParamsMask;
			}
			else
			{
				GDR_FAILED("UNKNWON NODE TYPE");
			}
		}
	}
}
