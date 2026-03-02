#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.village.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "..\Minecraft.Client\Textures.h"
#include "SynchedEntityData.h"
#include "VillagerGolem.h"
#include "ParticleTypes.h"

VillagerGolem::VillagerGolem(Level *level) : Golem(level)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that the derived version of the function is called
	health = getMaxHealth();

	villageUpdateInterval = 0;
	village = weak_ptr<Village>();
	attackAnimationTick = 0;
	offerFlowerTick = 0;

	this->textureIdx = TN_MOB_VILLAGER_GOLEM; // "/mob/villager_golem.png";
	this->setSize(1.4f, 2.9f);

	getNavigation()->setAvoidWater(true);

	// 4J-JEV: These speed values are as they appear before 1.6.4 (1.5),
	// as the movement speed system changes then. (Mob attributes added)
	goalSelector.addGoal(1, new MeleeAttackGoal(this, 0.25f, true));
	goalSelector.addGoal(2, new MoveTowardsTargetGoal(this, 0.22f, 32));
	goalSelector.addGoal(3, new MoveThroughVillageGoal(this, 0.16f, true));
	goalSelector.addGoal(4, new MoveTowardsRestrictionGoal(this, 0.16f));
	goalSelector.addGoal(5, new OfferFlowerGoal(this));
	goalSelector.addGoal(6, new RandomStrollGoal(this, 0.16f));
	goalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Player), 6));
	goalSelector.addGoal(8, new RandomLookAroundGoal(this));

	targetSelector.addGoal(1, new DefendVillageTargetGoal(this));
	targetSelector.addGoal(2, new HurtByTargetGoal(this, false));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Monster), 16, 0, false, true));
}

void VillagerGolem::defineSynchedData()
{
	Golem::defineSynchedData();
	entityData->define(DATA_FLAGS_ID, (byte) 0);
}

bool VillagerGolem::useNewAi()
{
	return true;
}

void VillagerGolem::serverAiMobStep()
{
	if (--villageUpdateInterval <= 0)
	{
		villageUpdateInterval = 70 + random->nextInt(50);
		shared_ptr<Village> _village = level->villages->getClosestVillage(Mth::floor(x), Mth::floor(y), Mth::floor(z), Villages::MaxDoorDist);
		village = _village;
		if (_village == NULL) clearRestriction();
		else
		{
			Pos *center = _village->getCenter();
			restrictTo(center->x, center->y, center->z, (int)((float)_village->getRadius()) * 0.6f);
		}
	}

	Golem::serverAiMobStep();
}

int VillagerGolem::getMaxHealth()
{
	return 100;
}

int VillagerGolem::decreaseAirSupply(int currentSupply)
{
	// infinite air supply
	return currentSupply;
}

void VillagerGolem::aiStep()
{
	Golem::aiStep();

	if (attackAnimationTick > 0) --attackAnimationTick;
	if (offerFlowerTick > 0) --offerFlowerTick;

	if (xd * xd + zd * zd > MoveControl::MIN_SPEED_SQR && random->nextInt(5) == 0)
	{
		int xt = Mth::floor(x);
		int yt = Mth::floor(y - 0.2f - this->heightOffset);
		int zt = Mth::floor(z);
		int t = level->getTile(xt, yt, zt);
		int d = level->getData(xt, yt, zt);
		if (t > 0)
		{
			level->addParticle(PARTICLE_TILECRACK(t,d), x + (random->nextFloat() - 0.5) * bbWidth, bb->y0 + 0.1, z + (random->nextFloat() - 0.5) * bbWidth, 4 * (random->nextFloat() - 0.5), .5,
				(random->nextFloat() - 0.5) * 4);
		}
	}
}

bool VillagerGolem::canAttackType(eINSTANCEOF targetType)
{
	if (isPlayerCreated() && (eTYPE_PLAYER & targetType) == eTYPE_PLAYER ) return false;
	return Golem::canAttackType(targetType);
}

void VillagerGolem::addAdditonalSaveData(CompoundTag *tag)
{
	Golem::addAdditonalSaveData(tag);
	tag->putBoolean(L"PlayerCreated", isPlayerCreated());
}

void VillagerGolem::readAdditionalSaveData(CompoundTag *tag)
{
	Golem::readAdditionalSaveData(tag);
	setPlayerCreated(tag->getBoolean(L"PlayerCreated"));
}

bool VillagerGolem::doHurtTarget(shared_ptr<Entity> target)
{
	attackAnimationTick = 10;
	level->broadcastEntityEvent(shared_from_this(), EntityEvent::START_ATTACKING);
	bool hurt = target->hurt(DamageSource::mobAttack(dynamic_pointer_cast<Mob>(shared_from_this())), 7 + random->nextInt(15));
	if (hurt) target->yd += 0.4f;
	level->playSound(shared_from_this(), eSoundType_MOB_IRONGOLEM_THROW, 1, 1);
	return hurt;
}

void VillagerGolem::handleEntityEvent(byte id)
{
	if (id == EntityEvent::START_ATTACKING)
	{
		attackAnimationTick = 10;
		level->playSound(shared_from_this(), eSoundType_MOB_IRONGOLEM_THROW, 1, 1);
	}
	else if (id == EntityEvent::OFFER_FLOWER)
	{
		offerFlowerTick = OfferFlowerGoal::OFFER_TICKS;
	}
	else Golem::handleEntityEvent(id);
}

shared_ptr<Village> VillagerGolem::getVillage()
{
	return village.lock();
}

int VillagerGolem::getAttackAnimationTick()
{
	return attackAnimationTick;
}

void VillagerGolem::offerFlower(bool offer)
{
	offerFlowerTick = offer ? OfferFlowerGoal::OFFER_TICKS : 0;
	level->broadcastEntityEvent(shared_from_this(), EntityEvent::OFFER_FLOWER);
}

int VillagerGolem::getAmbientSound()
{
	return -1;
}

int VillagerGolem::getHurtSound()
{
	return eSoundType_MOB_IRONGOLEM_HIT;
}

int VillagerGolem::getDeathSound()
{
	return eSoundType_MOB_IRONGOLEM_DEATH;
}

void VillagerGolem::playStepSound(int xt, int yt, int zt, int t)
{
	level->playSound(shared_from_this(), eSoundType_MOB_IRONGOLEM_WALK, 1, 1);
}

void VillagerGolem::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	int roses = random->nextInt(3);
	for (int i = 0; i < roses; i++)
	{
		spawnAtLocation(Tile::rose_Id, 1);
	}
	int iron = 3 + random->nextInt(3);
	for (int i = 0; i < iron; i++)
	{
		spawnAtLocation(Item::ironIngot_Id, 1);
	}
}

int VillagerGolem::getOfferFlowerTick()
{
	return offerFlowerTick;
}

bool VillagerGolem::isPlayerCreated()
{
	return (entityData->getByte(DATA_FLAGS_ID) & 0x01) != 0;
}

void VillagerGolem::setPlayerCreated(bool value)
{
	byte current = entityData->getByte(DATA_FLAGS_ID);
	if (value)
	{
		entityData->set(DATA_FLAGS_ID, (byte) (current | 0x01));
	}
	else
	{
		entityData->set(DATA_FLAGS_ID, (byte) (current & ~0x01));
	}
}

void VillagerGolem::die(DamageSource *source)
{
	if (!isPlayerCreated() && lastHurtByPlayer != NULL && village.lock() != NULL)
	{
		village.lock()->modifyStanding(lastHurtByPlayer->getName(), -5);
	}
	Golem::die(source);
}