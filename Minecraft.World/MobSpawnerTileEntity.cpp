#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "TileEntity.h"
#include "LevelEvent.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.network.packet.h"
#include "SharedConstants.h"
#include "MobSpawnerTileEntity.h"



const int MobSpawnerTileEntity::MAX_DIST = 16;

MobSpawnerTileEntity::MobSpawnerTileEntity() : TileEntity()
{
	spin = 0;
	oSpin = 0;

	// entityId = "Skeleton";
	entityId = L"Pig";
	m_bEntityIdUpdated = false;

	spawnData = NULL;

	spawnDelay = 20;

	minSpawnDelay = SharedConstants::TICKS_PER_SECOND * 10;
	maxSpawnDelay = SharedConstants::TICKS_PER_SECOND * 40;
	spawnCount = 4;
	displayEntity = nullptr;
}

wstring MobSpawnerTileEntity::getEntityId() 
{
	return entityId;
}

void MobSpawnerTileEntity::setEntityId(const wstring& entityId)
{
	this->entityId = entityId;
}

bool MobSpawnerTileEntity::isNearPlayer()
{
	return level->getNearestPlayer(x + 0.5, y + 0.5, z + 0.5, MAX_DIST) != NULL;
}

void MobSpawnerTileEntity::tick()
{
	if (!isNearPlayer())
	{
		return;
	}

	if (level->isClientSide)
	{
		double xP = x + level->random->nextFloat();
		double yP = y + level->random->nextFloat();
		double zP = z + level->random->nextFloat();
		level->addParticle(eParticleType_smoke, xP, yP, zP, 0, 0, 0);
		level->addParticle(eParticleType_flame, xP, yP, zP, 0, 0, 0);

		oSpin = spin;
		spin += 1000 / 220.0f;
		while (spin > 360)
		{
			spin -= 360;
		}
		while(oSpin > 360)
		{
			oSpin -= 360;
		}
	}
	else
	{

		if (spawnDelay == -1) delay();

		if (spawnDelay > 0) 
		{
			spawnDelay--;
			return;
		}

		for (int c = 0; c < spawnCount; c++) 
		{
			shared_ptr<Mob> entity = dynamic_pointer_cast<Mob> (EntityIO::newEntity(entityId, level));
			if (entity == NULL) return;

			vector<shared_ptr<Entity> > *vecNearby = level->getEntitiesOfClass(typeid(*entity), AABB::newTemp(x, y, z, x + 1, y + 1, z + 1)->grow(8, 4, 8));
			int nearBy = (int)vecNearby->size(); //4J - IB, TODO, Mob contains no getClass
			delete vecNearby;

			if (nearBy >= 6)
			{
				delay();
				return;
			}

			// 4J added - our mobspawner tiles should only be spawning monsters. Also respect the global limits we have for those so we don't go
			// creating silly numbers of them. Have set this limit slightly higher than the main spawner has so that this tile entity is more likely to
			// actually make something (60 rather than 50)
			if(level->countInstanceOf( eTYPE_MONSTER, false) >= 60 )
			{
				return;
			}

			if (entity != NULL) 
			{
				double xp = x + (level->random->nextDouble() - level->random->nextDouble()) * 4;
				double yp = y + level->random->nextInt(3) - 1;
				double zp = z + (level->random->nextDouble() - level->random->nextDouble()) * 4;
				shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>( entity );

				entity->moveTo(xp, yp, zp, level->random->nextFloat() * 360, 0);

				if (mob == NULL || mob->canSpawn()) 
				{
					fillExtraData(entity);

					level->addEntity(entity);

					level->levelEvent(LevelEvent::PARTICLES_MOBTILE_SPAWN, x, y, z, 0);

					if (mob != NULL) mob->spawnAnim();
					delay();
				}
			}
		}
	}

	TileEntity::tick();
}

void MobSpawnerTileEntity::fillExtraData(shared_ptr<Entity> entity)
{
	if (spawnData != NULL)
	{
		CompoundTag *data = new CompoundTag();
		entity->save(data);

		vector<Tag *> *allTags = spawnData->getAllTags();
		for(AUTO_VAR(it, allTags->begin()); it != allTags->end(); ++it)
		{
			Tag *tag = *it;
			data->put((wchar_t *)tag->getName().c_str(), tag->copy());
		}
		if(allTags != NULL) delete allTags;

		entity->load(data);
	}
}

void MobSpawnerTileEntity::delay() 
{
	spawnDelay = minSpawnDelay + level->random->nextInt(maxSpawnDelay - minSpawnDelay);
}

void MobSpawnerTileEntity::load(CompoundTag *tag)
{
	TileEntity::load(tag);
	entityId = tag->getString(L"EntityId");
	m_bEntityIdUpdated = true;

	spawnDelay = tag->getShort(L"Delay");

	if (tag->contains(L"SpawnData"))
	{
		spawnData = tag->getCompound(L"SpawnData");
	}
	else
	{
		spawnData = NULL;
	}

	if (tag->contains(L"MinSpawnDelay"))
	{
		minSpawnDelay = tag->getShort(L"MinSpawnDelay");
		maxSpawnDelay = tag->getShort(L"MaxSpawnDelay");
		spawnCount = tag->getShort(L"SpawnCount");
	}
}

void MobSpawnerTileEntity::save(CompoundTag *tag)
{
	TileEntity::save(tag);
	tag->putString(L"EntityId", entityId );
	tag->putShort(L"Delay", (short) spawnDelay);
	tag->putShort(L"MinSpawnDelay", (short) minSpawnDelay);
	tag->putShort(L"MaxSpawnDelay", (short) maxSpawnDelay);
	tag->putShort(L"SpawnCount", (short) spawnCount);

	if (spawnData != NULL)
	{
		tag->putCompound(L"SpawnData", spawnData);
	}
}

shared_ptr<Entity> MobSpawnerTileEntity::getDisplayEntity()
{
	if (displayEntity == NULL || m_bEntityIdUpdated)
	{
		shared_ptr<Entity> e = EntityIO::newEntity(getEntityId(), NULL);
		fillExtraData(e);
		displayEntity = e;
		m_bEntityIdUpdated = false;
	}

	return displayEntity;
}

shared_ptr<Packet> MobSpawnerTileEntity::getUpdatePacket()
{
	CompoundTag *tag = new CompoundTag();
	save(tag);
	return shared_ptr<TileEntityDataPacket>( new TileEntityDataPacket(x, y, z, TileEntityDataPacket::TYPE_MOB_SPAWNER, tag) );
}

// 4J Added
shared_ptr<TileEntity> MobSpawnerTileEntity::clone()
{
	shared_ptr<MobSpawnerTileEntity> result = shared_ptr<MobSpawnerTileEntity>( new MobSpawnerTileEntity() );
	TileEntity::clone(result);

	result->entityId = entityId;
	result->spawnDelay = spawnDelay;
	return result;
}