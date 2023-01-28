#pragma once
#include "def.h"
#include "render_mesh.h"

//project namespace
namespace gdr
{
	// model representation type
	struct model
	{
		std::string Name;
		render_mesh Rnd;
	};

	class models_manager
	{
		private:
			engine* Eng;
		public:
			void Init(engine* eng)
			{
				Eng = eng;
			}

			gdr_index AddModel(mesh_import_data ImportData);
			void CloneModel(gdr_index SrcModel, gdr_index DstModel);
			void DeleteModel(gdr_index Model);

			std::vector<model> ModelsPool;
	};
}