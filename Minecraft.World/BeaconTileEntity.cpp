#include "stdafx.h"
#include "net.minecraft.network.packet.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "BeaconTileEntity.h"

shared_ptr<TileEntity> BeaconTileEntity::clone()
{
	shared_ptr<BeaconTileEntity> result = shared_ptr<BeaconTileEntity>( new BeaconTileEntity() );
	TileEntity::clone(result);

	result->primaryPower = primaryPower;
	result->secondaryPower = secondaryPower;
	result->levels = levels;

	return result;
}

MobEffect *BeaconTileEntity::BEACON_EFFECTS[BeaconTileEntity::BEACON_EFFECTS_TIERS][BeaconTileEntity::BEACON_EFFECTS_EFFECTS];

void BeaconTileEntity::staticCtor()
{
	for(unsigned int tier = 0; tier < BEACON_EFFECTS_TIERS; ++tier)
	{
		for(unsigned int effect = 0; effect < BEACON_EFFECTS_EFFECTS; ++effect)
		{
			BEACON_EFFECTS[tier][effect] = NULL;
		}
	}
	BEACON_EFFECTS[0][0] = MobEffect::movementSpeed;
	BEACON_EFFECTS[0][1] = MobEffect::digSpeed;
	BEACON_EFFECTS[1][0] = MobEffect::damageResistance;
	BEACON_EFFECTS[1][1] = MobEffect::jump;
	BEACON_EFFECTS[2][0] = MobEffect::damageBoost;
	BEACON_EFFECTS[3][0] = MobEffect::regeneration;
}

BeaconTileEntity::BeaconTileEntity()
{
	clientSideRenderTick = 0;
	clientSideRenderScale = 0.0f;

	isActive = false;
	levels = -1;

	primaryPower = 0;
	secondaryPower = 0;

	paymentItem = nullptr;
	name = L"";
}

void BeaconTileEntity::tick()
{
	// 4J Stu - Added levels check to force an initial tick
	if ( (!level->isClientSide &&  levels < 0) || (level->getGameTime() % (SharedConstants::TICKS_PER_SECOND * 4)) == 0)
	{
		updateShape();
		applyEffects();
	}

}

void BeaconTileEntity::applyEffects()
{
	if (isActive && levels > 0 && !level->isClientSide && primaryPower > 0)
	{

		double range = (levels * 10) + 10;
		int baseAmp = 0;
		if (levels >= 4 && primaryPower == secondaryPower)
		{
			baseAmp = 1;
		}

		AABB *bb = AABB::newTemp(x, y, z, x + 1, y + 1, z + 1)->grow(range, range, range);
		bb->y1 = level->getMaxBuildHeight();
		vector<shared_ptr<Entity> > *players = level->getEntitiesOfClass(typeid(Player), bb);
		for (AUTO_VAR(it,players->begin()); it != players->end(); ++it)
		{
			shared_ptr<Player> player = dynamic_pointer_cast<Player>(*it);
			player->addEffect(new MobEffectInstance(primaryPower, SharedConstants::TICKS_PER_SECOND * 9, baseAmp, true));
		}

		if (levels >= 4 && primaryPower != secondaryPower && secondaryPower > 0)
		{
			for (AUTO_VAR(it,players->begin()); it != players->end(); ++it)
			{
				shared_ptr<Player> player = dynamic_pointer_cast<Player>(*it);
				player->addEffect(new MobEffectInstance(secondaryPower, SharedConstants::TICKS_PER_SECOND * 9, 0, true));
			}
		}
		delete players;
	}
}

void BeaconTileEntity::updateShape()
{

	if (!level->canSeeSky(x, y + 1, z))
	{
		isActive = false;
		levels = 0;
	}
	else
	{
		isActive = true;

		levels = 0;
		for (int step = 1; step <= 4; step++)
		{

			int ly = y - step;
			if (ly < 0)
			{
				break;
			}

			bool isOk = true;
			for (int lx = x - step; lx <= x + step && isOk; lx++)
			{
				for (int lz = z - step; lz <= z + step; lz++)
				{
					int tile = level->getTile(lx, ly, lz);
					if (tile != Tile::emeraldBlock_Id && tile != Tile::goldBlock_Id && tile != Tile::diamondBlock_Id && tile != Tile::ironBlock_Id)
					{
						isOk = false;
						break;
					}
				}
			}
			if (isOk)
			{
				levels = step;
			}
			else
			{
				break;
			}
		}
		if (levels == 0)
		{
			isActive = false;
		}
	}

}

float BeaconTileEntity::getAndUpdateClientSideScale()
{

	if (!isActive)
	{
		return 0;
	}

	int renderDelta = (int) (level->getGameTime() - clientSideRenderTick);
	clientSideRenderTick = level->getGameTime();
	if (renderDelta > 1)
	{
		clientSideRenderScale -= ((float) renderDelta / (float) SCALE_TIME);

		if (clientSideRenderScale < 0)
		{
			clientSideRenderScale = 0;
		}
	}
	clientSideRenderScale += (1.0f / (float) SCALE_TIME);
	if (clientSideRenderScale > 1)
	{
		clientSideRenderScale = 1;
	}
	return clientSideRenderScale;
}

int BeaconTileEntity::getPrimaryPower()
{
	return primaryPower;
}

int BeaconTileEntity::getSecondaryPower()
{
	return secondaryPower;
}

int BeaconTileEntity::getLevels()
{
	return levels;
}

// client-side method used by GUI
void BeaconTileEntity::setLevels(int levels)
{
	this->levels = levels;
}

void BeaconTileEntity::setPrimaryPower(int primaryPower)
{
	this->primaryPower = 0;

	// verify power
	for (int tier = 0; tier < levels && tier < 3; tier++)
	{
		for(unsigned int e = 0; e < BEACON_EFFECTS_EFFECTS; ++e)
		{
			MobEffect *effect = BEACON_EFFECTS[tier][e];
			if(effect == NULL) break;

			if (effect->id == primaryPower)
			{
				this->primaryPower = primaryPower;
				return;
			}
		}
	}
}

void BeaconTileEntity::setSecondaryPower(int secondaryPower)
{
	this->secondaryPower = 0;

	// verify power
	if (levels >= 4)
	{
		for (int tier = 0; tier < 4; tier++)
		{
			for(unsigned int e = 0; e < BEACON_EFFECTS_EFFECTS; ++e)
			{
				MobEffect *effect = BEACON_EFFECTS[tier][e];
				if(effect == NULL) break;

				if (effect->id == secondaryPower)
				{
					this->secondaryPower = secondaryPower;
					return;
				}
			}
		}
	}
}

shared_ptr<Packet> BeaconTileEntity::getUpdatePacket()
{
	CompoundTag *tag = new CompoundTag();
	save(tag);
	return shared_ptr<TileEntityDataPacket>( new TileEntityDataPacket(x, y, z, TileEntityDataPacket::TYPE_BEACON, tag) );
}

double BeaconTileEntity::getViewDistance()
{
	return 256 * 256;
}

void BeaconTileEntity::load(CompoundTag *tag)
{
	TileEntity::load(tag);

	primaryPower = tag->getInt(L"Primary");
	secondaryPower = tag->getInt(L"Secondary");
	levels = tag->getInt(L"Levels");
}

void BeaconTileEntity::save(CompoundTag *tag)
{
	TileEntity::save(tag);

	tag->putInt(L"Primary", primaryPower);
	tag->putInt(L"Secondary", secondaryPower);
	// this value is re-calculated, but save it anyway to avoid update lag
	tag->putInt(L"Levels", levels);
}

unsigned int BeaconTileEntity::getContainerSize()
{
	return 1;
}

shared_ptr<ItemInstance> BeaconTileEntity::getItem(unsigned int slot)
{
	if (slot == 0)
	{
		return paymentItem;
	}
	return nullptr;
}

shared_ptr<ItemInstance> BeaconTileEntity::removeItem(unsigned int slot, int count)
{
	if (slot == 0 && paymentItem != NULL)
	{
		if (count >= paymentItem->count)
		{
			shared_ptr<ItemInstance> returnItem = paymentItem;
			paymentItem = nullptr;
			return returnItem;
		}
		else
		{
			paymentItem->count -= count;
			return shared_ptr<ItemInstance>( new ItemInstance(paymentItem->id, count, paymentItem->getAuxValue()) );
		}
	}
	return nullptr;
}

shared_ptr<ItemInstance> BeaconTileEntity::removeItemNoUpdate(int slot)
{
	if (slot == 0 && paymentItem != NULL)
	{
		shared_ptr<ItemInstance> returnItem = paymentItem;
		paymentItem = nullptr;
		return returnItem;
	}
	return nullptr;
}

void BeaconTileEntity::setItem(unsigned int slot, shared_ptr<ItemInstance> item)
{
	if (slot == 0)
	{
		paymentItem = item;
	}
}

wstring BeaconTileEntity::getName()
{
	return hasCustomName() ? name : app.GetString(IDS_CONTAINER_BEACON);
}

wstring BeaconTileEntity::getCustomName()
{
	return hasCustomName() ? name : L"";
}

bool BeaconTileEntity::hasCustomName()
{
	return !name.empty();
}

void BeaconTileEntity::setCustomName(const wstring &name)
{
	this->name = name;
}

int BeaconTileEntity::getMaxStackSize() const
{
	return 1;
}

bool BeaconTileEntity::stillValid(shared_ptr<Player> player)
{
	if (level->getTileEntity(x, y, z) != shared_from_this()) return false;
	if (player->distanceToSqr(x + 0.5, y + 0.5, z + 0.5) > 8 * 8) return false;
	return true;
}

void BeaconTileEntity::startOpen()
{
}

void BeaconTileEntity::stopOpen()
{
}

bool BeaconTileEntity::canPlaceItem(int slot, shared_ptr<ItemInstance> item)
{
	return (item->id == Item::emerald_Id || item->id == Item::diamond_Id || item->id == Item::goldIngot_Id || item->id == Item::ironIngot_Id);
}