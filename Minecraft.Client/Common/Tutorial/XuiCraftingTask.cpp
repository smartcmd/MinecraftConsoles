#include "stdafx.h"
#include "../../../Minecraft.World/ItemInstance.h"
#if !(defined _XBOX) && !(defined __PSVITA__)
#include "../UI/UI.h"
#endif
#include "Tutorial.h"
#include "XuiCraftingTask.h"

bool XuiCraftingTask::isCompleted()
{
#ifndef __PSVITA__
	// This doesn't seem to work
	//IUIScene_CraftingMenu *craftScene = reinterpret_cast<IUIScene_CraftingMenu *>(tutorial->getScene());
#ifdef _XBOX
	CXuiSceneCraftingPanel *craftScene = (CXuiSceneCraftingPanel *)(tutorial->getScene());
#else
	UIScene_CraftingMenu *craftScene = reinterpret_cast<UIScene_CraftingMenu *>(tutorial->getScene());
#endif

	bool completed = false;

	switch(m_type)
	{
	case e_Crafting_SelectGroup:
		if(craftScene != NULL && craftScene->getCurrentGroup() == m_group)
		{
			completed = true;
		}
		break;
	case e_Crafting_SelectItem:
		if(craftScene != NULL && craftScene->isItemSelected(m_item))
		{
			completed = true;
		}
		break;
	}

	return completed;
#else
	return true;
#endif
}
