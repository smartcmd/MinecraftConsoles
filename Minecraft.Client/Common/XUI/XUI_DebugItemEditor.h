#pragma once
using namespace std;
#include "../media/xuiscene_debug_item_editor.h"

#include "XUI_Ctrl_CraftIngredientSlot.h"
#include "XUI_Ctrl_4JEdit.h"
#include "..\..\..\Minecraft.World\ItemInstance.h"

class CScene_DebugItemEditor : public CXuiSceneImpl
{
#ifdef _DEBUG_MENUS_ENABLED	
public:
	typedef struct _ItemEditorInput
	{
		int iPad;
		Slot *slot;
		AbstractContainerMenu *menu;
	} ItemEditorInput;
private:
	int m_iPad;
	shared_ptr<ItemInstance> m_item;
	Slot *m_slot;
	AbstractContainerMenu *m_menu;

	CXuiCtrlCraftIngredientSlot *m_icon;
	CXuiControl m_generatedXml, m_itemName;
	CXuiCtrl4JEdit m_itemId, m_itemAuxValue, m_itemCount, m_item4JData;

protected:
	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_VALUE_CHANGED( OnNotifyValueChanged )
	XUI_END_MSG_MAP()

	// Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_itemId, m_itemId)
		MAP_CONTROL(IDC_itemAuxValue, m_itemAuxValue)
		MAP_CONTROL(IDC_itemCount, m_itemCount)
		MAP_CONTROL(IDC_item4JData, m_item4JData)
		MAP_OVERRIDE(IDC_icon, m_icon)
		MAP_CONTROL(IDC_ruleXml, m_generatedXml)
		MAP_CONTROL(IDC_itemName, m_itemName)
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL &bHandled);
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_DebugItemEditor, L"CScene_DebugItemEditor", XUI_CLASS_SCENE )
#endif
};