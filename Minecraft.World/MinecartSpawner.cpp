#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "MinecartSpawner.h"

MinecartSpawner::MinecartMobSpawner::MinecartMobSpawner(MinecartSpawner *parent)
{
	m_parent = parent;
}

void MinecartSpawner::MinecartMobSpawner::broadcastEvent(int id)
{
	m_parent->level->broadcastEntityEvent(m_parent->shared_from_this(), (byte) id);
}

Level *MinecartSpawner::MinecartMobSpawner::getLevel()
{
	return m_parent->level;
}

int MinecartSpawner::MinecartMobSpawner::getX()
{
	return Mth::floor(m_parent->x);
}

int MinecartSpawner::MinecartMobSpawner::getY()
{
	return Mth::floor(m_parent->y);
}

int MinecartSpawner::MinecartMobSpawner::getZ()
{
	return Mth::floor(m_parent->z);
}

MinecartSpawner::MinecartSpawner(Level *level) : Minecart(level)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	spawner = new MinecartMobSpawner(this);
}

MinecartSpawner::MinecartSpawner(Level *level, double x, double y, double z) : Minecart(level, x, y, z)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	spawner = new MinecartMobSpawner(this);
}

MinecartSpawner::~MinecartSpawner()
{
	delete spawner;
}

int MinecartSpawner::getType()
{
	return TYPE_SPAWNER;
}

Tile *MinecartSpawner::getDefaultDisplayTile()
{
	return Tile::mobSpawner;
}

void MinecartSpawner::readAdditionalSaveData(CompoundTag *tag)
{
	Minecart::readAdditionalSaveData(tag);
	spawner->load(tag);
}

void MinecartSpawner::addAdditonalSaveData(CompoundTag *tag)
{
	Minecart::addAdditonalSaveData(tag);
	spawner->save(tag);
}

void MinecartSpawner::handleEntityEvent(byte eventId)
{
	spawner->onEventTriggered(eventId);
}

void MinecartSpawner::tick()
{
	Minecart::tick();
	spawner->tick();
}

BaseMobSpawner *MinecartSpawner::getSpawner()
{
	return spawner;
}