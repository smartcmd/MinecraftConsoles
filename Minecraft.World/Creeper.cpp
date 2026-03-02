#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.damagesource.h"
#include "GeneralStat.h"
#include "Skeleton.h"
#include "Creeper.h"
#include "Arrow.h"
#include "..\Minecraft.Client\Textures.h"
#include "SoundTypes.h"



void Creeper::_init()
{
	swell = 0;
	oldSwell = 0;
}

Creeper::Creeper(Level *level) : Monster( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that the derived version of the function is called
	health = getMaxHealth();

	_init();
	
    this->textureIdx = TN_MOB_CREEPER;	// 4J was L"/mob/creeper.png";

	goalSelector.addGoal(1, new FloatGoal(this));
	goalSelector.addGoal(2, new SwellGoal(this));
	goalSelector.addGoal(3, new AvoidPlayerGoal(this, typeid(Ozelot), 6, 0.25f, 0.30f));
	goalSelector.addGoal(4, new MeleeAttackGoal(this, 0.25f, false));
	goalSelector.addGoal(5, new RandomStrollGoal(this, 0.20f));
	goalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 8));
	goalSelector.addGoal(6, new RandomLookAroundGoal(this));

	targetSelector.addGoal(1, new NearestAttackableTargetGoal(this, typeid(Player), 16, 0, true));
	targetSelector.addGoal(2, new HurtByTargetGoal(this, false));
}

bool Creeper::useNewAi()
{
	return true;
}

int Creeper::getMaxHealth()
{
	return 20;
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
}

void Creeper::readAdditionalSaveData(CompoundTag *tag)
{
    Monster::readAdditionalSaveData(tag);
    entityData->set(DATA_IS_POWERED, (byte) (tag->getBoolean(L"powered") ? 1 : 0));
}

void Creeper::tick()
{
    oldSwell = swell;
    if (isAlive())
	{
        int swellDir = getSwellDir();
        if (swellDir > 0 && swell == 0)
		{
            level->playSound(shared_from_this(), eSoundType_RANDOM_FUSE, 1, 0.5f);
        }
        swell += swellDir;
        if (swell < 0) swell = 0;
		if (swell >= MAX_SWELL)
		{
			swell = MAX_SWELL;
			if (!level->isClientSide)
			{
				bool destroyBlocks = true; //level.getGameRules().getBoolean(GameRules.RULE_MOBGRIEFING);
				if (isPowered()) level->explode(shared_from_this(), x, y, z, 6, destroyBlocks);
				else level->explode(shared_from_this(), x, y, z, 3,destroyBlocks);
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

    if ( dynamic_pointer_cast<Skeleton>(source->getEntity()) != NULL )
	{
        spawnAtLocation(Item::record_01_Id + random->nextInt(12), 1);
    }

	shared_ptr<Player> player = dynamic_pointer_cast<Player>(source->getEntity());
	if ( (dynamic_pointer_cast<Arrow>(source->getDirectEntity()) != NULL) && (player != NULL) )
	{
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
    return (oldSwell + (swell - oldSwell) * a) / (MAX_SWELL - 2);
}

int Creeper::getDeathLoot()
{
    return Item::sulphur->id;
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
