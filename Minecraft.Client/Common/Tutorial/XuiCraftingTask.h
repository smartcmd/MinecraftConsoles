#pragma once
#include "TutorialTask.h"
#include "..\..\..\Minecraft.World\Recipy.h"

class XuiCraftingTask : public TutorialTask
{
public:
	enum eCraftingTaskType
	{
		e_Crafting_SelectGroup,
		e_Crafting_SelectItem,
	};

	// Select group
	XuiCraftingTask(Tutorial *tutorial, int descriptionId, Recipy::_eGroupType groupToSelect, bool enablePreCompletion = false, vector<TutorialConstraint *> *inConstraints = nullptr, 
		bool bShowMinimumTime=false,  bool bAllowFade=true, bool m_bTaskReminders=true )
		: TutorialTask(tutorial, descriptionId, enablePreCompletion, inConstraints,	bShowMinimumTime, bAllowFade, m_bTaskReminders ),
		m_group(groupToSelect),
		m_type( e_Crafting_SelectGroup )
	{}

	// Select Item
	XuiCraftingTask(Tutorial *tutorial, int descriptionId, int itemId, bool enablePreCompletion = false, vector<TutorialConstraint *> *inConstraints = nullptr, 
		bool bShowMinimumTime=false,  bool bAllowFade=true, bool m_bTaskReminders=true )
		: TutorialTask(tutorial, descriptionId, enablePreCompletion, inConstraints,	bShowMinimumTime, bAllowFade, m_bTaskReminders ),
		m_item(itemId),
		m_type( e_Crafting_SelectItem )
	{}

	virtual bool isCompleted();

private:
	eCraftingTaskType m_type;
	Recipy::_eGroupType m_group;
	int m_item;
};