#pragma once
using namespace std;
#include "..\Media\xuiscene_trap.h"
#include "XUI_Scene_AbstractContainer.h"
#include "..\UI\IUIScene_DispenserMenu.h"
#include "XUI_CustomMessages.h"

class Container;
class DispenserTileEntity;

//--------------------------------------------------------------------------------------
// Scene implementation class.
//--------------------------------------------------------------------------------------
class CXuiSceneTrap : public CXuiSceneAbstractContainer, public IUIScene_DispenserMenu
{
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CXuiSceneTrap, L"CXuiSceneTrap", XUI_CLASS_SCENE )

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
		
		MAP_CONTROL(IDC_DispenserText,m_DispenserText)
		MAP_OVERRIDE(IDC_Trap, m_trapControl)
		END_MAP_CHILD_CONTROLS()
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnDestroy();
	//HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled) {return S_OK;}

	virtual void InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex = 0);
private:
	CXuiCtrlSlotList* m_trapControl;
	CXuiControl m_sceneGroup;
	CXuiControl m_DispenserText;

	virtual CXuiControl*	GetSectionControl( ESceneSection eSection );
	virtual CXuiCtrlSlotList*	GetSectionSlotList( ESceneSection eSection );

};