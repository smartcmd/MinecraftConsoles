#include "stdafx.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\Font.h"
#include "..\..\Lighting.h"
#include "..\..\MultiPlayerLocalPlayer.h"
#include "XUI_Scene_Enchant.h"
#include "XUI_Ctrl_EnchantButton.h"

//-----------------------------------------------------------------------------
HRESULT CXuiCtrlEnchantmentButton::OnInit(XUIMessageInit* pInitData, BOOL& rfHandled)
{
	HRESULT hr=S_OK;

	Minecraft *pMinecraft=Minecraft::GetInstance();

	ScreenSizeCalculator ssc(pMinecraft->options, pMinecraft->width_phys, pMinecraft->height_phys);
	m_fScreenWidth=(float)pMinecraft->width_phys;
	m_fRawWidth=(float)ssc.rawWidth;
	m_fScreenHeight=(float)pMinecraft->height_phys;
	m_fRawHeight=(float)ssc.rawHeight;

	HXUIOBJ parent = m_hObj;
	HXUICLASS hcInventoryClass = XuiFindClass( L"CXuiSceneEnchant" );
	HXUICLASS currentClass;

	do
	{
		XuiElementGetParent(parent,&parent);
		currentClass = XuiGetObjectClass( parent );
	} while (parent != NULL && !XuiClassDerivesFrom( currentClass, hcInventoryClass ) );

	assert( parent != NULL );

	VOID *pObj;
	XuiObjectFromHandle( parent, &pObj );
	m_containerScene = (CXuiSceneEnchant *)pObj;

	m_index = 0;
	m_lastCost = 0;
	m_iPad = 0;
	m_costString = L"";

	return hr;
}

void CXuiCtrlEnchantmentButton::SetData(int iPad, int index)
{
	m_iPad = iPad;
	m_index = index;
}

HRESULT CXuiCtrlEnchantmentButton::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled)
{
	EnchantmentMenu *menu = m_containerScene->getMenu();

	int cost = menu->costs[m_index];

	if(cost != m_lastCost)
	{
		Minecraft *pMinecraft = Minecraft::GetInstance();
		if(cost > pMinecraft->localplayers[m_iPad]->experienceLevel && !pMinecraft->localplayers[m_iPad]->abilities.instabuild)
		{
			// Dark background
			SetEnable(FALSE);
		}
		else
		{
			// Light background and focus background
			SetEnable(TRUE);
		}
		m_costString = _toString<int>(cost);
		m_lastCost = cost;
	}
	if(cost == 0)
	{
		// Dark background
		SetEnable(FALSE);
		pGetSourceTextData->bDisplay = FALSE;
	}
	else
	{		
		pGetSourceTextData->szText = m_costString.c_str();
		pGetSourceTextData->bDisplay = TRUE;
	}

	bHandled = TRUE;

	return S_OK;
}