#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.phys.h"
#include "MinecartHopper.h"

const int MinecartHopper::MOVE_ITEM_SPEED = HopperTileEntity::MOVE_ITEM_SPEED / 2;

void MinecartHopper::_init()
{
	enabled = true;
	cooldownTime = -1;

	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
}

MinecartHopper::MinecartHopper(Level *level) : MinecartContainer(level)
{
	_init();
}

MinecartHopper::MinecartHopper(Level *level, double x, double y, double z) : MinecartContainer(level, x, y, z)
{
	_init();
}

int MinecartHopper::getType()
{
	return TYPE_HOPPER;
}

Tile *MinecartHopper::getDefaultDisplayTile()
{
	return Tile::hopper;
}

int MinecartHopper::getDefaultDisplayOffset()
{
	return 1;
}

unsigned int MinecartHopper::getContainerSize()
{
	return 5;
}

bool MinecartHopper::interact(shared_ptr<Player> player)
{
	if (!level->isClientSide)
	{
		player->openHopper(dynamic_pointer_cast<MinecartHopper>(shared_from_this()));
	}

	return true;
}

void MinecartHopper::activateMinecart(int xt, int yt, int zt, bool state)
{
	bool newEnabled = !state;

	if (newEnabled != isEnabled())
	{
		setEnabled(newEnabled);
	}
}

bool MinecartHopper::isEnabled()
{
	return enabled;
}

void MinecartHopper::setEnabled(bool enabled)
{
	this->enabled = enabled;
}

Level *MinecartHopper::getLevel()
{
	return level;
}

double MinecartHopper::getLevelX()
{
	return x;
}

double MinecartHopper::getLevelY()
{
	return y;
}

double MinecartHopper::getLevelZ()
{
	return z;
}

void MinecartHopper::tick()
{
	MinecartContainer::tick();

	if (!level->isClientSide && isAlive() && isEnabled())
	{
		cooldownTime--;
		if (!isOnCooldown())
		{
			setCooldown(0);

			if (suckInItems())
			{
				setCooldown(MOVE_ITEM_SPEED);
				MinecartContainer::setChanged();
			}
		}
	}
}

bool MinecartHopper::suckInItems()
{
	if (HopperTileEntity::suckInItems(this)) return true;

	vector<shared_ptr<Entity> > *items = level->getEntitiesOfClass(typeid(ItemEntity), bb->grow(0.25f, 0, 0.25f), EntitySelector::ENTITY_STILL_ALIVE);

	if (items->size() > 0)
	{
		HopperTileEntity::addItem( this, dynamic_pointer_cast<ItemEntity>(items->at(0)) );
	}
	delete items;

	return false;
}

void MinecartHopper::destroy(DamageSource *source)
{
	MinecartContainer::destroy(source);

	spawnAtLocation(Tile::hopper_Id, 1, 0);
}

void MinecartHopper::addAdditonalSaveData(CompoundTag *base)
{
	MinecartContainer::addAdditonalSaveData(base);
	base->putInt(L"TransferCooldown", cooldownTime);
}

void MinecartHopper::readAdditionalSaveData(CompoundTag *base)
{
	MinecartContainer::readAdditionalSaveData(base);
	cooldownTime = base->getInt(L"TransferCooldown");
}

void MinecartHopper::setCooldown(int time)
{
	cooldownTime = time;
}

bool MinecartHopper::isOnCooldown()
{
	return cooldownTime > 0;
}