#pragma once
using namespace std;
#include "..\Media\xuiscene_brewingstand.h"
#include "XUI_Scene_AbstractContainer.h"
#include "..\UI\IUIScene_BrewingMenu.h"

class CXuiCtrlSlotList;
class CXuiCtrlBrewProgress;
class CXuiCtrlBubblesProgress;

//--------------------------------------------------------------------------------------
// Scene implementation class.
//--------------------------------------------------------------------------------------
class CXuiSceneBrewingStand : public CXuiSceneAbstractContainer, public IUIScene_BrewingMenu
{
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CXuiSceneBrewingStand, L"CXuiSceneBrewingStand", XUI_CLASS_SCENE )

protected:
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN( OnKeyDown )
		XUI_ON_XM_DESTROY( OnDestroy )
		XUI_ON_XM_TIMER( OnTimer )			// Poll stick input on a timer.
		XUI_ON_XM_TRANSITION_START(OnTransitionStart)

		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)

	XUI_END_MSG_MAP()

	// Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_Group, m_sceneGroup)
		BEGIN_MAP_CHILD_CONTROLS( m_sceneGroup )
			// Common to all abstract container scenes
			MAP_OVERRIDE(IDC_Inventory, m_inventoryControl)
			MAP_OVERRIDE(IDC_UseRow, m_useRowControl)
			MAP_OVERRIDE(IDC_Pointer, m_pointerControl)
			MAP_CONTROL(IDC_InventoryText,m_InventoryText)

			MAP_OVERRIDE(IDC_Ingredient, m_ingredientControl)
			MAP_OVERRIDE(IDC_Bottle1, m_bottle1Control)
			MAP_OVERRIDE(IDC_Bottle2, m_bottle2Control)
			MAP_OVERRIDE(IDC_Bottle3, m_bottle3Control)

			MAP_OVERRIDE(IDC_Progress, m_progressControl)
			MAP_OVERRIDE(IDC_Bubbles, m_bubbleProgress)
			MAP_CONTROL(IDC_BrewingStandText,m_BrewingStandText)
		END_MAP_CHILD_CONTROLS()
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnDestroy();
//	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);

	virtual void InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex = 0);

private:
	CXuiCtrlSlotList *m_ingredientControl;
	CXuiCtrlSlotList *m_bottle1Control;
	CXuiCtrlSlotList *m_bottle2Control;
	CXuiCtrlSlotList *m_bottle3Control;

	CXuiCtrlBrewProgress *m_progressControl;
	CXuiCtrlBubblesProgress *m_bubbleProgress;
	CXuiControl m_BrewingStandText;

	CXuiControl m_sceneGroup;
	
	virtual CXuiControl*	GetSectionControl( ESceneSection eSection );
	virtual CXuiCtrlSlotList*	GetSectionSlotList( ESceneSection eSection );
};