#pragma once
using namespace std;

#include "../media/xuiscene_craftingpanel_2x2.h"
#include "XUI_Ctrl_MinecraftSlot.h"
#include "../../../Minecraft.World/Recipy.h" 
#include "XUI_Ctrl_CraftIngredientSlot.h"
#include "../../../Minecraft.World/Item.h"
#include "XUI_CustomMessages.h"
#include "../../Common/Tutorial/TutorialEnum.h"
#include "../UI/IUIScene_CraftingMenu.h"

class CXuiCtrlSlotList;

class CXuiSceneCraftingPanel : public CXuiSceneImpl, public IUIScene_CraftingMenu
{
public:
	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CXuiSceneCraftingPanel, L"CXuiSceneCraftingPanel", XUI_CLASS_SCENE )

	protected:
	// Control and Element wrapper objects.

	CXuiImageElement m_hPanel;
	CXuiImageElement m_hHighlight;
	CXuiImageElement m_hScrollBar3;
	CXuiImageElement m_hScrollBar2;
	CXuiControl m_GroupName;
	CXuiHtmlControl m_DescriptionText;
	CXuiControl m_ItemName,m_InventoryText;
	CXuiElement m_Group;
	CXuiElement m_hGrid;
	CXuiElement m_hGridInventory;
	CXuiImageElement m_hTabGroupA[m_iMaxGroup3x3];
	CXuiControl m_hGroupIconA[m_iMaxGroup3x3];
	CXuiControl m_pHSlotsCraftingCursors[m_iMaxHCraftingSlotC];
	CXuiControl m_pHSlotsCraftingTableCursors[m_iMaxHSlotC];
	CXuiControl *m_pCursors;
	CXuiControl m_hCraftIngredientA[m_iIngredients3x3SlotC];
	CXuiControl m_hCraftIngredientDescA[4]; // Max ingredients is 4 for bread
	CXuiControl m_hCraftOutput;
	CXuiControl m_sceneGroup;

	CXuiCtrlSlotList* m_inventoryControl;
	CXuiCtrlSlotList* m_useRowControl;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_TRANSITION_END( OnTransitionEnd)
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceImage)
		XUI_ON_XM_DESTROY( OnDestroy )
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_KILL_FOCUS( OnKillFocus)
		XUI_ON_XM_INVENTORYUPDATED_MESSAGE( OnCustomMessage_InventoryUpdated )
	XUI_END_MSG_MAP()

		   // Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_Group, m_sceneGroup)
		BEGIN_MAP_CHILD_CONTROLS(m_sceneGroup)
			MAP_CONTROL(IDC_MainPanel,m_hPanel)
			MAP_CONTROL(IDC_XuiGroupName,m_GroupName)

			MAP_CONTROL(IDC_SceneCraftScrollGroup, m_Group)
			BEGIN_MAP_CHILD_CONTROLS(m_Group)
				MAP_CONTROL(IDC_XuiHighlight,m_hHighlight)
				MAP_CONTROL(IDC_XuiImageScrollBar,m_hScrollBar3)
				MAP_CONTROL(IDC_XuiImageScrollBar2Slot,m_hScrollBar2)
				MAP_OVERRIDE(IDC_XuiHSlot0, m_pHSlotsBrushImageControl[0])
				MAP_OVERRIDE(IDC_XuiHSlot1, m_pHSlotsBrushImageControl[1])
				MAP_OVERRIDE(IDC_XuiHSlot2, m_pHSlotsBrushImageControl[2])
				MAP_OVERRIDE(IDC_XuiHSlot3, m_pHSlotsBrushImageControl[3])
				MAP_OVERRIDE(IDC_XuiHSlot4, m_pHSlotsBrushImageControl[4])
				MAP_OVERRIDE(IDC_XuiHSlot5, m_pHSlotsBrushImageControl[5])
				MAP_OVERRIDE(IDC_XuiHSlot6, m_pHSlotsBrushImageControl[6])
				MAP_OVERRIDE(IDC_XuiHSlot7, m_pHSlotsBrushImageControl[7])
				MAP_OVERRIDE(IDC_XuiHSlot8, m_pHSlotsBrushImageControl[8])
				MAP_OVERRIDE(IDC_XuiHSlot9, m_pHSlotsBrushImageControl[9])
				MAP_OVERRIDE(IDC_XuiHSlot10, m_pHSlotsBrushImageControl[10])
				MAP_OVERRIDE(IDC_XuiHSlot11, m_pHSlotsBrushImageControl[11])


				MAP_OVERRIDE(IDC_XuiVSlot0, m_pVSlotsBrushImageControl[0])
				MAP_OVERRIDE(IDC_XuiVSlot1, m_pVSlotsBrushImageControl[1])
				MAP_OVERRIDE(IDC_XuiVSlot2, m_pVSlotsBrushImageControl[2])

			END_MAP_CHILD_CONTROLS()

			MAP_CONTROL(IDC_Group_Tab_Icons, m_Group)
			BEGIN_MAP_CHILD_CONTROLS(m_Group)
				MAP_CONTROL(IDC_Icon_1,m_hGroupIconA[0])
				MAP_CONTROL(IDC_Icon_2,m_hGroupIconA[1])
				MAP_CONTROL(IDC_Icon_3,m_hGroupIconA[2])
				MAP_CONTROL(IDC_Icon_4,m_hGroupIconA[3])
				MAP_CONTROL(IDC_Icon_5,m_hGroupIconA[4])
				MAP_CONTROL(IDC_Icon_6,m_hGroupIconA[5])
				MAP_CONTROL(IDC_Icon_7,m_hGroupIconA[6])

			END_MAP_CHILD_CONTROLS()


			MAP_CONTROL(IDC_Group_Tab_Images, m_Group)
			BEGIN_MAP_CHILD_CONTROLS(m_Group)
				MAP_CONTROL(IDC_TabImage1,m_hTabGroupA[0])
				MAP_CONTROL(IDC_TabImage2,m_hTabGroupA[1])
				MAP_CONTROL(IDC_TabImage3,m_hTabGroupA[2])
				MAP_CONTROL(IDC_TabImage4,m_hTabGroupA[3])
				MAP_CONTROL(IDC_TabImage5,m_hTabGroupA[4])
				MAP_CONTROL(IDC_TabImage6,m_hTabGroupA[5])
				MAP_CONTROL(IDC_TabImage7,m_hTabGroupA[6])

			END_MAP_CHILD_CONTROLS()


			MAP_CONTROL(IDC_Grid, m_hGrid)
			BEGIN_MAP_CHILD_CONTROLS(m_hGrid)
				MAP_CONTROL(IDC_XuiHTMLText,m_DescriptionText)
				MAP_CONTROL(IDC_Inventory,m_InventoryText)
				MAP_CONTROL(IDC_XuiItemName,m_ItemName)
				MAP_CONTROL(IDC_CraftingInput1,m_hCraftIngredientA[0])
				MAP_CONTROL(IDC_CraftingInput2,m_hCraftIngredientA[1])
				MAP_CONTROL(IDC_CraftingInput3,m_hCraftIngredientA[2])
				MAP_CONTROL(IDC_CraftingInput4,m_hCraftIngredientA[3])
				MAP_CONTROL(IDC_CraftingInput5,m_hCraftIngredientA[4])
				MAP_CONTROL(IDC_CraftingInput6,m_hCraftIngredientA[5])
				MAP_CONTROL(IDC_CraftingInput7,m_hCraftIngredientA[6])
				MAP_CONTROL(IDC_CraftingInput8,m_hCraftIngredientA[7])
				MAP_CONTROL(IDC_CraftingInput9,m_hCraftIngredientA[8])
				MAP_CONTROL(IDC_Ingredient1,m_hCraftIngredientDescA[0])
				MAP_CONTROL(IDC_Ingredient2,m_hCraftIngredientDescA[1])
				MAP_CONTROL(IDC_Ingredient3,m_hCraftIngredientDescA[2])
				MAP_CONTROL(IDC_Ingredient4,m_hCraftIngredientDescA[3])

				MAP_CONTROL(IDC_CraftingOutputRed,m_hCraftOutput)
			END_MAP_CHILD_CONTROLS()
	
			MAP_CONTROL(IDC_InventoryGrid, m_hGridInventory)
			BEGIN_MAP_CHILD_CONTROLS(m_hGridInventory)
				MAP_OVERRIDE(IDC_Inventory, m_inventoryControl)
				MAP_OVERRIDE(IDC_UseRow, m_useRowControl)
			END_MAP_CHILD_CONTROLS()
		END_MAP_CHILD_CONTROLS()
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnTransitionEnd( XUIMessageTransition *pTransData, BOOL& bHandled);
	HRESULT OnGetSourceImage(XUIMessageGetSourceImage* pData, BOOL& rfHandled);
	HRESULT OnDestroy();
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);
	HRESULT OnTimer(XUIMessageTimer *pData,BOOL& rfHandled);
	HRESULT OnKillFocus(HXUIOBJ hObjGettingFocus, BOOL& bHandled);
	HRESULT OnCustomMessage_InventoryUpdated();

	CXuiSceneCraftingPanel();
	~CXuiSceneCraftingPanel() { }

private:
	float m_fSlotSize;
	D3DXVECTOR3  m_vSlot0Pos;
	D3DXVECTOR3  m_vSlot0HighlightPos;
	D3DXVECTOR3  m_vSlot0V2ScrollPos;
	D3DXVECTOR3  m_vSlot0V3ScrollPos;

	CXuiCtrlCraftIngredientSlot *m_pCraftingOutput;
	CXuiCtrlCraftIngredientSlot *m_pCraftingIngredientA[m_iIngredients3x3SlotC];
	CXuiCtrlCraftIngredientSlot *m_pCraftIngredientDescA[4];
	CXuiCtrlMinecraftSlot *m_pHSlotsBrushImageControl[m_iMaxHSlotC];
	CXuiCtrlMinecraftSlot *m_pVSlotsBrushImageControl[m_iMaxDisplayedVSlotC];
	CXuiControl *GroupTypeIconA[Recipy::eGroupType_Max];
	int m_iPad;
	D3DXVECTOR3 m_OriginalPosition;

	int mapVKToAction(int vk);

protected:
	virtual int getPad();
	virtual void hideAllHSlots();
	virtual void hideAllVSlots();
	virtual void hideAllIngredientsSlots();
	virtual void setCraftHSlotItem(int iPad, int iIndex, shared_ptr<ItemInstance> item, unsigned int uiAlpha);
	virtual void setCraftVSlotItem(int iPad, int iIndex, shared_ptr<ItemInstance> item, unsigned int uiAlpha);
	virtual void setCraftingOutputSlotItem(int iPad, shared_ptr<ItemInstance> item);
	virtual void setCraftingOutputSlotRedBox(bool show);
	virtual void setIngredientSlotItem(int iPad, int index, shared_ptr<ItemInstance> item);
	virtual void setIngredientSlotRedBox(int index, bool show);
	virtual void setIngredientDescriptionItem(int iPad, int index, shared_ptr<ItemInstance> item);
	virtual void setIngredientDescriptionRedBox(int index, bool show);
	virtual void setIngredientDescriptionText(int index, LPCWSTR text);
	virtual void setShowCraftHSlot(int iIndex, bool show);
	virtual void showTabHighlight(int iIndex, bool show);
	virtual void setGroupText(LPCWSTR text);
	virtual void setDescriptionText(LPCWSTR text);
	virtual void setItemText(LPCWSTR text);
	virtual void scrollDescriptionUp();
	virtual void scrollDescriptionDown();
	virtual void updateHighlightAndScrollPositions();
	virtual void updateVSlotPositions(int iSlots, int i);
	
	virtual void UpdateMultiPanel();
};
