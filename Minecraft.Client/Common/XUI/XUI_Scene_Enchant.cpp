#include "stdafx.h"

#include "..\..\..\Minecraft.World\AbstractContainerMenu.h"
#include "..\..\..\Minecraft.World\Slot.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.dimension.h"

//#include "..\..\..\Minecraft.World\net.minecraft.stats.h"
#include "..\..\MultiplayerLocalPlayer.h"
#include "..\..\Common\Tutorial\Tutorial.h"
#include "..\..\Common\Tutorial\TutorialMode.h"
#include "..\..\Common\Tutorial\TutorialEnum.h"
#include "..\..\Minecraft.h"
#include "XUI_Ctrl_SlotList.h"

#include "XUI_Scene_Enchant.h"
#include "XUI_Scene_Inventory.h"
#include "XUI_Ctrl_EnchantButton.h"

//--------------------------------------------------------------------------------------
// Name: CXuiSceneEnchant::OnInit
// Desc: Message handler for XM_INIT
//--------------------------------------------------------------------------------------
HRESULT CXuiSceneEnchant::OnInit( XUIMessageInit *pInitData, BOOL &bHandled )
{
	D3DXVECTOR3 vec;
	MapChildControls();
	
	XuiControlSetText(m_EnchantText,app.GetString(IDS_ENCHANT));

	Minecraft *pMinecraft = Minecraft::GetInstance();

	EnchantingScreenInput *initData = static_cast<EnchantingScreenInput *>(pInitData->pvInitData);
	m_iPad=initData->iPad;
	m_bSplitscreen=initData->bSplitscreen;

	// if we are in splitscreen, then we need to figure out if we want to move this scene

	if(m_bSplitscreen)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);
	}

#ifdef _XBOX
	if( pMinecraft->localgameModes[m_iPad] != nullptr )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[m_iPad];
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Enchanting_Menu, this);
	}
#endif

	EnchantmentMenu *menu = new EnchantmentMenu(initData->inventory, initData->level, initData->x, initData->y, initData->z);
	
	InitDataAssociations(initData->iPad, menu);

	m_enchant1->SetEnable(FALSE);
	m_enchant2->SetEnable(FALSE);
	m_enchant3->SetEnable(FALSE);
	m_enchant1->SetData(m_iPad, 0);
	m_enchant2->SetData(m_iPad, 1);
	m_enchant3->SetData(m_iPad, 2);

	CXuiSceneAbstractContainer::Initialize( initData->iPad, menu, false, EnchantmentMenu::INV_SLOT_START, CXuiSceneAbstractContainer::eSectionEnchantUsing, CXuiSceneAbstractContainer::eSectionEnchantMax );

	delete initData;

	return S_OK;
}

HRESULT CXuiSceneEnchant::OnDestroy()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();

#ifdef _XBOX
	if( pMinecraft->localgameModes[m_iPad] != nullptr )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[m_iPad];
		if(gameMode != nullptr) gameMode->getTutorial()->changeTutorialState(m_previousTutorialState);
	}
#endif

	// 4J Stu - Fix for #11302 - TCR 001: Network Connectivity: Host crashed after being killed by the client while accessing a chest during burst packet loss.
	// We need to make sure that we call closeContainer() anytime this menu is closed, even if it is forced to close by some other reason (like the player dying)	
	if(Minecraft::GetInstance()->localplayers[m_iPad] != nullptr) Minecraft::GetInstance()->localplayers[m_iPad]->closeContainer();
	return S_OK;
}

CXuiControl* CXuiSceneEnchant::GetSectionControl( ESceneSection eSection )
{
	switch( eSection )
	{
		case CXuiSceneAbstractContainer::eSectionEnchantInventory:
			return (CXuiControl *)m_inventoryControl;
			break;
		case CXuiSceneAbstractContainer::eSectionEnchantUsing:
			return (CXuiControl *)m_useRowControl;
			break;
		case CXuiSceneAbstractContainer::eSectionEnchantSlot:
			return (CXuiControl *)m_ingredientControl;
			break;			
		case eSectionEnchantButton1:
			return m_enchant1;
			break;
		case eSectionEnchantButton2:
			return m_enchant2;
			break;
		case eSectionEnchantButton3:
			return m_enchant3;
			break;
		default:
			assert( false );
			break;
	}
	return nullptr;
}

CXuiCtrlSlotList* CXuiSceneEnchant::GetSectionSlotList( ESceneSection eSection )
{
	switch( eSection )
	{
		case CXuiSceneAbstractContainer::eSectionEnchantInventory:
			return m_inventoryControl;
			break;
		case CXuiSceneAbstractContainer::eSectionEnchantUsing:
			return m_useRowControl;
			break;
		case CXuiSceneAbstractContainer::eSectionEnchantSlot:
			return m_ingredientControl;
			break;
		default:
			assert( false );
			break;
	}
	return nullptr;
}

// 4J Stu - Added to support auto-save. Need to re-associate on a navigate back
void CXuiSceneEnchant::InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex /*= 0*/)
{
	m_ingredientControl->SetData( iPad, menu,  1, 1, EnchantmentMenu::INGREDIENT_SLOT );

	CXuiSceneAbstractContainer::InitDataAssociations(iPad, menu, EnchantmentMenu::INV_SLOT_START);
}