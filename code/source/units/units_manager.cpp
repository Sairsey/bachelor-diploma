#include "p_header.h"

// Custom remover
void gdr::units_manager::BeforeRemoveJob(gdr_index index)
{
	if (Get(index) == nullptr)
		return;
	// unbind it from parent
	if (IsExist(Get(index)->ParentUnit))
	{
		UINT32 del_index = NONE_INDEX;

		for (UINT32 i = 0; i < Get(Get(index)->ParentUnit)->ChildUnits.size() && del_index == NONE_INDEX; i++)
			if (Get(Get(index)->ParentUnit)->ChildUnits[i] == index)
				del_index = i;

		if (del_index != NONE_INDEX)
			Get(Get(index)->ParentUnit)->ChildUnits.erase(Get(Get(index)->ParentUnit)->ChildUnits.begin() + del_index);
	}
	else // if we are trying to remove a scene...
	{
		SceneRoot = NONE_INDEX;
	}

	// delete unit
	delete Get(index);
	GetEditable(index) = nullptr;
}

// Initialize only when needed
void gdr::units_manager::BeforeUpdateJob(ID3D12GraphicsCommandList* pCommandList)
{
	bool isSomeoneInitializing = false;

	for (gdr_index i = 0; i < AllocatedSize() && !isSomeoneInitializing; i++)
		if (IsExist(i) && !Get(i)->IsInited)
			isSomeoneInitializing = true;

	if (isSomeoneInitializing)
	{
		Engine->GetDevice().WaitAllUploadLists();
		Engine->GetDevice().WaitGPUIdle();
		Engine->GetDevice().ResizeUpdateBuffer(false);
		for (gdr_index i = 0; i < AllocatedSize(); i++)
			if (IsExist(i) && !Get(i)->IsInited)
			{
				GetEditable(i)->Engine = Engine;
				GetEditable(i)->Initialize();
				GetEditable(i)->IsInited = true;
			}
		Engine->GetDevice().WaitAllUploadLists();
		Engine->GetDevice().WaitGPUIdle();
		Engine->GetDevice().ResizeUpdateBuffer(true);
	}
}

// update unit recursive function
void gdr::units_manager::ResponseUnit(gdr_index index)
{
	PROFILE_CPU_BEGIN(Get(index)->GetName().c_str());
	Get(index)->Response();
	for (int i = 0; i < Get(index)->ChildUnits.size(); i++)
		ResponseUnit(Get(index)->ChildUnits[i]);
	PROFILE_CPU_END();
}

// update unit physically recursive function
void gdr::units_manager::ResponsePhysUnit(gdr_index index)
{
	PROFILE_CPU_BEGIN(Get(index)->GetName().c_str());
	Get(index)->ResponsePhys();
	for (int i = 0; i < Get(index)->ChildUnits.size(); i++)
		ResponsePhysUnit(Get(index)->ChildUnits[i]);
	PROFILE_CPU_END();
}
gdr::units_manager::units_manager(engine* Eng) : Engine(Eng), resource_pool_subsystem(Eng)
{
};

gdr_index gdr::units_manager::Add(unit_base* Unit, gdr_index ParentUnit)
{
	if (Unit == nullptr)
		return NONE_INDEX;
	GDR_ASSERT(ParentUnit == NONE_INDEX || IsExist(ParentUnit));
	gdr_index newIndex = resource_pool_subsystem::Add();
	GetEditable(newIndex) = Unit;
	GetEditable(newIndex)->Me = newIndex;

	if (ParentUnit == NONE_INDEX)
	{
		if (SceneRoot == NONE_INDEX)
			SceneRoot = newIndex;
		else
			ParentUnit = SceneRoot;
	}
	GetEditable(newIndex)->ParentUnit = ParentUnit;

	if (ParentUnit != NONE_INDEX)
		GetEditable(ParentUnit)->ChildUnits.push_back(newIndex);
	return newIndex;
}

// if we are removing unit, we need to remove all his child too
void gdr::units_manager::Remove(gdr_index index)
{
	if (IsExist(index))
		for (int i = 0; i < Get(index)->ChildUnits.size(); i++)
			Remove(Get(index)->ChildUnits[i]);
	resource_pool_subsystem::Remove(index);
}

// on update
void gdr::units_manager::Update(bool isPhysTick)
{
	if (isPhysTick)
	{
		PROFILE_CPU_BEGIN("Units phys update tick");
		if (SceneRoot != NONE_INDEX)
			ResponsePhysUnit(SceneRoot);
		PROFILE_CPU_END();
	}

	PROFILE_CPU_BEGIN("Units update frame");
	if (SceneRoot != NONE_INDEX)
		ResponseUnit(SceneRoot);
	PROFILE_CPU_END();
}
