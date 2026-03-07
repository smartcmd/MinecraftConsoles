#pragma once
#include "../Media/xuiscene_container.h"
#include "XUI_Scene_AbstractContainer.h"
#include "XUI_CustomMessages.h"
#include "../UI/IUIScene_ContainerMenu.h"

//--------------------------------------------------------------------------------------
// Scene implementation class.
//--------------------------------------------------------------------------------------
class CXuiSceneContainer : public CXuiSceneAbstractContainer, public IUIScene_ContainerMenu
{
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CXuiSceneContainer, L"CXuiSceneContainer", XUI_CLASS_SCENE )

protected:
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN( OnKeyDown )
		XUI_ON_XM_DESTROY( OnDestroy )
		XUI_ON_XM_TIMER( OnTimer )			// Poll stick input on a timer.
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
		XUI_ON_XM_TRANSITION_START(OnTransitionStart)

	XUI_END_MSG_MAP()

	// Control mapping to objects
	BEGIN_CONTROL_MAP()
		// Common to all abstract container scenes
		MAP_CONTROL(IDC_Group, m_sceneGroup)
		BEGIN_MAP_CHILD_CONTROLS( m_sceneGroup )
		MAP_OVERRIDE(IDC_Inventory, m_inventoryControl)
		MAP_OVERRIDE(IDC_UseRow, m_useRowControl)
		MAP_OVERRIDE(IDC_Pointer, m_pointerControl)
		MAP_CONTROL(IDC_InventoryText,m_InventoryText)

		MAP_OVERRIDE(IDC_Container, m_containerControl)
		MAP_CONTROL(IDC_ChestText,m_ChestText)
		END_MAP_CHILD_CONTROLS()
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnDestroy();
	//HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled) { return S_OK;}
	
	virtual void InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex = 0);

private:
	CXuiCtrlSlotList* m_containerControl;
	CXuiControl m_sceneGroup;
	CXuiControl m_ChestText;

	virtual CXuiControl*	GetSectionControl( ESceneSection eSection );
	virtual CXuiCtrlSlotList*	GetSectionSlotList( ESceneSection eSection );
};
