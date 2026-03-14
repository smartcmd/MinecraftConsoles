#include "stdafx.h"

#include "..\..\..\Minecraft.World\FurnaceMenu.h"
#include "..\..\..\Minecraft.World\FurnaceTileEntity.h"
#include "XUI_Scene_Furnace.h"
#include "XUI_Ctrl_BurnProgress.h"

int CXuiCtrlBurnProgress::GetValue()
{
	void* pvUserData;
	this->GetUserData( &pvUserData );

	if( pvUserData != nullptr )
	{
		FurnaceTileEntity *pFurnaceTileEntity = static_cast<FurnaceTileEntity *>(pvUserData);

		// TODO This param is a magic number in Java but we should really define it somewhere with a name
		// I think it is the number of states of the progress display (ie the max value)
		return pFurnaceTileEntity->getBurnProgress( 24 );
	}

	return 0;
}

void CXuiCtrlBurnProgress::GetRange(int *pnRangeMin, int *pnRangeMax)
{
	*pnRangeMin = 0;
	*pnRangeMax = 24;
}