#pragma once
using namespace std;
#include "../Media/xuiscene_inventory.h"
#include "XUI_Scene_AbstractContainer.h"
#include "XUI_CustomMessages.h"
#include "../UI/IUIScene_InventoryMenu.h"

#define INVENTORY_UPDATE_EFFECTS_TIMER_ID (10)
#define INVENTORY_UPDATE_EFFECTS_TIMER_TIME (1000) // 1 second

class CXuiCtrlMobEffect;

//--------------------------------------------------------------------------------------
// Scene implementation class.
//--------------------------------------------------------------------------------------
class CXuiSceneInventory : public CXuiSceneAbstractContainer, public IUIScene_InventoryMenu
{
private:
	static const int MAX_EFFECTS = 10;
public:

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CXuiSceneInventory, L"CXuiSceneInventory", XUI_CLASS_SCENE )

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
		MAP_CONTROL(IDC_Group, m_sceneGroup)
		BEGIN_MAP_CHILD_CONTROLS( m_sceneGroup )
			// Common to all abstract container scenes
			MAP_OVERRIDE(IDC_Inventory, m_inventoryControl)
			MAP_OVERRIDE(IDC_UseRow, m_useRowControl)
			MAP_OVERRIDE(IDC_Pointer, m_pointerControl)
			MAP_CONTROL(IDC_InventoryText,m_InventoryText)

			MAP_OVERRIDE(IDC_Armor, m_armorGroup)
		END_MAP_CHILD_CONTROLS()

		MAP_CONTROL(IDC_EffectsGroup, m_effectsGroup)
		BEGIN_MAP_CHILD_CONTROLS( m_effectsGroup )
			MAP_OVERRIDE(IDC_Effect1, m_hEffectDisplayA[0])
			MAP_OVERRIDE(IDC_Effect2, m_hEffectDisplayA[1])
			MAP_OVERRIDE(IDC_Effect3, m_hEffectDisplayA[2])
			MAP_OVERRIDE(IDC_Effect4, m_hEffectDisplayA[3])
			MAP_OVERRIDE(IDC_Effect5, m_hEffectDisplayA[4])
			MAP_OVERRIDE(IDC_Effect6, m_hEffectDisplayA[5])
			MAP_OVERRIDE(IDC_Effect7, m_hEffectDisplayA[6])
			MAP_OVERRIDE(IDC_Effect8, m_hEffectDisplayA[7])
			MAP_OVERRIDE(IDC_Effect9, m_hEffectDisplayA[8])
			MAP_OVERRIDE(IDC_Effect10, m_hEffectDisplayA[9])
		END_MAP_CHILD_CONTROLS()
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnDestroy();
	//HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled) {return S_OK;}

	virtual void InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex = 0);

	virtual HRESULT handleCustomTimer( XUIMessageTimer *pTimer, BOOL& bHandled );

private:
	float m_effectDisplayHeight;
	float m_effectDisplaySpacing;
	float m_effectAreaHeight;
	
	CXuiCtrlSlotList* m_armorGroup;

	CXuiControl m_sceneGroup;

	CXuiControl m_effectsGroup;
	CXuiCtrlMobEffect *m_hEffectDisplayA[MAX_EFFECTS];

	virtual CXuiControl*	GetSectionControl( ESceneSection eSection );
	virtual CXuiCtrlSlotList*	GetSectionSlotList( ESceneSection eSection );

	void updateEffectsDisplay();
};