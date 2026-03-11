#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.monster.h"
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
	registerAttributes();
	setHealth(getMaxHealth());

	interestedAngle = interestedAngleO = 0.0f;
	m_isWet = isShaking = false;
	shakeAnim = shakeAnimO = 0.0f;

	this->setSize(0.60f, 0.8f);

	getNavigation()->setAvoidWater(true);
	goalSelector.addGoal(1, new FloatGoal(this));
	goalSelector.addGoal(2, sitGoal, false);
	goalSelector.addGoal(3, new LeapAtTargetGoal(this, 0.4));
	goalSelector.addGoal(4, new MeleeAttackGoal(this, 1.0, true));
	goalSelector.addGoal(5, new FollowOwnerGoal(this, 1.0, 10, 2));
	goalSelector.addGoal(6, new BreedGoal(this, 1.0));
	goalSelector.addGoal(7, new RandomStrollGoal(this, 1.0));
	goalSelector.addGoal(8, new BegGoal(this, 8));
	goalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 8));
	goalSelector.addGoal(9, new RandomLookAroundGoal(this));

	targetSelector.addGoal(1, new OwnerHurtByTargetGoal(this));
	targetSelector.addGoal(2, new OwnerHurtTargetGoal(this));
	targetSelector.addGoal(3, new HurtByTargetGoal(this, true));
	targetSelector.addGoal(4, new NonTameRandomTargetGoal(this, typeid(Sheep), 200, false));

	setTame(false); // Initialize health
}

void Wolf::registerAttributes()
{
	TamableAnimal::registerAttributes();

	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.3f);

	if (isTame())
	{
		getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(TAME_HEALTH);
	}
	else
	{
		getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(START_HEALTH);
	}
}

bool Wolf::useNewAi()
{
	return true;
}

void Wolf::setTarget(shared_ptr<LivingEntity> target)
{
	TamableAnimal::setTarget(target);
	if ( target == nullptr )
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

void Wolf::defineSynchedData() 
{
	TamableAnimal::defineSynchedData();
	entityData->define(DATA_HEALTH_ID, getHealth());
	entityData->define(DATA_INTERESTED_ID, static_cast<byte>(0));
	entityData->define(DATA_COLLAR_COLOR, static_cast<byte>(ColoredTile::getTileDataForItemAuxValue(DyePowderItem::RED)));
}

void Wolf::playStepSound(int xt, int yt, int zt, int t)
{
	playSound(eSoundType_MOB_WOLF_STEP, 0.15f, 1);
}

void Wolf::addAdditonalSaveData(CompoundTag *tag) 
{
	TamableAnimal::addAdditonalSaveData(tag);

	tag->putBoolean(L"Angry", isAngry());
	tag->putByte(L"CollarColor", static_cast<byte>(getCollarColor()));
}

void Wolf::readAdditionalSaveData(CompoundTag *tag) 
{
	TamableAnimal::readAdditionalSaveData(tag);

	setAngry(tag->getBoolean(L"Angry"));
	if (tag->contains(L"CollarColor")) setCollarColor(tag->getByte(L"CollarColor"));
}

int Wolf::getAmbientSound() 
{
	if (isAngry()) 
	{
		return eSoundType_MOB_WOLF_GROWL;
	}
	if (random->nextInt(3) == 0) 
	{
		if (isTame() && entityData->getFloat(DATA_HEALTH_ID) < 10) 
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
				playSound(eSoundType_MOB_WOLF_SHAKE, getSoundVolume(), (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);
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
				float yt = static_cast<float>(bb->y0);
				int shakeCount = static_cast<int>(Mth::sin((shakeAnim - 0.4f) * PI) * 7.0f);
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

bool Wolf::hurt(DamageSource *source, float dmg) 
{
	// 4J: Protect owned wolves from untrusted players
	if (isTame())
	{		
		shared_ptr<Entity> entity = source->getDirectEntity();
		if (entity != nullptr && entity->instanceof(eTYPE_PLAYER))
		{
			shared_ptr<Player> attacker = dynamic_pointer_cast<Player>(entity);
			attacker->canHarmPlayer(getOwnerUUID());
		}
	}

	if (isInvulnerable()) return false;
	shared_ptr<Entity> sourceEntity = source->getEntity();
	sitGoal->wantToSit(false);
	if (sourceEntity != nullptr && !(sourceEntity->instanceof(eTYPE_PLAYER) || sourceEntity->instanceof(eTYPE_ARROW))) 
	{
		// Take half damage from non-players and arrows
		dmg = (dmg + 1) / 2;
	}
	return TamableAnimal::hurt(source, dmg);
}

bool Wolf::doHurtTarget(shared_ptr<Entity> target)
{
	int damage = isTame() ? 4 : 2;
	return target->hurt(DamageSource::mobAttack(dynamic_pointer_cast<Mob>(shared_from_this())), damage);
}

void Wolf::setTame(bool value)
{
	TamableAnimal::setTame(value);

	if (value)
	{
		getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(TAME_HEALTH);
	}
	else
	{
		getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(START_HEALTH);
	}
}

void Wolf::tame(const wstring &wsOwnerUUID, bool bDisplayTamingParticles, bool bSetSitting) 
{
	setTame(true);
	setPath(nullptr);
	setTarget(nullptr);
	sitGoal->wantToSit(bSetSitting);
	setHealth(TAME_HEALTH);

	setOwnerUUID(wsOwnerUUID);

	// We'll not show the taming particles if this is a baby wolf
	spawnTamingParticles(bDisplayTamingParticles);
}

bool Wolf::mobInteract(shared_ptr<Player> player) 
{
	shared_ptr<ItemInstance> item = player->inventory->getSelected();

	if (isTame()) 
	{
		if (item != nullptr)
		{
			if(dynamic_cast<FoodItem *>(Item::items[item->id]) != nullptr)
			{
				FoodItem *food = dynamic_cast<FoodItem *>( Item::items[item->id] );

				if (food->isMeat() && entityData->getFloat(DATA_HEALTH_ID) < MAX_HEALTH)
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
			}
			else if (item->id == Item::dye_powder_Id)
			{
				int color = ColoredTile::getTileDataForItemAuxValue(item->getAuxValue());
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
				setPath(nullptr);
				setAttackTarget(nullptr);
				setTarget(nullptr);
			}
		}
	}
	else
	{
		if (item != nullptr && item->id == Item::bone->id && !isAngry()) 
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
		if((item != nullptr) && isFood(item))
		{
			return false;
		}
	}
	return TamableAnimal::mobInteract(player);
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
		return (0.55f - (MAX_HEALTH - entityData->getFloat(DATA_HEALTH_ID)) * 0.02f) * PI;
	}
	return 0.20f * PI;
}

bool Wolf::isFood(shared_ptr<ItemInstance> item)
{
	if (item == nullptr) return false;
	if (dynamic_cast<FoodItem *>(Item::items[item->id]) == nullptr) return false;
	return static_cast<FoodItem *>(Item::items[item->id])->isMeat();
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
		entityData->set(DATA_FLAGS_ID, static_cast<byte>(current | 0x02));
	} 
	else
	{
		entityData->set(DATA_FLAGS_ID, static_cast<byte>(current & ~0x02));
	}
}

int Wolf::getCollarColor()
{
	return entityData->getByte(DATA_COLLAR_COLOR) & 0xF;
}

void Wolf::setCollarColor(int color)
{
	entityData->set(DATA_COLLAR_COLOR, static_cast<byte>(color & 0xF));
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
		shared_ptr<Wolf> pBabyWolf = std::make_shared<Wolf>(level);

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
	if (value)
	{
		entityData->set(DATA_INTERESTED_ID, static_cast<byte>(1));
	}
	else
	{
		entityData->set(DATA_INTERESTED_ID, static_cast<byte>(0));
	}
}

bool Wolf::canMate(shared_ptr<Animal> animal)
{
	if (animal == shared_from_this()) return false;
	if (!isTame()) return false;

	if (!animal->instanceof(eTYPE_WOLF)) return false;
	shared_ptr<Wolf> partner = dynamic_pointer_cast<Wolf>(animal);

	if (partner == nullptr) return false;
	if (!partner->isTame()) return false;
	if (partner->isSitting()) return false;

	return isInLove() && partner->isInLove();
}

bool Wolf::isInterested()
{
	return entityData->getByte(DATA_INTERESTED_ID) == 1;
}

bool Wolf::removeWhenFarAway()
{
	return !isTame() && tickCount > SharedConstants::TICKS_PER_SECOND * 60 * 2;
}

bool Wolf::wantsToAttack(shared_ptr<LivingEntity> target, shared_ptr<LivingEntity> owner)
{
	// filter un-attackable mobs
	if (target->GetType() == eTYPE_CREEPER || target->GetType() == eTYPE_GHAST)
	{
		return false;
	}
	// never target wolves that has this player as owner
	if (target->GetType() == eTYPE_WOLF)
	{
		shared_ptr<Wolf> wolfTarget = dynamic_pointer_cast<Wolf>(target);
		if (wolfTarget->isTame() && wolfTarget->getOwner() == owner)
		{
			return false;
		}
	}
	if ( target->instanceof(eTYPE_PLAYER) && owner->instanceof(eTYPE_PLAYER) && !dynamic_pointer_cast<Player>(owner)->canHarmPlayer(dynamic_pointer_cast<Player>(target) ))
	{
		// pvp is off
		return false;
	}
	// don't attack tame horses
	if ((target->GetType() == eTYPE_HORSE) && dynamic_pointer_cast<EntityHorse>(target)->isTamed())
	{
		return false;
	}
	return true;
}