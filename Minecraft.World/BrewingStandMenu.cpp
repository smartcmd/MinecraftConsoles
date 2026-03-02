#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.alchemy.h"
#include "net.minecraft.stats.h"
#include "BrewingStandMenu.h"

BrewingStandMenu::BrewingStandMenu(shared_ptr<Inventory> inventory, shared_ptr<BrewingStandTileEntity> brewingStand)
{
	tc = 0;

	this->brewingStand = brewingStand;

	addSlot(new PotionSlot(dynamic_pointer_cast<Player>( inventory->player->shared_from_this() ), brewingStand, 0, 56, 46));
	addSlot(new PotionSlot(dynamic_pointer_cast<Player>( inventory->player->shared_from_this() ), brewingStand, 1, 79, 53));
	addSlot(new PotionSlot(dynamic_pointer_cast<Player>( inventory->player->shared_from_this() ), brewingStand, 2, 102, 46));
	ingredientSlot = addSlot(new IngredientsSlot(brewingStand, 3, 79, 17));

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

void BrewingStandMenu::addSlotListener(ContainerListener *listener)
{
	AbstractContainerMenu::addSlotListener(listener);
	listener->setContainerData(this, 0, brewingStand->getBrewTime());
}

void BrewingStandMenu::broadcastChanges()
{
	AbstractContainerMenu::broadcastChanges();

	//for (int i = 0; i < containerListeners->size(); i++)
	for(AUTO_VAR(it, containerListeners->begin()); it != containerListeners->end(); ++it)
	{
		ContainerListener *listener = *it; //containerListeners.at(i);
		if (tc != brewingStand->getBrewTime())
		{
			listener->setContainerData(this, 0, brewingStand->getBrewTime());
		}
	}
	tc = brewingStand->getBrewTime();
}

void BrewingStandMenu::setData(int id, int value)
{
	if (id == 0) brewingStand->setBrewTime(value);
}

bool BrewingStandMenu::stillValid(shared_ptr<Player> player)
{
	return brewingStand->stillValid(player);
}

shared_ptr<ItemInstance> BrewingStandMenu::quickMoveStack(shared_ptr<Player> player, int slotIndex)
{
	shared_ptr<ItemInstance> clicked = nullptr;
	Slot *slot = slots->at(slotIndex);
	Slot *IngredientSlot = slots->at(INGREDIENT_SLOT);
	Slot *PotionSlot1 = slots->at(BOTTLE_SLOT_START);
	Slot *PotionSlot2 = slots->at(BOTTLE_SLOT_START+1);
	Slot *PotionSlot3 = slots->at(BOTTLE_SLOT_START+2);

	if (slot != NULL && slot->hasItem())
	{
		shared_ptr<ItemInstance> stack = slot->getItem();
		clicked = stack->copy();

		if ((slotIndex >= BOTTLE_SLOT_START && slotIndex <= BOTTLE_SLOT_END) || (slotIndex == INGREDIENT_SLOT))
		{
			if (!moveItemStackTo(stack, INV_SLOT_START, USE_ROW_SLOT_END, true))
			{
				return nullptr;
			}
			slot->onQuickCraft(stack, clicked);
		}
		else if (!ingredientSlot->hasItem() && ingredientSlot->mayPlace(stack))
		{
			if (!moveItemStackTo(stack, INGREDIENT_SLOT, INGREDIENT_SLOT + 1, false))
			{
				return nullptr;
			}
		}
		else if (PotionSlot::mayPlaceItem(clicked))
		{
			if (!moveItemStackTo(stack, BOTTLE_SLOT_START, BOTTLE_SLOT_END + 1, false))
			{
				return nullptr;
			}
		}
		else if (slotIndex >= INV_SLOT_START && slotIndex < INV_SLOT_END)
		{
			// 4J-PB - if the item is an ingredient, quickmove it into the ingredient slot
			if( (Item::items[stack->id]->hasPotionBrewingFormula() || (stack->id == Item::netherStalkSeeds_Id) ) && 
				(!IngredientSlot->hasItem() || (stack->id==IngredientSlot->getItem()->id) ) )
			{
				if(!moveItemStackTo(stack, INGREDIENT_SLOT, INGREDIENT_SLOT+1, false))
				{
					return nullptr;
				}
			}
			// potion?
			else if((stack->id==Item::potion_Id) &&(!PotionSlot1->hasItem() || !PotionSlot2->hasItem() || !PotionSlot3->hasItem()))
			{
				if(!moveItemStackTo(stack, BOTTLE_SLOT_START, BOTTLE_SLOT_END+1, false))
				{
					return nullptr;
				}
			}
			else if (!moveItemStackTo(stack, USE_ROW_SLOT_START, USE_ROW_SLOT_END, false))
			{
				return nullptr;
			}
		}
		else if (slotIndex >= USE_ROW_SLOT_START && slotIndex < USE_ROW_SLOT_END)
		{
			// 4J-PB - if the item is an ingredient, quickmove it into the ingredient slot
			if((Item::items[stack->id]->hasPotionBrewingFormula() || (stack->id == Item::netherStalkSeeds_Id)) && 
				(!IngredientSlot->hasItem() || (stack->id==IngredientSlot->getItem()->id) ))
			{
				if(!moveItemStackTo(stack, INGREDIENT_SLOT, INGREDIENT_SLOT+1, false))
				{
					return nullptr;
				}
			}
			// potion?
			else if((stack->id==Item::potion_Id) &&(!PotionSlot1->hasItem() || !PotionSlot2->hasItem() || !PotionSlot3->hasItem()))
			{
				if(!moveItemStackTo(stack, BOTTLE_SLOT_START, BOTTLE_SLOT_END+1, false))
				{
					return nullptr;
				}
			}			
			else if (!moveItemStackTo(stack, INV_SLOT_START, INV_SLOT_END, false))
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

BrewingStandMenu::PotionSlot::PotionSlot(shared_ptr<Player> player, shared_ptr<Container> container, int slot, int x, int y) : Slot(container, slot, x, y)
{
	this->player = player;
}

bool BrewingStandMenu::PotionSlot::mayPlace(shared_ptr<ItemInstance> item)
{
	return mayPlaceItem(item);
}

int BrewingStandMenu::PotionSlot::getMaxStackSize()
{
	return 1;
}

void BrewingStandMenu::PotionSlot::onTake(shared_ptr<Player> player, shared_ptr<ItemInstance> carried)
{
	carried->onCraftedBy(this->player->level, dynamic_pointer_cast<Player>( this->player->shared_from_this() ), 1);
	if (carried->id == Item::potion_Id && carried->getAuxValue() > 0)
		this->player->awardStat(GenericStats::potion(),GenericStats::param_potion());
	Slot::onTake(player, carried);
}

bool BrewingStandMenu::PotionSlot::mayCombine(shared_ptr<ItemInstance> second)
{
	return false;
}

bool BrewingStandMenu::PotionSlot::mayPlaceItem(shared_ptr<ItemInstance> item)
{
	return item != NULL && (item->id == Item::potion_Id || item->id == Item::glassBottle_Id);
}



BrewingStandMenu::IngredientsSlot::IngredientsSlot(shared_ptr<Container> container, int slot, int x, int y) : Slot(container, slot, x ,y)
{
}

bool BrewingStandMenu::IngredientsSlot::mayPlace(shared_ptr<ItemInstance> item)
{
	if (item != NULL)
	{
		if (PotionBrewing::SIMPLIFIED_BREWING)
		{
			return Item::items[item->id]->hasPotionBrewingFormula();
		}
		else
		{
			return Item::items[item->id]->hasPotionBrewingFormula() || item->id == Item::netherStalkSeeds_Id || item->id == Item::bucket_water_Id;
		}
	}
	return false;
}

bool BrewingStandMenu::IngredientsSlot::mayCombine(shared_ptr<ItemInstance> second)
{
	return false;
}

int BrewingStandMenu::IngredientsSlot::getMaxStackSize()
{
	return 64;
}
