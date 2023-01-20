#pragma once
#include "def.h"

//project namespace
namespace gdr
{
	// Node of render mesh
	struct render_mesh_node
	{
		std::string Name;
		gdr_hier_node_type Type;

		// Hierarchy
		UINT ParentIndex = NONE_INDEX;
		UINT ChildIndex = NONE_INDEX;
		UINT NextIndex = NONE_INDEX;

		// Node
		gdr_index NodeTransform = NONE_INDEX;

		// Mesh
		gdr_index DrawCommand = NONE_INDEX;
	};

	// renderable mesh representation type
	struct render_mesh
	{
		std::vector<render_mesh_node> Hierarchy; // Hierarchy
		gdr_index RootTransform;                 // Root transform
	};
}