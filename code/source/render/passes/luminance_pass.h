#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
	/* Pass representation class */
	class luminance_pass : public base_pass
	{
		private:
			// Our shaders with PSO
			ID3DBlob* FirstVertexShader;
			ID3DBlob* FirstPixelShader;
			ID3D12PipelineState* FirstPSO;

			ID3DBlob* CopyVertexShader;
			ID3DBlob* CopyPixelShader;
			ID3D12PipelineState* CopyPSO;

			ID3DBlob* FinalComputeShader;
			ID3D12PipelineState* FinalPSO;

			// Root signature
			ID3D12RootSignature* DrawRootSignature;
			ID3D12RootSignature* ComputeRootSignature;

			// fullscreen rect
			GPUResource ScreenVertexBuffer;
			D3D12_VERTEX_BUFFER_VIEW ScreenVertexBufferView;

			// indices in root_parameters
			enum struct root_parameters_draw_indices
			{
				input_texture_index,    // root parameter for luminance input texture
				total_root_parameters,
			};

			enum struct root_parameters_final_indices
			{
				globals_buffer_index,   // root parameter for globals
				input_texture_index,    // root parameter for luminance input texture
				luminance_buffer_index, // root parameter for luminance buffer
				total_root_parameters,
			};
		public:
			/* Function to get name */
			std::string GetName(void) override
			{
				return "luminance_pass";
			};

			/* Function to Initialize every PSO/InputLayout/Shaders we need */
			void Initialize(void) override;

			/* Function to call Direct draw shader */
			void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

			/* Virtual Destructor */
			~luminance_pass() override;
	};
}