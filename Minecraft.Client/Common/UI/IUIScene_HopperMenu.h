#pragma once

#include "IUIScene_AbstractContainerMenu.h"
#include "../../../Minecraft.World/Container.h"
#include "../../../Minecraft.World/Inventory.h"

class IUIScene_HopperMenu : public virtual IUIScene_AbstractContainerMenu
{
public:
	virtual ESceneSection GetSectionAndSlotInDirection(ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY);
	int getSectionStartOffset(ESceneSection eSection);
};
