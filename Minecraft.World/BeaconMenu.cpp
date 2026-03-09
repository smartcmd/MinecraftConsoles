#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "BeaconMenu.h"

BeaconMenu::BeaconMenu(shared_ptr<Container> inventory, shared_ptr<BeaconTileEntity> beacon)
{
	this->beacon = beacon;

	addSlot(paymentSlot = new BeaconMenu::PaymentSlot(beacon, PAYMENT_SLOT, 136, 110));

	int xo = 36;
	int yo = 137;

	for (int y = 0; y < 3; y++)
	{
		for (int x = 0; x < 9; x++)
		{
			addSlot(new Slot(inventory, x + y * 9 + 9, xo + x * 18, yo + y * 18));
		}
	}
	for (int x = 0; x < 9; x++)
	{
		addSlot(new Slot(inventory, x, xo + x * 18, 58 + yo));
	}

	levels = beacon->getLevels();
	primaryPower = beacon->getPrimaryPower();
	secondaryPower = beacon->getSecondaryPower();
}


void BeaconMenu::addSlotListener(ContainerListener *listener)
{
	AbstractContainerMenu::addSlotListener(listener);

	listener->setContainerData(this, 0, levels);
	listener->setContainerData(this, 1, primaryPower);
	listener->setContainerData(this, 2, secondaryPower);
}

void BeaconMenu::setData(int id, int value)
{
	if (id == 0) beacon->setLevels(value);
	if (id == 1) beacon->setPrimaryPower(value);
	if (id == 2) beacon->setSecondaryPower(value);
}

shared_ptr<BeaconTileEntity> BeaconMenu::getBeacon()
{
	return beacon;
}

bool BeaconMenu::stillValid(shared_ptr<Player> player)
{
	return beacon->stillValid(player);
}

shared_ptr<ItemInstance> BeaconMenu::quickMoveStack(shared_ptr<Player> player, int slotIndex)
{
	shared_ptr<ItemInstance> clicked = nullptr;
	Slot *slot = slots.at(slotIndex);
	if (slot != nullptr && slot->hasItem())
	{
		shared_ptr<ItemInstance> stack = slot->getItem();
		clicked = stack->copy();

		if (slotIndex == PAYMENT_SLOT)
		{
			if (!moveItemStackTo(stack, INV_SLOT_START, USE_ROW_SLOT_END, true))
			{
				return nullptr;
			}
			slot->onQuickCraft(stack, clicked);
		}
		else if (!paymentSlot->hasItem() && paymentSlot->mayPlace(stack) && stack->count == 1)
		{
			if (!moveItemStackTo(stack, PAYMENT_SLOT, PAYMENT_SLOT + 1, false))
			{
				return nullptr;
			}
		}
		else if (slotIndex >= INV_SLOT_START && slotIndex < INV_SLOT_END)
		{
			if (!moveItemStackTo(stack, USE_ROW_SLOT_START, USE_ROW_SLOT_END, false))
			{
				return nullptr;
			}
		}
		else if (slotIndex >= USE_ROW_SLOT_START && slotIndex < USE_ROW_SLOT_END)
		{
			if (!moveItemStackTo(stack, INV_SLOT_START, INV_SLOT_END, false))
			{
				return nullptr;
			}
		}
		else
		{
			if (!moveItemStackTo(stack, INV_SLOT_START, USE_ROW_SLOT_END, false))
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
		if (stack->count == clicked->count)
		{
			return nullptr;
		}
		else
		{
			slot->onTake(player, stack);
		}
	}
	return clicked;
}

BeaconMenu::PaymentSlot::PaymentSlot(shared_ptr<Container> container, int slot, int x, int y) : Slot(container, slot, x, y)
{
}

bool BeaconMenu::PaymentSlot::mayPlace(shared_ptr<ItemInstance> item)
{
	if (item != nullptr)
	{
		return (item->id == Item::emerald_Id || item->id == Item::diamond_Id || item->id == Item::goldIngot_Id || item->id == Item::ironIngot_Id);
	}
	return false;
}

int BeaconMenu::PaymentSlot::getMaxStackSize() const
{
	return 1;
}