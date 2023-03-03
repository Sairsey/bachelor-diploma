#pragma once
#include "def.h"
#include "render_model.h"

//project namespace
namespace gdr
{
	// model representation type
	struct model
	{
		std::string Name;
		render_model Render;
	};

	class models_manager : public resource_pool_subsystem<model, gdr_index_types::model, 0>
	{
		protected:
			void BeforeRemoveJob(gdr_index index) override;

			engine *Engine;
		public:
			models_manager(engine* Eng);

			gdr_index Add()
			{
				GDR_FAILED("IMPOSSIBLE TO GENERATE MODEL WITHOUT IMPORT DATA");
				return NONE_INDEX;
			}

			gdr_index Add(const model_import_data& ImportData);
			void Clone(gdr_index SrcModel, gdr_index DstModel);
	};
}