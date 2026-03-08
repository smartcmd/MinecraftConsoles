#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.damagesource.h"
#include "GeneralStat.h"
#include "Skeleton.h"
#include "Creeper.h"
#include "Arrow.h"
#include "../Minecraft.Client/Textures.h"
#include "SoundTypes.h"



void Creeper::_init()
{
	swell = 0;
	oldSwell = 0;
	maxSwell = 30;
	explosionRadius = 3;
}

Creeper::Creeper(Level *level) : Monster( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	_init();

	goalSelector.addGoal(1, new FloatGoal(this));
	goalSelector.addGoal(2, new SwellGoal(this));
	goalSelector.addGoal(3, new AvoidPlayerGoal(this, typeid(Ocelot), 6, 1.0, 1.2));
	goalSelector.addGoal(4, new MeleeAttackGoal(this, 1.0, false));
	goalSelector.addGoal(5, new RandomStrollGoal(this, 0.8));
	goalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 8));
	goalSelector.addGoal(6, new RandomLookAroundGoal(this));

	targetSelector.addGoal(1, new NearestAttackableTargetGoal(this, typeid(Player), 0, true));
	targetSelector.addGoal(2, new HurtByTargetGoal(this, false));
}

void Creeper::registerAttributes()
{
	Monster::registerAttributes();

	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25f);
}

bool Creeper::useNewAi()
{
	return true;
}

int Creeper::getMaxFallDistance()
{
	if (getTarget() == NULL) return 3;
	// As long as they survive the fall they should try.
	return 3 + (int) (getHealth() - 1);
}

void Creeper::causeFallDamage(float distance)
{
	Monster::causeFallDamage(distance);

	swell += distance * 1.5f;
	if (swell > maxSwell - 5) swell = maxSwell - 5;
}

void Creeper::defineSynchedData()
{
	Monster::defineSynchedData();

	entityData->define(DATA_SWELL_DIR, (byte) -1);
	entityData->define(DATA_IS_POWERED, (byte) 0);
}

void Creeper::addAdditonalSaveData(CompoundTag *entityTag)
{
	Monster::addAdditonalSaveData(entityTag);
	if (entityData->getByte(DATA_IS_POWERED) == 1) entityTag->putBoolean(L"powered", true);
	entityTag->putShort(L"Fuse", (short) maxSwell);
	entityTag->putByte(L"ExplosionRadius", (byte) explosionRadius);
}

void Creeper::readAdditionalSaveData(CompoundTag *tag)
{
	Monster::readAdditionalSaveData(tag);
	entityData->set(DATA_IS_POWERED, (byte) (tag->getBoolean(L"powered") ? 1 : 0));
	if (tag->contains(L"Fuse")) maxSwell = tag->getShort(L"Fuse");
	if (tag->contains(L"ExplosionRadius")) explosionRadius = tag->getByte(L"ExplosionRadius");
}

void Creeper::tick()
{
	oldSwell = swell;
	if (isAlive())
	{
		int swellDir = getSwellDir();
		if (swellDir > 0 && swell == 0)
		{
			playSound(eSoundType_RANDOM_FUSE, 1, 0.5f);
		}
		swell += swellDir;
		if (swell < 0) swell = 0;
		if (swell >= maxSwell)
		{
			swell = maxSwell;
			if (!level->isClientSide)
			{
				bool destroyBlocks = level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING);
				if (isPowered()) level->explode(shared_from_this(), x, y, z, explosionRadius * 2, destroyBlocks);
				else level->explode(shared_from_this(), x, y, z, explosionRadius, destroyBlocks);
				remove();
			}
		}
	}
	Monster::tick();
}

int Creeper::getHurtSound()
{
	return eSoundType_MOB_CREEPER_HURT;
}

int Creeper::getDeathSound()
{
	return eSoundType_MOB_CREEPER_DEATH;
}

void Creeper::die(DamageSource *source)
{
	Monster::die(source);

	if ( source->getEntity() != NULL && source->getEntity()->instanceof(eTYPE_SKELETON) )
	{
		int recordId = Item::record_01_Id + random->nextInt(Item::record_12_Id - Item::record_01_Id + 1);
		spawnAtLocation(recordId, 1);
	}

	if ( source->getDirectEntity() != NULL && source->getDirectEntity()->instanceof(eTYPE_ARROW) && source->getEntity() != NULL && source->getEntity()->instanceof(eTYPE_PLAYER) )
	{
		shared_ptr<Player> player = dynamic_pointer_cast<Player>(source->getEntity());
		player->awardStat(GenericStats::archer(), GenericStats::param_archer());
	}
}

bool Creeper::doHurtTarget(shared_ptr<Entity> target)
{
	return true;
}

bool Creeper::isPowered()
{
	return entityData->getByte(DATA_IS_POWERED) == 1;
}

float Creeper::getSwelling(float a)
{
	return (oldSwell + (swell - oldSwell) * a) / (maxSwell - 2);
}

int Creeper::getDeathLoot()
{
	return Item::gunpowder_Id;
}

int Creeper::getSwellDir()
{
	return (int) (char) entityData->getByte(DATA_SWELL_DIR);
}

void Creeper::setSwellDir(int dir)
{
	entityData->set(DATA_SWELL_DIR, (byte) dir);
}

void Creeper::thunderHit(const LightningBolt *lightningBolt) 
{
	Monster::thunderHit(lightningBolt);
	entityData->set(DATA_IS_POWERED, (byte) 1);
}
