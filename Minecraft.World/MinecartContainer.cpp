#include "stdafx.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.inventory.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.redstone.h"
#include "MinecartContainer.h"

void MinecartContainer::_init()
{
	items = ItemInstanceArray(9 * 4);
	dropEquipment = true;

	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
}

MinecartContainer::MinecartContainer(Level *level) : Minecart(level)
{
	_init();
}

MinecartContainer::MinecartContainer(Level *level, double x, double y, double z) : Minecart(level, x, y, z)
{
	_init();
}

void MinecartContainer::destroy(DamageSource *source)
{
	Minecart::destroy(source);

	for (int i = 0; i < getContainerSize(); i++)
	{
		shared_ptr<ItemInstance> item = getItem(i);
		if (item != NULL)
		{
			float xo = random->nextFloat() * 0.8f + 0.1f;
			float yo = random->nextFloat() * 0.8f + 0.1f;
			float zo = random->nextFloat() * 0.8f + 0.1f;

			while (item->count > 0)
			{
				int count = random->nextInt(21) + 10;
				if (count > item->count) count = item->count;
				item->count -= count;

				shared_ptr<ItemEntity> itemEntity = shared_ptr<ItemEntity>( new ItemEntity(level, x + xo, y + yo, z + zo, shared_ptr<ItemInstance>( new ItemInstance(item->id, count, item->getAuxValue()))) );
				float pow = 0.05f;
				itemEntity->xd = (float) random->nextGaussian() * pow;
				itemEntity->yd = (float) random->nextGaussian() * pow + 0.2f;
				itemEntity->zd = (float) random->nextGaussian() * pow;
				level->addEntity(itemEntity);
			}
		}
	}
}

shared_ptr<ItemInstance> MinecartContainer::getItem(unsigned int slot)
{
	return items[slot];
}

shared_ptr<ItemInstance> MinecartContainer::removeItem(unsigned int slot, int count)
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

shared_ptr<ItemInstance> MinecartContainer::removeItemNoUpdate(int slot)
{
	if (items[slot] != NULL)
	{
		shared_ptr<ItemInstance> item = items[slot];
		items[slot] = nullptr;
		return item;
	}
	return nullptr;
}

void MinecartContainer::setItem(unsigned int slot, shared_ptr<ItemInstance> item)
{
	items[slot] = item;
	if (item != NULL && item->count > getMaxStackSize()) item->count = getMaxStackSize();
}

void MinecartContainer::setChanged()
{
}

bool MinecartContainer::stillValid(shared_ptr<Player> player)
{
	if (removed) return false;
	if (player->distanceToSqr(shared_from_this()) > 8 * 8) return false;
	return true;
}

void MinecartContainer::startOpen()
{
}

void MinecartContainer::stopOpen()
{
}

bool MinecartContainer::canPlaceItem(int slot, shared_ptr<ItemInstance> item)
{
	return true;
}

wstring MinecartContainer::getName()
{
	return hasCustomName() ? getCustomName() : app.GetString(IDS_CONTAINER_MINECART);
}

int MinecartContainer::getMaxStackSize() const
{
	return Container::LARGE_MAX_STACK_SIZE;
}

void MinecartContainer::changeDimension(int i)
{
	dropEquipment = false;
	Minecart::changeDimension(i);
}

void MinecartContainer::remove()
{
	if (dropEquipment)
	{
		for (int i = 0; i < getContainerSize(); i++)
		{
			shared_ptr<ItemInstance> item = getItem(i);
			if (item != NULL)
			{
				float xo = random->nextFloat() * 0.8f + 0.1f;
				float yo = random->nextFloat() * 0.8f + 0.1f;
				float zo = random->nextFloat() * 0.8f + 0.1f;

				while (item->count > 0)
				{
					int count = random->nextInt(21) + 10;
					if (count > item->count) count = item->count;
					item->count -= count;

					shared_ptr<ItemEntity> itemEntity = shared_ptr<ItemEntity>( new ItemEntity(level, x + xo, y + yo, z + zo, shared_ptr<ItemInstance>( new ItemInstance(item->id, count, item->getAuxValue()))));

					if (item->hasTag())
					{
						itemEntity->getItem()->setTag((CompoundTag *) item->getTag()->copy());
					}

					float pow = 0.05f;
					itemEntity->xd = (float) random->nextGaussian() * pow;
					itemEntity->yd = (float) random->nextGaussian() * pow + 0.2f;
					itemEntity->zd = (float) random->nextGaussian() * pow;
					level->addEntity(itemEntity);
				}
			}
		}
	}

	Minecart::remove();
}

void MinecartContainer::addAdditonalSaveData(CompoundTag *base)
{
	Minecart::addAdditonalSaveData(base);

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
}

void MinecartContainer::readAdditionalSaveData(CompoundTag *base)
{
	Minecart::readAdditionalSaveData(base);

	ListTag<CompoundTag> *inventoryList = (ListTag<CompoundTag> *) base->getList(L"Items");
	delete [] items.data;
	items = ItemInstanceArray(getContainerSize());
	for (int i = 0; i < inventoryList->size(); i++)
	{
		CompoundTag *tag = inventoryList->get(i);
		int slot = tag->getByte(L"Slot") & 0xff;
		if (slot >= 0 && slot < items.length) items[slot] = ItemInstance::fromTag(tag);
	}
}

bool MinecartContainer::interact(shared_ptr<Player> player)
{
	if (!level->isClientSide)
	{
		player->openContainer( dynamic_pointer_cast<Container>(shared_from_this()));
	}

	return true;
}

void MinecartContainer::applyNaturalSlowdown()
{
	shared_ptr<Container> container = dynamic_pointer_cast<Container>(shared_from_this());
	int emptiness = Redstone::SIGNAL_MAX - AbstractContainerMenu::getRedstoneSignalFromContainer(container);
	float keep = 0.98f + (emptiness * 0.001f);

	xd *= keep;
	yd *= 0;
	zd *= keep;
}