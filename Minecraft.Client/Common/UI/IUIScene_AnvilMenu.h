#pragma once
#include "IUIScene_AbstractContainerMenu.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.ContainerListener.h"

// The 0-indexed slot in the inventory list that lines up with the result slot
#define ANVIL_SCENE_RESULT_SLOT_UP_OFFSET 5
#define ANVIL_SCENE_RESULT_SLOT_DOWN_OFFSET 5
#define ANVIL_SCENE_ITEM1_SLOT_UP_OFFSET 3
#define ANVIL_SCENE_ITEM1_SLOT_DOWN_OFFSET 3
#define ANVIL_SCENE_ITEM2_SLOT_UP_OFFSET 4
#define ANVIL_SCENE_ITEM2_SLOT_DOWN_OFFSET 4

class Inventory;
class AnvilMenu;

class IUIScene_AnvilMenu : public virtual IUIScene_AbstractContainerMenu, public net_minecraft_world_inventory::ContainerListener
{
protected:
	shared_ptr<Inventory> m_inventory;
	AnvilMenu *m_repairMenu;
	wstring m_itemName;

protected:
	IUIScene_AnvilMenu();

	virtual ESceneSection GetSectionAndSlotInDirection( ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY );
	int getSectionStartOffset(ESceneSection eSection);	
	virtual void handleOtherClicked(int iPad, ESceneSection eSection, int buttonNum, bool quickKey);
	bool IsSectionSlotList( ESceneSection eSection );

	void handleTick();

	// Anvil only
	virtual void handleEditNamePressed() = 0;
	virtual void setEditNameValue(const wstring &name) = 0;
	virtual void setEditNameEditable(bool enabled) = 0;
	virtual void setCostLabel(const wstring &label, bool canAfford) = 0;
	virtual void showCross(bool show) = 0;
	void updateItemName();

	// ContainerListenr
	void refreshContainer(AbstractContainerMenu *container, vector<shared_ptr<ItemInstance> > *items);
	void slotChanged(AbstractContainerMenu *container, int slotIndex, shared_ptr<ItemInstance> item);
	void setContainerData(AbstractContainerMenu *container, int id, int value);
};