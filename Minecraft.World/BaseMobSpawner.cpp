#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "BaseMobSpawner.h"

BaseMobSpawner::BaseMobSpawner()
{
	spawnPotentials = nullptr;
	spawnDelay = 20;
	entityId = L"Pig";
	nextSpawnData = nullptr;
	spin = oSpin = 0.0;

	minSpawnDelay = SharedConstants::TICKS_PER_SECOND * 10;
	maxSpawnDelay = SharedConstants::TICKS_PER_SECOND * 40;
	spawnCount = 4;
	displayEntity = nullptr;
	maxNearbyEntities = 6;
	requiredPlayerRange = 16;
	spawnRange = 4;
}

BaseMobSpawner::~BaseMobSpawner()
{
	if(spawnPotentials)
	{
		for( auto& it : *spawnPotentials )
		{
			delete it;
		}
		delete spawnPotentials;
	}
}

wstring BaseMobSpawner::getEntityId()
{
	if (getNextSpawnData() == nullptr)
	{
		if (entityId.compare(L"Minecart") == 0)
		{
			entityId = L"MinecartRideable";
		}
		return entityId;
	}
	else
	{
		return getNextSpawnData()->type;
	}
}

void BaseMobSpawner::setEntityId(const wstring &entityId)
{
	this->entityId = entityId;
}

bool BaseMobSpawner::isNearPlayer()
{
	return getLevel()->getNearestPlayer(getX() + 0.5, getY() + 0.5, getZ() + 0.5, requiredPlayerRange) != nullptr;
}

void BaseMobSpawner::tick()
{
	if (!isNearPlayer())
	{
		return;
	}

	if (getLevel()->isClientSide)
	{
		double xP = getX() + getLevel()->random->nextFloat();
		double yP = getY() + getLevel()->random->nextFloat();
		double zP = getZ() + getLevel()->random->nextFloat();
		getLevel()->addParticle(eParticleType_smoke, xP, yP, zP, 0, 0, 0);
		getLevel()->addParticle(eParticleType_flame, xP, yP, zP, 0, 0, 0);

		if (spawnDelay > 0) spawnDelay--;
		oSpin = spin;
		spin = static_cast<int>(spin + 1000 / (spawnDelay + 200.0f)) % 360;
	}
	else
	{
		if (spawnDelay == -1) delay();

		if (spawnDelay > 0)
		{
			spawnDelay--;
			return;
		}

		bool _delay = false;

		for (int c = 0; c < spawnCount; c++)
		{
			shared_ptr<Entity> entity = EntityIO::newEntity(getEntityId(), getLevel());
			if (entity == nullptr) return;

			int nearBy = getLevel()->getEntitiesOfClass( typeid(entity.get()), AABB::newTemp(getX(), getY(), getZ(), getX() + 1, getY() + 1, getZ() + 1)->grow(spawnRange * 2, 4, spawnRange * 2))->size();
			if (nearBy >= maxNearbyEntities)
			{
				delay();
				return;
			}

			double xp = getX() + (getLevel()->random->nextDouble() - getLevel()->random->nextDouble()) * spawnRange;
			double yp = getY() + getLevel()->random->nextInt(3) - 1;
			double zp = getZ() + (getLevel()->random->nextDouble() - getLevel()->random->nextDouble()) * spawnRange;
			shared_ptr<Mob> mob = entity->instanceof(eTYPE_MOB) ? dynamic_pointer_cast<Mob>( entity ) : nullptr;

			entity->moveTo(xp, yp, zp, getLevel()->random->nextFloat() * 360, 0);

			if (mob == nullptr || mob->canSpawn())
			{
				loadDataAndAddEntity(entity);
				getLevel()->levelEvent(LevelEvent::PARTICLES_MOBTILE_SPAWN, getX(), getY(), getZ(), 0);

				if (mob != nullptr)
				{
					mob->spawnAnim();
				}

				_delay = true;
			}
		}

		if (_delay) delay();
	}
}

shared_ptr<Entity> BaseMobSpawner::loadDataAndAddEntity(shared_ptr<Entity> entity)
{
	if (getNextSpawnData() != nullptr)
	{
		CompoundTag *data = new CompoundTag();
		entity->save(data);

		vector<Tag *> *tags = getNextSpawnData()->tag->getAllTags();
		if ( tags )
		{
			for (auto& tag : *tags)
			{
				data->put(tag->getName(), tag->copy());
			}
			delete tags;
		}

		entity->load(data);
		if (entity->level != nullptr) entity->level->addEntity(entity);

		// add mounts
		shared_ptr<Entity> rider = entity;
		while (data->contains(Entity::RIDING_TAG))
		{
			CompoundTag *ridingTag = data->getCompound(Entity::RIDING_TAG);
			shared_ptr<Entity> mount = EntityIO::newEntity(ridingTag->getString(L"id"), entity->level);
			if (mount != nullptr)
			{
				CompoundTag *mountData = new CompoundTag();
				mount->save(mountData);

				vector<Tag *> *ridingTags = ridingTag->getAllTags();
				if ( ridingTags )
				{
					for (auto& tag : *ridingTags)
					{
						mountData->put(tag->getName(), tag->copy());
					}
					delete ridingTags;
				}
				mount->load(mountData);
				mount->moveTo(rider->x, rider->y, rider->z, rider->yRot, rider->xRot);

				if (entity->level != nullptr) entity->level->addEntity(mount);
				rider->ride(mount);
			}
			rider = mount;
			data = ridingTag;
		}

	}
	else if (entity->instanceof(eTYPE_LIVINGENTITY) && entity->level != nullptr)
	{
		dynamic_pointer_cast<Mob>( entity )->finalizeMobSpawn(nullptr);
		getLevel()->addEntity(entity);
	}

	return entity;
}

void BaseMobSpawner::delay()
{
	if (maxSpawnDelay <= minSpawnDelay)
	{
		spawnDelay = minSpawnDelay;
	}
	else
	{
		spawnDelay = minSpawnDelay + getLevel()->random->nextInt(maxSpawnDelay - minSpawnDelay);
	}

	if ( (spawnPotentials != nullptr) && (spawnPotentials->size() > 0) )
	{
		setNextSpawnData( static_cast<SpawnData *>(WeighedRandom::getRandomItem((Random *)getLevel()->random, (vector<WeighedRandomItem *> *)spawnPotentials)) );
	}

	broadcastEvent(EVENT_SPAWN);
}

void BaseMobSpawner::load(CompoundTag *tag)
{
	entityId = tag->getString(L"EntityId");
	spawnDelay = tag->getShort(L"Delay");

	if (tag->contains(L"SpawnPotentials"))
	{
		spawnPotentials = new vector<SpawnData *>();
		ListTag<CompoundTag> *potentials = (ListTag<CompoundTag> *) tag->getList(L"SpawnPotentials");

		for (int i = 0; i < potentials->size(); i++)
		{
			spawnPotentials->push_back(new SpawnData(potentials->get(i)));
		}
	}
	else
	{
		spawnPotentials = nullptr;
	}

	if (tag->contains(L"SpawnData"))
	{
		setNextSpawnData(new SpawnData(tag->getCompound(L"SpawnData"), entityId));
	}
	else
	{
		setNextSpawnData(nullptr);
	}

	if (tag->contains(L"MinSpawnDelay"))
	{
		minSpawnDelay = tag->getShort(L"MinSpawnDelay");
		maxSpawnDelay = tag->getShort(L"MaxSpawnDelay");
		spawnCount = tag->getShort(L"SpawnCount");
	}

	if (tag->contains(L"MaxNearbyEntities"))
	{
		maxNearbyEntities = tag->getShort(L"MaxNearbyEntities");
		requiredPlayerRange = tag->getShort(L"RequiredPlayerRange");
	}

	if (tag->contains(L"SpawnRange")) spawnRange = tag->getShort(L"SpawnRange");

	if (getLevel() != nullptr && getLevel()->isClientSide)
	{
		displayEntity = nullptr;
	}
}

void BaseMobSpawner::save(CompoundTag *tag)
{
	tag->putString(L"EntityId", getEntityId());
	tag->putShort(L"Delay", static_cast<short>(spawnDelay));
	tag->putShort(L"MinSpawnDelay", static_cast<short>(minSpawnDelay));
	tag->putShort(L"MaxSpawnDelay", static_cast<short>(maxSpawnDelay));
	tag->putShort(L"SpawnCount", static_cast<short>(spawnCount));
	tag->putShort(L"MaxNearbyEntities", static_cast<short>(maxNearbyEntities));
	tag->putShort(L"RequiredPlayerRange", static_cast<short>(requiredPlayerRange));
	tag->putShort(L"SpawnRange", static_cast<short>(spawnRange));

	if (getNextSpawnData() != nullptr)
	{
		tag->putCompound(L"SpawnData", static_cast<CompoundTag *>(getNextSpawnData()->tag->copy()));
	}

	if (getNextSpawnData() != nullptr || (spawnPotentials != nullptr && spawnPotentials->size() > 0))
	{
		ListTag<CompoundTag> *list = new ListTag<CompoundTag>();

		if (spawnPotentials && spawnPotentials->size() > 0)
		{
			for ( auto& data : *spawnPotentials )
			{
				list->add(data->save());
			}
		}
		else
		{
			list->add(getNextSpawnData()->save());
		}

		tag->put(L"SpawnPotentials", list);
	}
}

shared_ptr<Entity> BaseMobSpawner::getDisplayEntity()
{
	if (displayEntity == nullptr)
	{
		shared_ptr<Entity> e = EntityIO::newEntity(getEntityId(), nullptr);
		e = loadDataAndAddEntity(e);
		displayEntity = e;
	}

	return displayEntity;
}

bool BaseMobSpawner::onEventTriggered(int id)
{
	if (id == EVENT_SPAWN && getLevel()->isClientSide)
	{
		spawnDelay = minSpawnDelay;
		return true;
	}
	return false;
}

BaseMobSpawner::SpawnData *BaseMobSpawner::getNextSpawnData()
{
	return nextSpawnData;
}

void BaseMobSpawner::setNextSpawnData(SpawnData *nextSpawnData)
{
	this->nextSpawnData = nextSpawnData;
}

BaseMobSpawner::SpawnData::SpawnData(CompoundTag *base) : WeighedRandomItem(base->getInt(L"Weight"))
{
	CompoundTag *tag = base->getCompound(L"Properties");
	wstring _type = base->getString(L"Type");

	if (_type.compare(L"Minecart") == 0)
	{
		if (tag != nullptr)
		{
			switch (tag->getInt(L"Type"))
			{
			case Minecart::TYPE_CHEST:
				type = L"MinecartChest";
				break;
			case Minecart::TYPE_FURNACE:
				type = L"MinecartFurnace";
				break;
			case Minecart::TYPE_RIDEABLE:
				type = L"MinecartRideable";
				break;
			}
		}
		else
		{
			type = L"MinecartRideable";
		}
	}

	this->tag = tag;
	this->type = _type;
}

BaseMobSpawner::SpawnData::SpawnData(CompoundTag *tag, wstring _type) : WeighedRandomItem(1)
{
	if (_type.compare(L"Minecart") == 0)
	{
		if (tag != nullptr)
		{
			switch (tag->getInt(L"Type"))
			{
			case Minecart::TYPE_CHEST:
				_type = L"MinecartChest";
				break;
			case Minecart::TYPE_FURNACE:
				_type = L"MinecartFurnace";
				break;
			case Minecart::TYPE_RIDEABLE:
				_type = L"MinecartRideable";
				break;
			}
		}
		else
		{
			_type = L"MinecartRideable";
		}
	}

	this->tag = tag;
	this->type = _type;
}

BaseMobSpawner::SpawnData::~SpawnData()
{
	delete tag;
}

CompoundTag *BaseMobSpawner::SpawnData::save()
{
	CompoundTag *result = new CompoundTag();

	result->putCompound(L"Properties", tag);
	result->putString(L"Type", type);
	result->putInt(L"Weight", randomWeight);

	return result;
}