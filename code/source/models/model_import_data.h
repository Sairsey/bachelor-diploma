#pragma once
#include "def.h"

//project namespace
namespace gdr
{
	struct import_model_node
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
		std::vector<GDRVertex> Vertices;
		std::vector<UINT> Indices;
		std::vector<UINT> BonesMapping; // mapping of Bones to Hierarchy
		gdr_index MaterialIndex;
		UINT Params = 0;
	};

	// model import data representation class
	struct model_import_data
	{
		std::string FileName;                         // File name
		std::vector<GDRGPUMaterial> Materials;        // Materials
		std::vector<std::string> TexturesPaths;       // Textures
		std::vector<import_model_node> HierarchyNodes; // Hierarchy
		GDRGPUObjectTransform RootTransform;          // Root transform

		bool IsEmpty(void)
		{
			return FileName == "";
		}
	};

	model_import_data ImportModelFromAssimp(std::string filename);
	std::vector<model_import_data> ImportSplittedModelFromAssimp(std::string filename);
}