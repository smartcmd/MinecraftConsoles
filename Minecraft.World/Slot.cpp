#include "stdafx.h"

#include "Container.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.crafting.h"
#include "Slot.h"

Slot::Slot(shared_ptr<Container> container, int slot, int x, int y) : container( container ), slot( slot )
{
	this->x = x;
	this->y = y;

	this->index = 0;
}

void Slot::onQuickCraft(shared_ptr<ItemInstance> picked, shared_ptr<ItemInstance> original)
{
	if (picked == nullptr || original == nullptr)
	{
		return;
	}
	if (picked->id != original->id)
	{
		return;
	}
	int count = original->count - picked->count;
	if (count > 0)
	{
		onQuickCraft(picked, count);
	}
}


void Slot::onQuickCraft(shared_ptr<ItemInstance> picked, int count)
{
}

void Slot::checkTakeAchievements(shared_ptr<ItemInstance> picked)
{
}

void Slot::swap(Slot *other)
{
	shared_ptr<ItemInstance> item1 = container->getItem(slot);
	shared_ptr<ItemInstance> item2 = other->container->getItem(other->slot);

	if (item1 != nullptr && item1->count > other->getMaxStackSize())
	{
		if (item2 != nullptr) return;
		item2 = item1->remove(item1->count - other->getMaxStackSize());
	}
	if (item2 != nullptr && item2->count > getMaxStackSize())
	{
		if (item1 != nullptr) return;
		item1 = item2->remove(item2->count - getMaxStackSize());
	}
	other->container->setItem(other->slot, item1);

	container->setItem(slot, item2);
	setChanged();
}

void Slot::onTake(shared_ptr<Player> player, shared_ptr<ItemInstance> carried)
{
	setChanged();
}

bool Slot::mayPlace(shared_ptr<ItemInstance> item)
{
	return true;
}

shared_ptr<ItemInstance> Slot::getItem()
{
	return container->getItem(slot);
}

bool Slot::hasItem()
{
	return getItem() != nullptr;
}

void Slot::set(shared_ptr<ItemInstance> item)
{
	container->setItem(slot, item);
	setChanged();
}

void Slot::setChanged()
{
	container->setChanged();
}

int Slot::getMaxStackSize() const
{
	return container->getMaxStackSize();
}

Icon *Slot::getNoItemIcon()
{
	return nullptr;
}

shared_ptr<ItemInstance> Slot::remove(int c)
{
	return container->removeItem(slot, c);
}

bool Slot::isAt(shared_ptr<Container> c, int s)
{
	return c == container && s == slot;
}

bool Slot::mayPickup(shared_ptr<Player> player)
{
	return true;
}

bool Slot::isActive()
{
	return true;
}

bool Slot::mayCombine(shared_ptr<ItemInstance> second)
{
	shared_ptr<ItemInstance> first = getItem();

	if(first == nullptr || second == nullptr) return false;

	ArmorItem *thisItem = dynamic_cast<ArmorItem *>(first->getItem());
	if(thisItem)
	{
		bool thisIsDyableArmor = thisItem->getMaterial() == ArmorItem::ArmorMaterial::CLOTH;
		bool itemIsDye = second->id == Item::dye_powder_Id;
		return thisIsDyableArmor && itemIsDye;
	}
	// 4J Stu - This condition taken from Recipes::getItemFor to repair items, but added the damaged check to skip when the result is pointless
	else if (first != nullptr && second != nullptr && first->id == second->id && first->count == 1 && second->count == 1 && Item::items[first->id]->canBeDepleted() && (first->isDamaged() || second->isDamaged()) )
	{
		// 4J Stu - Don't allow combinining enchanted items, the enchantment will be lost. They can use the anvil for this
		return !first->isEnchanted() && !second->isEnchanted();
	}
	return false;
}

shared_ptr<ItemInstance> Slot::combine(shared_ptr<ItemInstance> item)
{
	shared_ptr<ItemInstance> result = nullptr;
	shared_ptr<ItemInstance> first = getItem();

	shared_ptr<CraftingContainer> craftSlots = std::make_shared<CraftingContainer>(nullptr, 2, 2);
	craftSlots->setItem(0, item);
	craftSlots->setItem(1, first);

	ArmorItem *thisItem = dynamic_cast<ArmorItem *>(first->getItem());
	if(thisItem)
	{
		result = ArmorDyeRecipe::assembleDyedArmor(craftSlots);
	}
	else
	{
		result = Recipes::getInstance()->getItemFor(craftSlots, nullptr);
	}

	craftSlots->setItem(0, nullptr);
	craftSlots->setItem(1, nullptr);
	return result;
}
