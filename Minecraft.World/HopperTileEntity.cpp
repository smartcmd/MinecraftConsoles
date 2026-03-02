#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.h"
#include "HopperTileEntity.h"

HopperTileEntity::HopperTileEntity()
{
	items = ItemInstanceArray(5);
	name = L"";
	cooldownTime = -1;
}

HopperTileEntity::~HopperTileEntity()
{
	delete [] items.data;
}

void HopperTileEntity::load(CompoundTag *base)
{
	TileEntity::load(base);

	ListTag<CompoundTag> *inventoryList = (ListTag<CompoundTag> *) base->getList(L"Items");
	delete[] items.data;
	items = ItemInstanceArray(getContainerSize());
	if (base->contains(L"CustomName")) name = base->getString(L"CustomName");
	cooldownTime = base->getInt(L"TransferCooldown");
	for (int i = 0; i < inventoryList->size(); i++)
	{
		CompoundTag *tag = inventoryList->get(i);
		int slot = tag->getByte(L"Slot");
		if (slot >= 0 && slot < items.length) items[slot] = ItemInstance::fromTag(tag);
	}
}

void HopperTileEntity::save(CompoundTag *base)
{
	TileEntity::save(base);
	ListTag<CompoundTag> *listTag = new ListTag<CompoundTag>();

	for (int i = 0; i < items.length; i++)
	{
		if (items[i] != NULL)
		{
			CompoundTag *tag = new CompoundTag();
			tag->putByte(L"Slot", (byte) i);
			items[i]->save(tag);
			listTag->add(tag);
		}
	}
	base->put(L"Items", listTag);
	base->putInt(L"TransferCooldown", cooldownTime);
	if (hasCustomName()) base->putString(L"CustomName", name);
}

void HopperTileEntity::setChanged()
{
	TileEntity::setChanged();
}

unsigned int HopperTileEntity::getContainerSize()
{
	return items.length;
}

shared_ptr<ItemInstance> HopperTileEntity::getItem(unsigned int slot)
{
	return items[slot];
}

shared_ptr<ItemInstance> HopperTileEntity::removeItem(unsigned int slot, int count)
{
	if (items[slot] != NULL)
	{
		if (items[slot]->count <= count)
		{
			shared_ptr<ItemInstance> item = items[slot];
			items[slot] = nullptr;
			return item;
		}
		else
		{
			shared_ptr<ItemInstance> i = items[slot]->remove(count);
			if (items[slot]->count == 0) items[slot] = nullptr;
			return i;
		}
	}
	return nullptr;
}

shared_ptr<ItemInstance> HopperTileEntity::removeItemNoUpdate(int slot)
{
	if (items[slot] != NULL)
	{
		shared_ptr<ItemInstance> item = items[slot];
		items[slot] = nullptr;
		return item;
	}
	return nullptr;
}

void HopperTileEntity::setItem(unsigned int slot, shared_ptr<ItemInstance> item)
{
	items[slot] = item;
	if (item != NULL && item->count > getMaxStackSize()) item->count = getMaxStackSize();
}

wstring HopperTileEntity::getName()
{
	return hasCustomName() ? name : app.GetString(IDS_CONTAINER_HOPPER);
}

wstring HopperTileEntity::getCustomName()
{
	return hasCustomName() ? name : L"";
}

bool HopperTileEntity::hasCustomName()
{
	return !name.empty();
}

void HopperTileEntity::setCustomName(const wstring &name)
{
	this->name = name;
}

int HopperTileEntity::getMaxStackSize() const
{
	return Container::LARGE_MAX_STACK_SIZE;
}

bool HopperTileEntity::stillValid(shared_ptr<Player> player)
{
	if (level->getTileEntity(x, y, z) != shared_from_this()) return false;
	if (player->distanceToSqr(x + 0.5, y + 0.5, z + 0.5) > 8 * 8) return false;
	return true;
}

void HopperTileEntity::startOpen()
{
}

void HopperTileEntity::stopOpen()
{
}

bool HopperTileEntity::canPlaceItem(int slot, shared_ptr<ItemInstance> item)
{
	return true;
}

void HopperTileEntity::tick()
{
	if (level == NULL || level->isClientSide) return;

	cooldownTime--;

	if (!isOnCooldown())
	{
		setCooldown(0);
		tryMoveItems();
	}
}

bool HopperTileEntity::tryMoveItems()
{
	if (level == NULL || level->isClientSide) return false;

	if (!isOnCooldown() && HopperTile::isTurnedOn(getData()))
	{
		bool changed = ejectItems();
		changed = suckInItems(this) || changed;

		if (changed)
		{
			setCooldown(MOVE_ITEM_SPEED);
			setChanged();
			return true;
		}
	}

	return false;
}

bool HopperTileEntity::ejectItems()
{
	shared_ptr<Container> container = getAttachedContainer();
	if (container == NULL)
	{
		return false;
	}

	for (int slot = 0; slot < getContainerSize(); slot++)
	{
		if (getItem(slot) == NULL) continue;

		shared_ptr<ItemInstance> original = getItem(slot)->copy();
		shared_ptr<ItemInstance> result = addItem(container.get(), removeItem(slot, 1), Facing::OPPOSITE_FACING[HopperTile::getAttachedFace(getData())]);

		if (result == NULL || result->count == 0)
		{
			container->setChanged();
			return true;
		}
		else
		{
			setItem(slot, original);
		}
	}

	return false;
}

bool HopperTileEntity::suckInItems(Hopper *hopper)
{
	shared_ptr<Container> container = getSourceContainer(hopper);

	if (container != NULL)
	{
		int face = Facing::DOWN;

		shared_ptr<WorldlyContainer> worldly = dynamic_pointer_cast<WorldlyContainer>(container);
		if ( (worldly != NULL) && (face > -1) )
		{
			intArray slots = worldly->getSlotsForFace(face);

			for (int i = 0; i < slots.length; i++)
			{
				if (tryTakeInItemFromSlot(hopper, container.get(), slots[i], face)) return true;
			}
		}
		else
		{
			int size = container->getContainerSize();
			for (int i = 0; i < size; i++)
			{
				if (tryTakeInItemFromSlot(hopper, container.get(), i, face)) return true;
			}
		}
	}
	else
	{
		shared_ptr<ItemEntity> above = getItemAt(hopper->getLevel(), hopper->getLevelX(), hopper->getLevelY() + 1, hopper->getLevelZ());

		if (above != NULL)
		{
			return addItem(hopper, above);
		}
	}

	return false;
}

bool HopperTileEntity::tryTakeInItemFromSlot(Hopper *hopper, Container *container, int slot, int face)
{
	shared_ptr<ItemInstance> item = container->getItem(slot);

	if (item != NULL && canTakeItemFromContainer(container, item, slot, face))
	{
		shared_ptr<ItemInstance> original = item->copy();
		shared_ptr<ItemInstance> result = addItem(hopper, container->removeItem(slot, 1), -1);

		if (result == NULL || result->count == 0)
		{
			container->setChanged();
			return true;
		}
		else
		{
			container->setItem(slot, original);
		}
	}

	return false;
}

bool HopperTileEntity::addItem(Container *container, shared_ptr<ItemEntity> item)
{
	bool changed = false;
	if (item == NULL) return false;

	shared_ptr<ItemInstance> copy = item->getItem()->copy();
	shared_ptr<ItemInstance> result = addItem(container, copy, -1);

	if (result == NULL || result->count == 0)
	{
		changed = true;

		item->remove();
	}
	else
	{
		item->setItem(result);
	}

	return changed;
}

shared_ptr<ItemInstance> HopperTileEntity::addItem(Container *container, shared_ptr<ItemInstance> item, int face)
{
	if (dynamic_cast<WorldlyContainer *>( container ) != NULL && face > -1)
	{
		WorldlyContainer *worldly = (WorldlyContainer *) container;
		intArray slots = worldly->getSlotsForFace(face);

		for (int i = 0; i < slots.length && item != NULL && item->count > 0; i++)
		{
			item = tryMoveInItem(container, item, slots[i], face);
		}
	}
	else
	{
		int size = container->getContainerSize();
		for (int i = 0; i < size && item != NULL && item->count > 0; i++)
		{
			item = tryMoveInItem(container, item, i, face);
		}
	}

	if (item != NULL && item->count == 0)
	{
		item = nullptr;
	}

	return item;
}

bool HopperTileEntity::canPlaceItemInContainer(Container *container, shared_ptr<ItemInstance> item, int slot, int face)
{
	if (!container->canPlaceItem(slot, item)) return false;
	if ( dynamic_cast<WorldlyContainer *>( container ) != NULL && !dynamic_cast<WorldlyContainer *>( container )->canPlaceItemThroughFace(slot, item, face)) return false;
	return true;
}

bool HopperTileEntity::canTakeItemFromContainer(Container *container, shared_ptr<ItemInstance> item, int slot, int face)
{
	if (dynamic_cast<WorldlyContainer *>( container ) != NULL && !dynamic_cast<WorldlyContainer *>( container )->canTakeItemThroughFace(slot, item, face)) return false;
	return true;
}

shared_ptr<ItemInstance> HopperTileEntity::tryMoveInItem(Container *container, shared_ptr<ItemInstance> item, int slot, int face)
{
	shared_ptr<ItemInstance> current = container->getItem(slot);

	if (canPlaceItemInContainer(container, item, slot, face))
	{
		bool success = false;
		if (current == NULL)
		{
			container->setItem(slot, item);
			item = nullptr;
			success = true;
		}
		else if (canMergeItems(current, item))
		{
			int space = item->getMaxStackSize() - current->count;
			int count = min(item->count, space);

			item->count -= count;
			current->count += count;
			success = count > 0;
		}
		if (success)
		{
			HopperTileEntity *hopper = dynamic_cast<HopperTileEntity *>(container);
			if (hopper != NULL)
			{
				hopper->setCooldown(MOVE_ITEM_SPEED);
				container->setChanged();
			}
			container->setChanged();
		}
	}
	return item;
}

shared_ptr<Container> HopperTileEntity::getAttachedContainer()
{
	int face = HopperTile::getAttachedFace(getData());
	return getContainerAt(getLevel(), x + Facing::STEP_X[face], y + Facing::STEP_Y[face], z + Facing::STEP_Z[face]);
}

shared_ptr<Container> HopperTileEntity::getSourceContainer(Hopper *hopper)
{
	return getContainerAt(hopper->getLevel(), hopper->getLevelX(), hopper->getLevelY() + 1, hopper->getLevelZ());
}

shared_ptr<ItemEntity> HopperTileEntity::getItemAt(Level *level, double xt, double yt, double zt)
{
	vector<shared_ptr<Entity> > *entities = level->getEntitiesOfClass(typeid(ItemEntity), AABB::newTemp(xt, yt, zt, xt + 1, yt + 1, zt + 1), EntitySelector::ENTITY_STILL_ALIVE);

	if (entities->size() > 0)
	{
		shared_ptr<ItemEntity> out = dynamic_pointer_cast<ItemEntity>( entities->at(0) );
		delete entities;
		return out;
	}
	else
	{
		delete entities;
		return nullptr;
	}
}

shared_ptr<Container> HopperTileEntity::getContainerAt(Level *level, double x, double y, double z)
{
	shared_ptr<Container> result = nullptr;

	int xt = Mth::floor(x);
	int yt = Mth::floor(y);
	int zt = Mth::floor(z);

	shared_ptr<TileEntity> entity = level->getTileEntity(xt, yt, zt);

	result = dynamic_pointer_cast<Container>(entity);
	if (result != NULL)
	{
		if ( dynamic_pointer_cast<ChestTileEntity>(result) != NULL )
		{
			int id = level->getTile(xt, yt, zt);
			Tile *tile = Tile::tiles[id];

			if ( dynamic_cast<ChestTile *>( tile ) != NULL )
			{
				result = ((ChestTile *) tile)->getContainer(level, xt, yt, zt);
			}
		}
	}

	if (result == NULL)
	{
		vector<shared_ptr<Entity> > *entities = level->getEntities(nullptr, AABB::newTemp(x, y, z, x + 1, y + 1, z + 1), EntitySelector::CONTAINER_ENTITY_SELECTOR);

		if ( (entities != NULL) && (entities->size() > 0) )
		{
			result = dynamic_pointer_cast<Container>( entities->at( level->random->nextInt(entities->size()) ) );
		}
	}

	return result;
}

bool HopperTileEntity::canMergeItems(shared_ptr<ItemInstance> a, shared_ptr<ItemInstance> b)
{
	if (a->id != b->id) return false;
	if (a->getAuxValue() != b->getAuxValue()) return false;
	if (a->count > a->getMaxStackSize()) return false;
	if (!ItemInstance::tagMatches(a, b)) return false;
	return true;
}

Level *HopperTileEntity::getLevel()
{
	return TileEntity::getLevel();
}

double HopperTileEntity::getLevelX()
{
	return x;
}

double HopperTileEntity::getLevelY()
{
	return y;
}

double HopperTileEntity::getLevelZ()
{
	return z;
}

void HopperTileEntity::setCooldown(int time)
{
	cooldownTime = time;
}

bool HopperTileEntity::isOnCooldown()
{
	return cooldownTime > 0;
}

// 4J Added
shared_ptr<TileEntity> HopperTileEntity::clone()
{
	shared_ptr<HopperTileEntity> result = shared_ptr<HopperTileEntity>( new HopperTileEntity() );
	TileEntity::clone(result);

	result->name = name;
	result->cooldownTime = cooldownTime;
	for (unsigned int i = 0; i < items.length; i++)
	{
		if (items[i] != NULL)
		{
			result->items[i] = ItemInstance::clone(items[i]);
		}
	}
	return result;
}