#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
	/* Pass representation class */
	class oit_compose_pass : public base_pass
	{
		private:
			// Our shaders
			ID3DBlob* VertexShader;
			ID3DBlob* PixelShader;

			// fullscreen rect
			GPUResource ScreenVertexBuffer;
			D3D12_VERTEX_BUFFER_VIEW ScreenVertexBufferView;

			// Root signature
			ID3D12RootSignature* RootSignature;
			// Pipeline state object
			ID3D12PipelineState* PSO;

			// indices in root_parameters
			enum struct root_parameters_draw_indices
			{
				globals_buffer_index = 0, // root parameter for global buffer
				oit_texture_index,
				oit_pool_index,
				total_root_parameters,
			};
		public:
			/* Function to get name */
			std::string GetName(void) override
			{
				return "oit_compose";
			};

			/* Function to Initialize every PSO/InputLayout/Shaders we need */
			void Initialize(void) override;

			/* Function to call Direct draw shader */
			void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

			/* Virtual Destructor */
			~oit_compose_pass() override;
	};
}