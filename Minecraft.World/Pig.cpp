#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.stats.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.h"
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

	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that the derived version of the function is called
	health = getMaxHealth();

	this->textureIdx = TN_MOB_PIG; // 4J - was L"/mob/pig.png";
	this->setSize(0.9f, 0.9f);

	getNavigation()->setAvoidWater(true);
	float walkSpeed = 0.25f;
	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new PanicGoal(this, 0.38f));
	goalSelector.addGoal(2, controlGoal = new ControlledByPlayerGoal(this, 0.34f, walkSpeed));
	goalSelector.addGoal(3, new BreedGoal(this, walkSpeed));
	goalSelector.addGoal(4, new TemptGoal(this, 0.3f, Item::carrotOnAStick_Id, false));
	goalSelector.addGoal(4, new TemptGoal(this, 0.25f, Item::carrots_Id, false));
	goalSelector.addGoal(5, new FollowParentGoal(this, 0.28f));
	goalSelector.addGoal(6, new RandomStrollGoal(this, walkSpeed));
	goalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Player), 6));
	goalSelector.addGoal(8, new RandomLookAroundGoal(this));
}

bool Pig::useNewAi()
{
	return true;
}

int Pig::getMaxHealth()
{
	return 10;
}

bool Pig::canBeControlledByRider()
{
	shared_ptr<ItemInstance> item = dynamic_pointer_cast<Player>(rider.lock())->getCarriedItem();

	return item != NULL && item->id == Item::carrotOnAStick_Id;
}

void Pig::defineSynchedData() 
{
	Animal::defineSynchedData();
	entityData->define(DATA_SADDLE_ID, (byte) 0);
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

bool Pig::interact(shared_ptr<Player> player)
{
	if(!Animal::interact(player))
	{
		if (hasSaddle() && !level->isClientSide && (rider.lock() == NULL || rider.lock() == player)) 
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
		entityData->set(DATA_SADDLE_ID, (byte) 1);
	} 
	else 
	{
		entityData->set(DATA_SADDLE_ID, (byte) 0);
	}
}

void Pig::thunderHit(const LightningBolt *lightningBolt)
{
	if (level->isClientSide) return;
	shared_ptr<PigZombie> pz = shared_ptr<PigZombie>( new PigZombie(level) );
	pz->moveTo(x, y, z, yRot, xRot);
	level->addEntity(pz);
	remove();
}

void Pig::causeFallDamage(float distance) 
{
	Animal::causeFallDamage(distance);
	if (distance > 5 && dynamic_pointer_cast<Player>( rider.lock() ) != NULL)
	{
		(dynamic_pointer_cast<Player>(rider.lock()))->awardStat(GenericStats::flyPig(),GenericStats::param_flyPig());
	}
}

shared_ptr<AgableMob> Pig::getBreedOffspring(shared_ptr<AgableMob> target)
{
	// 4J - added limit to number of animals that can be bred
	if( level->canCreateMore( GetType(), Level::eSpawnType_Breed) )
	{
		return shared_ptr<Pig>( new Pig(level) );
	}
	else
	{
		return nullptr;
	}
}

bool Pig::isFood(shared_ptr<ItemInstance> itemInstance)
{
	return itemInstance != NULL && itemInstance->id == Item::carrots_Id;
}

ControlledByPlayerGoal *Pig::getControlGoal()
{
	return controlGoal;
}
