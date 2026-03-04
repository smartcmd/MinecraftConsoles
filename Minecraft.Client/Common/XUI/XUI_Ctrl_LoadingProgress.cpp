#include "stdafx.h"

#include "XUI_Ctrl_LoadingProgress.h"
#include "..\..\Minecraft.h"
#include "..\..\ProgressRenderer.h"

int CXuiCtrlLoadingProgress::GetValue()
{
	int currentValue = 0;

	Minecraft *pMinecraft=Minecraft::GetInstance();
	currentValue = pMinecraft->progressRenderer->getCurrentPercent();
	//printf("About to render progress of %d\n", currentValue);
	return currentValue;
}

void CXuiCtrlLoadingProgress::GetRange(int *pnRangeMin, int *pnRangeMax)
{
	*pnRangeMin = 0;
	*pnRangeMax = 100;
}