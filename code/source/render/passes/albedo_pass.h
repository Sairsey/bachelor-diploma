#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
	/* Pass representation class */
	class albedo_pass : public base_pass
	{
		private:
			// Our shaders
			ID3DBlob* VertexShader;
			ID3DBlob* PixelShader;

			// Root signature
			ID3D12RootSignature* RootSignature;
			// Pipeline state object
			ID3D12PipelineState* PSO;

			// Command signatures
			ID3D12CommandSignature* CommandSignature;

			// indices in root_parameters
			enum struct root_parameters_draw_indices
			{
				globals_buffer_index = 0,           // root parameter for global buffer
				index_buffer_index,                 // root parameter for buffer with indices in pools
				object_transform_pool_index,        // root parameter for buffer with per object transforms
				node_transform_pool_index,          // root parameter for buffer with nodes transforms
				material_pool_index,                // root parameter for buffer with materials
				texture_pool_index,                 // root parameter for buffer with textures
				total_root_parameters,
			};
		public:
			/* Function to get name */
			std::string GetName(void) override
			{
				return "albedo_pass";
			};

			/* Function to Initialize every PSO/InputLayout/Shaders we need */
			void Initialize(void) override;

			/* Function to call Direct draw shader */
			void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

			/* Function to call Indirect draw shader */
			void CallIndirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

			/* Virtual Destructor */
			~albedo_pass() override;
	};
}