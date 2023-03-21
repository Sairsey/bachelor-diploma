#pragma once
#include "def.h"
#include "unit_base.h"

//project namespace
namespace gdr
{
	class engine;
  class units_manager : public resource_pool_subsystem<unit_base *, gdr_index_types::unit, 0>
  {
		protected:
			// Custom remover
			void BeforeRemoveJob(gdr_index index) override;
			// Initialize only when needed
			void BeforeUpdateJob(ID3D12GraphicsCommandList* pCommandList) override;

			// update unit recursive function
			void ResponseUnit(gdr_index index);

			// update unit physically recursive function
			void ResponsePhysUnit(gdr_index index);

			engine* Engine;
			gdr_index SceneRoot = NONE_INDEX;

		public:
			units_manager(engine* Eng);

			gdr_index Add()
			{
				GDR_FAILED("IMPOSSIBLE TO ADD UNIT WITHOUT PARAMETER");
				return NONE_INDEX;
			}

			gdr_index Add(unit_base* Unit, gdr_index ParentUnit = NONE_INDEX);

			// if we are removing unit, we need to remove all his child too
			void Remove(gdr_index index);

			// on update
			void Update(bool isPhysTick);

			gdr_index GetSceneRoot(void)
			{
				return SceneRoot;
			}
  };
}