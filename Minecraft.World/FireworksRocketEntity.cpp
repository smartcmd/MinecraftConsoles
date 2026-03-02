#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "FireworksRocketEntity.h"

FireworksRocketEntity::FireworksRocketEntity(Level *level) : Entity(level)
{
	defineSynchedData();

	life = 0;
	lifetime = 0;
	setSize(0.25f, 0.25f);
}

void FireworksRocketEntity::defineSynchedData()
{
	entityData->defineNULL(DATA_ID_FIREWORKS_ITEM, NULL);
}

bool FireworksRocketEntity::shouldRenderAtSqrDistance(double distance)
{
	return distance < 64 * 64;
}

FireworksRocketEntity::FireworksRocketEntity(Level *level, double x, double y, double z, shared_ptr<ItemInstance> sourceItem) : Entity(level)
{
	defineSynchedData();

	life = 0;

	setSize(0.25f, 0.25f);

	setPos(x, y, z);
	heightOffset = 0;

	int flightCount = 1;
	if (sourceItem != NULL && sourceItem->hasTag())
	{
		entityData->set(DATA_ID_FIREWORKS_ITEM, sourceItem);

		CompoundTag *tag = sourceItem->getTag();
		CompoundTag *compound = tag->getCompound(FireworksItem::TAG_FIREWORKS);
		if (compound != NULL)
		{
			flightCount += compound->getByte(FireworksItem::TAG_FLIGHT);
		}
	}
	xd = random->nextGaussian() * .001;
	zd = random->nextGaussian() * .001;
	yd = 0.05;

	lifetime = (SharedConstants::TICKS_PER_SECOND / 2) * flightCount + random->nextInt(6) + random->nextInt(7);
}

void FireworksRocketEntity::lerpMotion(double xd, double yd, double zd)
{
	xd = xd;
	yd = yd;
	zd = zd;
	if (xRotO == 0 && yRotO == 0)
	{
		double sd = Mth::sqrt(xd * xd + zd * zd);
		yRotO = yRot = (float) (atan2(xd, zd) * 180 / PI);
		xRotO = xRot = (float) (atan2(yd, sd) * 180 / PI);
	}
}

void FireworksRocketEntity::tick()
{
	xOld = x;
	yOld = y;
	zOld = z;
	Entity::tick();

	xd *= 1.15;
	zd *= 1.15;
	yd += .04;
	move(xd, yd, zd);

	double sd = Mth::sqrt(xd * xd + zd * zd);
	yRot = (float) (atan2(xd, zd) * 180 / PI);
	xRot = (float) (atan2(yd, sd) * 180 / PI);

	while (xRot - xRotO < -180)
		xRotO -= 360;
	while (xRot - xRotO >= 180)
		xRotO += 360;

	while (yRot - yRotO < -180)
		yRotO -= 360;
	while (yRot - yRotO >= 180)
		yRotO += 360;

	xRot = xRotO + (xRot - xRotO) * 0.2f;
	yRot = yRotO + (yRot - yRotO) * 0.2f;

	if (!level->isClientSide )
	{
		if (life == 0)
		{
			level->playEntitySound(shared_from_this(), eSoundType_FIREWORKS_LAUNCH, 3, 1);
		}
	}

	life++;
	if (level->isClientSide && (life % 2) < 2)
	{
		level->addParticle(eParticleType_fireworksspark, x, y - .3, z, random->nextGaussian() * .05, -yd * .5, random->nextGaussian() * .05);
	}
	if (!level->isClientSide && life > lifetime)
	{
		level->broadcastEntityEvent(shared_from_this(), EntityEvent::FIREWORKS_EXPLODE);
		remove();
	}
}

void FireworksRocketEntity::handleEntityEvent(byte eventId)
{
	if (eventId == EntityEvent::FIREWORKS_EXPLODE && level->isClientSide)
	{
		shared_ptr<ItemInstance> sourceItem = entityData->getItemInstance(DATA_ID_FIREWORKS_ITEM);
		CompoundTag *tag = NULL;
		if (sourceItem != NULL && sourceItem->hasTag())
		{
			tag = sourceItem->getTag()->getCompound(FireworksItem::TAG_FIREWORKS);
		}
		level->createFireworks(x, y, z, xd, yd, zd, tag);
	}
	Entity::handleEntityEvent(eventId);
}

void FireworksRocketEntity::addAdditonalSaveData(CompoundTag *tag)
{
	tag->putInt(L"Life", life);
	tag->putInt(L"LifeTime", lifetime);
	shared_ptr<ItemInstance> itemInstance = entityData->getItemInstance(DATA_ID_FIREWORKS_ITEM);
	if (itemInstance != NULL)
	{
		CompoundTag *itemTag = new CompoundTag();
		itemInstance->save(itemTag);
		tag->putCompound(L"FireworksItem", itemTag);
	}

}

void FireworksRocketEntity::readAdditionalSaveData(CompoundTag *tag)
{
	life = tag->getInt(L"Life");
	lifetime = tag->getInt(L"LifeTime");

	CompoundTag *itemTag = tag->getCompound(L"FireworksItem");
	if (itemTag != NULL)
	{
		shared_ptr<ItemInstance> fromTag = ItemInstance::fromTag(itemTag);
		if (fromTag != NULL)
		{
			entityData->set(DATA_ID_FIREWORKS_ITEM, fromTag);
		}
	}
}

float FireworksRocketEntity::getShadowHeightOffs()
{
	return 0;
}

float FireworksRocketEntity::getBrightness(float a)
{
	return Entity::getBrightness(a);
}

int FireworksRocketEntity::getLightColor(float a)
{
	return Entity::getLightColor(a);
}

bool FireworksRocketEntity::isAttackable()
{
	return false;
}