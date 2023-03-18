#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
	/* Pass representation class */
	class topmost_pass : public base_pass
	{
		private:
			// Our shaders
			ID3DBlob* VertexShader;
			ID3DBlob* PixelShader;

			// Root signature
			ID3D12RootSignature* RootSignature;
			// Pipeline state object
			ID3D12PipelineState* PSO;

			// indices in root_parameters
			enum struct root_parameters_draw_indices
			{
				globals_buffer_index = 0,           // root parameter for global buffer
				index_buffer_index,                 // root parameter for buffer with indices in pools
				enviroment_buffer_index,            // root parameter for enviroment buffer
				object_transform_pool_index,        // root parameter for buffer with per object transforms
				node_transform_pool_index,          // root parameter for buffer with nodes transforms
				material_pool_index,                // root parameter for buffer with materials
				texture_pool_index,                 // root parameter for buffer with textures
				cube_texture_pool_index,            // root parameter for buffer with textures
				lights_pool_index,                  // root parameter for buffer with light sources
				bone_mapping_pool_index,            // root parameter for buffer with bone mappings
				shadow_map_pool_index,              // root parameter for buffer with shadow maps
				total_root_parameters,
			};
		public:
			/* Function to get name */
			std::string GetName(void) override
			{
				return "topmost_pass";
			};

			/* Function to Initialize every PSO/InputLayout/Shaders we need */
			void Initialize(void) override;

			/* Function to call Direct draw shader */
			void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

			/* Virtual Destructor */
			~topmost_pass() override;
	};
}