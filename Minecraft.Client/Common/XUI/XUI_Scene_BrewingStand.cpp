#include "stdafx.h"

#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../Common/Tutorial/Tutorial.h"
#include "../../Common/Tutorial/TutorialMode.h"
#include "../../Common/Tutorial/TutorialEnum.h"
#include "../../Minecraft.h"
#include "XUI_Ctrl_SlotList.h"
#include "XUI_Scene_BrewingStand.h"
#include "XUI_Ctrl_BrewProgress.h"
#include "XUI_Ctrl_BubblesProgress.h"


//--------------------------------------------------------------------------------------
// Name: CXuiSceneBrewingStand::OnInit
// Desc: Message handler for XM_INIT
//--------------------------------------------------------------------------------------
HRESULT CXuiSceneBrewingStand::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	MapChildControls();
	
	XuiControlSetText(m_BrewingStandText,app.GetString(IDS_BREWING_STAND));

	Minecraft *pMinecraft = Minecraft::GetInstance();

	BrewingScreenInput* initData = static_cast<BrewingScreenInput *>(pInitData->pvInitData);
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
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Brewing_Menu, this);
	}
#endif

	BrewingStandMenu* menu = new BrewingStandMenu( initData->inventory, initData->brewingStand );
	

	InitDataAssociations(m_iPad, menu);

	m_progressControl->SetUserData( initData->brewingStand.get() );
	
	m_bubbleProgress->SetUserData( initData->brewingStand.get() );

	delete initData;

	CXuiSceneAbstractContainer::Initialize( m_iPad, menu, true, BrewingStandMenu::INV_SLOT_START, eSectionBrewingUsing, eSectionBrewingMax );
	
	//app.SetRichPresenceContextValue(m_iPad,CONTEXT_GAME_STATE_FORGING);

	return S_OK;
}

HRESULT CXuiSceneBrewingStand::OnDestroy()
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

CXuiControl* CXuiSceneBrewingStand::GetSectionControl( ESceneSection eSection )
{
	switch( eSection )
	{
		case eSectionBrewingBottle1:
			return (CXuiControl *)m_bottle1Control;
			break;
		case eSectionBrewingBottle2:
			return (CXuiControl *)m_bottle2Control;
			break;
		case eSectionBrewingBottle3:
			return (CXuiControl *)m_bottle3Control;
			break;
		case eSectionBrewingIngredient:
			return (CXuiControl *)m_ingredientControl;
			break;
		case eSectionBrewingInventory:
			return (CXuiControl *)m_inventoryControl;
			break;
		case eSectionBrewingUsing:
			return (CXuiControl *)m_useRowControl;
			break;
		default:
			assert( false );
			break;
	}
	return nullptr;
}

CXuiCtrlSlotList* CXuiSceneBrewingStand::GetSectionSlotList( ESceneSection eSection )
{
	switch( eSection )
	{
		case eSectionBrewingBottle1:
			return m_bottle1Control;
			break;
		case eSectionBrewingBottle2:
			return m_bottle2Control;
			break;
		case eSectionBrewingBottle3:
			return m_bottle3Control;
			break;
		case eSectionBrewingIngredient:
			return m_ingredientControl;
			break;
		case eSectionBrewingInventory:
			return m_inventoryControl;
			break;
		case eSectionBrewingUsing:
			return m_useRowControl;
			break;
		default:
			assert( false );
			break;
	}
	return nullptr;
}

// 4J Stu - Added to support auto-save. Need to re-associate on a navigate back
void CXuiSceneBrewingStand::InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex /*= 0*/)
{
	// TODO Inventory dimensions need defined as constants
	m_ingredientControl->SetData( iPad, menu, 1, 1, BrewingStandMenu::INGREDIENT_SLOT );
	
	m_bottle1Control->SetData( iPad, menu, 1, 1, BrewingStandMenu::BOTTLE_SLOT_START );
	m_bottle2Control->SetData( iPad, menu, 1, 1, BrewingStandMenu::BOTTLE_SLOT_START + 1);
	m_bottle3Control->SetData( iPad, menu, 1, 1, BrewingStandMenu::BOTTLE_SLOT_START + 2);

	//m_litProgressControl->SetUserData( initData->furnace.get() );
	
	//m_burnProgress->SetUserData( initData->furnace.get() );

	CXuiSceneAbstractContainer::InitDataAssociations(iPad, menu, BrewingStandMenu::INV_SLOT_START);
}