#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "Container.h"
#include "Slot.h"
#include "TrapMenu.h"

TrapMenu::TrapMenu(shared_ptr<Container> inventory, shared_ptr<DispenserTileEntity> trap)
{
	this->trap = trap;

	for (int y = 0; y < 3; y++)
	{
		for (int x = 0; x < 3; x++)
		{
			addSlot(new Slot(trap, x + y * 3, 62 + x * 18, 17 + y * 18));
		}
	}

	for (int y = 0; y < 3; y++)
	{
		for (int x = 0; x < 9; x++)
		{
			addSlot(new Slot(inventory, x + y * 9 + 9, 8 + x * 18, 84 + y * 18));
		}
	}
	for (int x = 0; x < 9; x++)
	{
		addSlot(new Slot(inventory, x, 8 + x * 18, 70 + 4 * 18));
	}
}

bool TrapMenu::stillValid(shared_ptr<Player> player)
{
	return trap->stillValid(player);
}

// 4J Stu - Brought forward from 1.2
shared_ptr<ItemInstance> TrapMenu::quickMoveStack(shared_ptr<Player> player, int slotIndex)
{
	shared_ptr<ItemInstance> clicked = nullptr;
	Slot *slot = slots.at(slotIndex);
	if (slot != NULL && slot->hasItem())
	{
		shared_ptr<ItemInstance> stack = slot->getItem();
		clicked = stack->copy();

		if (slotIndex < INV_SLOT_START)
		{
			if (!moveItemStackTo(stack, INV_SLOT_START, USE_ROW_SLOT_END, true))
			{
				return nullptr;
			}
		}
		else
		{
			if (!moveItemStackTo(stack, 0, INV_SLOT_START, false))
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
			// nothing moved
			return nullptr;
		}
		else
		{
			slot->onTake(player, stack);
		}
	}
	return clicked;
}