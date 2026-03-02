#include "stdafx.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.stats.h"
#include "net.minecraft.world.damagesource.h"
#include "SharedConstants.h"
#include "Skeleton.h"
#include "..\Minecraft.Client\Textures.h"
#include "SoundTypes.h"



Skeleton::Skeleton(Level *level) : Monster( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that the derived version of the function is called
	health = getMaxHealth();

    this->textureIdx = TN_MOB_SKELETON; // 4J was L"/mob/skeleton.png";

	runSpeed = 0.25f;

	goalSelector.addGoal(1, new FloatGoal(this));
	goalSelector.addGoal(2, new RestrictSunGoal(this));
	goalSelector.addGoal(3, new FleeSunGoal(this, runSpeed));
	goalSelector.addGoal(4, new ArrowAttackGoal(this, runSpeed, ArrowAttackGoal::ArrowType, SharedConstants::TICKS_PER_SECOND * 3));
	goalSelector.addGoal(5, new RandomStrollGoal(this, runSpeed));
	goalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 8));
	goalSelector.addGoal(6, new RandomLookAroundGoal(this));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, false));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 16, 0, true));
}

bool Skeleton::useNewAi()
{
	return true;
}

int Skeleton::getMaxHealth()
{
	return 20;
}

int Skeleton::getAmbientSound() 
{
    return eSoundType_MOB_SKELETON_AMBIENT;
}

int Skeleton::getHurtSound()
{
    return eSoundType_MOB_SKELETON_HURT;
}

int Skeleton::getDeathSound() 
{
    return eSoundType_MOB_SKELETON_HURT;
}

shared_ptr<ItemInstance> Skeleton::bow;

shared_ptr<ItemInstance> Skeleton::getCarriedItem() 
{
    return bow;
}

MobType Skeleton::getMobType() 
{
	return UNDEAD;
}

void Skeleton::aiStep()
{
	// isClientSide check brought forward from 1.8  (I assume it's related to the lighting changes)
	if (level->isDay() && !level->isClientSide) 
	{
        float br = getBrightness(1);
        if (br > 0.5f)
		{
            if (level->canSeeSky( Mth::floor(x), Mth::floor(y), Mth::floor(z)) && random->nextFloat() * 30 < (br - 0.4f) * 2)
			{
                setOnFire(8);
            }
        }
    }

    Monster::aiStep();
}

void Skeleton::die(DamageSource *source)
{
	Monster::die(source);
	shared_ptr<Player> player = dynamic_pointer_cast<Player>( source->getEntity() );
	if ( dynamic_pointer_cast<Arrow>( source->getDirectEntity() ) != NULL && player != NULL)
	{
		double xd = player->x - x;
		double zd = player->z - z;
		if (xd * xd + zd * zd >= 50 * 50)
		{
			player->awardStat(GenericStats::snipeSkeleton(), GenericStats::param_snipeSkeleton());
		}
	}
}

int Skeleton::getDeathLoot() 
{
    return Item::arrow->id;
}

void Skeleton::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
    // drop some arrows
    int count = random->nextInt(3 + playerBonusLevel);
    for (int i = 0; i < count; i++) 
	{
        spawnAtLocation(Item::arrow->id, 1);
    }
    // and some bones
    count = random->nextInt(3 + playerBonusLevel);
    for (int i = 0; i < count; i++)
	{
        spawnAtLocation(Item::bone->id, 1);
    }
}

void Skeleton::dropRareDeathLoot(int rareLootLevel)
{
	if (rareLootLevel > 0)
	{
		shared_ptr<ItemInstance> bow = shared_ptr<ItemInstance>( new ItemInstance(Item::bow) );
		EnchantmentHelper::enchantItem(random, bow, 5);
		spawnAtLocation(bow, 0);
	}
	else
	{
		spawnAtLocation(Item::bow_Id, 1);
	}
}

void Skeleton::staticCtor()
{
	Skeleton::bow = shared_ptr<ItemInstance>( new ItemInstance(Item::bow, 1) );
}
