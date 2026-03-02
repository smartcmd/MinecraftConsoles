#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.crafting.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "Material.h"
#include "FurnaceTileEntity.h"



const int FurnaceTileEntity::BURN_INTERVAL = 10 * 20;

// 4J Stu - Need a ctor to initialise member variables
FurnaceTileEntity::FurnaceTileEntity() : TileEntity()
{
	items = new ItemInstanceArray(3);

	litTime = 0;

	litDuration = 0;

	tickCount = 0;

	m_charcoalUsed = false;
}

FurnaceTileEntity::~FurnaceTileEntity()
{
	delete[] items->data;
	delete items;
}


unsigned int FurnaceTileEntity::getContainerSize()
{
	return items->length;
}


shared_ptr<ItemInstance> FurnaceTileEntity::getItem(unsigned int slot)
{
	return (*items)[slot];
}


shared_ptr<ItemInstance> FurnaceTileEntity::removeItem(unsigned int slot, int count)
{
	m_charcoalUsed = false;

	if ((*items)[slot] != NULL)
	{
		if ((*items)[slot]->count <= count)
		{
			shared_ptr<ItemInstance> item = (*items)[slot];
			(*items)[slot] = nullptr;
			// 4J Stu - Fix for duplication glitch
			if(item->count <= 0) return nullptr;
			return item;
		}
		else
		{
			shared_ptr<ItemInstance> i = (*items)[slot]->remove(count);
			if ((*items)[slot]->count == 0) (*items)[slot] = nullptr;
			// 4J Stu - Fix for duplication glitch
			if(i->count <= 0) return nullptr;
			return i;
		}
	}
	return nullptr;
}

shared_ptr<ItemInstance> FurnaceTileEntity::removeItemNoUpdate(int slot)
{
	m_charcoalUsed = false;

	if (items->data[slot] != NULL)
	{
		shared_ptr<ItemInstance> item = items->data[slot];
		items->data[slot] = nullptr;
		return item;
	}
	return nullptr;
}


void FurnaceTileEntity::setItem(unsigned int slot, shared_ptr<ItemInstance> item)
{
	(*items)[slot] = item;
	if (item != NULL && item->count > getMaxStackSize()) item->count = getMaxStackSize();
}


int FurnaceTileEntity::getName()
{
	return IDS_TILE_FURNACE;
}

void FurnaceTileEntity::load(CompoundTag *base)
{
	TileEntity::load(base);
	ListTag<CompoundTag> *inventoryList = (ListTag<CompoundTag> *) base->getList(L"Items");
	items = new ItemInstanceArray(getContainerSize());
	for (int i = 0; i < inventoryList->size(); i++)
	{
		CompoundTag *tag = inventoryList->get(i);
		unsigned int slot = tag->getByte(L"Slot");
		if (slot >= 0 && slot < items->length) (*items)[slot] = ItemInstance::fromTag(tag);
	}

	litTime = base->getShort(L"BurnTime");
	tickCount = base->getShort(L"CookTime");
	litDuration = getBurnDuration((*items)[FUEL_SLOT]);

	m_charcoalUsed = base->getBoolean(L"CharcoalUsed");
}


void FurnaceTileEntity::save(CompoundTag *base)
{
	TileEntity::save(base);
	base->putShort(L"BurnTime", (short) (litTime));
	base->putShort(L"CookTime", (short) (tickCount));
	ListTag<CompoundTag> *listTag = new ListTag<CompoundTag>();

	for (unsigned int i = 0; i < items->length; i++)
	{
		if ((*items)[i] != NULL)
		{
			CompoundTag *tag = new CompoundTag();
			tag->putByte(L"Slot", (byte) i);
			(*items)[i]->save(tag);
			listTag->add(tag);
		}
	}
	base->put(L"Items", listTag);

	base->putBoolean(L"CharcoalUsed", m_charcoalUsed);
}


int FurnaceTileEntity::getMaxStackSize()
{
	return Container::LARGE_MAX_STACK_SIZE;
}


int FurnaceTileEntity::getBurnProgress(int max)
{
	return tickCount * max / BURN_INTERVAL;
}


int FurnaceTileEntity::getLitProgress(int max)
{
	if (litDuration == 0) litDuration = BURN_INTERVAL;
	return litTime * max / litDuration;
}


bool FurnaceTileEntity::isLit()
{
	return litTime > 0;
}


void FurnaceTileEntity::tick()
{
	bool wasLit = litTime > 0;
	bool changed = false;
	if (litTime > 0)
	{
		litTime--;
	}

	if ( level != NULL && !level->isClientSide)
	{
		if (litTime == 0 && canBurn())
		{
			litDuration = litTime = getBurnDuration((*items)[FUEL_SLOT]);
			if (litTime > 0)
			{
				changed = true;
				if ((*items)[FUEL_SLOT] != NULL)
				{
					// 4J Added: Keep track of whether charcoal was used in production of current stack.
					if ( (*items)[FUEL_SLOT]->getItem()->id == Item::coal_Id 
						&& (*items)[FUEL_SLOT]->getAuxValue() == CoalItem::CHAR_COAL)
					{
						m_charcoalUsed = true;
					}

					(*items)[FUEL_SLOT]->count--;
					if ((*items)[FUEL_SLOT]->count == 0)
					{
						Item *remaining = (*items)[FUEL_SLOT]->getItem()->getCraftingRemainingItem();
						(*items)[FUEL_SLOT] = remaining != NULL ? shared_ptr<ItemInstance>(new ItemInstance(remaining)) : nullptr;
					}
				}
			}
		}

		if (isLit() && canBurn())
		{
			tickCount++;
			if (tickCount == BURN_INTERVAL)
			{
				tickCount = 0;
				burn();
				changed = true;
			}
		}
		else
		{
			tickCount = 0;
		}

		if (wasLit != litTime > 0)
		{
			changed = true;
			FurnaceTile::setLit(litTime > 0, level, x, y, z);
		}
	}

	if (changed) setChanged();
}


bool FurnaceTileEntity::canBurn()
{
	if ((*items)[INPUT_SLOT] == NULL) return false;
	ItemInstance *burnResult = FurnaceRecipes::getInstance()->getResult((*items)[0]->getItem()->id);
	if (burnResult == NULL) return false;
	if ((*items)[RESULT_SLOT] == NULL) return true;
	if (!(*items)[RESULT_SLOT]->sameItem_not_shared(burnResult)) return false;
	if ((*items)[RESULT_SLOT]->count < getMaxStackSize() && (*items)[RESULT_SLOT]->count < (*items)[RESULT_SLOT]->getMaxStackSize()) return true;
	if ((*items)[RESULT_SLOT]->count < burnResult->getMaxStackSize()) return true;
	return false;
}


void FurnaceTileEntity::burn()
{
	if (!canBurn()) return;

	ItemInstance *result = FurnaceRecipes::getInstance()->getResult((*items)[0]->getItem()->id);
	if ((*items)[RESULT_SLOT] == NULL) (*items)[RESULT_SLOT] = result->copy();
	else if ((*items)[RESULT_SLOT]->id == result->id) (*items)[RESULT_SLOT]->count++;

	(*items)[INPUT_SLOT]->count--;
	if ((*items)[INPUT_SLOT]->count <= 0) (*items)[INPUT_SLOT] = nullptr;
}


int FurnaceTileEntity::getBurnDuration(shared_ptr<ItemInstance> itemInstance)
{
	if (itemInstance == NULL) return 0;
	int id = itemInstance->getItem()->id;

	Item *item = itemInstance->getItem();

	if (id < 256 && Tile::tiles[id] != NULL)
	{
		Tile *tile = Tile::tiles[id];

		if (tile == Tile::woodSlabHalf)
		{
			return BURN_INTERVAL * 3 / 4;
		}

		if (tile->material == Material::wood)
		{
			return BURN_INTERVAL * 3 / 2;
		}
	}

	if (dynamic_cast<DiggerItem *>(item) && ((DiggerItem *) item)->getTier() == Item::Tier::WOOD)
	{
		return BURN_INTERVAL;
	}
	else if (dynamic_cast<WeaponItem *>(item) && ((WeaponItem *) item)->getTier() == Item::Tier::WOOD)
	{
		return BURN_INTERVAL;
	}
	else if (dynamic_cast<HoeItem *>(item) && ((HoeItem *) item)->getTier() == Item::Tier::WOOD)
	{
		return BURN_INTERVAL;
	}

	if (id == Item::stick->id)
	{
		return BURN_INTERVAL / 2;
	}

	if (id == Item::coal->id) return BURN_INTERVAL * 8;

	if (id == Item::bucket_lava->id) return BURN_INTERVAL * 100;

	if (id == Tile::sapling_Id) return BURN_INTERVAL / 2;

	if (id == Item::blazeRod_Id) return BURN_INTERVAL * 12;

	return 0;
}

bool FurnaceTileEntity::isFuel(shared_ptr<ItemInstance> item)
{
	return getBurnDuration(item) > 0;
}

bool FurnaceTileEntity::stillValid(shared_ptr<Player> player)
{
	if (level->getTileEntity(x, y, z) != shared_from_this() ) return false;
	if (player->distanceToSqr(x + 0.5, y + 0.5, z + 0.5) > 8 * 8) return false;
	return true;
}

void FurnaceTileEntity::setChanged()
{
	return TileEntity::setChanged();
}

void FurnaceTileEntity::startOpen()
{
}

void FurnaceTileEntity::stopOpen()
{
}

// 4J Added
shared_ptr<TileEntity> FurnaceTileEntity::clone()
{
	shared_ptr<FurnaceTileEntity> result = shared_ptr<FurnaceTileEntity>( new FurnaceTileEntity() );
	TileEntity::clone(result);

	result->litTime = litTime;
	result->tickCount = tickCount;
	result->litDuration = litDuration;

	for (unsigned int i = 0; i < items->length; i++)
	{
		if ((*items)[i] != NULL)
		{
			(*result->items)[i] = ItemInstance::clone((*items)[i]);
		}
	}
	return result;
}