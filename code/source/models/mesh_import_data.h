#pragma once
#include "def.h"

//project namespace
namespace gdr
{
	struct mesh_import_node
	{
		std::string Name;
		gdr_hier_node_type Type;

		// Hierarchy
		UINT ParentIndex;
		UINT ChildIndex;
		UINT NextIndex;

		// Node
		mth::matr4f LocalTransform;
		mth::matr4f BoneOffset;

		// Mesh
		std::vector<GDRVertex> vertices;
		std::vector<UINT> indices;
		UINT MaterialIndex;
		UINT Params = 0;
	};

	// mesh import data representation class
	struct mesh_import_data
	{
		std::string FileName;                         // File name
		std::vector<GDRGPUMaterial> Materials;        // Materials
		std::vector<mesh_import_node> HierarchyNodes; // Hierarchy
		GDRGPUObjectTransform RootTransform;          // Root transform
	};
}