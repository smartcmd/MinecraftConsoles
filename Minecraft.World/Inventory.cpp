#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.stats.h"
#include "Material.h"
#include "Inventory.h"

const int Inventory::POP_TIME_DURATION = 5;
const int Inventory::MAX_INVENTORY_STACK_SIZE = 64;

const int Inventory::INVENTORY_SIZE = 4 * 9;
const int Inventory::SELECTION_SIZE = 9;

// 4J Stu - The Pllayer is managed by shared_ptrs elsewhere, but it owns us so we don't want to also
// keep a shared_ptr of it. If we pass it on we should use shared_from_this() though
Inventory::Inventory(Player *player)
{
	items = ItemInstanceArray( INVENTORY_SIZE );
	armor = ItemInstanceArray( 4 );

	selected = 0;

	carried = nullptr;

	changed = false;

	this->player = player;
}

Inventory::~Inventory()
{
	delete [] items.data;
	delete [] armor.data;
}

shared_ptr<ItemInstance> Inventory::getSelected()
{
	// sanity checking to prevent exploits
	if (selected < SELECTION_SIZE && selected >= 0)
	{
		return items[selected];
	}
	return nullptr;
}

// 4J-PB - Added for the in-game tooltips
bool Inventory::IsHeldItem()
{
	// sanity checking to prevent exploits
	if (selected < SELECTION_SIZE && selected >= 0)
	{
		if(items[selected])
		{
			return true;
		}
	}
	return false;
}

int Inventory::getSelectionSize()
{
	return SELECTION_SIZE;
}


int Inventory::getSlot(int tileId)
{
	for (unsigned int i = 0; i < items.length; i++)
	{
		if (items[i] != NULL && items[i]->id == tileId) return i;
	}
	return -1;
}

int Inventory::getSlot(int tileId, int data)
{
	for (int i = 0; i < items.length; i++)
	{
		if (items[i] != NULL && items[i]->id == tileId && items[i]->getAuxValue() == data) return i;
	}
	return -1;
}

int Inventory::getSlotWithRemainingSpace(shared_ptr<ItemInstance> item)
{
	for (unsigned int i = 0; i < items.length; i++)
	{
		if (items[i] != NULL && items[i]->id == item->id && items[i]->isStackable()
			&& items[i]->count < items[i]->getMaxStackSize() && items[i]->count < getMaxStackSize()
			&& (!items[i]->isStackedByData() || items[i]->getAuxValue() == item->getAuxValue())
			&& ItemInstance::tagMatches(items[i], item))
		{
			return i;
		}
	}
	return -1;
}

int Inventory::getFreeSlot()
{
	for (unsigned int i = 0; i < items.length; i++)
	{
		if (items[i] == NULL) return i;
	}
	return -1;
}


void Inventory::grabTexture(int id, int data, bool checkData, bool mayReplace)
{
	int slot = -1;
	heldItem = getSelected();
	if (checkData)
	{
		slot = getSlot(id, data);
	}
	else
	{
		slot = getSlot(id);
	}
	if (slot >= 0 && slot < 9)
	{
		selected = slot;
		return;
	}

	if (mayReplace)
	{
		if (id > 0)
		{
			int firstEmpty = getFreeSlot();
			if (firstEmpty >= 0 && firstEmpty < 9)
			{
				selected = firstEmpty;
			}

			replaceSlot(Item::items[id], data);
		}
	}
}

void Inventory::swapPaint(int wheel)
{
	if (wheel > 0) wheel = 1;
	if (wheel < 0) wheel = -1;

	selected -= wheel;

	while (selected < 0)
		selected += 9;
	while (selected >= 9)
		selected -= 9;
}

int Inventory::clearInventory(int id, int data)
{
	int count = 0;
	for (int i = 0; i < items.length; i++)
	{
		shared_ptr<ItemInstance> item = items[i];
		if (item == NULL) continue;
		if (id > -1 && item->id != id) continue;
		if (data > -1 && item->getAuxValue() != data) continue;

		count += item->count;
		items[i] = nullptr;
	}
	for (int i = 0; i < armor.length; i++)
	{
		shared_ptr<ItemInstance> item = armor[i];
		if (item == NULL) continue;
		if (id > -1 && item->id != id) continue;
		if (data > -1 && item->getAuxValue() != data) continue;

		count += item->count;
		armor[i] = nullptr;
	}

	if (carried != NULL)
	{
		if (id > -1 && carried->id != id) return count;
		if (data > -1 && carried->getAuxValue() != data) return count;

		count += carried->count;
		setCarried(nullptr);
	}

	return count;
}

void Inventory::replaceSlot(Item *item, int data)
{
	if (item != NULL)
	{
		// It's too easy to accidentally pick block and lose enchanted items.
		if (heldItem != NULL && heldItem->isEnchantable() && getSlot(heldItem->id, heldItem->getDamageValue()) == selected)
		{
			return;
		}

		int oldSlot = getSlot(item->id, data);
		if (oldSlot >= 0)
		{
			int oldSlotCount = items[oldSlot]->count;
			items[oldSlot] = items[selected];
			items[selected] = shared_ptr<ItemInstance>( new ItemInstance(Item::items[item->id], oldSlotCount, data) );
		}
		else
		{
			items[selected] = shared_ptr<ItemInstance>(new ItemInstance(Item::items[item->id], 1, data));
		}
	}
}


int Inventory::addResource(shared_ptr<ItemInstance> itemInstance)
{

	int type = itemInstance->id;
	int count = itemInstance->count;

	// 4J Stu - Brought forward from 1.2
	if (itemInstance->getMaxStackSize() == 1)
	{
		int slot = getFreeSlot();
		if (slot < 0) return count;
		if (items[slot] == NULL)
		{
			items[slot] = ItemInstance::clone(itemInstance);
			player->handleCollectItem(itemInstance);
		}
		return 0;
	}

	int slot = getSlotWithRemainingSpace(itemInstance);
	if (slot < 0) slot = getFreeSlot();
	if (slot < 0) return count;
	if (items[slot] == NULL)
	{
		items[slot] = shared_ptr<ItemInstance>( new ItemInstance(type, 0, itemInstance->getAuxValue()) );
		// 4J Stu - Brought forward from 1.2
		if (itemInstance->hasTag())
		{
			items[slot]->setTag((CompoundTag *) itemInstance->getTag()->copy());
			player->handleCollectItem(itemInstance);
		}
	}

	int toAdd = count;
	if (toAdd > items[slot]->getMaxStackSize() - items[slot]->count)
	{
		toAdd = items[slot]->getMaxStackSize() - items[slot]->count;
	}
	if (toAdd > getMaxStackSize() - items[slot]->count)
	{
		toAdd = getMaxStackSize() - items[slot]->count;
	}

	if (toAdd == 0) return count;

	count -= toAdd;
	items[slot]->count += toAdd;
	items[slot]->popTime = POP_TIME_DURATION;

	return count;
}


void Inventory::tick()
{
	for (unsigned int i = 0; i < items.length; i++)
	{
		if (items[i] != NULL)
		{
			items[i]->inventoryTick(player->level, player->shared_from_this(), i, selected == i);
		}
	}
}

bool Inventory::removeResource(int type)
{
	int slot = getSlot(type);
	if (slot < 0) return false;
	if (--items[slot]->count <= 0) items[slot] = nullptr;

	return true;
}

bool Inventory::removeResource(int type,int iAuxVal)
{
	int slot = getSlot(type,iAuxVal);
	if (slot < 0) return false;
	if (--items[slot]->count <= 0) items[slot] = nullptr;

	return true;
}

void Inventory::removeResources(shared_ptr<ItemInstance> item)
{
	if(item == NULL) return;

	int countToRemove = item->count;
	for (unsigned int i = 0; i < items.length; i++)
	{
		if (items[i] != NULL && items[i]->sameItemWithTags(item))
		{
			int slotCount = items[i]->count;
			items[i]->count -= countToRemove;
			if(slotCount < countToRemove)
			{
				countToRemove -= slotCount;
			}
			else
			{
				countToRemove = 0;
			}
			if(items[i]->count <= 0) items[i] = nullptr;
		}
	}
}

shared_ptr<ItemInstance> Inventory::getResourceItem(int type)
{
	int slot = getSlot(type);
	if (slot < 0) return nullptr;
	return getItem( slot );
}

shared_ptr<ItemInstance> Inventory::getResourceItem(int type,int iAuxVal)
{
	int slot = getSlot(type,iAuxVal);
	if (slot < 0) return nullptr;
	return getItem( slot );
}

bool Inventory::hasResource(int type)
{
	int slot = getSlot(type);
	if (slot < 0) return false;

	return true;
}

int Inventory::countResource(int type, int auxVal)
{
	int count = 0;
	for (unsigned int i = 0; i < items.length; i++)
	{
		if (items[i] != NULL && items[i]->id == type &&
		    (auxVal == -1 || items[i]->getAuxValue() == auxVal))
			count += items[i]->count;
	}
	return count;
}

void Inventory::swapSlots(int from, int to)
{
	shared_ptr<ItemInstance> tmp = items[to];
	items[to] = items[from];
	items[from] = tmp;
}

bool Inventory::add(shared_ptr<ItemInstance> item)
{
	if (item == NULL) return false;
	if (item->count == 0) return false;

	if (!item->isDamaged())
	{
		int lastSize;
		int count = item->count;
		do
		{
			lastSize = item->count;
			item->count = addResource(item);
		} while (item->count > 0 && item->count < lastSize);
		if (item->count == lastSize && player->abilities.instabuild)
		{
			// silently destroy the item when having a full inventory
			item->count = 0;
			return true;
		}
		if( item->count < lastSize )
		{
			player->awardStat(
				GenericStats::itemsCollected(item->id, item->getAuxValue()),
				GenericStats::param_itemsCollected(item->id, item->getAuxValue(), count)
				);
			return true;
		}
		else
			return false;
	}

	int slot = getFreeSlot();
	if (slot >= 0)
	{
		player->handleCollectItem(item);

		player->awardStat(
			GenericStats::itemsCollected(item->id, item->getAuxValue()),
			GenericStats::param_itemsCollected(item->id, item->getAuxValue(), item->GetCount()));

		items[slot] = ItemInstance::clone(item);
		items[slot]->popTime = POP_TIME_DURATION;
		item->count = 0;
		return true;
	}
	else if (player->abilities.instabuild)
	{
		// silently destroy the item when having a full inventory
		item->count = 0;
		return true;
	}
	return false;
}

shared_ptr<ItemInstance> Inventory::removeItem(unsigned int slot, int count)
{

	ItemInstanceArray pile = items;
	if (slot >= items.length)
	{
		pile = armor;
		slot -= items.length;
	}

	if (pile[slot] != NULL)
	{
		if (pile[slot]->count <= count)
		{
			shared_ptr<ItemInstance> item = pile[slot];
			pile[slot] = nullptr;
			return item;
		}
		else
		{
			shared_ptr<ItemInstance> i = pile[slot]->remove(count);
			if (pile[slot]->count == 0) pile[slot] = nullptr;
			return i;
		}
	}
	return nullptr;
}

shared_ptr<ItemInstance> Inventory::removeItemNoUpdate(int slot)
{
	ItemInstanceArray pile = items;
	if (slot >= items.length)
	{
		pile = armor;
		slot -= items.length;
	}

	if (pile[slot] != NULL)
	{
		shared_ptr<ItemInstance> item = pile[slot];
		pile[slot] = nullptr;
		return item;
	}
	return nullptr;
}

void Inventory::setItem(unsigned int slot, shared_ptr<ItemInstance> item)
{
#ifdef _DEBUG
	if(item!=NULL)
	{
		wstring itemstring=item->toString();
		app.DebugPrintf("Inventory::setItem - slot = %d,\t item = %d ",slot,item->id);
		//OutputDebugStringW(itemstring.c_str());
		app.DebugPrintf("\n");
	}
#else
	if(item!=NULL)
	{
		app.DebugPrintf("Inventory::setItem - slot = %d,\t item = %d, aux = %d\n",slot,item->id,item->getAuxValue());
	}
#endif
	// 4J Stu - Changed this a little from Java to be less funn
	if( slot >= items.length )
	{
		armor[slot - items.length] = item;
	}
	else
	{
		items[slot] = item;
	}
	player->handleCollectItem(item);
	/*
	ItemInstanceArray& pile = items;
	if (slot >= pile.length)
	{
	slot -= pile.length;
	pile = armor;
	}

	pile[slot] = item;
	*/
}

float Inventory::getDestroySpeed(Tile *tile)
{
	float speed = 1.0f;
	if (items[selected] != NULL) speed *= items[selected]->getDestroySpeed(tile);
	return speed;
}

ListTag<CompoundTag> *Inventory::save(ListTag<CompoundTag> *listTag)
{
	for (unsigned int i = 0; i < items.length; i++)
	{
		if (items[i] != NULL)
		{
			CompoundTag *tag = new CompoundTag();
			tag->putByte(L"Slot", (byte) i);
			items[i]->save(tag);
			listTag->add(tag);
		}
	}
	for (unsigned int i = 0; i < armor.length; i++)
	{
		if (armor[i] != NULL)
		{
			CompoundTag *tag = new CompoundTag();
			tag->putByte(L"Slot", (byte) (i + 100));
			armor[i]->save(tag);
			listTag->add(tag);
		}
	}
	return listTag;
}

void Inventory::load(ListTag<CompoundTag> *inventoryList)
{
	if( items.data != NULL)
	{
		delete[] items.data;
		items.data = NULL;
	}
	if( armor.data != NULL)
	{
		delete[] armor.data;
		armor.data = NULL;

	}
	items = ItemInstanceArray( INVENTORY_SIZE );
	armor = ItemInstanceArray( 4 );
	for (int i = 0; i < inventoryList->size(); i++)
	{
		CompoundTag *tag = inventoryList->get(i);
		unsigned int slot = tag->getByte(L"Slot") & 0xff;
		shared_ptr<ItemInstance> item = shared_ptr<ItemInstance>( ItemInstance::fromTag(tag) );
		if (item != NULL)
		{
			if (slot >= 0 && slot < items.length) items[slot] = item;
			if (slot >= 100 && slot < armor.length + 100) armor[slot - 100] = item;
		}
	}
}

unsigned int Inventory::getContainerSize()
{
	return items.length + 4;
}

shared_ptr<ItemInstance> Inventory::getItem(unsigned int slot)
{
	// 4J Stu - Changed this a little from the Java so it's less funny
	if( slot >= items.length )
	{
		return armor[ slot - items.length ];
	}
	else
	{
		return items[ slot ];
	}
	/*
	ItemInstanceArray pile = items;
	if (slot >= pile.length)
	{
	slot -= pile.length;
	pile = armor;
	}

	return pile[slot];
	*/
}

wstring Inventory::getName()
{
	return app.GetString(IDS_INVENTORY);
}

wstring Inventory::getCustomName()
{
	return L"";
}

bool Inventory::hasCustomName()
{
	return false;
}

int Inventory::getMaxStackSize() const
{
	return MAX_INVENTORY_STACK_SIZE;
}

bool Inventory::canDestroy(Tile *tile)
{
	if (tile->material->isAlwaysDestroyable()) return true;

	shared_ptr<ItemInstance> item = getItem(selected);
	if (item != NULL) return item->canDestroySpecial(tile);
	return false;
}

shared_ptr<ItemInstance> Inventory::getArmor(int layer)
{
	return armor[layer];
}

int Inventory::getArmorValue()
{
	int val = 0;
	for (unsigned int i = 0; i < armor.length; i++)
	{
		if (armor[i] != NULL &&  dynamic_cast<ArmorItem *>( armor[i]->getItem() ) != NULL )
		{
			int baseProtection = dynamic_cast<ArmorItem *>(armor[i]->getItem())->defense;

			val += baseProtection;
		}
	}
	return val;
}

void Inventory::hurtArmor(float dmg)
{
	dmg = dmg / 4;
	if (dmg < 1)
	{
		dmg = 1;
	}
	for (unsigned int i = 0; i < armor.length; i++)
	{
		if (armor[i] != NULL && dynamic_cast<ArmorItem *>( armor[i]->getItem() ) != NULL )
		{
			armor[i]->hurtAndBreak( (int) dmg, dynamic_pointer_cast<LivingEntity>( player->shared_from_this() ) );
			if (armor[i]->count == 0)
			{
				armor[i] = nullptr;
			}
		}
	}
}

void Inventory::dropAll()
{
	for (unsigned int i = 0; i < items.length; i++)
	{
		if (items[i] != NULL)
		{
			player->drop(items[i], true);
			items[i] = nullptr;
		}
	}
	for (unsigned int i = 0; i < armor.length; i++)
	{
		if (armor[i] != NULL)
		{
			player->drop(armor[i], true);
			armor[i] = nullptr;
		}
	}
}

void Inventory::setChanged()
{
	changed = true;
}

bool Inventory::isSame(shared_ptr<Inventory> copy)
{
	for (unsigned int i = 0; i < items.length; i++)
	{
		if (!isSame( copy->items[i], items[i])) return false;
	}
	for (unsigned int i = 0; i < armor.length; i++)
	{
		if (!isSame( copy->armor[i], armor[i])) return false;
	}
	return true;
}


bool Inventory::isSame(shared_ptr<ItemInstance> a, shared_ptr<ItemInstance> b)
{
	if (a == NULL && b == NULL) return true;
	if (a == NULL || b == NULL) return false;

	return a->id == b->id && a->count == b->count && a->getAuxValue() == b->getAuxValue();
}


shared_ptr<Inventory> Inventory::copy()
{
	shared_ptr<Inventory> copy = shared_ptr<Inventory>( new Inventory(NULL) );
	for (unsigned int i = 0; i < items.length; i++)
	{
		copy->items[i] = items[i] != NULL ? items[i]->copy() : nullptr;
	}
	for (unsigned int i = 0; i < armor.length; i++)
	{
		copy->armor[i] = armor[i] != NULL ? armor[i]->copy() : nullptr;
	}
	return copy;
}

void Inventory::setCarried(shared_ptr<ItemInstance> carried)
{
	this->carried = carried;
	player->handleCollectItem(carried);
}

shared_ptr<ItemInstance> Inventory::getCarried()
{
	return carried;
}

bool Inventory::stillValid(shared_ptr<Player> player)
{
	if (this->player->removed) return false;
	if (player->distanceToSqr(this->player->shared_from_this()) > 8 * 8) return false;
	return true;
}

bool Inventory::contains(shared_ptr<ItemInstance> itemInstance)
{
	for (unsigned int i = 0; i < armor.length; i++)
	{
		if (armor[i] != NULL && armor[i]->sameItem(itemInstance)) return true;
	}
	for (unsigned int i = 0; i < items.length; i++)
	{
		if (items[i] != NULL && items[i]->sameItem(itemInstance)) return true;
	}
	return false;
}

void Inventory::startOpen()
{
	// TODO Auto-generated method stub
}

void Inventory::stopOpen()
{
	// TODO Auto-generated method stub
}

bool Inventory::canPlaceItem(int slot, shared_ptr<ItemInstance> item)
{
	return true;
}

void Inventory::replaceWith(shared_ptr<Inventory> other)
{
	for (int i = 0; i < items.length; i++)
	{
		items[i] = ItemInstance::clone(other->items[i]);
	}
	for (int i = 0; i < armor.length; i++)
	{
		armor[i] = ItemInstance::clone(other->armor[i]);
	}

	selected = other->selected;
}

int Inventory::countMatches(shared_ptr<ItemInstance> itemInstance)
{
	if(itemInstance == NULL) return 0;
	int count = 0;
	//for (unsigned int i = 0; i < armor.length; i++)
	//{
	//	if (armor[i] != NULL && armor[i]->sameItem(itemInstance)) count += items[i]->count;
	//}
	for (unsigned int i = 0; i < items.length; i++)
	{
		if (items[i] != NULL && items[i]->sameItemWithTags(itemInstance)) count += items[i]->count;
	}
	return count;
}
