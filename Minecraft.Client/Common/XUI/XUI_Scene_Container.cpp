#include "stdafx.h"

#include "../../../Minecraft.World/Container.h"
#include "../../../Minecraft.World/ContainerMenu.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "XUI_Ctrl_SlotList.h"
#include "XUI_Scene_Container.h"
#include "XUI_Ctrl_SlotItemListItem.h"
#include "XUI_Ctrl_SlotItem.h"
#include "../../Common/Tutorial/Tutorial.h"
#include "../../Common/Tutorial/TutorialMode.h"
#include "../../Common/Tutorial/TutorialEnum.h"

// The height of one row of slots
//#define ROW_HEIGHT 42.0f - comes from the pointer height in the xui

// The number of container rows that are visible in the Xui file at it's default size
#define CONTAINER_DEFAULT_ROWS 3


//--------------------------------------------------------------------------------------
// Name: CXuiSceneContainer::OnInit
// Desc: Message handler for XM_INIT
//--------------------------------------------------------------------------------------
HRESULT CXuiSceneContainer::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	D3DXVECTOR3 vec;
	MapChildControls();

	Minecraft *pMinecraft = Minecraft::GetInstance();

	ContainerScreenInput* initData = (ContainerScreenInput*)pInitData->pvInitData;
	
	XuiControlSetText(m_ChestText,app.GetString(initData->container->getName()));

	ContainerMenu* menu = new ContainerMenu( initData->inventory, initData->container );

	shared_ptr<Container> container = initData->container;
	m_iPad=initData->iPad;
	m_bSplitscreen=initData->bSplitscreen;

#ifdef _XBOX
	if( pMinecraft->localgameModes[initData->iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[initData->iPad];
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Container_Menu, this);
	}
#endif

	// if we are in splitscreen, then we need to figure out if we want to move this scene
	int rows = container->getContainerSize() / 9;
	// use the pointer size in the xui to set the row height
	float fPointerWidth,fPointerHeight;

	if(m_bSplitscreen)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);
	}
	m_pointerControl->GetBounds(&fPointerWidth, &fPointerHeight);

	// Adjust the height to show the correct number of container rows
	float height, width;
	this->GetBounds( &width, &height );	
	int rowDiff = CONTAINER_DEFAULT_ROWS - rows;
	//height = height - (rowDiff * ROW_HEIGHT);
	height = height - (rowDiff * fPointerHeight);
	this->SetBounds( width, height );

	// Update the position after the height change so that we are still centred
	D3DXVECTOR3 vPos;
	this->GetPosition( &vPos );
	vPos.y = vPos.y + ( (rowDiff * fPointerHeight) / 2 );
	// Make sure that the y offset is even for SD modes, as the y in xui coordinates will end up being scaled by a factor of 1.5
	// to get it into actual back buffer coordinates, and we need those to remain whole numbers to avoid issues with point sampling
	if(!RenderManager.IsHiDef())
	{
		int iY = (int)(vPos.y);
		iY &= 0xfffffffe;
		vPos.y = (float)iY;
	}
	this->SetPosition( &vPos );

	InitDataAssociations(initData->iPad, menu);

	CXuiSceneAbstractContainer::Initialize( initData->iPad, menu, true, container->getContainerSize(), eSectionContainerUsing, eSectionContainerMax );

	delete initData;

	return S_OK;
}

HRESULT CXuiSceneContainer::OnDestroy()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();

#ifdef _XBOX
	if( pMinecraft->localgameModes[m_iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[m_iPad];
		if(gameMode != NULL) gameMode->getTutorial()->changeTutorialState(m_previousTutorialState);
	}
#endif

	// 4J Stu - Fix for #11302 - TCR 001: Network Connectivity: Host crashed after being killed by the client while accessing a chest during burst packet loss.
	// We need to make sure that we call closeContainer() anytime this menu is closed, even if it is forced to close by some other reason (like the player dying)	
	if(Minecraft::GetInstance()->localplayers[m_iPad] != NULL) Minecraft::GetInstance()->localplayers[m_iPad]->closeContainer();
	return S_OK;
}

CXuiControl* CXuiSceneContainer::GetSectionControl( ESceneSection eSection )
{
	switch( eSection )
	{
		case eSectionContainerChest:
			return (CXuiControl *)m_containerControl;
			break;
		case eSectionContainerInventory:
			return (CXuiControl *)m_inventoryControl;
			break;
		case eSectionContainerUsing:
			return (CXuiControl *)m_useRowControl;
			break;
		default:
			assert( false );
			break;
	}
	return NULL;
}

CXuiCtrlSlotList* CXuiSceneContainer::GetSectionSlotList( ESceneSection eSection )
{
	switch( eSection )
	{
		case eSectionContainerChest:
			return m_containerControl;
			break;
		case eSectionContainerInventory:
			return m_inventoryControl;
			break;
		case eSectionContainerUsing:
			return m_useRowControl;
			break;
		default:
			assert( false );
			break;
	}
	return NULL;
}

// 4J Stu - Added to support auto-save. Need to re-associate on a navigate back
void CXuiSceneContainer::InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex /*= 0*/)
{
	int containerSize = menu->getSize() - (27 + 9);
	int rows = containerSize / 9;

	// TODO Inventory dimensions need defined as constants
	m_containerControl->SetData( iPad, menu, rows, 9, 0 );
	
	CXuiSceneAbstractContainer::InitDataAssociations(iPad, menu, containerSize);
}

