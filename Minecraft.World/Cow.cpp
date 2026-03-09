#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.stats.h"
#include "Cow.h"
#include "..\Minecraft.Client\Textures.h"
#include "MobCategory.h"



Cow::Cow(Level *level) : Animal( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	this->setSize(0.9f, 1.3f);

	getNavigation()->setAvoidWater(true);
	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new PanicGoal(this, 2.0f));
	goalSelector.addGoal(2, new BreedGoal(this, 1.0f));
	goalSelector.addGoal(3, new TemptGoal(this, 1.25f, Item::wheat_Id, false));
	goalSelector.addGoal(4, new FollowParentGoal(this, 1.25f));
	goalSelector.addGoal(5, new RandomStrollGoal(this, 1.0f));
	goalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 6));
	goalSelector.addGoal(7, new RandomLookAroundGoal(this));
}

bool Cow::useNewAi()
{
	return true;
}

void Cow::registerAttributes()
{
	Animal::registerAttributes();

	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(10);
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.2f);
}

int Cow::getAmbientSound() 
{
	return eSoundType_MOB_COW_AMBIENT;
}

int Cow::getHurtSound() 
{
	return eSoundType_MOB_COW_HURT;
}

int Cow::getDeathSound() 
{
	return eSoundType_MOB_COW_HURT;
}

void Cow::playStepSound(int xt, int yt, int zt, int t)
{
	playSound(eSoundType_MOB_COW_STEP, 0.15f, 1);
}

float Cow::getSoundVolume() 
{
	return 0.4f;
}

int Cow::getDeathLoot() 
{
	return Item::leather->id;
}

void Cow::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	// drop some leather
	int count = random->nextInt(3) + random->nextInt(1 + playerBonusLevel);
	for (int i = 0; i < count; i++)
	{
		spawnAtLocation(Item::leather_Id, 1);
	}
	// and some meat
	count = random->nextInt(3) + 1 + random->nextInt(1 + playerBonusLevel);
	for (int i = 0; i < count; i++)
	{
		if (isOnFire())
		{
			spawnAtLocation(Item::beef_cooked_Id, 1);
		}
		else
		{
			spawnAtLocation(Item::beef_raw_Id, 1);
		}
	}
}

bool Cow::mobInteract(shared_ptr<Player> player) 
{
	shared_ptr<ItemInstance> item = player->inventory->getSelected();
	if (item != nullptr && item->id == Item::bucket_empty->id && !player->abilities.instabuild) 
	{
		player->awardStat(GenericStats::cowsMilked(),GenericStats::param_cowsMilked());

		if (item->count-- == 0) 
		{
			player->inventory->setItem(player->inventory->selected, std::make_shared<ItemInstance>(Item::bucket_milk));
		} 
		else if (!player->inventory->add(std::make_shared<ItemInstance>(Item::bucket_milk))) 
		{
			player->drop(std::make_shared<ItemInstance>(Item::bucket_milk));
		}
		
		return true;
	}
	return Animal::mobInteract(player);
}

shared_ptr<AgableMob> Cow::getBreedOffspring(shared_ptr<AgableMob> target)
{
	// 4J - added limit to number of animals that can be bred
	if( level->canCreateMore( GetType(), Level::eSpawnType_Breed) )
	{
		return std::make_shared<Cow>(level);
	}
	else
	{
		return nullptr;
	}
}
