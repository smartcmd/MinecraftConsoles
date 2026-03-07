#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.phys.h"
#include "SynchedEntityData.h"
#include "StringHelpers.h"
#include "../Minecraft.Client/Textures.h"
#include "GenericStats.h"
#include "Ozelot.h"

const float Ozelot::SNEAK_SPEED = 0.18f;
const float Ozelot::WALK_SPEED = 0.23f;
const float Ozelot::FOLLOW_SPEED = 0.3f;
const float Ozelot::SPRINT_SPEED = 0.4f;

const int Ozelot::DATA_TYPE_ID = 18;

const int Ozelot::TYPE_OZELOT = 0;
const int Ozelot::TYPE_BLACK = 1;
const int Ozelot::TYPE_RED = 2;
const int Ozelot::TYPE_SIAMESE = 3;

Ozelot::Ozelot(Level *level) : TamableAnimal(level)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that the derived version of the function is called
	health = getMaxHealth();

	this->textureIdx = TN_MOB_OZELOT; // "/mob/ozelot.png";
	this->setSize(0.6f, 0.8f);

	getNavigation()->setAvoidWater(true);
	goalSelector.addGoal(1, new FloatGoal(this));
	goalSelector.addGoal(2, sitGoal, false);
	goalSelector.addGoal(3, temptGoal = new TemptGoal(this, SNEAK_SPEED, Item::fish_raw_Id, true), false);
	goalSelector.addGoal(4, new AvoidPlayerGoal(this, typeid(Player), 16, WALK_SPEED, SPRINT_SPEED));
	goalSelector.addGoal(5, new FollowOwnerGoal(this, FOLLOW_SPEED, 10, 5));
	goalSelector.addGoal(6, new OcelotSitOnTileGoal(this, SPRINT_SPEED));
	goalSelector.addGoal(7, new LeapAtTargetGoal(this, 0.3f));
	goalSelector.addGoal(8, new OzelotAttackGoal(this));
	goalSelector.addGoal(9, new BreedGoal(this, WALK_SPEED));
	goalSelector.addGoal(10, new RandomStrollGoal(this, WALK_SPEED));
	goalSelector.addGoal(11, new LookAtPlayerGoal(this, typeid(Player), 10));

	targetSelector.addGoal(1, new NonTameRandomTargetGoal(this, typeid(Chicken), 14, 750, false));
}

void Ozelot::defineSynchedData()
{
	TamableAnimal::defineSynchedData();

	entityData->define(DATA_TYPE_ID, (byte) TYPE_OZELOT);
}

void Ozelot::serverAiMobStep()
{
	if (getMoveControl()->hasWanted())
	{
		float speed = getMoveControl()->getSpeed();
		if (speed == SNEAK_SPEED)
		{
			setSneaking(true);
			setSprinting(false);
		}
		else if (speed == SPRINT_SPEED)
		{
			setSneaking(false);
			setSprinting(true);
		}
		else
		{
			setSneaking(false);
			setSprinting(false);
		}
	}
	else
	{
		setSneaking(false);
		setSprinting(false);
	}
}

bool Ozelot::removeWhenFarAway()
{
	return Animal::removeWhenFarAway() && !isTame();
}

int Ozelot::getTexture()
{
	switch (getCatType())
	{
	case TYPE_OZELOT:
		return TN_MOB_OZELOT; //"/mob/ozelot.png";
	case TYPE_BLACK:
		return TN_MOB_CAT_BLACK; //"/mob/cat_black.png";
	case TYPE_RED:
		return TN_MOB_CAT_RED; //"/mob/cat_red.png";
	case TYPE_SIAMESE:
		return TN_MOB_CAT_SIAMESE; //"/mob/cat_siamese.png";
	}
	return TamableAnimal::getTexture();
}

bool Ozelot::useNewAi()
{
	return true;
}

int Ozelot::getMaxHealth()
{
	return 10;
}

void Ozelot::causeFallDamage(float distance)
{
	// do nothing
}

void Ozelot::addAdditonalSaveData(CompoundTag *tag)
{
	TamableAnimal::addAdditonalSaveData(tag);
	tag->putInt(L"CatType", getCatType());
}

void Ozelot::readAdditionalSaveData(CompoundTag *tag)
{
	TamableAnimal::readAdditionalSaveData(tag);
	if(isTame())
	{
		setCatType(tag->getInt(L"CatType"));
	}
	else
	{
		setCatType(TYPE_OZELOT);
	}
}

int Ozelot::getAmbientSound()
{
	if (isTame())
	{
		if (isInLove())
		{
			return eSoundType_MOB_CAT_PURR;
		}
		if (random->nextInt(4) == 0)
		{
			return eSoundType_MOB_CAT_PURREOW;
		}
		return eSoundType_MOB_CAT_MEOW;
	}

	return -1;
}

int Ozelot::getHurtSound()
{
	return eSoundType_MOB_CAT_HITT;
}

int Ozelot::getDeathSound()
{
	return eSoundType_MOB_CAT_HITT;
}

float Ozelot::getSoundVolume()
{
	return 0.4f;
}

int Ozelot::getDeathLoot()
{
	return Item::leather_Id;
}

bool Ozelot::doHurtTarget(shared_ptr<Entity> target)
{
	return target->hurt(DamageSource::mobAttack(dynamic_pointer_cast<Mob>(shared_from_this())), 3);
}

bool Ozelot::hurt(DamageSource *source, int dmg)
{
	sitGoal->wantToSit(false);
	return TamableAnimal::hurt(source, dmg);
}

void Ozelot::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
}

bool Ozelot::interact(shared_ptr<Player> player)
{
	shared_ptr<ItemInstance> item = player->inventory->getSelected();
	if (isTame())
	{
		if (equalsIgnoreCase(player->getUUID(), getOwnerUUID()))
		{
			if (!level->isClientSide && !isFood(item))
			{
				sitGoal->wantToSit(!isSitting());
			}
		}
	}
	else
	{
		if (temptGoal->isRunning() && item != NULL && item->id == Item::fish_raw_Id && player->distanceToSqr(shared_from_this()) < 3 * 3)
		{
			// 4J-PB - don't lose the fish in creative mode
			if (!player->abilities.instabuild) item->count--;
			if (item->count <= 0)
			{
				player->inventory->setItem(player->inventory->selected, nullptr);
			}

			if (!level->isClientSide)
			{
				if (random->nextInt(3) == 0)
				{
					setTame(true);

					// 4J-JEV, hook for durango event.
					player->awardStat(GenericStats::tamedEntity(eTYPE_OZELOT),GenericStats::param_tamedEntity(eTYPE_OZELOT));

					setCatType(1 + level->random->nextInt(3));
					setOwnerUUID(player->getUUID());
					spawnTamingParticles(true);
					sitGoal->wantToSit(true);
					level->broadcastEntityEvent(shared_from_this(), EntityEvent::TAMING_SUCCEEDED);
				}
				else
				{
					spawnTamingParticles(false);
					level->broadcastEntityEvent(shared_from_this(), EntityEvent::TAMING_FAILED);
				}
			}
			return true;
		}
	}
	return TamableAnimal::interact(player);
}

shared_ptr<AgableMob> Ozelot::getBreedOffspring(shared_ptr<AgableMob> target)
{
	// 4J - added limit to number of animals that can be bred
	if( level->canCreateMore( GetType(), Level::eSpawnType_Breed) )
	{
		shared_ptr<Ozelot> offspring = shared_ptr<Ozelot>( new Ozelot(level) );
		if (isTame())
		{
			offspring->setOwnerUUID(getOwnerUUID());
			offspring->setTame(true);
			offspring->setCatType(getCatType());
		}
		return offspring;
	}
	else
	{
		return nullptr;
	}
}

bool Ozelot::isFood(shared_ptr<ItemInstance> itemInstance)
{
	return itemInstance != NULL && itemInstance->id == Item::fish_raw_Id;
}

bool Ozelot::canMate(shared_ptr<Animal> animal)
{
	if (animal == shared_from_this()) return false;
	if (!isTame()) return false;

	shared_ptr<Ozelot> partner = dynamic_pointer_cast<Ozelot>(animal);
	if (partner == NULL) return false;
	if (!partner->isTame()) return false;

	return isInLove() && partner->isInLove();
}

int Ozelot::getCatType()
{
	return entityData->getByte(DATA_TYPE_ID);
}

void Ozelot::setCatType(int type)
{
	entityData->set(DATA_TYPE_ID, (byte) type);
}

bool Ozelot::canSpawn()
{
	// artificially make ozelots more rare
	if (level->random->nextInt(3) == 0)
	{
		return false;
	}
	if (level->isUnobstructed(bb) && level->getCubes(shared_from_this(), bb)->empty() && !level->containsAnyLiquid(bb))
	{
		int xt = Mth::floor(x);
		int yt = Mth::floor(bb->y0);
		int zt = Mth::floor(z);
		if (yt < level->seaLevel)
		{
			return false;
		}

		int tile = level->getTile(xt, yt - 1, zt);
		if (tile == Tile::grass_Id || tile == Tile::leaves_Id)
		{
			return true;
		}
	}
	return false;
}

wstring Ozelot::getAName()
{
	if (isTame())
	{
		return L"entity.Cat.name";
	}
	return TamableAnimal::getAName();
}
