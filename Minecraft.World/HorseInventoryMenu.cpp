#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.animal.h"
#include "HorseInventoryMenu.h"

HorseSaddleSlot::HorseSaddleSlot( shared_ptr<Container> horseInventory ) : Slot(horseInventory, EntityHorse::INV_SLOT_SADDLE, 8, 18)
{
}

bool HorseSaddleSlot::mayPlace(shared_ptr<ItemInstance> item)
{
	return Slot::mayPlace(item) && item->id == Item::saddle_Id && !hasItem();
}


HorseArmorSlot::HorseArmorSlot( HorseInventoryMenu *parent, shared_ptr<Container> horseInventory ) : Slot(horseInventory, EntityHorse::INV_SLOT_ARMOR, 8, 18 * 2)
{
	m_parent = parent;
}

bool HorseArmorSlot::mayPlace(shared_ptr<ItemInstance> item)
{
	return Slot::mayPlace(item) && m_parent->horse->canWearArmor() && EntityHorse::isHorseArmor(item->id);
}

bool HorseArmorSlot::isActive()
{
	return m_parent->horse->canWearArmor();
}


HorseInventoryMenu::HorseInventoryMenu(shared_ptr<Container> playerInventory, shared_ptr<Container> horseInventory, shared_ptr<EntityHorse> horse)
{
	horseContainer = horseInventory;
	this->horse = horse;
	int containerRows = 3;
	horseInventory->startOpen();

	int yo = (containerRows - 4) * 18;

	// equipment slots
	addSlot(new HorseSaddleSlot(horseInventory) );
	addSlot(new HorseArmorSlot(this, horseInventory) );

	if (horse->isChestedHorse())
	{
		for (int y = 0; y < containerRows; y++)
		{
			for (int x = 0; x < 5; x++)
			{
				addSlot(new Slot(horseInventory, EntityHorse::INV_BASE_COUNT + x + y * 5, 80 + x * 18, 18 + y * 18));
			}
		}
	}

	for (int y = 0; y < 3; y++)
	{
		for (int x = 0; x < 9; x++)
		{
			addSlot(new Slot(playerInventory, x + y * 9 + 9, 8 + x * 18, 102 + y * 18 + yo));
		}
	}
	for (int x = 0; x < 9; x++)
	{
		addSlot(new Slot(playerInventory, x, 8 + x * 18, 160 + yo));
	}
}

bool HorseInventoryMenu::stillValid(shared_ptr<Player> player)
{
	return horseContainer->stillValid(player) && horse->isAlive() && horse->distanceTo(player) < 8;
}

shared_ptr<ItemInstance> HorseInventoryMenu::quickMoveStack(shared_ptr<Player> player, int slotIndex)
{
	shared_ptr<ItemInstance> clicked = nullptr;
	Slot *slot = slots.at(slotIndex);
	if (slot != NULL && slot->hasItem())
	{
		shared_ptr<ItemInstance> stack = slot->getItem();
		clicked = stack->copy();

		if (slotIndex < horseContainer->getContainerSize())
		{
			if (!moveItemStackTo(stack, horseContainer->getContainerSize(), slots.size(), true))
			{
				return nullptr;
			}
		}
		else
		{
			if (getSlot(EntityHorse::INV_SLOT_ARMOR)->mayPlace(stack) && !getSlot(EntityHorse::INV_SLOT_ARMOR)->hasItem())
			{
				if (!moveItemStackTo(stack, EntityHorse::INV_SLOT_ARMOR, EntityHorse::INV_SLOT_ARMOR + 1, false))
				{
					return nullptr;
				}
			}
			else if (getSlot(EntityHorse::INV_SLOT_SADDLE)->mayPlace(stack))
			{
				if (!moveItemStackTo(stack, EntityHorse::INV_SLOT_SADDLE, EntityHorse::INV_SLOT_SADDLE + 1, false))
				{
					return nullptr;
				}
			}
			else if (horseContainer->getContainerSize() <= EntityHorse::INV_BASE_COUNT || !moveItemStackTo(stack, EntityHorse::INV_BASE_COUNT, horseContainer->getContainerSize(), false))
			{
				return nullptr;
			}
		}
		if (stack->count == 0)
		{
			slot->set(nullptr);
		}
		else
		{
			slot->setChanged();
		}
	}
	return clicked;
}

void HorseInventoryMenu::removed(shared_ptr<Player> player)
{
	AbstractContainerMenu::removed(player);
	horseContainer->stopOpen();
}

shared_ptr<Container> HorseInventoryMenu::getContainer()
{
	return horseContainer;
}