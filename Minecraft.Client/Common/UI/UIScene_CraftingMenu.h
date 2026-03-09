#pragma once

#include "UIScene.h"
#include "UIControl_SlotList.h"
#include "UIControl_Label.h"
#include "IUIScene_CraftingMenu.h"

#define CRAFTING_INVENTORY_SLOT_START 0
#define CRAFTING_INVENTORY_SLOT_END (CRAFTING_INVENTORY_SLOT_START + 27)

#define CRAFTING_HOTBAR_SLOT_START CRAFTING_INVENTORY_SLOT_END
#define CRAFTING_HOTBAR_SLOT_END (CRAFTING_HOTBAR_SLOT_START + 9)

// Ingredients etc should go here
#define CRAFTING_INGREDIENTS_DESCRIPTION_START CRAFTING_HOTBAR_SLOT_END
#define CRAFTING_INGREDEINTS_DESCRIPTION_END (CRAFTING_INGREDIENTS_DESCRIPTION_START + 4)

#define CRAFTING_OUTPUT_SLOT_START CRAFTING_INGREDEINTS_DESCRIPTION_END
#define CRAFTING_OUTPUT_SLOT_END (CRAFTING_OUTPUT_SLOT_START + 1)

#define CRAFTING_INGREDIENTS_LAYOUT_START CRAFTING_OUTPUT_SLOT_END
#define CRAFTING_INGREDIENTS_LAYOUT_END (CRAFTING_INGREDIENTS_LAYOUT_START+9)

#define CRAFTING_V_SLOT_START CRAFTING_INGREDIENTS_LAYOUT_END
#define CRAFTING_V_SLOT_END (CRAFTING_V_SLOT_START+3)

// H slots should go last in the count as it's dependent on which size of crafting panel we have
#define CRAFTING_H_SLOT_START CRAFTING_V_SLOT_END

class UIScene_CraftingMenu : public UIScene, public IUIScene_CraftingMenu
{
private:
	typedef struct _SlotInfo
	{
		shared_ptr<ItemInstance> item;
		unsigned int alpha;
		bool show;

		_SlotInfo()
		{
			item = nullptr;
			alpha = 31;
			show = true;
		}
	} SlotInfo;

	SlotInfo m_hSlotsInfo[m_iMaxHSlotC];
	SlotInfo m_vSlotsInfo[m_iMaxDisplayedVSlotC];
	SlotInfo m_ingredientsSlotsInfo[m_iIngredients3x3SlotC];
	SlotInfo m_craftingOutputSlotInfo;
	SlotInfo m_ingredientsInfo[4];

	AbstractContainerMenu *m_menu;

	int m_iMenuInventoryStart;
	int m_iMenuHotBarStart;

public:
	UIScene_CraftingMenu(int iPad, void *initData, UILayer *parentLayer);

	virtual void handleDestroy();

	virtual EUIScene getSceneType();

	virtual void customDraw(IggyCustomDrawCallbackRegion *region);

#ifdef __PSVITA__
	virtual void handleTouchInput(unsigned int iPad, S32 x, S32 y, int iId, bool bPressed, bool bRepeat, bool bReleased);
	virtual void handleTouchBoxRebuild();
	virtual void handleTimerComplete(int id);
#endif
#if defined(__PSVITA__) || defined(_WINDOWS64)
	virtual UIControl* GetMainPanel();
#endif
#ifdef _WINDOWS64
	virtual bool handleMouseClick(F32 x, F32 y);
	// Cached from customDraw — H slot bounding boxes in SWF space
	F32 m_hSlotX0, m_hSlotY0, m_hSlotY1;
	F32 m_hSlotSpacing; // x distance between slot 0 and slot 1
	bool m_hSlotBoundsValid;
#endif

protected:
	UIControl m_controlMainPanel;
	UIControl m_control1Selector, m_control2Selector, m_control3Selector;
	UIControl_SlotList m_slotListCraftingHSlots;
	UIControl_SlotList m_slotListCrafting1VSlots, m_slotListCrafting2VSlots[2], m_slotListCrafting3VSlots[3];
	UIControl_SlotList m_slotListIngredientsLayout, m_slotListCraftingOutput;
	UIControl_SlotList m_slotListIngredients[4];
	UIControl_SlotList m_slotListInventory, m_slotListHotBar;
	UIControl_Label m_labelIngredientsDesc[4];
	UIControl_HTMLLabel m_labelDescription;
	UIControl_Label m_labelGroupName, m_labelItemName, m_labelInventory, m_labelIngredients;

	IggyName m_funcMoveSelector, m_funcSelectVerticalItem, m_funcSetActiveTab;
	IggyName m_funcShowPanelDisplay, m_funcShowIngredientSlot;

#ifdef __PSVITA__
	enum ETouchInput
	{
		ETouchInput_TouchPanel_0,
		ETouchInput_TouchPanel_1,
		ETouchInput_TouchPanel_2,
		ETouchInput_TouchPanel_3,
		ETouchInput_TouchPanel_4,
		ETouchInput_TouchPanel_5,
		ETouchInput_TouchPanel_6,
		ETouchInput_CraftingHSlots,

		ETouchInput_Count,
	};
	UIControl_Touch m_TouchInput[ETouchInput_Count];
	S32 m_iCraftingSlotTouchStartY;
#endif

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_controlMainPanel, "MainPanel" )
		UI_BEGIN_MAP_CHILD_ELEMENTS( m_controlMainPanel )
			UI_MAP_ELEMENT( m_slotListCraftingHSlots, "CraftingHSlots")

			UI_MAP_ELEMENT( m_control3Selector, "SlotSelector3" )
			UI_BEGIN_MAP_CHILD_ELEMENTS( m_control3Selector)
				UI_MAP_ELEMENT( m_slotListCrafting3VSlots[0], "Crafting3VSlot1")
				UI_MAP_ELEMENT( m_slotListCrafting3VSlots[1], "Crafting3VSlot2")
				UI_MAP_ELEMENT( m_slotListCrafting3VSlots[2], "Crafting3VSlot3")
			UI_END_MAP_CHILD_ELEMENTS()

			UI_MAP_ELEMENT( m_control2Selector, "SlotSelector2" )
			UI_BEGIN_MAP_CHILD_ELEMENTS( m_control2Selector)
				UI_MAP_ELEMENT( m_slotListCrafting2VSlots[0], "Crafting2VSlot1")
				UI_MAP_ELEMENT( m_slotListCrafting2VSlots[1], "Crafting2VSlot2")
			UI_END_MAP_CHILD_ELEMENTS()

			UI_MAP_ELEMENT( m_control1Selector, "CraftingSelector" )
			UI_BEGIN_MAP_CHILD_ELEMENTS( m_control1Selector)
				UI_MAP_ELEMENT( m_slotListCrafting1VSlots, "Crafting1VSlot1")
			UI_END_MAP_CHILD_ELEMENTS()

			UI_MAP_ELEMENT( m_slotListIngredientsLayout, "IngredientsLayout")
			UI_MAP_ELEMENT( m_slotListCraftingOutput, "CraftingOutput")

			UI_MAP_ELEMENT( m_slotListIngredients[0], "Ingredient1")
			UI_MAP_ELEMENT( m_slotListIngredients[1], "Ingredient2")
			UI_MAP_ELEMENT( m_slotListIngredients[2], "Ingredient3")
			UI_MAP_ELEMENT( m_slotListIngredients[3], "Ingredient4")

			UI_MAP_ELEMENT( m_labelIngredientsDesc[0], "Ingredient1Desc")
			UI_MAP_ELEMENT( m_labelIngredientsDesc[1], "Ingredient2Desc")
			UI_MAP_ELEMENT( m_labelIngredientsDesc[2], "Ingredient3Desc")
			UI_MAP_ELEMENT( m_labelIngredientsDesc[3], "Ingredient4Desc")

			UI_MAP_ELEMENT( m_labelIngredients, "IngredientsLabel")

			UI_MAP_ELEMENT( m_labelDescription, "DescriptionText")

			UI_MAP_ELEMENT( m_slotListInventory, "Inventory")
			UI_MAP_ELEMENT( m_slotListHotBar, "HotBar")

			UI_MAP_ELEMENT( m_labelGroupName, "GroupName")
			UI_MAP_ELEMENT( m_labelItemName, "ItemName")
			UI_MAP_ELEMENT( m_labelInventory, "InventoryLabel")

			UI_MAP_NAME( m_funcMoveSelector, L"MoveSelector")
			UI_MAP_NAME( m_funcSelectVerticalItem, L"SelectVerticalItem")
			UI_MAP_NAME( m_funcSetActiveTab, L"SetActiveTab")
			UI_MAP_NAME( m_funcShowPanelDisplay, L"showPanelDisplay")
			UI_MAP_NAME( m_funcShowIngredientSlot, L"ShowIngredient")

#ifdef __PSVITA__
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_0], "TouchPanel_0" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_1], "TouchPanel_1" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_2], "TouchPanel_2" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_3], "TouchPanel_3" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_4], "TouchPanel_4" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_5], "TouchPanel_5" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_6], "TouchPanel_6" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_CraftingHSlots], "TouchPanel_CraftingHSlots" )
#endif

		UI_END_MAP_CHILD_ELEMENTS()
	UI_END_MAP_ELEMENTS_AND_NAMES()

	virtual wstring getMoviePath();
	virtual void handleReload();

	virtual bool allowRepeat(int key);
	void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

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
	
	virtual void HandleMessage(EUIMessage message, void *data);
	void handleInventoryUpdated(LPVOID data);

	// 4J - TomK If update tooltips is called then make sure the correct parent is invoked! (both UIScene AND IUIScene_CraftingMenu have an instance of said function!)
	virtual void updateTooltips() { IUIScene_CraftingMenu::UpdateTooltips(); }
};
