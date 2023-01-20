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
			NewModel.Rnd.RootTransform = Eng->ObjectTransformsSystem->AddElementInPool();
			Eng->ObjectTransformsSystem->CPUData[NewModel.Rnd.RootTransform] = ImportData.RootTransform;
		}

		// Second -> Copy all materials
		std::vector<gdr_index> MaterialsIndices;
		{
			for (int i = 0; i < ImportData.Materials.size(); i++)
			{
				MaterialsIndices.push_back(Eng->MaterialsSystem->AddElementInPool());
				Eng->MaterialsSystem->CPUData[MaterialsIndices[i]] = ImportData.Materials[i];
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
					NewModel.Rnd.Hierarchy[i].NodeTransform = Eng->NodeTransformsSystem->AddElementInPool();
				}
			}
			//Second pass 
			// -> Fix hierarchy for nodes
			// -> For all meshes init everything
			for (int i = 0; i < ImportData.HierarchyNodes.size(); i++)
			{
				if (NewModel.Rnd.Hierarchy[i].Type == gdr_hier_node_type::node)
				{
					Eng->NodeTransformsSystem->CPUData[NewModel.Rnd.Hierarchy[i].NodeTransform].LocalTransform = ImportData.HierarchyNodes[i].LocalTransform;
					Eng->NodeTransformsSystem->CPUData[NewModel.Rnd.Hierarchy[i].NodeTransform].BoneOffset = ImportData.HierarchyNodes[i].BoneOffset;
					Eng->NodeTransformsSystem->CPUData[NewModel.Rnd.Hierarchy[i].NodeTransform].ParentIndex = 
						(NewModel.Rnd.Hierarchy[i].ParentIndex == NONE_INDEX) ?
						NONE_INDEX :
						NewModel.Rnd.Hierarchy[NewModel.Rnd.Hierarchy[i].ParentIndex].NodeTransform;

					Eng->NodeTransformsSystem->CPUData[NewModel.Rnd.Hierarchy[i].NodeTransform].NextIndex =
						(NewModel.Rnd.Hierarchy[i].NextIndex == NONE_INDEX) ?
						NONE_INDEX :
						NewModel.Rnd.Hierarchy[NewModel.Rnd.Hierarchy[i].NextIndex].NodeTransform;
					Eng->NodeTransformsSystem->CPUData[NewModel.Rnd.Hierarchy[i].NodeTransform].ChildIndex =
						(NewModel.Rnd.Hierarchy[i].ChildIndex == NONE_INDEX) ?
						NONE_INDEX :
						NewModel.Rnd.Hierarchy[NewModel.Rnd.Hierarchy[i].NextIndex].ChildIndex;
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

					Eng->GeometrySystem->CreateGeometry(
						vertices.data(),
						vertices.size(),
						indices.data(),
						indices.size());
					NewModel.Rnd.Hierarchy[i].DrawCommand = Eng->DrawCommandsSystem->AddElementInPool(Eng->GeometrySystem->CPUData[Eng->GeometrySystem->CPUData.size() - 1]);
					Eng->DrawCommandsSystem->CPUData[NewModel.Rnd.Hierarchy[i].DrawCommand].Indices.ObjectTransformIndex = NewModel.Rnd.RootTransform;
					Eng->DrawCommandsSystem->CPUData[NewModel.Rnd.Hierarchy[i].DrawCommand].Indices.ObjectMaterialIndex = MaterialsIndices[ImportData.HierarchyNodes[i].MaterialIndex];
					Eng->DrawCommandsSystem->CPUData[NewModel.Rnd.Hierarchy[i].DrawCommand].Indices.ObjectParamsMask = ImportData.HierarchyNodes[i].Params;
				}
			}
		}
	}
	return ModelsPool.size() - 1;
}
