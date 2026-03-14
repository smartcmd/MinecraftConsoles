#pragma once
#include "IUIScene_AbstractContainerMenu.h"
#include "../../../Minecraft.World/AbstractContainerMenu.h"
// 4J Stu - This class is for code that is common between XUI and Iggy

class SimpleContainer;

class IUIScene_CreativeMenu : public virtual IUIScene_AbstractContainerMenu
{
public:
	// 4J Stu - These map directly to the tabs seenon the screen
	enum ECreativeInventoryTabs
	{
		eCreativeInventoryTab_BuildingBlocks = 0,
		eCreativeInventoryTab_Decorations,
		eCreativeInventoryTab_RedstoneAndTransport,
		eCreativeInventoryTab_Materials,
		eCreativeInventoryTab_Food,
		eCreativeInventoryTab_ToolsWeaponsArmor,
		eCreativeInventoryTab_Brewing,
		eCreativeInventoryTab_Misc,
		eCreativeInventoryTab_COUNT,
	};

	// 4J Stu - These are logical groupings of items, and be be combined for tabs on-screen
	enum ECreative_Inventory_Groups
	{
		eCreativeInventory_BuildingBlocks,
		eCreativeInventory_Decoration,
		eCreativeInventory_Redstone,
		eCreativeInventory_Transport,
		eCreativeInventory_Materials,
		eCreativeInventory_Food,
		eCreativeInventory_ToolsArmourWeapons,
		eCreativeInventory_Brewing,
		eCreativeInventory_Potions_Basic,
		eCreativeInventory_Potions_Level2,
		eCreativeInventory_Potions_Extended,
		eCreativeInventory_Potions_Level2_Extended,
		eCreativeInventory_Misc,
		eCreativeInventory_ArtToolsDecorations,
		eCreativeInventory_ArtToolsMisc,
		eCreativeInventoryGroupsCount
	};

	// 4J JEV - Keeping all the tab specifications in one place.
	struct TabSpec
	{
	public:
		// 4J JEV - Layout
		static const int rows = 5;
		static const int columns = 10;
		static const int MAX_SIZE = rows * columns;

		// 4J JEV - Images
		const LPCWSTR m_icon;
		const int m_descriptionId;
		const int m_staticGroupsCount;
		ECreative_Inventory_Groups *m_staticGroupsA;
		const int m_dynamicGroupsCount;
		ECreative_Inventory_Groups *m_dynamicGroupsA;
		const int m_debugGroupsCount;
		ECreative_Inventory_Groups *m_debugGroupsA;

	private:
		unsigned int m_pages;
		unsigned int m_staticPerPage;
		unsigned int m_staticItems;
		unsigned int m_debugItems;

	public:
		TabSpec( LPCWSTR icon, int descriptionId, int staticGroupsCount, ECreative_Inventory_Groups *staticGroups, int dynamicGroupsCount = 0, ECreative_Inventory_Groups *dynamicGroups = nullptr, int debugGroupsCount = 0, ECreative_Inventory_Groups *debugGroups = nullptr );
		~TabSpec();

		void populateMenu(AbstractContainerMenu *menu, int dynamicIndex, unsigned int page);
		unsigned int getPageCount();
	};

	class ItemPickerMenu : public AbstractContainerMenu
	{
	protected:
		shared_ptr<SimpleContainer> creativeContainer;
		shared_ptr<Inventory> inventory;

	public:
		ItemPickerMenu(	shared_ptr<SimpleContainer> creativeContainer, shared_ptr<Inventory> inventory );

		virtual bool stillValid(shared_ptr<Player> player);
		bool isOverrideResultClick(int slotNum, int buttonNum);
	protected:
		// 4J Stu - Brought forward from 1.2 to fix infinite recursion bug in creative
		virtual void loopClick(int slotIndex, int buttonNum, bool quickKeyHeld, shared_ptr<Player> player) { } // do nothing
	} *itemPickerMenu;

protected:
	static vector< shared_ptr<ItemInstance> > categoryGroups[eCreativeInventoryGroupsCount];
	// 4J JEV - Tabs
	static TabSpec **specs;

	bool m_bCarryingCreativeItem;
	int m_creativeSlotX, m_creativeSlotY, m_inventorySlotX, m_inventorySlotY;

public:
	static void staticCtor();
	IUIScene_CreativeMenu();

protected:
	ECreativeInventoryTabs m_curTab;
	int m_tabDynamicPos[eCreativeInventoryTab_COUNT];
	int m_tabPage[eCreativeInventoryTab_COUNT];

	void switchTab(ECreativeInventoryTabs tab);	
	void ScrollBar(UIVec2D pointerPos);
	virtual void updateTabHighlightAndText(ECreativeInventoryTabs tab) = 0;
	virtual void updateScrollCurrentPage(int currentPage, int pageCount) = 0;
	virtual ESceneSection GetSectionAndSlotInDirection( ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY );
	virtual bool handleValidKeyPress(int iUserIndex, int buttonNum, BOOL quickKeyHeld);
	virtual void handleOutsideClicked(int iPad, int buttonNum, BOOL quickKeyHeld);
	virtual void handleAdditionalKeyPress(int iAction);
	virtual void handleSlotListClicked(ESceneSection eSection, int buttonNum, BOOL quickKeyHeld);
	bool getEmptyInventorySlot(shared_ptr<ItemInstance> item, int &slotX);
	int getSectionStartOffset(ESceneSection eSection);
	virtual bool IsSectionSlotList( ESceneSection eSection );
	virtual bool CanHaveFocus( ESceneSection eSection );

	virtual bool overrideTooltips(
		ESceneSection sectionUnderPointer,
		shared_ptr<ItemInstance> itemUnderPointer,
		bool bIsItemCarried,
		bool bSlotHasItem,
		bool bCarriedIsSameAsSlot,
		int iSlotStackSizeRemaining,
		EToolTipItem &buttonA,
		EToolTipItem &buttonX,
		EToolTipItem &buttonY,
		EToolTipItem &buttonRT,
		EToolTipItem &buttonBack
	);

	static void BuildFirework(vector<shared_ptr<ItemInstance> > *list, byte type, int color, int sulphur, bool flicker, bool trail, int fadeColor = -1);
};