#include "stdafx.h"

#include "../../../Minecraft.World/FurnaceMenu.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../Common/Tutorial/Tutorial.h"
#include "../../Common/Tutorial/TutorialMode.h"
#include "../../Common/Tutorial/TutorialEnum.h"
#include "../../Minecraft.h"
#include "XUI_Ctrl_SlotList.h"
#include "XUI_Scene_Furnace.h"
#include "XUI_Ctrl_BurnProgress.h"
#include "XUI_Ctrl_FireProgress.h"


//--------------------------------------------------------------------------------------
// Name: CXuiSceneFurnace::OnInit
// Desc: Message handler for XM_INIT
//--------------------------------------------------------------------------------------
HRESULT CXuiSceneFurnace::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	MapChildControls();
	
	XuiControlSetText(m_FurnaceText,app.GetString(IDS_FURNACE));	
	XuiControlSetText(m_IngredientText,app.GetString(IDS_INGREDIENT));	
	XuiControlSetText(m_FuelText,app.GetString(IDS_FUEL));

	Minecraft *pMinecraft = Minecraft::GetInstance();

	FurnaceScreenInput* initData = static_cast<FurnaceScreenInput *>(pInitData->pvInitData);
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
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Furnace_Menu, this);
	}
#endif

	FurnaceMenu* menu = new FurnaceMenu( initData->inventory, initData->furnace );
	

	InitDataAssociations(m_iPad, menu);

	m_litProgressControl->SetUserData( initData->furnace.get() );
	
	m_burnProgress->SetUserData( initData->furnace.get() );

	CXuiSceneAbstractContainer::Initialize( m_iPad, menu, true, FurnaceMenu::INV_SLOT_START, eSectionFurnaceUsing, eSectionFurnaceMax );
	
	app.SetRichPresenceContext(m_iPad,CONTEXT_GAME_STATE_FORGING);

	delete initData;

	return S_OK;
}

HRESULT CXuiSceneFurnace::OnDestroy()
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

CXuiControl* CXuiSceneFurnace::GetSectionControl( ESceneSection eSection )
{
	switch( eSection )
	{
		case eSectionFurnaceResult:
			return (CXuiControl *)m_resultControl;
			break;
		case eSectionFurnaceFuel:
			return (CXuiControl *)m_fuelControl;
			break;
		case eSectionFurnaceIngredient:
			return (CXuiControl *)m_ingredientControl;
			break;
		case eSectionFurnaceInventory:
			return (CXuiControl *)m_inventoryControl;
			break;
		case eSectionFurnaceUsing:
			return (CXuiControl *)m_useRowControl;
			break;
		default:
			assert( false );
			break;
	}
	return nullptr;
}

CXuiCtrlSlotList* CXuiSceneFurnace::GetSectionSlotList( ESceneSection eSection )
{
	switch( eSection )
	{
		case eSectionFurnaceResult:
			return m_resultControl;
			break;
		case eSectionFurnaceFuel:
			return m_fuelControl;
			break;
		case eSectionFurnaceIngredient:
			return m_ingredientControl;
			break;
		case eSectionFurnaceInventory:
			return m_inventoryControl;
			break;
		case eSectionFurnaceUsing:
			return m_useRowControl;
			break;
		default:
			assert( false );
			break;
	}
	return nullptr;
}

// 4J Stu - Added to support auto-save. Need to re-associate on a navigate back
void CXuiSceneFurnace::InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex /*= 0*/)
{
	// TODO Inventory dimensions need defined as constants
	m_ingredientControl->SetData( iPad, menu, 1, 1, FurnaceMenu::INGREDIENT_SLOT );
	
	m_fuelControl->SetData( iPad, menu, 1, 1, FurnaceMenu::FUEL_SLOT );
	
	m_resultControl->SetData( iPad, menu, 1, 1, FurnaceMenu::RESULT_SLOT );

	//m_litProgressControl->SetUserData( initData->furnace.get() );
	
	//m_burnProgress->SetUserData( initData->furnace.get() );

	CXuiSceneAbstractContainer::InitDataAssociations(iPad, menu, FurnaceMenu::INV_SLOT_START);
}

