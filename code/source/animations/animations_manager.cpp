#include "p_header.h"

gdr::animation_manager::animation_manager(engine* Eng) : resource_pool_subsystem(Eng), Engine(Eng)
{
	SavedAnimation = NONE_INDEX;
}

void gdr::animation_manager::BeforeRemoveJob(gdr_index index)
{
	if (IsExist(index))
	{
		animation& AnimationToDelete = GetEditable(index);
		AnimationToDelete.Name = "GDR_ANIMATION_DELETED";
		AnimationToDelete.Duration = 0;
		AnimationToDelete.Hierarchy.clear();
	}
}

gdr_index gdr::animation_manager::Add(const gdr::model_import_data& ImportData)
{
	gdr_index NewAnimationIndex = NONE_INDEX;
	for (gdr_index i = 0; i < AllocatedSize(); i++)
		if (IsExist(i) && Get(i).Name == ImportData.FileName)
		{
			IncreaseReferenceCount(i);
			return i;
		}
	NewAnimationIndex = resource_pool_subsystem::Add();
	animation &NewAnimation = GetEditable(NewAnimationIndex);

	NewAnimation.Name = ImportData.FileName;
	NewAnimation.Duration = ImportData.AnimationDuration;
	NewAnimation.Hierarchy.resize(ImportData.HierarchyNodes.size());

	for (int i = 0; i < NewAnimation.Hierarchy.size(); i++)
	{
		NewAnimation.Hierarchy[i].Name = ImportData.HierarchyNodes[i].Name;
		NewAnimation.Hierarchy[i].LocalKeyframes = ImportData.HierarchyNodes[i].LocalKeyframes;
		NewAnimation.Hierarchy[i].GlobalKeyframes = ImportData.HierarchyNodes[i].GlobalKeyframes;
	}
	
	return NewAnimationIndex;
}

void gdr::animation_manager::SetAnimationTime(gdr_index ModelIndex, gdr_index AnimationIndex, float time, float offset, float duration)
{
	if (!Engine->ModelsManager->IsExist(ModelIndex) || !IsExist(AnimationIndex))
		return;

	const model& ModelToAnimate = Engine->ModelsManager->Get(ModelIndex);
	const animation& Animation = Get(AnimationIndex);

	if (Animation.Hierarchy[0].GlobalKeyframes.size() == 0 || Animation.Hierarchy.size() != ModelToAnimate.Render.Hierarchy.size())
		return;

	if (duration == -1)
		duration = Animation.Duration;

	time = fmod(time, duration - offset) + offset;

	{
		const render_model& RenderModel = ModelToAnimate.Render;
		const animation_node& RootNode = Animation.Hierarchy[0];

		// if we already computed this frame -> Copy values
		if (SavedAnimation == AnimationIndex && SavedTime == time)
		{
			for (int i = 0; i < RenderModel.Hierarchy.size(); i++)
				if (RenderModel.Hierarchy[i].Type == gdr_hier_node_type::node)
				{
					const render_model_node& NodeToAnimate = RenderModel.Hierarchy[i];
					GDRGPUNodeTransform& Node = Engine->NodeTransformsSystem->GetEditable(NodeToAnimate.NodeTransform);
					Node.LocalTransform = SavedLocal[i];
					Node.GlobalTransform = SavedGlobal[i];
					Node.IsNeedRecalc = false;
				}
			return;
		}

		SavedAnimation = AnimationIndex;
		SavedTime = time;
		SavedLocal.clear();
		SavedGlobal.clear();
		SavedLocal.resize(RenderModel.Hierarchy.size());
		SavedGlobal.resize(RenderModel.Hierarchy.size());

		// find left and right keys
		int leftIndex = 0;
		int rightIndex = RootNode.GlobalKeyframes.size() - 1;
		for (int j = 0; j < RootNode.GlobalKeyframes.size(); j++)
		{
			if (RootNode.GlobalKeyframes[j].time <= time)
				leftIndex = j;

			if (RootNode.GlobalKeyframes[j].time >= time)
			{
				rightIndex = j;
				break;
			}
		}

		float alpha = 0;
		if (leftIndex != rightIndex)
			alpha = (time - RootNode.GlobalKeyframes[leftIndex].time) /
				(RootNode.GlobalKeyframes[rightIndex].time - RootNode.GlobalKeyframes[leftIndex].time);

		for (int i = 0; i < RenderModel.Hierarchy.size(); i++)
			if (RenderModel.Hierarchy[i].Type == gdr_hier_node_type::node)
			{
				const render_model_node& NodeToAnimate = RenderModel.Hierarchy[i];
				const animation_node& AnimationNode = Animation.Hierarchy[i];
				
				mth::vec3f localPosition = AnimationNode.LocalKeyframes[leftIndex].pos * (1.0 - alpha) + AnimationNode.LocalKeyframes[rightIndex].pos * alpha;
				mth::vec3f localScale = AnimationNode.LocalKeyframes[leftIndex].scale * (1.0 - alpha) + AnimationNode.LocalKeyframes[rightIndex].scale * alpha;
				mth::vec4f localRotation = AnimationNode.LocalKeyframes[leftIndex].rotationQuat.slerp(AnimationNode.LocalKeyframes[rightIndex].rotationQuat, alpha);

				mth::vec3f globalPosition = AnimationNode.GlobalKeyframes[leftIndex].pos * (1.0 - alpha) + AnimationNode.GlobalKeyframes[rightIndex].pos * alpha;
				mth::vec3f globalScale = AnimationNode.GlobalKeyframes[leftIndex].scale * (1.0 - alpha) + AnimationNode.GlobalKeyframes[rightIndex].scale * alpha;
				mth::vec4f globalRotation = AnimationNode.GlobalKeyframes[leftIndex].rotationQuat.slerp(AnimationNode.GlobalKeyframes[rightIndex].rotationQuat, alpha);

				GDRGPUNodeTransform &Node = Engine->NodeTransformsSystem->GetEditable(NodeToAnimate.NodeTransform);
				Node.LocalTransform = mth::matr4f::BuildTransform(localScale, localRotation, localPosition);
				Node.GlobalTransform = mth::matr4f::BuildTransform(globalScale, globalRotation, globalPosition);
				Node.IsNeedRecalc = false;

				SavedLocal[i] = Node.LocalTransform;
				SavedGlobal[i] = Node.GlobalTransform;
			}
	}
}