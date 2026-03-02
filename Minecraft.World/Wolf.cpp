#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.level.pathfinder.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.stats.h"
#include "Sheep.h"
#include "Wolf.h"
#include "..\Minecraft.Client\Textures.h"
#include "SoundTypes.h"



Wolf::Wolf(Level *level) : TamableAnimal( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that the derived version of the function is called
	health = getMaxHealth();

	interestedAngle = interestedAngleO = 0.0f;
	m_isWet = isShaking = false;
	shakeAnim = shakeAnimO = 0.0f;

	this->textureIdx = TN_MOB_WOLF;	// 4J - was L"/mob/wolf.png";
	this->setSize(0.60f, 0.8f);
	runSpeed = 0.3f;

	getNavigation()->setAvoidWater(true);
	goalSelector.addGoal(1, new FloatGoal(this));
	goalSelector.addGoal(2, sitGoal, false);
	goalSelector.addGoal(3, new LeapAtTargetGoal(this, 0.4f));
	goalSelector.addGoal(4, new MeleeAttackGoal(this, runSpeed, true));
	goalSelector.addGoal(5, new FollowOwnerGoal(this, runSpeed, 10, 2));
	goalSelector.addGoal(6, new BreedGoal(this, runSpeed));
	goalSelector.addGoal(7, new RandomStrollGoal(this, runSpeed));
	goalSelector.addGoal(8, new BegGoal(this, 8));
	goalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 8));
	goalSelector.addGoal(9, new RandomLookAroundGoal(this));

	targetSelector.addGoal(1, new OwnerHurtByTargetGoal(this));
	targetSelector.addGoal(2, new OwnerHurtTargetGoal(this));
	targetSelector.addGoal(3, new HurtByTargetGoal(this, true));
	targetSelector.addGoal(4, new NonTameRandomTargetGoal(this, typeid(Sheep), 16, 200, false));
}

bool Wolf::useNewAi()
{
	return true;
}

void Wolf::setTarget(shared_ptr<Mob> target)
{
	TamableAnimal::setTarget(target);
	if ( dynamic_pointer_cast<Player>(target) == NULL )
	{
		setAngry(false);
	}
	else if(!isTame())
	{
		setAngry(true);
	}
}

void Wolf::serverAiMobStep()
{
	entityData->set(DATA_HEALTH_ID, getHealth());
}

int Wolf::getMaxHealth()
{
	if (isTame())
	{
		return TAME_HEALTH;
	}
	return START_HEALTH;
}

void Wolf::defineSynchedData() 
{
	TamableAnimal::defineSynchedData();
	entityData->define(DATA_HEALTH_ID, getHealth());
	entityData->define(DATA_INTERESTED_ID, (byte)0);
	entityData->define(DATA_COLLAR_COLOR, (byte) ClothTile::getTileDataForItemAuxValue(DyePowderItem::RED));
}

bool Wolf::makeStepSound() 
{
	return false;
}

int Wolf::getTexture() 
{
	if (isTame()) 
	{
		return TN_MOB_WOLF_TAME;	// 4J was L"/mob/wolf_tame.png";
	}
	if (isAngry()) 
	{
		return TN_MOB_WOLF_ANGRY;	// 4J was L"/mob/wolf_angry.png";
	}
	return TamableAnimal::getTexture();
}

void Wolf::addAdditonalSaveData(CompoundTag *tag) 
{
	TamableAnimal::addAdditonalSaveData(tag);

	tag->putBoolean(L"Angry", isAngry());
	tag->putByte(L"CollarColor", (byte) getCollarColor());
}

void Wolf::readAdditionalSaveData(CompoundTag *tag) 
{
	TamableAnimal::readAdditionalSaveData(tag);

	setAngry(tag->getBoolean(L"Angry"));
	if (tag->contains(L"CollarColor")) setCollarColor(tag->getByte(L"CollarColor"));
}

bool Wolf::removeWhenFarAway() 
{
	return !isTame();
}

int Wolf::getAmbientSound() 
{
	if (isAngry()) 
	{
		return eSoundType_MOB_WOLF_GROWL;
	}
	if (random->nextInt(3) == 0) 
	{
		if (isTame() && entityData->getInteger(DATA_HEALTH_ID) < 10) 
		{
			return eSoundType_MOB_WOLF_WHINE;
		}
		return eSoundType_MOB_WOLF_PANTING;
	}
	return eSoundType_MOB_WOLF_BARK;
}

int Wolf::getHurtSound() 
{
	return eSoundType_MOB_WOLF_HURT;
}

int Wolf::getDeathSound() 
{
	return eSoundType_MOB_WOLF_DEATH;
}

float Wolf::getSoundVolume() 
{
	return 0.4f;
}

int Wolf::getDeathLoot() 
{
	return -1;
}

void Wolf::aiStep() 
{
	TamableAnimal::aiStep();

	if (!level->isClientSide && m_isWet && !isShaking && !isPathFinding() && onGround) 
	{
		isShaking = true;
		shakeAnim = 0;
		shakeAnimO = 0;

		level->broadcastEntityEvent(shared_from_this(), EntityEvent::SHAKE_WETNESS);
	}
}

void Wolf::tick() 
{
	TamableAnimal::tick();

	interestedAngleO = interestedAngle;
	if (isInterested()) 
	{
		interestedAngle = interestedAngle + (1 - interestedAngle) * 0.4f;
	} 
	else 
	{
		interestedAngle = interestedAngle + (0 - interestedAngle) * 0.4f;
	}
	if (isInterested()) 
	{
		lookTime = 10;
	}

	if (isInWaterOrRain()) 
	{
		m_isWet = true;
		isShaking = false;
		shakeAnim = 0;
		shakeAnimO = 0;
	} 
	else if (m_isWet || isShaking) 
	{
		if (isShaking) 
		{

			if (shakeAnim == 0) 
			{
				level->playSound(shared_from_this(), eSoundType_MOB_WOLF_SHAKE, getSoundVolume(), (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);
			}

			shakeAnimO = shakeAnim;
			shakeAnim += 0.05f;

			if (shakeAnimO >= 2) 
			{
				m_isWet = false;
				isShaking = false;
				shakeAnimO = 0;
				shakeAnim = 0;
			}

			if (shakeAnim > 0.4f) 
			{
				float yt = (float) bb->y0;
				int shakeCount = (int) (Mth::sin((shakeAnim - 0.4f) * PI) * 7.0f);
				for (int i = 0; i < shakeCount; i++) 
				{
					float xo = (random->nextFloat() * 2 - 1) * bbWidth * 0.5f;
					float zo = (random->nextFloat() * 2 - 1) * bbWidth * 0.5f;
					level->addParticle(eParticleType_splash, x + xo, yt + 0.8f, z + zo, xd, yd, zd);
				}
			}
		}
	}
}

bool Wolf::isWet() 
{
	return m_isWet;
}

float Wolf::getWetShade(float a) 
{
	return 0.75f + ((shakeAnimO + (shakeAnim - shakeAnimO) * a) / 2.0f) * 0.25f;
}

float Wolf::getBodyRollAngle(float a, float offset) 
{
	float progress = ((shakeAnimO + (shakeAnim - shakeAnimO) * a) + offset) / 1.8f;
	if (progress < 0) 
	{
		progress = 0;
	} 
	else if (progress > 1) 
	{
		progress = 1;
	}
	return Mth::sin(progress * PI) * Mth::sin(progress * PI * 11.0f) * 0.15f * PI;
}

float Wolf::getHeadRollAngle(float a) 
{
	return (interestedAngleO + (interestedAngle - interestedAngleO) * a) * 0.15f * PI;
}

float Wolf::getHeadHeight() 
{
	return bbHeight * 0.8f;
}

int Wolf::getMaxHeadXRot() 
{
	if (isSitting()) 
	{
		return 20;
	}
	return TamableAnimal::getMaxHeadXRot();
}

bool Wolf::hurt(DamageSource *source, int dmg) 
{
	if (isInvulnerable()) return false;
	shared_ptr<Entity> sourceEntity = source->getEntity();
	sitGoal->wantToSit(false);
	if (sourceEntity != NULL && !(dynamic_pointer_cast<Player>(sourceEntity) != NULL || dynamic_pointer_cast<Arrow>(sourceEntity) != NULL)) 
	{
		// take half damage from non-players and arrows
		dmg = (dmg + 1) / 2;
	}
	return TamableAnimal::hurt(source, dmg);
}

bool Wolf::doHurtTarget(shared_ptr<Entity> target)
{
	int damage = isTame() ? 4 : 2;
	return target->hurt(DamageSource::mobAttack(dynamic_pointer_cast<Mob>(shared_from_this())), damage);
}

void Wolf::tame(const wstring &wsOwnerUUID, bool bDisplayTamingParticles, bool bSetSitting) 
{
	setTame(true);
	setPath(NULL);
	setTarget(nullptr);
	sitGoal->wantToSit(bSetSitting);
	setHealth(TAME_HEALTH);

	setOwnerUUID(wsOwnerUUID);

	// We'll not show the taming particles if this is a baby wolf
	spawnTamingParticles(bDisplayTamingParticles);
}

bool Wolf::interact(shared_ptr<Player> player) 
{
	shared_ptr<ItemInstance> item = player->inventory->getSelected();

	if (isTame()) 
	{
		if (item != NULL)
		{
			if(dynamic_cast<FoodItem *>(Item::items[item->id]) != NULL)
			{
				FoodItem *food = dynamic_cast<FoodItem *>( Item::items[item->id] );

				if (food->isMeat())
				{
					if(entityData->getInteger(DATA_HEALTH_ID) < MAX_HEALTH)
					{
						heal(food->getNutrition());
						// 4J-PB - don't lose the bone in creative mode
						if (player->abilities.instabuild==false)
						{
							item->count--;
							if (item->count <= 0) 
							{
								player->inventory->setItem(player->inventory->selected, nullptr);
							}
						}
						return true;
					}
					else return TamableAnimal::interact(player);
				}			
			}
			else if (item->id == Item::dye_powder_Id)
			{
				int color = ClothTile::getTileDataForItemAuxValue(item->getAuxValue());
				if (color != getCollarColor())
				{
					setCollarColor(color);

					if (!player->abilities.instabuild && --item->count <= 0)
					{
						player->inventory->setItem(player->inventory->selected, nullptr);
					}

					return true;
				}
			}
		}
		if (equalsIgnoreCase(player->getUUID(), getOwnerUUID()))
		{
			if (!level->isClientSide && !isFood(item))
			{
				sitGoal->wantToSit(!isSitting());
				jumping = false;
				setPath(NULL);
			}
		}
	}
	else
	{
		if (item != NULL && item->id == Item::bone->id && !isAngry()) 
		{
			// 4J-PB - don't lose the bone in creative mode
			if (player->abilities.instabuild==false)
			{
				item->count--;
				if (item->count <= 0)
				{
					player->inventory->setItem(player->inventory->selected, nullptr);
				}
			}

			if (!level->isClientSide) 
			{
				if (random->nextInt(3) == 0) 
				{
					// 4J : WESTY: Added for new acheivements.
					player->awardStat(GenericStats::tamedEntity(eTYPE_WOLF),GenericStats::param_tamedEntity(eTYPE_WOLF));

					// 4J Changed to this
					tame(player->getUUID(),true,true);

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

		// 4J-PB - stop wild wolves going in to Love Mode (even though they do on Java, but don't breed)
		if((item != NULL) && isFood(item))
		{
			return false;
		}
	}
	return TamableAnimal::interact(player);
}

void Wolf::handleEntityEvent(byte id) 
{
	if (id == EntityEvent::SHAKE_WETNESS)
	{
		isShaking = true;
		shakeAnim = 0;
		shakeAnimO = 0;
	} 
	else 
	{
		TamableAnimal::handleEntityEvent(id);
	}
}

float Wolf::getTailAngle()
{
	if (isAngry()) 
	{
		return 0.49f * PI;
	} 
	else if (isTame()) 
	{
		return (0.55f - (MAX_HEALTH - entityData->getInteger(DATA_HEALTH_ID)) * 0.02f) * PI;
	}
	return 0.20f * PI;
}

bool Wolf::isFood(shared_ptr<ItemInstance> item)
{
	if (item == NULL) return false;
	if (dynamic_cast<FoodItem *>(Item::items[item->id]) == NULL) return false;
	return ((FoodItem *) Item::items[item->id])->isMeat();
}

int Wolf::getMaxSpawnClusterSize()
{
	// 4J - changed - was 8 but we have a limit of only 8 wolves in the world so doesn't seem right potentially spawning them all in once cluster
	return 4;
}

bool Wolf::isAngry() 
{
	return (entityData->getByte(DATA_FLAGS_ID) & 0x02) != 0;
}

void Wolf::setAngry(bool value) 
{
	byte current = entityData->getByte(DATA_FLAGS_ID);
	if (value) 
	{
		entityData->set(DATA_FLAGS_ID, (byte) (current | 0x02));
	} 
	else
	{
		entityData->set(DATA_FLAGS_ID, (byte) (current & ~0x02));
	}
}

int Wolf::getCollarColor()
{
	return entityData->getByte(DATA_COLLAR_COLOR) & 0xF;
}

void Wolf::setCollarColor(int color)
{
	entityData->set(DATA_COLLAR_COLOR, (byte) (color & 0xF));
}

// 4J-PB added for tooltips
int Wolf::GetSynchedHealth()	
{
	return getEntityData()->getInteger(DATA_HEALTH_ID);
}	

shared_ptr<AgableMob> Wolf::getBreedOffspring(shared_ptr<AgableMob> target)
{
	// 4J - added limit to wolves that can be bred
	if( level->canCreateMore( GetType(), Level::eSpawnType_Breed) )
	{
		shared_ptr<Wolf> pBabyWolf = shared_ptr<Wolf>( new Wolf(level) );

		if(!getOwnerUUID().empty())
		{
			// set the baby wolf to be tame, and assign the owner
			pBabyWolf->tame(getOwnerUUID(),false,false);
		}
		return pBabyWolf;
	}
	else
	{
		return nullptr;
	}
}

void Wolf::setIsInterested(bool value)
{
	//byte current = entityData->getByte(DATA_INTERESTED_ID);

	if (value)
	{
		entityData->set(DATA_INTERESTED_ID, (byte) 1);
	}
	else
	{
		entityData->set(DATA_INTERESTED_ID, (byte) 0);
	}
}

bool Wolf::canMate(shared_ptr<Animal> animal)
{
	if (animal == shared_from_this()) return false;
	if (!isTame()) return false;
	shared_ptr<Wolf> partner = dynamic_pointer_cast<Wolf>(animal);
	if (partner == NULL) return false;
	if (!partner->isTame()) return false;
	if (partner->isSitting()) return false;

	return isInLove() && partner->isInLove();
}

bool Wolf::isInterested()
{
	return entityData->getByte(DATA_INTERESTED_ID) == 1;
}
