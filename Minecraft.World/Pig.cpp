#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.stats.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.global.h"
#include "Pig.h"
#include "..\Minecraft.Client\Textures.h"
#include "MobCategory.h"



Pig::Pig(Level *level) : Animal( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	setSize(0.9f, 0.9f);

	getNavigation()->setAvoidWater(true);
	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new PanicGoal(this, 1.25));
	goalSelector.addGoal(2, controlGoal = new ControlledByPlayerGoal(this, 0.3f, 0.25f));
	goalSelector.addGoal(3, new BreedGoal(this, 1.0));
	goalSelector.addGoal(4, new TemptGoal(this, 1.2, Item::carrotOnAStick_Id, false));
	goalSelector.addGoal(4, new TemptGoal(this, 1.2, Item::carrots_Id, false));
	goalSelector.addGoal(5, new FollowParentGoal(this, 1.1));
	goalSelector.addGoal(6, new RandomStrollGoal(this, 1.0));
	goalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Player), 6));
	goalSelector.addGoal(8, new RandomLookAroundGoal(this));
}

bool Pig::useNewAi()
{
	return true;
}

void Pig::registerAttributes()
{
	Animal::registerAttributes();

	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(10);
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25f);
}

void Pig::newServerAiStep()
{
	Animal::newServerAiStep();
}

bool Pig::canBeControlledByRider()
{
	shared_ptr<ItemInstance> item = dynamic_pointer_cast<Player>(rider.lock())->getCarriedItem();

	return item != nullptr && item->id == Item::carrotOnAStick_Id;
}

void Pig::defineSynchedData() 
{
	Animal::defineSynchedData();
	entityData->define(DATA_SADDLE_ID, static_cast<byte>(0));
}

void Pig::addAdditonalSaveData(CompoundTag *tag) 
{
	Animal::addAdditonalSaveData(tag);
	tag->putBoolean(L"Saddle", hasSaddle());
}

void Pig::readAdditionalSaveData(CompoundTag *tag) 
{
	Animal::readAdditionalSaveData(tag);
	setSaddle(tag->getBoolean(L"Saddle"));
}

int Pig::getAmbientSound() 
{
	return eSoundType_MOB_PIG_AMBIENT;
}

int Pig::getHurtSound() 
{
	return eSoundType_MOB_PIG_AMBIENT;
}

int Pig::getDeathSound() 
{
	return eSoundType_MOB_PIG_DEATH;
}

void Pig::playStepSound(int xt, int yt, int zt, int t)
{
	playSound(eSoundType_MOB_PIG_STEP, 0.15f, 1);
}

bool Pig::mobInteract(shared_ptr<Player> player)
{
	if(!Animal::mobInteract(player))
	{
		if (hasSaddle() && !level->isClientSide && (rider.lock() == nullptr || rider.lock() == player)) 
		{
			// 4J HEG - Fixed issue with player not being able to dismount pig (issue #4479)
			player->ride( rider.lock() == player ? nullptr : shared_from_this() );
			return true;
		}
		return false;
	}
	return true;
}

int Pig::getDeathLoot() 
{
	if (this->isOnFire() ) return Item::porkChop_cooked->id;
	return Item::porkChop_raw_Id;
}

void Pig::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	int count = random->nextInt(3) + 1 + random->nextInt(1 + playerBonusLevel);

	for (int i = 0; i < count; i++)
	{
		if (isOnFire())
		{
			spawnAtLocation(Item::porkChop_cooked_Id, 1);
		}
		else
		{
			spawnAtLocation(Item::porkChop_raw_Id, 1);
		}
	}
	if (hasSaddle()) spawnAtLocation(Item::saddle_Id, 1);
}

bool Pig::hasSaddle() 
{
	return (entityData->getByte(DATA_SADDLE_ID) & 1) != 0;
}

void Pig::setSaddle(bool value) 
{
	if (value) 
	{
		entityData->set(DATA_SADDLE_ID, static_cast<byte>(1));
	} 
	else 
	{
		entityData->set(DATA_SADDLE_ID, static_cast<byte>(0));
	}
}

void Pig::thunderHit(const LightningBolt *lightningBolt)
{
	if (level->isClientSide) return;
	shared_ptr<PigZombie> pz = std::make_shared<PigZombie>(level);
	pz->moveTo(x, y, z, yRot, xRot);
	level->addEntity(pz);
	remove();
}

void Pig::causeFallDamage(float distance) 
{
	Animal::causeFallDamage(distance);
	if ( (distance > 5) && rider.lock() != nullptr && rider.lock()->instanceof(eTYPE_PLAYER) )
	{
		(dynamic_pointer_cast<Player>(rider.lock()))->awardStat(GenericStats::flyPig(),GenericStats::param_flyPig());
	}
}

shared_ptr<AgableMob> Pig::getBreedOffspring(shared_ptr<AgableMob> target)
{
	// 4J - added limit to number of animals that can be bred
	if( level->canCreateMore( GetType(), Level::eSpawnType_Breed) )
	{
		return std::make_shared<Pig>(level);
	}
	else
	{
		return nullptr;
	}
}

bool Pig::isFood(shared_ptr<ItemInstance> itemInstance)
{
	return itemInstance != nullptr && itemInstance->id == Item::carrots_Id;
}

ControlledByPlayerGoal *Pig::getControlGoal()
{
	return controlGoal;
}
