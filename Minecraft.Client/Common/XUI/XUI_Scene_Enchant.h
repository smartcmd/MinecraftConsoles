#pragma once
using namespace std;

#include "../Media/xuiscene_enchant.h"

#include "../../BookModel.h"

#include "XUI_Ctrl_SlotList.h"
#include "XUI_Ctrl_EnchantmentBook.h"
#include "XUI_Scene_AbstractContainer.h"
#include "../UI/IUIScene_EnchantingMenu.h"
#include "XUI_CustomMessages.h"

#include "XUI_Scene_Enchant.h"

#include "../../../Minecraft.World/AbstractContainerMenu.h"
#include "../../../Minecraft.World/SimpleContainer.h"

class Level;
class CXuiCtrlEnchantmentButton;
class EnchantmentMenu;

//--------------------------------------------------------------------------------------
// Scene implementation class.
//--------------------------------------------------------------------------------------
class CXuiSceneEnchant : public CXuiSceneAbstractContainer, public IUIScene_EnchantingMenu
{
	friend class CXuiCtrlEnchantmentButtonText;
public:
    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CXuiSceneEnchant, L"CXuiSceneEnchant", XUI_CLASS_SCENE )

protected:
	CXuiCtrlEnchantmentButton *m_enchant1;
	CXuiCtrlEnchantmentButton *m_enchant2;
	CXuiCtrlEnchantmentButton *m_enchant3;
	CXuiControl m_sceneGroup;
	CXuiCtrlSlotList* m_ingredientControl;
	CXuiControl m_EnchantText;

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
			
			MAP_OVERRIDE(IDC_Ingredient, m_ingredientControl)
			MAP_OVERRIDE(IDC_EnchantButton1, m_enchant1)
			MAP_OVERRIDE(IDC_EnchantButton2, m_enchant2)
			MAP_OVERRIDE(IDC_EnchantButton3, m_enchant3)
			MAP_CONTROL(IDC_EnchantText,m_EnchantText)
		END_MAP_CHILD_CONTROLS()
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnDestroy();
	
	virtual void InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex = 0);

private:

	virtual CXuiControl*	GetSectionControl( ESceneSection eSection );
	virtual CXuiCtrlSlotList*	GetSectionSlotList( ESceneSection eSection );
};