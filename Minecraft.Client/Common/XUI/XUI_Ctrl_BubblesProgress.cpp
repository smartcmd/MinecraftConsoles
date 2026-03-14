#include "stdafx.h"

#include "../../../Minecraft.World/net.minecraft.world.level.tile.entity.h"
#include "XUI_Ctrl_BubblesProgress.h"

int CXuiCtrlBubblesProgress::GetValue()
{
	void* pvUserData;
	this->GetUserData( &pvUserData );

	if( pvUserData != nullptr )
	{
		BrewingStandTileEntity *pBrewingStandTileEntity = static_cast<BrewingStandTileEntity *>(pvUserData);

		int value = 0;
		int bubbleStep = (pBrewingStandTileEntity->getBrewTime() / 2) % 7;
		switch (bubbleStep)
		{
		case 0:
			value = 0;
			break;
		case 6:
			value = 5;
			break;
		case 5:
			value = 10;
			break;
		case 4:
			value = 15;
			break;
		case 3:
			value = 20;
			break;
		case 2:
			value = 25;
			break;
		case 1:
			value = 30;
			break;
		}

		return value;
	}

	return 0;
}

void CXuiCtrlBubblesProgress::GetRange(int *pnRangeMin, int *pnRangeMax)
{
	*pnRangeMin = 0;
	*pnRangeMax = 30;
}