#include "stdafx.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.phys.h"
#include "SynchedEntityData.h"
#include "StringHelpers.h"
#include "../Minecraft.Client/Textures.h"
#include "../Minecraft.Client/Minecraft.h"
#include "../Minecraft.Client/MultiPlayerLocalPlayer.h"
#include "GenericStats.h"
#include "Ocelot.h"

const double Ocelot::SNEAK_SPEED_MOD = 0.6;
const double Ocelot::WALK_SPEED_MOD = 0.8;
const double Ocelot::FOLLOW_SPEED_MOD = 1.0;
const double Ocelot::SPRINT_SPEED_MOD = 1.33;

const int Ocelot::DATA_TYPE_ID = 18;

Ocelot::Ocelot(Level *level) : TamableAnimal(level)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	setSize(0.6f, 0.8f);

	getNavigation()->setAvoidWater(true);
	goalSelector.addGoal(1, new FloatGoal(this));
	goalSelector.addGoal(2, sitGoal, false);
	goalSelector.addGoal(3, temptGoal = new TemptGoal(this, SNEAK_SPEED_MOD, Item::fish_raw_Id, true), false);
	goalSelector.addGoal(4, new AvoidPlayerGoal(this, typeid(Player), 16, WALK_SPEED_MOD, SPRINT_SPEED_MOD));
	goalSelector.addGoal(5, new FollowOwnerGoal(this, FOLLOW_SPEED_MOD, 10, 5));
	goalSelector.addGoal(6, new OcelotSitOnTileGoal(this, SPRINT_SPEED_MOD));
	goalSelector.addGoal(7, new LeapAtTargetGoal(this, 0.3f));
	goalSelector.addGoal(8, new OcelotAttackGoal(this));
	goalSelector.addGoal(9, new BreedGoal(this, WALK_SPEED_MOD));
	goalSelector.addGoal(10, new RandomStrollGoal(this, WALK_SPEED_MOD));
	goalSelector.addGoal(11, new LookAtPlayerGoal(this, typeid(Player), 10));

	targetSelector.addGoal(1, new NonTameRandomTargetGoal(this, typeid(Chicken), 750, false));
}

void Ocelot::defineSynchedData()
{
	TamableAnimal::defineSynchedData();

	entityData->define(DATA_TYPE_ID, (byte) 0);
}

void Ocelot::serverAiMobStep()
{
	if (getMoveControl()->hasWanted())
	{
		double speed = getMoveControl()->getSpeedModifier();
		if (speed == SNEAK_SPEED_MOD)
		{
			setSneaking(true);
			setSprinting(false);
		}
		else if (speed == SPRINT_SPEED_MOD)
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

bool Ocelot::removeWhenFarAway()
{
	return Animal::removeWhenFarAway() && !isTame() && tickCount > SharedConstants::TICKS_PER_SECOND * 60 * 2;
}

bool Ocelot::useNewAi()
{
	return true;
}

void Ocelot::registerAttributes()
{
	TamableAnimal::registerAttributes();

	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(10);
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.3f);
}

void Ocelot::causeFallDamage(float distance)
{
	// do nothing
}

void Ocelot::addAdditonalSaveData(CompoundTag *tag)
{
	TamableAnimal::addAdditonalSaveData(tag);
	tag->putInt(L"CatType", getCatType());
}

void Ocelot::readAdditionalSaveData(CompoundTag *tag)
{
	TamableAnimal::readAdditionalSaveData(tag);
	if(isTame())
	{
		setCatType(tag->getInt(L"CatType"));
	}
	else
	{
		setCatType(TYPE_OCELOT);
	}
}

int Ocelot::getAmbientSound()
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

int Ocelot::getHurtSound()
{
	return eSoundType_MOB_CAT_HIT;
}

int Ocelot::getDeathSound()
{
	return eSoundType_MOB_CAT_HIT;
}

float Ocelot::getSoundVolume()
{
	return 0.4f;
}

int Ocelot::getDeathLoot()
{
	return Item::leather_Id;
}

bool Ocelot::doHurtTarget(shared_ptr<Entity> target)
{
	return target->hurt(DamageSource::mobAttack(dynamic_pointer_cast<Mob>(shared_from_this())), 3);
}

bool Ocelot::hurt(DamageSource *source, float dmg)
{
	if (isInvulnerable()) return false;
	sitGoal->wantToSit(false);
	return TamableAnimal::hurt(source, dmg);
}

void Ocelot::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
}

bool Ocelot::mobInteract(shared_ptr<Player> player)
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
					player->awardStat(GenericStats::tamedEntity(eTYPE_OCELOT),GenericStats::param_tamedEntity(eTYPE_OCELOT));

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
	return TamableAnimal::mobInteract(player);
}

shared_ptr<AgableMob> Ocelot::getBreedOffspring(shared_ptr<AgableMob> target)
{
	// 4J - added limit to number of animals that can be bred
	if( level->canCreateMore( GetType(), Level::eSpawnType_Breed) )
	{
		shared_ptr<Ocelot> offspring = shared_ptr<Ocelot>( new Ocelot(level) );
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

bool Ocelot::isFood(shared_ptr<ItemInstance> itemInstance)
{
	return itemInstance != NULL && itemInstance->id == Item::fish_raw_Id;
}

bool Ocelot::canMate(shared_ptr<Animal> animal)
{
	if (animal == shared_from_this()) return false;
	if (!isTame()) return false;

	shared_ptr<Ocelot> partner = dynamic_pointer_cast<Ocelot>(animal);
	if (partner == NULL) return false;
	if (!partner->isTame()) return false;

	return isInLove() && partner->isInLove();
}

int Ocelot::getCatType()
{
	return entityData->getByte(DATA_TYPE_ID);
}

void Ocelot::setCatType(int type)
{
	entityData->set(DATA_TYPE_ID, (byte) type);
}

bool Ocelot::canSpawn()
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

wstring Ocelot::getAName()
{
	if (hasCustomName()) return getCustomName();
#ifdef _DEBUG
	if (isTame())
	{
		return L"entity.Cat.name";
	}
	return TamableAnimal::getAName();
#else
	return L"";
#endif
}

MobGroupData *Ocelot::finalizeMobSpawn(MobGroupData *groupData, int extraData /*= 0*/) // 4J Added extraData param
{
	groupData = TamableAnimal::finalizeMobSpawn(groupData);

#ifndef _CONTENT_PACKAGE
	if (app.DebugArtToolsOn() && (extraData != 0))
	{
		setTame(true);
		setCatType(extraData - 1);
		setOwnerUUID(Minecraft::GetInstance()->localplayers[ProfileManager.GetPrimaryPad()]->getUUID());
	}
	else
#endif
		if (level->random->nextInt(7) == 0)
	{
		for (int kitten = 0; kitten < 2; kitten++)
		{
			shared_ptr<Ocelot> ocelot = shared_ptr<Ocelot>( new Ocelot(level) );
			ocelot->moveTo(x, y, z, yRot, 0);
			ocelot->setAge(-20 * 60 * 20);
			level->addEntity(ocelot);
		}
	}
	return groupData;
}

void Ocelot::setSittingOnTile(bool val)
{
	byte current = entityData->getByte(DATA_FLAGS_ID);
	entityData->set(DATA_FLAGS_ID, val ? (byte) (current | 0x02) : (byte) (current & ~0x02) );
}

bool Ocelot::isSittingOnTile()
{
	byte current = entityData->getByte(DATA_FLAGS_ID);
	return (current & 0x02) > 0;
}