#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "Chicken.h"
#include "../Minecraft.Client/Textures.h"
#include "SoundTypes.h"
#include "MobCategory.h"

void Chicken::_init()
{
	sheared = false;
	flap = 0;
	flapSpeed = 0;
	flapping = 1;
	oFlapSpeed = oFlap = 0.0f;
	eggTime = 0;
}

Chicken::Chicken(Level *level) : Animal( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	_init();
	setSize(0.3f, 0.7f); // 4J Changed from 0.4 to 0.7 in 1.8.2
	eggTime = random->nextInt(20 * 60 * 5) + 20 * 60 * 5;

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new PanicGoal(this, 1.4));
	goalSelector.addGoal(2, new BreedGoal(this, 1.0));
	goalSelector.addGoal(3, new TemptGoal(this, 1.0, Item::seeds_wheat_Id, false));
	goalSelector.addGoal(4, new FollowParentGoal(this, 1.1));
	goalSelector.addGoal(5, new RandomStrollGoal(this, 1.0));
	goalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 6));
	goalSelector.addGoal(7, new RandomLookAroundGoal(this));
}

bool Chicken::useNewAi()
{
	return true;
}

void Chicken::registerAttributes()
{
	Animal::registerAttributes();

	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(4);
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25f);
}

void Chicken::aiStep()
{
	Animal::aiStep();

	oFlap = flap;
	oFlapSpeed = flapSpeed;

	flapSpeed += (onGround ? -1 : 4) * 0.3f;
	if (flapSpeed < 0) flapSpeed = 0;
	if (flapSpeed > 1) flapSpeed = 1;

	if (!onGround && flapping < 1) flapping = 1;
	flapping *= 0.9;

	if (!onGround && yd < 0) 
	{
		yd *= 0.6;
	}

	flap += flapping * 2;

	if (!isBaby())
	{
		if (!level->isClientSide && --eggTime <= 0) 
		{
			playSound( eSoundType_MOB_CHICKENPLOP, 1.0f, (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);
			spawnAtLocation(Item::egg->id, 1);
			eggTime = random->nextInt(20 * 60 * 5) + 20 * 60 * 5;
		}
	}

}

void Chicken::causeFallDamage(float distance) 
{
}


int Chicken::getAmbientSound() 
{
	return eSoundType_MOB_CHICKEN_AMBIENT;
}

int Chicken::getHurtSound() 
{
	return eSoundType_MOB_CHICKEN_HURT;
}

int Chicken::getDeathSound() 
{
	return eSoundType_MOB_CHICKEN_HURT;
}

void Chicken::playStepSound(int xt, int yt, int zt, int t)
{
	playSound(eSoundType_MOB_CHICKEN_STEP, 0.15f, 1);
}

int Chicken::getDeathLoot() 
{
	return Item::feather->id;
}

void Chicken::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	// drop some feathers
	int count = random->nextInt(3) + random->nextInt(1 + playerBonusLevel);
	for (int i = 0; i < count; i++)
	{
		spawnAtLocation(Item::feather_Id, 1);
	}
	// and some meat
	if (this->isOnFire()) 
	{
		spawnAtLocation(Item::chicken_cooked_Id, 1);
	}
	else
	{
		spawnAtLocation(Item::chicken_raw_Id, 1);
	}
}

shared_ptr<AgableMob> Chicken::getBreedOffspring(shared_ptr<AgableMob> target)
{
	// 4J - added limit to chickens that can be bred
	if( level->canCreateMore( GetType(), Level::eSpawnType_Breed) )
	{
		return shared_ptr<Chicken>(new Chicken(level));
	}
	else
	{
		return nullptr;
	}
}

bool Chicken::isFood(shared_ptr<ItemInstance> itemInstance)
{
	return (itemInstance->id == Item::seeds_wheat_Id) || (itemInstance->id == Item::netherwart_seeds_Id) || (itemInstance->id == Item::seeds_melon_Id) || (itemInstance->id == Item::seeds_pumpkin_Id);
}
