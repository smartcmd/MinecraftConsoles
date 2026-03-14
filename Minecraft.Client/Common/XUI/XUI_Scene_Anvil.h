#pragma once
using namespace std;
#include "../Media/xuiscene_anvil.h"
#include "XUI_Scene_AbstractContainer.h"
#include "../UI/IUIScene_AnvilMenu.h"
#include "Common/XUI/XUI_Ctrl_4JEdit.h"

#define ANVIL_UPDATE_TIMER_ID (10)
#define ANVIL_UPDATE_TIMER_TIME (1000) // 1 second

//--------------------------------------------------------------------------------------
// Scene implementation class.
//--------------------------------------------------------------------------------------
class CXuiSceneAnvil : public CXuiSceneAbstractContainer, public IUIScene_AnvilMenu
{
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CXuiSceneAnvil, L"CXuiSceneAnvil", XUI_CLASS_SCENE )

protected:
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN( OnKeyDown )
		XUI_ON_XM_DESTROY( OnDestroy )
		XUI_ON_XM_TIMER( OnTimer )			// Poll stick input on a timer.
		XUI_ON_XM_TRANSITION_START(OnTransitionStart)
		XUI_ON_XM_NOTIFY_VALUE_CHANGED(OnNotifyValueChanged)
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

			MAP_OVERRIDE(IDC_Ingredient, m_ingredient1Control)
			MAP_OVERRIDE(IDC_Ingredient2, m_ingredient2Control)
			MAP_OVERRIDE(IDC_Result, m_resultControl)

			MAP_CONTROL(IDC_AnvilTextInput, m_editName)

			MAP_CONTROL(IDC_AnvilText,m_anvilText)
			MAP_CONTROL(IDC_LabelAffordable,m_affordableText)
			MAP_CONTROL(IDC_LabelExpensive,m_expensiveText)
			MAP_CONTROL(IDC_AnvilCross,m_cross)
		END_MAP_CHILD_CONTROLS()
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnDestroy();
	HRESULT OnNotifyValueChanged (HXUIOBJ hObjSource, XUINotifyValueChanged* pValueChangedData, BOOL& rfHandled);
//	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);
	
	virtual HRESULT handleCustomTimer( XUIMessageTimer *pTimer, BOOL& bHandled );

	virtual void InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex = 0);

private:
	CXuiCtrlSlotList *m_ingredient1Control;
	CXuiCtrlSlotList *m_ingredient2Control;
	CXuiCtrlSlotList *m_resultControl;

	CXuiCtrl4JEdit m_editName;

	CXuiControl m_anvilText;
	CXuiControl m_affordableText;
	CXuiControl m_expensiveText;
	CXuiControl m_cross;
	CXuiControl m_sceneGroup;
	
	virtual CXuiControl*	GetSectionControl( ESceneSection eSection );
	virtual CXuiCtrlSlotList*	GetSectionSlotList( ESceneSection eSection );

protected:
	virtual void handleEditNamePressed();
	virtual void setEditNameValue(const wstring &name);
	virtual void setEditNameEditable(bool enabled);
	virtual void setCostLabel(const wstring &label, bool canAfford);
	virtual void showCross(bool show);
};