#include "stdafx.h"

#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "..\..\..\Minecraft.World\SharedConstants.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.alchemy.h"
#include "XUI_Ctrl_BrewProgress.h"

int CXuiCtrlBrewProgress::GetValue()
{
	void* pvUserData;
	this->GetUserData( &pvUserData );

	if( pvUserData != NULL )
	{
		BrewingStandTileEntity *pBrewingStandTileEntity = (BrewingStandTileEntity *)pvUserData;

		return pBrewingStandTileEntity->getBrewTime();
	}

	return 0;
}

void CXuiCtrlBrewProgress::GetRange(int *pnRangeMin, int *pnRangeMax)
{
	*pnRangeMin = 0;
	*pnRangeMax = PotionBrewing::BREWING_TIME_SECONDS * SharedConstants::TICKS_PER_SECOND;
}