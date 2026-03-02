#include "stdafx.h"
#include "net.minecraft.network.packet.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "MobSpawnerTileEntity.h"

MobSpawnerTileEntity::TileEntityMobSpawner::TileEntityMobSpawner(MobSpawnerTileEntity *parent)
{
	m_parent = parent;
}

void MobSpawnerTileEntity::TileEntityMobSpawner::broadcastEvent(int id)
{
	m_parent->level->tileEvent(m_parent->x, m_parent->y, m_parent->z, Tile::mobSpawner_Id, id, 0);
}

Level *MobSpawnerTileEntity::TileEntityMobSpawner::getLevel()
{
	return m_parent->level;
}

int MobSpawnerTileEntity::TileEntityMobSpawner::getX()
{
	return m_parent->x;
}

int MobSpawnerTileEntity::TileEntityMobSpawner::getY()
{
	return m_parent->y;
}

int MobSpawnerTileEntity::TileEntityMobSpawner::getZ()
{
	return m_parent->z;
}

void MobSpawnerTileEntity::TileEntityMobSpawner::setNextSpawnData(BaseMobSpawner::SpawnData *nextSpawnData)
{
	BaseMobSpawner::setNextSpawnData(nextSpawnData);
	if (getLevel() != NULL) getLevel()->sendTileUpdated(m_parent->x, m_parent->y, m_parent->z);
}

MobSpawnerTileEntity::MobSpawnerTileEntity()
{
	spawner = new TileEntityMobSpawner(this);
}

MobSpawnerTileEntity::~MobSpawnerTileEntity()
{
	delete spawner;
}

void MobSpawnerTileEntity::load(CompoundTag *tag)
{
	TileEntity::load(tag);
	spawner->load(tag);
}

void MobSpawnerTileEntity::save(CompoundTag *tag)
{
	TileEntity::save(tag);
	spawner->save(tag);
}

void MobSpawnerTileEntity::tick()
{
	spawner->tick();
	TileEntity::tick();
}

shared_ptr<Packet> MobSpawnerTileEntity::getUpdatePacket()
{
	CompoundTag *tag = new CompoundTag();
	save(tag);
	tag->remove(L"SpawnPotentials");
	return shared_ptr<TileEntityDataPacket>( new TileEntityDataPacket(x, y, z, TileEntityDataPacket::TYPE_MOB_SPAWNER, tag) );
}

bool MobSpawnerTileEntity::triggerEvent(int b0, int b1)
{
	if (spawner->onEventTriggered(b0)) return true;
	return TileEntity::triggerEvent(b0, b1);
}

BaseMobSpawner *MobSpawnerTileEntity::getSpawner()
{
	return spawner;
}

// 4J Added
shared_ptr<TileEntity> MobSpawnerTileEntity::clone()
{
	shared_ptr<MobSpawnerTileEntity> result = shared_ptr<MobSpawnerTileEntity>( new MobSpawnerTileEntity() );
	TileEntity::clone(result);

	return result;
}

void MobSpawnerTileEntity::setEntityId(const wstring &id)
{
	spawner->setEntityId(id);
}