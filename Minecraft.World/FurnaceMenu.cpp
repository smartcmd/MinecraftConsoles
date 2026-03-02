#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "FurnaceResultSlot.h"
#include "Slot.h"
#include "GenericStats.h"
#include "FurnaceMenu.h"
#include "FurnaceRecipes.h"

FurnaceMenu::FurnaceMenu(shared_ptr<Inventory> inventory, shared_ptr<FurnaceTileEntity> furnace) : AbstractContainerMenu()
{
	tc = 0;
	lt = 0;
	ld = 0;

	this->furnace = furnace;

	addSlot(new Slot(furnace, 0, 52 + 4, 13 + 4));
	addSlot(new Slot(furnace, 1, 52 + 4, 49 + 4));
	addSlot(new FurnaceResultSlot( dynamic_pointer_cast<Player>( inventory->player->shared_from_this() ), furnace, 2, 112 + 4, 31 + 4));

	for (int y = 0; y < 3; y++)
	{
		for (int x = 0; x < 9; x++)
		{
			addSlot(new Slot(inventory, x + y * 9 + 9, 8 + x * 18, 84 + y * 18));
		}
	}
	for (int x = 0; x < 9; x++)
	{
		addSlot(new Slot(inventory, x, 8 + x * 18, 142));
	}
}

void FurnaceMenu::addSlotListener(ContainerListener *listener)
{
	AbstractContainerMenu::addSlotListener(listener);
	listener->setContainerData(this, 0, furnace->tickCount);
	listener->setContainerData(this, 1, furnace->litTime);
	listener->setContainerData(this, 2, furnace->litDuration);
}

void FurnaceMenu::broadcastChanges()
{
	AbstractContainerMenu::broadcastChanges();
	
	AUTO_VAR(itEnd, containerListeners->end());
	for (AUTO_VAR(it, containerListeners->begin()); it != itEnd; it++)
	{
		ContainerListener *listener = *it; //containerListeners->at(i);
		if (tc != furnace->tickCount) 
		{
			listener->setContainerData(this, 0, furnace->tickCount);
		}
		if (lt != furnace->litTime)
		{
			listener->setContainerData(this, 1, furnace->litTime);
		}
		if (ld != furnace->litDuration) 
		{
			listener->setContainerData(this, 2, furnace->litDuration);
		}
	}

	tc = furnace->tickCount;
	lt = furnace->litTime;
	ld = furnace->litDuration;
}

void FurnaceMenu::setData(int id, int value)
{
	if (id == 0) furnace->tickCount = value;
	if (id == 1) furnace->litTime = value;
	if (id == 2) furnace->litDuration = value;
}

bool FurnaceMenu::stillValid(shared_ptr<Player> player)
{
	return furnace->stillValid(player);
}

shared_ptr<ItemInstance> FurnaceMenu::quickMoveStack(shared_ptr<Player> player, int slotIndex)
{
	shared_ptr<ItemInstance> clicked = nullptr;
	Slot *slot = slots->at(slotIndex);
	//Slot *IngredientSlot = slots->at(INGREDIENT_SLOT);

	bool charcoalUsed = furnace->wasCharcoalUsed();

	if (slot != NULL && slot->hasItem())
	{
		shared_ptr<ItemInstance> stack = slot->getItem();
		clicked = stack->copy();

		if (slotIndex == RESULT_SLOT)
		{
			if (!moveItemStackTo(stack, INV_SLOT_START, USE_ROW_SLOT_END, true))
			{
				return nullptr;
			}
			slot->onQuickCraft(stack, clicked);

			// 4J-JEV, hook for Durango achievement 'Renewable Energy'.
#ifdef _EXTENDED_ACHIEVEMENTS
			if ( charcoalUsed && stack->getItem()->id == Item::coal_Id && stack->getAuxValue() == CoalItem::CHAR_COAL)
				player->awardStat(GenericStats::renewableEnergy(),GenericStats::param_renewableEnergy());
#endif
		}
		else if (slotIndex == FUEL_SLOT || slotIndex == INGREDIENT_SLOT)
		{
			if (!moveItemStackTo(stack, INV_SLOT_START, USE_ROW_SLOT_END, false))
			{
				return nullptr;
			}
		}
		else if (FurnaceRecipes::getInstance()->getResult(stack->getItem()->id) != NULL)
		{
			if (!moveItemStackTo(stack, INGREDIENT_SLOT, INGREDIENT_SLOT + 1, false))
			{
				return nullptr;
			}
		}
		else if (FurnaceTileEntity::isFuel(stack))
		{
			if (!moveItemStackTo(stack, FUEL_SLOT, FUEL_SLOT + 1, false))
			{
				return nullptr;
			}
		} else if (slotIndex >= INV_SLOT_START && slotIndex < INV_SLOT_END)
		{
			if (!moveItemStackTo(stack, USE_ROW_SLOT_START, USE_ROW_SLOT_END, false))
			{
				return nullptr;
			}
		} else if (slotIndex >= USE_ROW_SLOT_START && slotIndex < USE_ROW_SLOT_END)
		{
			if (!moveItemStackTo(stack, INV_SLOT_START, INV_SLOT_END, false))
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

shared_ptr<ItemInstance> FurnaceMenu::clicked(int slotIndex, int buttonNum, int clickType, shared_ptr<Player> player)
{
	bool charcoalUsed = furnace->wasCharcoalUsed();

	shared_ptr<ItemInstance> out = AbstractContainerMenu::clicked(slotIndex, buttonNum, clickType, player);

#ifdef _EXTENDED_ACHIEVEMENTS
	if ( charcoalUsed && (out!=nullptr) && (buttonNum==0 || buttonNum==1) && clickType==CLICK_PICKUP
		&& out->getItem()->id == Item::coal_Id && out->getAuxValue() == CoalItem::CHAR_COAL	)
	{
		player->awardStat(GenericStats::renewableEnergy(),GenericStats::param_renewableEnergy());
	}
#endif

	return out;
}
