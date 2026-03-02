#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.crafting.h"
#include "net.minecraft.world.entity.player.h"
#include "ResultSlot.h"
#include "ArmorSlot.h"
#include "CraftingContainer.h"
#include "ResultContainer.h"
#include "InventoryMenu.h"
#include "Tile.h"
#include "GenericStats.h"

const int InventoryMenu::RESULT_SLOT = 0;
const int InventoryMenu::CRAFT_SLOT_START = 1;
const int InventoryMenu::CRAFT_SLOT_END = InventoryMenu::CRAFT_SLOT_START + 4;
const int InventoryMenu::ARMOR_SLOT_START = InventoryMenu::CRAFT_SLOT_END;
const int InventoryMenu::ARMOR_SLOT_END = InventoryMenu::ARMOR_SLOT_START + 4;
const int InventoryMenu::INV_SLOT_START = InventoryMenu::ARMOR_SLOT_END;
const int InventoryMenu::INV_SLOT_END = InventoryMenu::INV_SLOT_START + 9 * 3;
const int InventoryMenu::USE_ROW_SLOT_START = InventoryMenu::INV_SLOT_END;
const int InventoryMenu::USE_ROW_SLOT_END = InventoryMenu::USE_ROW_SLOT_START + 9;

InventoryMenu::InventoryMenu(shared_ptr<Inventory> inventory, bool active, Player *player) : AbstractContainerMenu()
{
	owner = player;
	_init( inventory, active );
}

void InventoryMenu::_init(shared_ptr<Inventory> inventory, bool active)
{
	craftSlots = shared_ptr<CraftingContainer>( new CraftingContainer(this, 2, 2) );
	resultSlots = shared_ptr<ResultContainer>( new ResultContainer() );

	this->active = active;
	addSlot(new ResultSlot( inventory->player, craftSlots, resultSlots, 0, 144, 36));

	for (int y = 0; y < 2; y++)
	{
		for (int x = 0; x < 2; x++)
		{
			addSlot(new Slot(craftSlots, x + y * 2, 88 + x * 18, 26 + y * 18));
		}
	}

	for (int i = 0; i < 4; i++)
	{
		// 4J Stu I removed an anonymous class that was here whose only purpose seemed to be a way of using the
		// loop counter i within the functions, rather than making it a member of the object. I have moved all that
		// out to the ArmorSlot class
		addSlot(new ArmorSlot(i, inventory, inventory->getContainerSize() - 1 - i, 8, 8 + i * 18) );
	}
	for (int y = 0; y < 3; y++)
	{
		for (int x = 0; x < 9; x++)
		{
			addSlot(new Slot(inventory, x + (y + 1) * 9, 8 + x * 18, 84 + y * 18));
		}
	}
	for (int x = 0; x < 9; x++)
	{
		addSlot(new Slot(inventory, x, 8 + x * 18, 142));
	}

	slotsChanged();  // 4J removed craftSlots parameter, see comment below
}

void InventoryMenu::slotsChanged()  // 4J used to take a shared_ptr<Container> but wasn't using it, so removed to simplify things
{
	MemSect(23);
	resultSlots->setItem(0, Recipes::getInstance()->getItemFor(craftSlots, owner->level) );
	MemSect(0);
}

void InventoryMenu::removed(shared_ptr<Player> player)
{
	AbstractContainerMenu::removed(player);
	for (int i = 0; i < 4; i++)
	{
		shared_ptr<ItemInstance> item = craftSlots->removeItemNoUpdate(i);
		if (item != NULL)
		{
			player->drop(item);
			craftSlots->setItem(i, nullptr);
		}
	}
	resultSlots->setItem(0, nullptr);
}

bool InventoryMenu::stillValid(shared_ptr<Player> player)
{
	return true;
}

shared_ptr<ItemInstance> InventoryMenu::quickMoveStack(shared_ptr<Player> player, int slotIndex)
{
	shared_ptr<ItemInstance> clicked = nullptr;
	Slot *slot = slots->at(slotIndex);
	
	Slot *HelmetSlot = slots->at(ARMOR_SLOT_START);
	Slot *ChestplateSlot = slots->at(ARMOR_SLOT_START+1);
	Slot *LeggingsSlot = slots->at(ARMOR_SLOT_START+2);
	Slot *BootsSlot = slots->at(ARMOR_SLOT_START+3);


	if (slot != NULL && slot->hasItem())
	{
		shared_ptr<ItemInstance> stack = slot->getItem();
		clicked = stack->copy();

		if (slotIndex == RESULT_SLOT)
		{
			// 4J Stu - Brought forward change from 1.2
			if(!moveItemStackTo(stack, INV_SLOT_START, USE_ROW_SLOT_END, true))
			{
				return nullptr;
			}
			slot->onQuickCraft(stack, clicked);
		}
		else if (slotIndex >= INV_SLOT_START && slotIndex < INV_SLOT_END)
		{
			// 4J-PB - added for quick equip
			if(ArmorRecipes::GetArmorType(stack->id)==ArmorRecipes::eArmorType_Helmet && (!HelmetSlot->hasItem() )  ) 
			{
				if(!moveItemStackTo(stack, ARMOR_SLOT_START, ARMOR_SLOT_START+1, false))
				{
					return nullptr;
				}
			}
			else if(ArmorRecipes::GetArmorType(stack->id)==ArmorRecipes::eArmorType_Chestplate && (!ChestplateSlot->hasItem() )  ) 
			{
				if(!moveItemStackTo(stack, ARMOR_SLOT_START+1, ARMOR_SLOT_START+2, false))
				{
					return nullptr;
				}
			}
			else if(ArmorRecipes::GetArmorType(stack->id)==ArmorRecipes::eArmorType_Leggings && (!LeggingsSlot->hasItem() )  ) 
			{
				if(!moveItemStackTo(stack, ARMOR_SLOT_START+2, ARMOR_SLOT_START+3, false))
				{
					return nullptr;
				}
			}
			else if(ArmorRecipes::GetArmorType(stack->id)==ArmorRecipes::eArmorType_Boots && (!BootsSlot->hasItem() )  ) 
			{
				if(!moveItemStackTo(stack, ARMOR_SLOT_START+3, ARMOR_SLOT_START+4, false))
				{
					return nullptr;
				}
			}
			// 4J Stu - Brought forward change from 1.2
			else if(!moveItemStackTo(stack, USE_ROW_SLOT_START, USE_ROW_SLOT_END, false))
			{
				return nullptr;
			}
		}
		else if (slotIndex >= USE_ROW_SLOT_START && slotIndex < USE_ROW_SLOT_END)
		{
			//ArmorRecipes::_eArmorType eArmourType=ArmorRecipes::GetArmorType(stack->id);

			if(ArmorRecipes::GetArmorType(stack->id)==ArmorRecipes::eArmorType_Helmet && (!HelmetSlot->hasItem() )  ) 
			{
				if(!moveItemStackTo(stack, ARMOR_SLOT_START, ARMOR_SLOT_START+1, false))
				{
					return nullptr;
				}
			}
			else if(ArmorRecipes::GetArmorType(stack->id)==ArmorRecipes::eArmorType_Chestplate && (!ChestplateSlot->hasItem() )  ) 
			{
				if(!moveItemStackTo(stack, ARMOR_SLOT_START+1, ARMOR_SLOT_START+2, false))
				{
					return nullptr;
				}
			}
			else if(ArmorRecipes::GetArmorType(stack->id)==ArmorRecipes::eArmorType_Leggings && (!LeggingsSlot->hasItem() )  ) 
			{
				if(!moveItemStackTo(stack, ARMOR_SLOT_START+2, ARMOR_SLOT_START+3, false))
				{
					return nullptr;
				}
			}
			else if(ArmorRecipes::GetArmorType(stack->id)==ArmorRecipes::eArmorType_Boots && (!BootsSlot->hasItem() )  ) 
			{
				if(!moveItemStackTo(stack, ARMOR_SLOT_START+3, ARMOR_SLOT_START+4, false))
				{
					return nullptr;
				}
			}
			// 4J Stu - Brought forward change from 1.2
			else if(!moveItemStackTo(stack, INV_SLOT_START, INV_SLOT_END, false))
			{
				return nullptr;
			}
		}
		else
		{
			// 4J Stu - Brought forward change from 1.2
			if(!moveItemStackTo(stack, INV_SLOT_START, USE_ROW_SLOT_END, false))
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

bool InventoryMenu::mayCombine(Slot *slot, shared_ptr<ItemInstance> item)
{
	return slot->mayCombine(item);
}

// 4J-JEV: Added for achievement 'Iron Man'.
shared_ptr<ItemInstance> InventoryMenu::clicked(int slotIndex, int buttonNum, int clickType, shared_ptr<Player> player)
{
	shared_ptr<ItemInstance> out = AbstractContainerMenu::clicked(slotIndex, buttonNum, clickType, player);

#ifdef _EXTENDED_ACHIEVEMENTS
	static int ironItems[4] = {Item::helmet_iron_Id,Item::chestplate_iron_Id,Item::leggings_iron_Id,Item::boots_iron_Id};
	for (int i = ARMOR_SLOT_START; i < ARMOR_SLOT_END; i++)
	{
		Slot *slot = slots->at(i);
		if ( (slot==NULL) || (!slot->hasItem()) || (slot->getItem()->getItem()->id != ironItems[i-ARMOR_SLOT_START]) )
		{
			return out;
		}
	}
	player->awardStat(GenericStats::ironMan(),GenericStats::param_ironMan());
#endif

	return out;
}
