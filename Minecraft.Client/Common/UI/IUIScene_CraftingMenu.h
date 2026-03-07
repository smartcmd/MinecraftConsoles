#pragma once
#include "../../../Minecraft.World/Recipy.h"
#include "../../../Minecraft.World/Item.h"

class LocalPlayer;

// 4J Stu - Crafting menu code that's shared across Iggy and XUI
class IUIScene_CraftingMenu
{
protected:
#define DISPLAY_INVENTORY 0
#define DISPLAY_DESCRIPTION 1
#define DISPLAY_INGREDIENTS 2
#define DISPLAY_MAX 3

	enum _eGroupTab
	{
		eGroupTab_Left,
		eGroupTab_Middle,
		eGroupTab_Right
	};

	static const int m_iMaxHSlotC = 12;
	static const int m_iMaxHCraftingSlotC = 10;
	static const int m_iMaxVSlotC = 17; 
	static const int m_iMaxDisplayedVSlotC = 3;
	static const int m_iIngredients3x3SlotC = 9;
	static const int m_iIngredients2x2SlotC = 4;

	static const int m_iMaxHSlot3x3C = 12;
	static const int m_iMaxHSlot2x2C = 10;

	static const int m_iMaxGroup3x3 = 7;
	static const int m_iMaxGroup2x2 = 6;

	static int m_iBaseTypeMapA[Item::eBaseItemType_MAXTYPES];

	typedef struct  
	{
		int iCount;
		int iItemBaseType;
		int iRecipeA[m_iMaxVSlotC]; // tiers of item that can be made
	}
	CANBEMADE;

	CANBEMADE CanBeMadeA[m_iMaxHSlotC];

	int m_iCurrentSlotHIndex;
	int m_iCurrentSlotVIndex;
	int m_iRecipeC;
	int m_iContainerType; // 2x2 or 3x3
	shared_ptr<LocalPlayer> m_pPlayer;
	int m_iGroupIndex;

	int iVSlotIndexA[3]; // index of the v slots currently displayed

	static LPCWSTR m_GroupIconNameA[m_iMaxGroup3x3];
	static Recipy::_eGroupType m_GroupTypeMapping4GridA[m_iMaxGroup2x2];
	static Recipy::_eGroupType m_GroupTypeMapping9GridA[m_iMaxGroup3x3];
	Recipy::_eGroupType *m_pGroupA;

	static LPCWSTR m_GroupTabNameA[3];	
	static _eGroupTab m_GroupTabBkgMapping2x2A[m_iMaxGroup2x2];
	static _eGroupTab m_GroupTabBkgMapping3x3A[m_iMaxGroup3x3];
	_eGroupTab *m_pGroupTabA;
	int m_iCraftablesMaxHSlotC;
	int m_iIngredientsMaxSlotC;
	int m_iDisplayDescription;
	int m_iIngredientsC;
	bool m_bIgnoreKeyPresses;
	bool m_bSplitscreen;

	eTutorial_State m_previousTutorialState;

	bool handleKeyDown(int iPad, int iAction, bool bRepeat);

public:
	IUIScene_CraftingMenu();

protected:
	LPCWSTR GetGroupNameText(int iGroupType);

	void CheckRecipesAvailable();
	void UpdateHighlight();
	void UpdateVerticalSlots();
	void DisplayIngredients();
	void UpdateTooltips();
	void UpdateDescriptionText(bool);
	void HandleInventoryUpdated();

public:
	Recipy::_eGroupType getCurrentGroup() { return m_pGroupA[m_iGroupIndex]; }
	bool isItemSelected(int itemId);

protected:
	virtual int getPad() = 0;
	virtual void hideAllHSlots() = 0;
	virtual void hideAllVSlots() = 0;
	virtual void hideAllIngredientsSlots() = 0;
	virtual void setCraftHSlotItem(int iPad, int iIndex, shared_ptr<ItemInstance> item, unsigned int uiAlpha) = 0;
	virtual void setCraftVSlotItem(int iPad, int iIndex, shared_ptr<ItemInstance> item, unsigned int uiAlpha) = 0;
	virtual void setCraftingOutputSlotItem(int iPad, shared_ptr<ItemInstance> item) = 0;
	virtual void setCraftingOutputSlotRedBox(bool show) = 0;
	virtual void setIngredientSlotItem(int iPad, int index, shared_ptr<ItemInstance> item) = 0;
	virtual void setIngredientSlotRedBox(int index, bool show) = 0;
	virtual void setIngredientDescriptionItem(int iPad, int index, shared_ptr<ItemInstance> item) = 0;
	virtual void setIngredientDescriptionRedBox(int index, bool show) = 0;
	virtual void setIngredientDescriptionText(int index, LPCWSTR text) = 0;
	virtual void setShowCraftHSlot(int iIndex, bool show) = 0;
	virtual void showTabHighlight(int iIndex, bool show) = 0;
	virtual void setGroupText(LPCWSTR text) = 0;
	virtual void setDescriptionText(LPCWSTR text) = 0;
	virtual void setItemText(LPCWSTR text) = 0;
	virtual void scrollDescriptionUp() = 0;
	virtual void scrollDescriptionDown() = 0;
	virtual void updateHighlightAndScrollPositions() = 0;
	virtual void updateVSlotPositions(int iSlots, int i) = 0;

	virtual void UpdateMultiPanel() = 0;
};