#include "stdafx.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.inventory.h"
#include "net.minecraft.world.item.crafting.h"
#include "ArmorSlot.h"

ArmorSlot::ArmorSlot(int slotNum, shared_ptr<Container> container, int id, int x, int y)
	: Slot( container, id, x, y ),
	slotNum( slotNum )
{
}

int ArmorSlot::getMaxStackSize() const
{
	return 1;
}

bool ArmorSlot::mayPlace(shared_ptr<ItemInstance> item)
{
	if (item == NULL)
	{
		return false;
	}
	if ( dynamic_cast<ArmorItem *>( item->getItem() ) != NULL)
	{
		return dynamic_cast<ArmorItem *>( item->getItem() )->slot == slotNum;
	}
	if (item->getItem()->id == Tile::pumpkin_Id || item->getItem()->id == Item::skull_Id)
	{
		return slotNum == 0;
	}
	return false;
}

Icon *ArmorSlot::getNoItemIcon()
{
	return ArmorItem::getEmptyIcon(slotNum);
}

//
//bool ArmorSlot::mayCombine(shared_ptr<ItemInstance> item)
//{
//	shared_ptr<ItemInstance> thisItemI = getItem();
//	if(thisItemI == NULL || item == NULL) return false;
//
//	ArmorItem *thisItem = (ArmorItem *)thisItemI->getItem();
//	bool thisIsDyableArmor = thisItem->getMaterial() == ArmorItem::ArmorMaterial::CLOTH;
//	bool itemIsDye = item->id == Item::dye_powder_Id;
//	return thisIsDyableArmor && itemIsDye;
//}
//
//shared_ptr<ItemInstance> ArmorSlot::combine(shared_ptr<ItemInstance> item)
//{
//	shared_ptr<CraftingContainer> craftSlots = shared_ptr<CraftingContainer>( new CraftingContainer(NULL, 2, 2) );
//	craftSlots->setItem(0, item);
//	craftSlots->setItem(1, getItem()); // Armour item needs to go second
//	shared_ptr<ItemInstance> result = ArmorDyeRecipe::assembleDyedArmor(craftSlots);
//	craftSlots->setItem(0, nullptr);
//	craftSlots->setItem(1, nullptr);
//	return result;
//}