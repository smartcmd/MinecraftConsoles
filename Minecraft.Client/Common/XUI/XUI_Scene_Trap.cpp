#include "stdafx.h"

#include "..\..\..\Minecraft.World\DispenserTileEntity.h"
#include "..\..\..\Minecraft.World\TrapMenu.h"
#include "..\..\MultiplayerLocalPlayer.h"
#include "XUI_Ctrl_SlotList.h"
#include "XUI_Scene_Trap.h"
#include "..\..\Common\Tutorial\Tutorial.h"
#include "..\..\Common\Tutorial\TutorialMode.h"
#include "..\..\Common\Tutorial\TutorialEnum.h"

//--------------------------------------------------------------------------------------
// Name: CXuiSceneTrap::OnInit
// Desc: Message handler for XM_INIT
//--------------------------------------------------------------------------------------
HRESULT CXuiSceneTrap::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	D3DXVECTOR3 vec;
	MapChildControls();
	
	XuiControlSetText(m_DispenserText,app.GetString(IDS_DISPENSER));

	Minecraft *pMinecraft = Minecraft::GetInstance();

	TrapScreenInput* initData = static_cast<TrapScreenInput *>(pInitData->pvInitData);
	m_iPad=initData->iPad;
	m_bSplitscreen=initData->bSplitscreen;

#ifdef _XBOX
	if( pMinecraft->localgameModes[initData->iPad] != nullptr )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[initData->iPad];
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Trap_Menu, this);
	}
#endif
	// if we are in splitscreen, then we need to figure out if we want to move this scene

	if(m_bSplitscreen)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);
	}

	TrapMenu* menu = new TrapMenu( initData->inventory, initData->trap );

	InitDataAssociations(initData->iPad, menu);

	CXuiSceneAbstractContainer::Initialize( initData->iPad, menu, true, initData->trap->getContainerSize(), eSectionTrapUsing, eSectionTrapMax );

	delete initData;

	return S_OK;
}

HRESULT CXuiSceneTrap::OnDestroy()
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

CXuiControl* CXuiSceneTrap::GetSectionControl( ESceneSection eSection )
{
	switch( eSection )
	{
		case eSectionTrapTrap:
			return (CXuiControl *)m_trapControl;
			break;
		case eSectionTrapInventory:
			return (CXuiControl *)m_inventoryControl;
			break;
		case eSectionTrapUsing:
			return (CXuiControl *)m_useRowControl;
			break;
		default:
			assert( false );
			break;
	}
	return nullptr;
}

CXuiCtrlSlotList* CXuiSceneTrap::GetSectionSlotList( ESceneSection eSection )
{
	switch( eSection )
	{
		case eSectionTrapTrap:
			return m_trapControl;
			break;
		case eSectionTrapInventory:
			return m_inventoryControl;
			break;
		case eSectionTrapUsing:
			return m_useRowControl;
			break;
		default:
			assert( false );
			break;
	}
	return nullptr;
}

// 4J Stu - Added to support auto-save. Need to re-associate on a navigate back
void CXuiSceneTrap::InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex /*= 0*/)
{
	int containerSize = menu->getSize() - (27 + 9);

	// TODO Inventory dimensions need defined as constants
	m_trapControl->SetData( iPad, menu, 3, 3, 0 );

	CXuiSceneAbstractContainer::InitDataAssociations(iPad, menu, containerSize);
}