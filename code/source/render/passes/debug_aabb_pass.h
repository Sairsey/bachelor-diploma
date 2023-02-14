#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
	/* Pass representation class */
	class debug_aabb_pass : public base_pass
	{
		private:
			// Our shaders
			ID3DBlob* VertexShader;
			ID3DBlob* PixelShader;

			struct ShaderParams
			{
				mth::matr4f VP;
				mth::matr4f transform;
				mth::vec3f minAABB;
				float pad1;
				mth::vec3f maxAABB;
				float pad2;
				mth::vec3f color;
				float pad3;
			};

			// Root signature
			ID3D12RootSignature* RootSignature;
			// Pipeline state object
			ID3D12PipelineState* PSO;

			// box with aabb
			GPUResource VertexBuffer;
			D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
			GPUResource IndexBuffer;
			D3D12_INDEX_BUFFER_VIEW IndexBufferView;
			UINT IndexCount;


			// indices in root_parameters
			enum struct root_parameters_draw_indices
			{
				params_buffer_index = 0,           // root parameter for params buffer
				total_root_parameters,
			};

			mth::vec3f TransparentBoxColor = {0, 0, 1};
			mth::vec3f OpaqueBoxColor = { 0, 1, 0 };
		public:
			/* Function to get name */
			std::string GetName(void) override
			{
				return "debug_aabb_pass";
			};

			/* Function to Initialize every PSO/InputLayout/Shaders we need */
			void Initialize(void) override;

			/* Function to call Direct draw shader */
			void CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList) override;

			/* Virtual Destructor */
			~debug_aabb_pass() override;
	};
}