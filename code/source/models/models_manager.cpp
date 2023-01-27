#include "p_header.h"

gdr_index gdr::models_manager::AddModel(mesh_import_data ImportData)
{
	ModelsPool.emplace_back();
	model& NewModel = ModelsPool[ModelsPool.size() - 1];

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

					// fix bone indices for skinning
					for (int i = 0; i < vertices.size(); i++)
					{
						vertices[i].BonesIndices.X = (vertices[i].BonesIndices.X == NONE_INDEX) ? NONE_INDEX : NewModel.Rnd.Hierarchy[vertices[i].BonesIndices.X].NodeTransform;
						vertices[i].BonesIndices.Y = (vertices[i].BonesIndices.Y == NONE_INDEX) ? NONE_INDEX : NewModel.Rnd.Hierarchy[vertices[i].BonesIndices.Y].NodeTransform;
						vertices[i].BonesIndices.Z = (vertices[i].BonesIndices.Z == NONE_INDEX) ? NONE_INDEX : NewModel.Rnd.Hierarchy[vertices[i].BonesIndices.Z].NodeTransform;
						vertices[i].BonesIndices.W = (vertices[i].BonesIndices.W == NONE_INDEX) ? NONE_INDEX : NewModel.Rnd.Hierarchy[vertices[i].BonesIndices.W].NodeTransform;
					}

					gdr_index geometry = Eng->GeometrySystem->Add(vertices.data(), vertices.size(), indices.data(), indices.size());
					NewModel.Rnd.Hierarchy[i].DrawCommand = Eng->DrawCommandsSystem->Add(geometry);
					Eng->DrawCommandsSystem->GetEditable(NewModel.Rnd.Hierarchy[i].DrawCommand).Indices.ObjectTransformIndex = NewModel.Rnd.RootTransform;
					Eng->DrawCommandsSystem->GetEditable(NewModel.Rnd.Hierarchy[i].DrawCommand).Indices.ObjectMaterialIndex = MaterialsIndices[ImportData.HierarchyNodes[i].MaterialIndex];
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

	Eng->ObjectTransformsSystem->Remove(ModelToDelete.Rnd.RootTransform);
	for (int i = 0; i < ModelToDelete.Rnd.Hierarchy.size(); i++)
	{
		gdr::render_mesh_node &node = ModelToDelete.Rnd.Hierarchy[i];
		if (node.Type == gdr_hier_node_type::node)
		{
			Eng->NodeTransformsSystem->Remove(node.NodeTransform);
		}
		else if (node.Type == gdr_hier_node_type::mesh)
		{
			auto &command = Eng->DrawCommandsSystem->Get(node.DrawCommand);
			Eng->MaterialsSystem->Remove(command.Indices.ObjectMaterialIndex);
			// Do not delete textures. We might need them later
			Eng->DrawCommandsSystem->Remove(node.DrawCommand);
		}
	}
}
