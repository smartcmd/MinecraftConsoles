#pragma once

#include "AbstractContainerMenu.h"
#include "Slot.h"

class HorseInventoryMenu;

class HorseSaddleSlot : public Slot
{
public:
	HorseSaddleSlot( shared_ptr<Container> horseInventory );

	bool mayPlace(shared_ptr<ItemInstance> item);
};

class HorseArmorSlot : public Slot
{
private:
	HorseInventoryMenu *m_parent;
public:
	HorseArmorSlot( HorseInventoryMenu *parent, shared_ptr<Container> horseInventory );

	bool mayPlace(shared_ptr<ItemInstance> item);
	bool isActive();
};

class HorseInventoryMenu : public AbstractContainerMenu
{
	friend class HorseArmorSlot;
private:
	shared_ptr<Container> horseContainer;
	shared_ptr<EntityHorse> horse;

public:
	HorseInventoryMenu(shared_ptr<Container> playerInventory, shared_ptr<Container> horseInventory, shared_ptr<EntityHorse> horse);

	bool stillValid(shared_ptr<Player> player);
	shared_ptr<ItemInstance> quickMoveStack(shared_ptr<Player> player, int slotIndex);
	void removed(shared_ptr<Player> player);
	shared_ptr<Container> getContainer();
};