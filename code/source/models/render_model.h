#pragma once
#include "def.h"

//project namespace
namespace gdr
{
	struct animation_keyframe
	{
		float time = 0; 
		mth::vec3f pos = {0, 0, 0};
		mth::vec4f rotationQuat = {0, 0, 0, 1};
		mth::vec3f scale = {1, 1, 1};
	};

	// Node of render mesh
	struct render_model_node
	{
		std::string Name;
		gdr_hier_node_type Type;

		// Hierarchy
		gdr_index ParentIndex = NONE_INDEX;
		gdr_index ChildIndex = NONE_INDEX;
		gdr_index NextIndex = NONE_INDEX;
		std::vector<animation_keyframe> LocalKeyframes;
		std::vector<animation_keyframe> GlobalKeyframes;

		// Node
		gdr_index NodeTransform = NONE_INDEX;

		// Mesh
		gdr_index DrawCommand = NONE_INDEX;
	};

	// renderable mesh representation type
	struct render_model
	{
		std::vector<render_model_node> Hierarchy; // Hierarchy
		std::vector<gdr_index> Materials;        // Materials
		std::vector<gdr_index> Meshes;           // Indices of meshes nodes in Hierarchy
		gdr_index RootTransform;                 // Root transform

		// some helper functions
		render_model_node& GetRootNode() { return Hierarchy[0]; }

		gdr_index FindNode(std::string name)
		{
			for (gdr_index i = 0; i < Hierarchy.size(); i++)
				if (Hierarchy[i].Name == name)
					return i;
			return NONE_INDEX;
		}
	};
}