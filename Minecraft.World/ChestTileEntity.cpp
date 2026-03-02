using namespace std;

#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.level.h"
#include "TileEntity.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.h"
#include "ChestTileEntity.h"
#include "SoundTypes.h"



ChestTileEntity::ChestTileEntity(bool isBonusChest/* = false*/) : TileEntity()
{
	items = new ItemInstanceArray(9 * 4);

	hasCheckedNeighbors = false;
	this->isBonusChest = isBonusChest;

	openness = 0.0f;
	oOpenness = 0.0f;
	openCount = 0;
	tickInterval = 0;
}

ChestTileEntity::~ChestTileEntity()
{
	delete[] items->data;
	delete items;
}

unsigned int ChestTileEntity::getContainerSize() 
{
	return 9 * 3;
}

shared_ptr<ItemInstance> ChestTileEntity::getItem(unsigned int slot)
{
	return items->data[slot];
}

shared_ptr<ItemInstance> ChestTileEntity::removeItem(unsigned int slot, int count) 
{
	if (items->data[slot] != NULL)
	{
		if (items->data[slot]->count <= count) 
		{
			shared_ptr<ItemInstance> item = items->data[slot];
			items->data[slot] = nullptr;
			this->setChanged();
			// 4J Stu - Fix for duplication glitch
			if(item->count <= 0) return nullptr;
			return item;
		} 
		else 
		{
			shared_ptr<ItemInstance> i = items->data[slot]->remove(count);
			if (items->data[slot]->count == 0) items->data[slot] = nullptr;
			this->setChanged();
			// 4J Stu - Fix for duplication glitch
			if(i->count <= 0) return nullptr;
			return i;
		}
	}
	return nullptr;
}

shared_ptr<ItemInstance> ChestTileEntity::removeItemNoUpdate(int slot)
{
	if (items->data[slot] != NULL)
	{
		shared_ptr<ItemInstance> item = items->data[slot];
		items->data[slot] = nullptr;
		return item;
	}
	return nullptr;
}

void ChestTileEntity::setItem(unsigned int slot, shared_ptr<ItemInstance> item)
{
	items->data[slot] = item;
	if (item != NULL && item->count > getMaxStackSize()) item->count = getMaxStackSize();
	this->setChanged();
}

int ChestTileEntity::getName()
{
	return IDS_TILE_CHEST;
}


void ChestTileEntity::load(CompoundTag *base)
{
	TileEntity::load(base);
	ListTag<CompoundTag> *inventoryList = (ListTag<CompoundTag> *) base->getList(L"Items");
	if( items )
	{
		delete [] items->data;
		delete items;
	}
	items = new ItemInstanceArray(getContainerSize());
	for (int i = 0; i < inventoryList->size(); i++)
	{
		CompoundTag *tag = inventoryList->get(i);
		unsigned int slot = tag->getByte(L"Slot") & 0xff;
		if (slot >= 0 && slot < items->length) (*items)[slot] = ItemInstance::fromTag(tag);
	}
	isBonusChest = base->getBoolean(L"bonus");
}

void ChestTileEntity::save(CompoundTag *base)
{
	TileEntity::save(base);
	ListTag<CompoundTag> *listTag = new ListTag<CompoundTag>;

	for (unsigned int i = 0; i < items->length; i++)
	{
		if (items->data[i] != NULL) 
		{
			CompoundTag *tag = new CompoundTag();
			tag->putByte(L"Slot", (byte) i);
			items->data[i]->save(tag);
			listTag->add(tag);
		}
	}
	base->put(L"Items", listTag);
	base->putBoolean(L"bonus", isBonusChest);
}

int ChestTileEntity::getMaxStackSize()
{
	return Container::LARGE_MAX_STACK_SIZE;
}

bool ChestTileEntity::stillValid(shared_ptr<Player> player)
{
	if (level->getTileEntity(x, y, z) != shared_from_this() ) return false;
	if (player->distanceToSqr(x + 0.5, y + 0.5, z + 0.5) > 8 * 8) return false;
	return true;
}

void ChestTileEntity::setChanged() 
{
	TileEntity::setChanged();
}

void ChestTileEntity::clearCache()
{
    TileEntity::clearCache();
    hasCheckedNeighbors = false;
}

void ChestTileEntity::checkNeighbors()
{
    if (hasCheckedNeighbors) return;

    hasCheckedNeighbors = true;
    n = weak_ptr<ChestTileEntity>();
    e = weak_ptr<ChestTileEntity>();
    w = weak_ptr<ChestTileEntity>();
    s = weak_ptr<ChestTileEntity>();

    if (level->getTile(x - 1, y, z) == Tile::chest_Id)
	{
        w = dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(x - 1, y, z));
    }
    if (level->getTile(x + 1, y, z) == Tile::chest_Id)
	{
        e = dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(x + 1, y, z));
    }
    if (level->getTile(x, y, z - 1) == Tile::chest_Id)
	{
        n = dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(x, y, z - 1));
    }
    if (level->getTile(x, y, z + 1) == Tile::chest_Id)
	{
        s = dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(x, y, z + 1));
    }

    if (n.lock() != NULL) n.lock()->clearCache();
    if (s.lock() != NULL) s.lock()->clearCache();
    if (e.lock() != NULL) e.lock()->clearCache();
    if (w.lock() != NULL) w.lock()->clearCache();
}

void ChestTileEntity::tick()
{
    TileEntity::tick();
    checkNeighbors();

    if (++tickInterval % 20 * 4 == 0)
	{
        //level->tileEvent(x, y, z, ChestTile::EVENT_SET_OPEN_COUNT, openCount);
    }

    oOpenness = openness;

    float speed = 0.10f;
    if (openCount > 0 && openness == 0)
	{
        if (n.lock() == NULL && w.lock() == NULL)
		{
            double xc = x + 0.5;
            double zc = z + 0.5;
            if (s.lock() != NULL) zc += 0.5;
            if (e.lock() != NULL) xc += 0.5;

			// 4J-PB - Seems the chest open volume is much louder than other sounds from user reports. We'll tone it down a bit		
            level->playSound(xc, y + 0.5, zc, eSoundType_RANDOM_CHEST_OPEN, 0.2f, level->random->nextFloat() * 0.1f + 0.9f);
        }
    }
    if ((openCount == 0 && openness > 0) || (openCount > 0 && openness < 1))
	{
		float oldOpen = openness;
        if (openCount > 0) openness += speed;
        else openness -= speed;
        if (openness > 1)
		{
            openness = 1;
        }
		float lim = 0.5f;
        if (openness < lim && oldOpen >= lim)
		{
			// Fix for #64546 - Customer Encountered: TU7: Chests placed by the Player are closing too fast.
            //openness = 0;
            if (n.lock() == NULL && w.lock() == NULL)
			{
                double xc = x + 0.5;
                double zc = z + 0.5;
                if (s.lock() != NULL) zc += 0.5;
                if (e.lock() != NULL) xc += 0.5;

				// 4J-PB - Seems the chest open volume is much louder than other sounds from user reports. We'll tone it down a bit	
                level->playSound(xc, y + 0.5, zc, eSoundType_RANDOM_CHEST_CLOSE, 0.2f, level->random->nextFloat() * 0.1f + 0.9f);
            }
		}
        if (openness < 0)
		{
            openness = 0;
        }
    }

}

void ChestTileEntity::triggerEvent(int b0, int b1)
{
    if (b0 == ChestTile::EVENT_SET_OPEN_COUNT)
	{
        openCount = b1;
    }
}

void ChestTileEntity::startOpen()
{
	openCount++;
	level->tileEvent(x, y, z, Tile::chest_Id, ChestTile::EVENT_SET_OPEN_COUNT, openCount);
}

void ChestTileEntity::stopOpen()
{
	openCount--;
	level->tileEvent(x, y, z, Tile::chest_Id, ChestTile::EVENT_SET_OPEN_COUNT, openCount);
}

void ChestTileEntity::setRemoved()
{
    clearCache();
    checkNeighbors();
    TileEntity::setRemoved();
}

// 4J Added
shared_ptr<TileEntity> ChestTileEntity::clone()
{
	shared_ptr<ChestTileEntity> result = shared_ptr<ChestTileEntity>( new ChestTileEntity() );
	TileEntity::clone(result);

	for (unsigned int i = 0; i < items->length; i++)
	{
		if (items->data[i] != NULL)
		{
			result->items->data[i] = ItemInstance::clone(items->data[i]);
		}
	}
	return result;
}