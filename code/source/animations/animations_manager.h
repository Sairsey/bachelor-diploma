#pragma once
#include "def.h"

//project namespace
namespace gdr
{
	struct model_import_data;
	struct animation_keyframe
	{
		float time = 0;
		mth::vec3f pos = { 0, 0, 0 };
		mth::vec4f rotationQuat = { 0, 0, 0, 1 };
		mth::vec3f scale = { 1, 1, 1 };
	};

	struct animation_node
	{
		std::string Name;
		std::vector<animation_keyframe> LocalKeyframes;
		std::vector<animation_keyframe> GlobalKeyframes;
	};

	struct animation
	{
		std::string Name;
		float Duration;
		std::vector<animation_node> Hierarchy;
	};

	class animation_manager : public resource_pool_subsystem<animation, 0>
	{
		protected:
			void BeforeRemoveJob(gdr_index index) override;

			engine *Engine;

			gdr_index SavedAnimation;
			float SavedTime;
			std::vector<mth::matr4f> SavedLocal;
			std::vector<mth::matr4f> SavedGlobal;
		public:
			animation_manager(engine* Eng);

			gdr_index Add()
			{
				GDR_FAILED("IMPOSSIBLE TO GENERATE ANIMATION WITHOUT IMPORT DATA");
				return NONE_INDEX;
			}

			gdr_index Add(const model_import_data& ImportData);

			void SetAnimationTime(gdr_index ModelIndex, gdr_index AnimationIndex, float time, float offset = 0, float duration = -1);
	};
}