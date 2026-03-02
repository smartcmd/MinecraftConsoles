#include "stdafx.h"
#include "JavaMath.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.sensing.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.item.alchemy.h"
#include "net.minecraft.world.item.enchantment.h"
#include "com.mojang.nbt.h"
#include "Mob.h"
#include "..\Minecraft.Client\Textures.h"
#include "SoundTypes.h"
#include "BasicTypeContainers.h"
#include "ParticleTypes.h"
#include "GenericStats.h"
#include "ItemEntity.h"

const double Mob::MIN_MOVEMENT_DISTANCE = 0.005;

void Mob::_init()
{
	invulnerableDuration = 20;
	timeOffs = 0.0f;

	yBodyRot = 0;
	yBodyRotO = 0;
	yHeadRot = 0;
	yHeadRotO = 0;

	oRun = 0.0f;
	run = 0.0f;

	animStep = 0.0f;
	animStepO = 0.0f;

	MemSect(31);
	hasHair = true;
	textureIdx = TN_MOB_CHAR;	// 4J was L"/mob/char.png";
	allowAlpha = true;
	rotOffs = 0;
	modelName = L"";
	bobStrength = 1;
	deathScore = 0;
	renderOffset = 0;
	MemSect(0);

	walkingSpeed = 0.1f;
	flyingSpeed = 0.02f;

	oAttackAnim = 0.0f;
	attackAnim = 0.0f;

	lastHealth = 0;
	dmgSpill = 0;

	ambientSoundTime = 0;

	hurtTime = 0;
	hurtDuration = 0;
	hurtDir = 0;
	deathTime = 0;
	attackTime = 0;
	oTilt = 0;
	tilt = 0;

	dead = false;
	xpReward = 0;

	modelNum = -1;
	animSpeed = (float) (Math::random() * 0.9f + 0.1f);

	walkAnimSpeedO = 0.0f;
	walkAnimSpeed = 0.0f;
	walkAnimPos = 0.0f;

	lastHurtByPlayer = nullptr;
	lastHurtByPlayerTime = 0;
	lastHurtByMob = nullptr;
	lastHurtByMobTime = 0;
	lastHurtMob = nullptr;

	arrowCount = 0;
	removeArrowTime = 0;

	lSteps = 0;
	lx = ly = lz = lyr = lxr = 0.0;

	fallTime = 0.0f;

	lastHurt = 0;

	noActionTime = 0;
	xxa = yya = yRotA = 0.0f;
	jumping = false;
	defaultLookAngle = 0.0f;
	runSpeed = 0.7f;
	noJumpDelay = 0;

	lookingAt = nullptr;
	lookTime = 0;

	effectsDirty = true;
	effectColor = 0;

	target = nullptr;
	sensing = NULL;
	speed = 0.0f;

	restrictCenter = new Pos(0, 0, 0);
	restrictRadius = -1.0f;
}

Mob::Mob( Level* level) : Entity(level)
{
	_init();

	// 4J Stu - This will not call the correct derived function, so moving to each derived class
	//health = getMaxHealth();
	health = 0;

	blocksBuilding = true;

	lookControl = new LookControl(this);
	moveControl = new MoveControl(this);
	jumpControl = new JumpControl(this);
	bodyControl = new BodyControl(this);
	navigation = new PathNavigation(this, level, 16);
	sensing = new Sensing(this);

	rotA = (float) (Math::random() + 1) * 0.01f;
	setPos(x, y, z);
	timeOffs = (float) Math::random() * 12398;
	yRot = (float) (Math::random() * PI * 2);
	yHeadRot = yRot;

	this->footSize = 0.5f;
}

Mob::~Mob()
{
	for(AUTO_VAR(it, activeEffects.begin()); it != activeEffects.end(); ++it)
	{
		delete it->second;
	}

	if(lookControl != NULL) delete lookControl;
	if(moveControl != NULL) delete moveControl;
	if(jumpControl != NULL) delete jumpControl;
	if(bodyControl != NULL) delete bodyControl;
	if(navigation != NULL) delete navigation;
	if(sensing != NULL) delete sensing;

	delete restrictCenter;
}

LookControl *Mob::getLookControl()
{
	return lookControl;
}

MoveControl *Mob::getMoveControl()
{
	return moveControl;
}

JumpControl *Mob::getJumpControl()
{
	return jumpControl;
}

PathNavigation *Mob::getNavigation()
{
	return navigation;
}

Sensing *Mob::getSensing()
{
	return sensing;
}

Random *Mob::getRandom()
{
	return random;
}

shared_ptr<Mob> Mob::getLastHurtByMob()
{
	return lastHurtByMob;
}

shared_ptr<Mob> Mob::getLastHurtMob()
{
	return lastHurtMob;
}

void Mob::setLastHurtMob(shared_ptr<Entity> target)
{
	shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(target);
	if (mob != NULL) lastHurtMob = mob;
}

int Mob::getNoActionTime()
{
	return noActionTime;
}

float Mob::getYHeadRot()
{
	return yHeadRot;
}

void Mob::setYHeadRot(float yHeadRot)
{
	this->yHeadRot = yHeadRot;
}

float Mob::getSpeed()
{
	return speed;
}

void Mob::setSpeed(float speed)
{
	this->speed = speed;
	setYya(speed);
}

bool Mob::doHurtTarget(shared_ptr<Entity> target)
{
	setLastHurtMob(target);
	return false;
}

shared_ptr<Mob> Mob::getTarget()
{
	return target;
}

void Mob::setTarget(shared_ptr<Mob> target)
{
	this->target = target;
}

bool Mob::canAttackType(eINSTANCEOF targetType)
{
	return !(targetType == eTYPE_CREEPER || targetType == eTYPE_GHAST);
}

// Called by eatTileGoal
void Mob::ate()
{
}

// might move to navigation, might make area
bool Mob::isWithinRestriction()
{
	return isWithinRestriction(Mth::floor(x), Mth::floor(y), Mth::floor(z));
}

bool Mob::isWithinRestriction(int x, int y, int z)
{
	if (restrictRadius == -1) return true;
	return restrictCenter->distSqr(x, y, z) < restrictRadius * restrictRadius;
}

void Mob::restrictTo(int x, int y, int z, int radius)
{
	restrictCenter->set(x, y, z);
	restrictRadius = radius;
}

Pos *Mob::getRestrictCenter()
{
	return restrictCenter;
}

float Mob::getRestrictRadius()
{
	return restrictRadius;
}

void Mob::clearRestriction()
{
	restrictRadius = -1;
}

bool Mob::hasRestriction()
{
	return restrictRadius != -1;
}

void Mob::setLastHurtByMob(shared_ptr<Mob> hurtBy)
{
	lastHurtByMob = hurtBy;
	lastHurtByMobTime = lastHurtByMob != NULL ? PLAYER_HURT_EXPERIENCE_TIME : 0;
}

void Mob::defineSynchedData() 
{
	entityData->define(DATA_EFFECT_COLOR_ID, effectColor);
}

bool Mob::canSee(shared_ptr<Entity> target) 
{
	HitResult *hres = level->clip(Vec3::newTemp(x, y + getHeadHeight(), z), Vec3::newTemp(target->x, target->y + target->getHeadHeight(), target->z));
	bool retVal = (hres == NULL);
	delete hres;
	return retVal;
}

int Mob::getTexture() 
{
	return textureIdx;
}

bool Mob::isPickable() 
{
	return !removed;
}

bool Mob::isPushable() 
{
	return !removed;
}

float Mob::getHeadHeight() 
{
	return bbHeight * 0.85f;
}

int Mob::getAmbientSoundInterval() 
{
	return 20 * 4;
}

void Mob::playAmbientSound() 
{
	MemSect(31);
	int ambient = getAmbientSound();
	if (ambient != -1) 
	{
		level->playSound(shared_from_this(), ambient, getSoundVolume(), getVoicePitch());
	}
	MemSect(0);
}

void Mob::baseTick() 
{
	oAttackAnim = attackAnim;
	Entity::baseTick();

	if (isAlive() && random->nextInt(1000) < ambientSoundTime++) 
	{
		ambientSoundTime = -getAmbientSoundInterval();

		playAmbientSound();		
	}

	if (isAlive() && isInWall()) 
	{
		hurt(DamageSource::inWall, 1);
	}

	if (isFireImmune() || level->isClientSide) clearFire();

	if (isAlive() && isUnderLiquid(Material::water) && !isWaterMob() && activeEffects.find(MobEffect::waterBreathing->id) == activeEffects.end()) 
	{
		setAirSupply(decreaseAirSupply(getAirSupply()));
		if (getAirSupply() == -20)
		{
			setAirSupply(0);
			if(canCreateParticles())
			{
				for (int i = 0; i < 8; i++)
				{
					float xo = random->nextFloat() - random->nextFloat();
					float yo = random->nextFloat() - random->nextFloat();
					float zo = random->nextFloat() - random->nextFloat();
					level->addParticle(eParticleType_bubble, x + xo, y + yo, z + zo, xd, yd, zd);
				}
			}
			hurt(DamageSource::drown, 2);
		}

		clearFire();
	} 
	else 
	{
		setAirSupply(TOTAL_AIR_SUPPLY);
	}

	oTilt = tilt;

	if (attackTime > 0) attackTime--;
	if (hurtTime > 0) hurtTime--;
	if (invulnerableTime > 0) invulnerableTime--;
	if (health <= 0) 
	{
		tickDeath();
	}

	if (lastHurtByPlayerTime > 0) lastHurtByPlayerTime--;
	else
	{
		// Note - this used to just set to nullptr, but that has to create a new shared_ptr and free an old one, when generally this won't be doing anything at all. This
		// is the lightweight but ugly alternative
		if( lastHurtByPlayer )
		{
			lastHurtByPlayer.reset();
		}
	}
	if (lastHurtMob != NULL && !lastHurtMob->isAlive()) lastHurtMob = nullptr;

	if (lastHurtByMob != NULL)
	{
		if (!lastHurtByMob->isAlive()) setLastHurtByMob(nullptr);
		else if (lastHurtByMobTime > 0) lastHurtByMobTime--;
		else setLastHurtByMob(nullptr);
	}

	// update effects
	tickEffects();

	animStepO = animStep;

	yBodyRotO = yBodyRot;
	yHeadRotO = yHeadRot;
	yRotO = yRot;
	xRotO = xRot;
}

void Mob::tickDeath()
{	
	deathTime++;
	if (deathTime == 20) 
	{
		// 4J Stu - Added level->isClientSide check from 1.2 to fix XP orbs being created client side
		if(!level->isClientSide && (lastHurtByPlayerTime > 0 || isAlwaysExperienceDropper()) )
		{
			if (!isBaby())
			{
				int xpCount = this->getExperienceReward(lastHurtByPlayer);
				while (xpCount > 0)
				{
					int newCount = ExperienceOrb::getExperienceValue(xpCount);
					xpCount -= newCount;
					level->addEntity(shared_ptr<ExperienceOrb>( new ExperienceOrb(level, x, y, z, newCount) ) );
				}
			}
		}

		remove();
		for (int i = 0; i < 20; i++) 
		{
			double xa = random->nextGaussian() * 0.02;
			double ya = random->nextGaussian() * 0.02;
			double za = random->nextGaussian() * 0.02;
			level->addParticle(eParticleType_explode, x + random->nextFloat() * bbWidth * 2 - bbWidth, y + random->nextFloat() * bbHeight, z + random->nextFloat() * bbWidth * 2 - bbWidth, xa, ya, za);
		}
	}
}

int Mob::decreaseAirSupply(int currentSupply)
{
	return currentSupply - 1;
}

int Mob::getExperienceReward(shared_ptr<Player> killedBy)
{
	return xpReward;
}

bool Mob::isAlwaysExperienceDropper()
{
	return false;
}

void Mob::spawnAnim() 
{
	for (int i = 0; i < 20; i++) 
	{
		double xa = random->nextGaussian() * 0.02;
		double ya = random->nextGaussian() * 0.02;
		double za = random->nextGaussian() * 0.02;
		double dd = 10;
		level->addParticle(eParticleType_explode, x + random->nextFloat() * bbWidth * 2 - bbWidth - xa * dd, y + random->nextFloat() * bbHeight - ya * dd, z + random->nextFloat() * bbWidth * 2 - bbWidth - za
			* dd, xa, ya, za);
	}
}

void Mob::rideTick() 
{
	Entity::rideTick();
	oRun = run;
	run = 0;
	fallDistance = 0;
}

void Mob::lerpTo(double x, double y, double z, float yRot, float xRot, int steps) 
{
	heightOffset = 0;
	lx = x;
	ly = y;
	lz = z;
	lyr = yRot;
	lxr = xRot;

	lSteps = steps;
}

void Mob::superTick() 
{
	Entity::tick();
}

void Mob::tick() 
{
	Entity::tick();

	if (arrowCount > 0)
	{
		if (removeArrowTime <= 0)
		{
			removeArrowTime = 20 * 3;
		}
		removeArrowTime--;
		if (removeArrowTime <= 0)
		{
			arrowCount--;
		}
	}

	aiStep();

	double xd = x - xo;
	double zd = z - zo;

	float sideDist = xd * xd + zd * zd;

	float yBodyRotT = yBodyRot;

	float walkSpeed = 0;
	oRun = run;
	float tRun = 0;
	if (sideDist <= 0.05f * 0.05f) 
	{
		// animStep = 0;
	} 
	else 
	{
		tRun = 1;
		walkSpeed = sqrt(sideDist) * 3;
		yBodyRotT = ((float) atan2(zd, xd) * 180 / (float) PI - 90);
	}
	if (attackAnim > 0) 
	{
		yBodyRotT = yRot;
	}
	if (!onGround) 
	{
		tRun = 0;
	}
	run = run + (tRun - run) * 0.3f;

	/*
	* float yBodyRotD = yRot-yBodyRot; while (yBodyRotD < -180) yBodyRotD
	* += 360; while (yBodyRotD >= 180) yBodyRotD -= 360; yBodyRot +=
	* yBodyRotD * 0.1f;
	*/

	if (useNewAi())
	{
		bodyControl->clientTick();
	}
	else
	{
		float yBodyRotD = Mth::wrapDegrees(yBodyRotT - yBodyRot);
		yBodyRot += yBodyRotD * 0.3f;

		float headDiff = Mth::wrapDegrees(yRot - yBodyRot);
		bool behind = headDiff < -90 || headDiff >= 90;
		if (headDiff < -75) headDiff = -75;
		if (headDiff >= 75) headDiff = +75;
		yBodyRot = yRot - headDiff;
		if (headDiff * headDiff > 50 * 50) 
		{
			yBodyRot += headDiff * 0.2f;
		}

		if (behind) 
		{
			walkSpeed *= -1;
		}
	}
	while (yRot - yRotO < -180)
		yRotO -= 360;
	while (yRot - yRotO >= 180)
		yRotO += 360;

	while (yBodyRot - yBodyRotO < -180)
		yBodyRotO -= 360;
	while (yBodyRot - yBodyRotO >= 180)
		yBodyRotO += 360;

	while (xRot - xRotO < -180)
		xRotO -= 360;
	while (xRot - xRotO >= 180)
		xRotO += 360;

	while (yHeadRot - yHeadRotO < -180)
		yHeadRotO -= 360;
	while (yHeadRot - yHeadRotO >= 180)
		yHeadRotO += 360;

	animStep += walkSpeed;
}

void Mob::heal(int heal) 
{
	if (health <= 0) return;
	health += heal;
	if (health > getMaxHealth()) health = getMaxHealth();
	invulnerableTime = invulnerableDuration / 2;
}

int Mob::getHealth()
{
	return health;
}

void Mob::setHealth(int health)
{
	this->health = health;
	if (health > getMaxHealth())
	{
		health = getMaxHealth();
	}
}

bool Mob::hurt(DamageSource *source, int dmg) 
{
	// 4J Stu - Reworked this function a bit to show hurt damage on the client before the server responds.
	// Fix for #8823 - Gameplay: Confirmation that a monster or animal has taken damage from an attack is highly delayed
	// 4J Stu - Change to the fix to only show damage when attacked, rather than collision damage
	// Fix for #10299 - When in corners, passive mobs may show that they are taking damage.
	// 4J Stu - Change to the fix for TU6, as source is never NULL due to changes in 1.8.2 to what source actually is
	if (level->isClientSide && dynamic_cast<EntityDamageSource *>(source) == NULL) return false;
	noActionTime = 0;
	if (health <= 0) return false;

	if ( source->isFire() && hasEffect(MobEffect::fireResistance) )
	{
		// 4J-JEV, for new achievement Stayin'Frosty, TODO merge with Java version.
		shared_ptr<Player> plr = dynamic_pointer_cast<Player>(shared_from_this());
		if ( plr != NULL && source == DamageSource::lava ) // Only award when in lava (not any fire).
		{
			plr->awardStat(GenericStats::stayinFrosty(),GenericStats::param_stayinFrosty());
		}
		return false;
	}

	this->walkAnimSpeed = 1.5f;

	bool sound = true;
	if (invulnerableTime > invulnerableDuration / 2.0f) 
	{
		if (dmg <= lastHurt) return false;
		if(!level->isClientSide) actuallyHurt(source, dmg - lastHurt);
		lastHurt = dmg;
		sound = false;
	} 
	else 
	{
		lastHurt = dmg;
		lastHealth = health;
		invulnerableTime = invulnerableDuration;
		if (!level->isClientSide) actuallyHurt(source, dmg);
		hurtTime = hurtDuration = 10;
	}

	hurtDir = 0;

	shared_ptr<Entity> sourceEntity = source->getEntity();
	if (sourceEntity != NULL)
	{
		if (dynamic_pointer_cast<Mob>(sourceEntity) != NULL) {
			setLastHurtByMob(dynamic_pointer_cast<Mob>(sourceEntity));

		}
		if (dynamic_pointer_cast<Player>(sourceEntity) != NULL)
		{
			lastHurtByPlayerTime = PLAYER_HURT_EXPERIENCE_TIME;
			lastHurtByPlayer = dynamic_pointer_cast<Player>(sourceEntity);
		}
		else if (dynamic_pointer_cast<Wolf>(sourceEntity))
		{
			shared_ptr<Wolf> w = dynamic_pointer_cast<Wolf>(sourceEntity);
			if (w->isTame())
			{
				lastHurtByPlayerTime = PLAYER_HURT_EXPERIENCE_TIME;
				lastHurtByPlayer = nullptr;
			}
		}
	}

	if (sound && level->isClientSide)
	{
		return false;
	}

	if (sound)
	{
		level->broadcastEntityEvent(shared_from_this(), EntityEvent::HURT);
		if (source != DamageSource::drown && source != DamageSource::controlledExplosion) markHurt();
		if (sourceEntity != NULL) 
		{
			double xd = sourceEntity->x - x;
			double zd = sourceEntity->z - z;
			while (xd * xd + zd * zd < 0.0001) 
			{
				xd = (Math::random() - Math::random()) * 0.01;
				zd = (Math::random() - Math::random()) * 0.01;
			}
			hurtDir = (float) (atan2(zd, xd) * 180 / PI) - yRot;
			knockback(sourceEntity, dmg, xd, zd);
		} 
		else 
		{
			hurtDir = (float) (int) ((Math::random() * 2) * 180); // 4J This cast is the same as Java
		}
	}

	MemSect(31);
	if (health <= 0) 
	{
		if (sound) level->playSound(shared_from_this(), getDeathSound(), getSoundVolume(), getVoicePitch());
		die(source);
	} 
	else 
	{
		if (sound) level->playSound(shared_from_this(), getHurtSound(), getSoundVolume(), getVoicePitch());
	}
	MemSect(0);

	return true;
}

float Mob::getVoicePitch()
{
	if (isBaby())
	{
		return (random->nextFloat() - random->nextFloat()) * 0.2f + 1.5f;

	}
	return (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f;
}

void Mob::animateHurt() 
{
	hurtTime = hurtDuration = 10;
	hurtDir = 0;
}

int Mob::getArmorValue()
{
	return 0;
}

void Mob::hurtArmor(int damage)
{
}

int Mob::getDamageAfterArmorAbsorb(DamageSource *damageSource, int damage)
{
	if (!damageSource->isBypassArmor())
	{
		int absorb = 25 - getArmorValue();
		int v = (damage) * absorb + dmgSpill;
		hurtArmor(damage);
		damage = v / 25;
		dmgSpill = v % 25;
	}
	return damage;
}

int Mob::getDamageAfterMagicAbsorb(DamageSource *damageSource, int damage)
{
	if (hasEffect(MobEffect::damageResistance))
	{
		int absorbValue = (getEffect(MobEffect::damageResistance)->getAmplifier() + 1) * 5;
		int absorb = 25 - absorbValue;
		int v = (damage) * absorb + dmgSpill;
		damage = v / 25;
		dmgSpill = v % 25;
	}
	return damage;
}

void Mob::actuallyHurt(DamageSource *source, int dmg) 
{
	dmg = getDamageAfterArmorAbsorb(source, dmg);
	dmg = getDamageAfterMagicAbsorb(source, dmg);
	health -= dmg;
}


float Mob::getSoundVolume() 
{
	return 1;
}

int Mob::getAmbientSound() 
{
	return -1;
}

int Mob::getHurtSound() 
{
	return eSoundType_DAMAGE_HURT;
}

int Mob::getDeathSound() 
{
	return eSoundType_DAMAGE_HURT;
}

void Mob::knockback(shared_ptr<Entity> source, int dmg, double xd, double zd) 
{
	hasImpulse = true;
	float dd = (float) sqrt(xd * xd + zd * zd);
	float pow = 0.4f;

	this->xd /= 2;
	this->yd /= 2;
	this->zd /= 2;

	this->xd -= xd / dd * pow;
	this->yd += pow;
	this->zd -= zd / dd * pow;

	if (this->yd > 0.4f) this->yd = 0.4f;
}

void Mob::die(DamageSource *source) 
{
	shared_ptr<Entity> sourceEntity = source->getEntity();
	if (deathScore >= 0 && sourceEntity != NULL) sourceEntity->awardKillScore(shared_from_this(), deathScore);

	if (sourceEntity != NULL) sourceEntity->killed( dynamic_pointer_cast<Mob>( shared_from_this() ) );

	dead = true;

	if (!level->isClientSide) 
	{
		int playerBonus = 0;
		shared_ptr<Player> player = dynamic_pointer_cast<Player>(sourceEntity);
		if (player != NULL)
		{
			playerBonus = EnchantmentHelper::getKillingLootBonus(player->inventory);
		}
		if (!isBaby())
		{
			dropDeathLoot(lastHurtByPlayerTime > 0, playerBonus);
			if (lastHurtByPlayerTime > 0)
			{
				int rareLoot = random->nextInt(200) - playerBonus;
				if (rareLoot < 5)
				{
					dropRareDeathLoot((rareLoot <= 0) ? 1 : 0);
				}
			}
		}

		// 4J-JEV, hook for Durango mobKill event.
		if (player != NULL)
		{
			player->awardStat(GenericStats::killMob(),GenericStats::param_mobKill(player, dynamic_pointer_cast<Mob>(shared_from_this()), source));
		}
	}

	level->broadcastEntityEvent(shared_from_this(), EntityEvent::DEATH);
}

/**
* Drop extra rare loot. Only occurs roughly 5% of the time, rareRootLevel
* is set to 1 (otherwise 0) 1% of the time.
* 
* @param rareLootLevel
*/
void Mob::dropRareDeathLoot(int rareLootLevel)
{

}

void Mob::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel) 
{
	int loot = getDeathLoot();
	if (loot > 0) 
	{
		int count = random->nextInt(3);
		if (playerBonusLevel > 0)
		{
			count += random->nextInt(playerBonusLevel + 1);
		}
		for (int i = 0; i < count; i++)
			spawnAtLocation(loot, 1);
	}
}

int Mob::getDeathLoot() 
{
	return 0;
}

void Mob::causeFallDamage(float distance) 
{
	Entity::causeFallDamage(distance);
	int dmg = (int) ceil(distance - 3);
	if (dmg > 0) 
	{
		// 4J - new sounds here brought forward from 1.2.3
		if (dmg > 4)
		{
			level->playSound(shared_from_this(), eSoundType_DAMAGE_FALL_BIG, 1, 1);
		}
		else
		{
			level->playSound(shared_from_this(), eSoundType_DAMAGE_FALL_SMALL, 1, 1);
		}
		hurt(DamageSource::fall, dmg);

		int t = level->getTile( Mth::floor(x), Mth::floor(y - 0.2f - this->heightOffset), Mth::floor(z));
		if (t > 0) 
		{
			const Tile::SoundType *soundType = Tile::tiles[t]->soundType;
			MemSect(31);
			level->playSound(shared_from_this(), soundType->getStepSound(), soundType->getVolume() * 0.5f, soundType->getPitch() * 0.75f);
			MemSect(0);
		}
	}
}

void Mob::travel(float xa, float ya) 
{
#ifdef __PSVITA__
	// AP - dynamic_pointer_cast is a non-trivial call
	Player *thisPlayer = NULL;
	if( (GetType() & eTYPE_PLAYER) == eTYPE_PLAYER )
	{
		thisPlayer = (Player*) this;
	}
#else
	shared_ptr<Player> thisPlayer = dynamic_pointer_cast<Player>(shared_from_this());
#endif
	if (isInWater() && !(thisPlayer && thisPlayer->abilities.flying) ) 
	{
		double yo = y;
		moveRelative(xa, ya, useNewAi() ? 0.04f : 0.02f);
		move(xd, yd, zd);

		xd *= 0.80f;
		yd *= 0.80f;
		zd *= 0.80f;
		yd -= 0.02;

		if (horizontalCollision && isFree(xd, yd + 0.6f - y + yo, zd)) 
		{
			yd = 0.3f;
		}
	} 
	else if (isInLava() && !(thisPlayer && thisPlayer->abilities.flying) ) 
	{
		double yo = y;
		moveRelative(xa, ya, 0.02f);
		move(xd, yd, zd);
		xd *= 0.50f;
		yd *= 0.50f;
		zd *= 0.50f;
		yd -= 0.02;

		if (horizontalCollision && isFree(xd, yd + 0.6f - y + yo, zd)) 
		{
			yd = 0.3f;
		}
	} 
	else 
	{
		float friction = 0.91f;
		if (onGround) 
		{
			friction = 0.6f * 0.91f;
			int t = level->getTile(Mth::floor(x), Mth::floor(bb->y0) - 1, Mth::floor(z));
			if (t > 0) 
			{
				friction = Tile::tiles[t]->friction * 0.91f;
			}
		}

		float friction2 = (0.6f * 0.6f * 0.91f * 0.91f * 0.6f * 0.91f) / (friction * friction * friction);

		float speed;
		if (onGround)
		{
			if (useNewAi()) speed = getSpeed();
			else speed = walkingSpeed;
			speed *= friction2;
		}
		else speed = flyingSpeed;

		moveRelative(xa, ya, speed);

		friction = 0.91f;
		if (onGround) 
		{
			friction = 0.6f * 0.91f;
			int t = level->getTile( Mth::floor(x), Mth::floor(bb->y0) - 1, Mth::floor(z));
			if (t > 0) 
			{
				friction = Tile::tiles[t]->friction * 0.91f;
			}
		}
		if (onLadder()) 
		{
			float max = 0.15f;
			if (xd < -max) xd = -max;
			if (xd > max) xd = max;
			if (zd < -max) zd = -max;
			if (zd > max) zd = max;
			this->fallDistance = 0;
			if (yd < -0.15) yd = -0.15;
			bool playerSneaking = isSneaking() && dynamic_pointer_cast<Player>(shared_from_this()) != NULL;
			if (playerSneaking && yd < 0) yd = 0;
		}

		move(xd, yd, zd);

		if (horizontalCollision && onLadder()) 
		{
			yd = 0.2;
		}

		yd -= 0.08;
		yd *= 0.98f;
		xd *= friction;
		zd *= friction;
	}

	walkAnimSpeedO = walkAnimSpeed;
	double xxd = x - xo;
	double zzd = z - zo;
	float wst = Mth::sqrt(xxd * xxd + zzd * zzd) * 4;
	if (wst > 1) wst = 1;
	walkAnimSpeed += (wst - walkAnimSpeed) * 0.4f;
	walkAnimPos += walkAnimSpeed;
}

bool Mob::onLadder() 
{
	int xt = Mth::floor(x);
	int yt = Mth::floor(bb->y0);
	int zt = Mth::floor(z);

	// 4J-PB - TU9 - add climbable vines
	int iTile = level->getTile(xt, yt, zt);
	return  (iTile== Tile::ladder_Id) || (iTile== Tile::vine_Id);
}


bool Mob::isShootable() 
{
	return true;
}

void Mob::addAdditonalSaveData(CompoundTag *entityTag) 
{
	entityTag->putShort(L"Health", (short) health);
	entityTag->putShort(L"HurtTime", (short) hurtTime);
	entityTag->putShort(L"DeathTime", (short) deathTime);
	entityTag->putShort(L"AttackTime", (short) attackTime);

	if (!activeEffects.empty())
	{
		ListTag<CompoundTag> *listTag = new ListTag<CompoundTag>();

		for(AUTO_VAR(it, activeEffects.begin()); it != activeEffects.end(); ++it)
		{
			MobEffectInstance *effect = it->second;

			CompoundTag *tag = new CompoundTag();
			tag->putByte(L"Id", (BYTE) effect->getId());
			tag->putByte(L"Amplifier", (char) effect->getAmplifier());
			tag->putInt(L"Duration", effect->getDuration());
			listTag->add(tag);
		}
		entityTag->put(L"ActiveEffects", listTag);
	}
}

void Mob::readAdditionalSaveData(CompoundTag *tag) 
{
	if (health < Short::MIN_VALUE) health = Short::MIN_VALUE;
	health = tag->getShort(L"Health");
	if (!tag->contains(L"Health")) health = getMaxHealth();
	hurtTime = tag->getShort(L"HurtTime");
	deathTime = tag->getShort(L"DeathTime");
	attackTime = tag->getShort(L"AttackTime");

	if (tag->contains(L"ActiveEffects"))
	{
		ListTag<CompoundTag> *effects = (ListTag<CompoundTag> *) tag->getList(L"ActiveEffects");
		for (int i = 0; i < effects->size(); i++)
		{
			CompoundTag *effectTag = effects->get(i);
			int id = effectTag->getByte(L"Id");
			int amplifier = effectTag->getByte(L"Amplifier");
			int duration = effectTag->getInt(L"Duration");

			activeEffects.insert( unordered_map<int, MobEffectInstance *>::value_type( id, new MobEffectInstance(id, duration, amplifier) ) );
		}
	}
}

bool Mob::isAlive() 
{
	return !removed && health > 0;
}

bool Mob::isWaterMob() 
{
	return false;
}

// 4J - added for more accurate lighting of mobs. Takes a weighted average of all tiles touched by the bounding volume of the entity - the method in the Entity class (which used to be used for
// mobs too) simply gets a single tile's lighting value causing sudden changes of lighting values when entities go in and out of lit areas, for example when bobbing in the water.
int Mob::getLightColor(float a)
{
	float accum[2] = {0,0};
	float totVol = ( bb->x1 - bb->x0 ) * ( bb->y1 - bb->y0 ) * ( bb->z1 - bb->z0 );
	int xmin = Mth::floor(bb->x0);
	int xmax = Mth::floor(bb->x1);
	int ymin = Mth::floor(bb->y0);
	int ymax = Mth::floor(bb->y1);
	int zmin = Mth::floor(bb->z0);
	int zmax = Mth::floor(bb->z1);
	for( int xt = xmin; xt <= xmax; xt++ )
		for( int yt = ymin; yt <= ymax; yt++ )
			for( int zt = zmin; zt <= zmax; zt++ )
			{
				float tilexmin = (float)xt;
				float tilexmax = (float)(xt+1);
				float tileymin = (float)yt;
				float tileymax = (float)(yt+1);
				float tilezmin = (float)zt;
				float tilezmax = (float)(zt+1);
				if( tilexmin < bb->x0 ) tilexmin = bb->x0;
				if( tilexmax > bb->x1 ) tilexmax = bb->x1;
				if( tileymin < bb->y0 ) tileymin = bb->y0;
				if( tileymax > bb->y1 ) tileymax = bb->y1;
				if( tilezmin < bb->z0 ) tilezmin = bb->z0;
				if( tilezmax > bb->z1 ) tilezmax = bb->z1;
				float tileVol = ( tilexmax - tilexmin ) * ( tileymax - tileymin ) * ( tilezmax - tilezmin );
				float frac = tileVol / totVol;
				int lc = level->getLightColor(xt, yt, zt, 0);
				accum[0] += frac * (float)( lc & 0xffff );
				accum[1] += frac * (float)( lc >> 16 );
			}

			if( accum[0] > 240.0f ) accum[0] = 240.0f;
			if( accum[1] > 240.0f ) accum[1] = 240.0f;

			return ( ( (int)accum[1])<<16) | ((int)accum[0]);
}

void Mob::setYya(float yya)
{
	this->yya = yya;
}

void Mob::setJumping(bool jump)
{
	jumping = jump;
}

void Mob::aiStep() 
{
	if (noJumpDelay > 0) noJumpDelay--;
	if (lSteps > 0) 
	{
		double xt = x + (lx - x) / lSteps;
		double yt = y + (ly - y) / lSteps;
		double zt = z + (lz - z) / lSteps;

		double yrd = Mth::wrapDegrees(lyr - yRot);
		double xrd = Mth::wrapDegrees(lxr - xRot);

		yRot += (float) ( (yrd) / lSteps );
		xRot += (float) ( (xrd) / lSteps );

		lSteps--;
		this->setPos(xt, yt, zt);
		this->setRot(yRot, xRot);

		// 4J - this collision is carried out to try and stop the lerping push the mob through the floor,
		// in which case gravity can then carry on moving the mob because the collision just won't work anymore.
		// BB for collision used to be calculated as: bb->shrink(1 / 32.0, 0, 1 / 32.0)
		// now using a reduced BB to try and get rid of some issues where mobs pop up the sides of walls, undersides of
		// trees etc.
		AABB *shrinkbb = bb->shrink(0.1, 0, 0.1);
		shrinkbb->y1 = shrinkbb->y0 + 0.1;
		AABBList *collisions = level->getCubes(shared_from_this(), shrinkbb);
		if (collisions->size() > 0)
		{
			double yTop = 0;
			AUTO_VAR(itEnd, collisions->end());
			for (AUTO_VAR(it, collisions->begin()); it != itEnd; it++)
			{
				AABB *ab = *it; //collisions->at(i);
				if (ab->y1 > yTop) yTop = ab->y1;
			}

			yt += yTop - bb->y0;
			setPos(xt, yt, zt);
		}
		if (abs(xd) < MIN_MOVEMENT_DISTANCE) xd = 0;
		if (abs(yd) < MIN_MOVEMENT_DISTANCE) yd = 0;
		if (abs(zd) < MIN_MOVEMENT_DISTANCE) zd = 0;
	}

	if (isImmobile()) 
	{
		jumping = false;
		xxa = 0;
		yya = 0;
		yRotA = 0;
	} 
	else 
	{
		MemSect(25);
		if (isEffectiveAI())
		{
			if (useNewAi())
			{
				newServerAiStep();
			}
			else
			{
				serverAiStep();
				yHeadRot = yRot;
			}
		}
		MemSect(0);
	}

	if (jumping) 
	{
		if (isInWater() || isInLava() ) 
		{
			yd += 0.04f;
		}
		else if (onGround) 
		{
			if (noJumpDelay == 0)
			{
				jumpFromGround();
				noJumpDelay = 10;
			}
		}
	}
	else
	{
		noJumpDelay = 0;
	}


	xxa *= 0.98f;
	yya *= 0.98f;
	yRotA *= 0.9f;

	float normalSpeed = walkingSpeed;
	walkingSpeed *= getWalkingSpeedModifier();
	travel(xxa, yya);
	walkingSpeed = normalSpeed;

	if(!level->isClientSide)
	{
		vector<shared_ptr<Entity> > *entities = level->getEntities(shared_from_this(), this->bb->grow(0.2f, 0, 0.2f));
		if (entities != NULL && !entities->empty()) 
		{
			AUTO_VAR(itEnd, entities->end());
			for (AUTO_VAR(it, entities->begin()); it != itEnd; it++)
			{
				shared_ptr<Entity> e = *it; //entities->at(i);
				if (e->isPushable()) e->push(shared_from_this());
			}
		}
	}
}

bool Mob::useNewAi()
{
	return false;
}

bool Mob::isEffectiveAI()
{
	return !level->isClientSide;
}

bool Mob::isImmobile() 
{
	return health <= 0;
}

bool Mob::isBlocking()
{
	return false;
}

void Mob::jumpFromGround() 
{
	yd = 0.42f;
	if (hasEffect(MobEffect::jump))
	{
		yd += (getEffect(MobEffect::jump)->getAmplifier() + 1) * .1f;
	}
	if (isSprinting())
	{
		float rr = yRot * Mth::RAD_TO_GRAD;

		xd -= Mth::sin(rr) * 0.2f;
		zd += Mth::cos(rr) * 0.2f;
	}
	this->hasImpulse = true;
}

bool Mob::removeWhenFarAway() 
{
	return true;
}

void Mob::checkDespawn() 
{
	shared_ptr<Entity> player = level->getNearestPlayer(shared_from_this(), -1);
	if (player != NULL) 
	{
		double xd = player->x - x;
		double yd = player->y - y;
		double zd = player->z - z;
		double sd = xd * xd + yd * yd + zd * zd;

		if (removeWhenFarAway() && sd > 128 * 128) 
		{
			remove();
		}

		if (noActionTime > 20 * 30 && random->nextInt(800) == 0 && sd > 32 * 32 && removeWhenFarAway()) 
		{
			remove();
		}
		else if (sd < 32 * 32) 
		{
			noActionTime = 0;
		}
	}
}

void Mob::newServerAiStep()
{
	MemSect(51);
	noActionTime++;
	checkDespawn();
	sensing->tick();
	targetSelector.tick();
	goalSelector.tick();
	navigation->tick();
	serverAiMobStep();
	moveControl->tick();
	lookControl->tick();
	jumpControl->tick();
	// Consider this for extra strolling if it is protected against despawning. We aren't interested in ones that aren't protected as the whole point of this
	// extra wandering is to potentially transition from protected to not protected.
	considerForExtraWandering( isDespawnProtected() );
	MemSect(0);
}

void Mob::serverAiMobStep()
{
}

void Mob::serverAiStep() 
{
	noActionTime++;

	checkDespawn();

	xxa = 0;
	yya = 0;

	float lookDistance = 8;
	if (random->nextFloat() < 0.02f) 
	{
		shared_ptr<Player> player = level->getNearestPlayer(shared_from_this(), lookDistance);
		if (player != NULL) 
		{
			lookingAt = player;
			lookTime = 10 + random->nextInt(20);
		} 
		else 
		{
			yRotA = (random->nextFloat() - 0.5f) * 20;
		}
	}

	if (lookingAt != NULL)
	{
		lookAt(lookingAt, 10.0f, (float) getMaxHeadXRot());
		if (lookTime-- <= 0 || lookingAt->removed || lookingAt->distanceToSqr(shared_from_this()) > lookDistance * lookDistance) 
		{
			lookingAt = nullptr;
		}
	} 
	else 
	{
		if (random->nextFloat() < 0.05f) 
		{
			yRotA = (random->nextFloat() - 0.5f) * 20;
		}
		yRot += yRotA;
		xRot = defaultLookAngle;
	}

	bool inWater = isInWater();
	bool inLava = isInLava();
	if (inWater || inLava) jumping = random->nextFloat() < 0.8f;
}

int Mob::getMaxHeadXRot() 
{
	return 40;
}

void Mob::lookAt(shared_ptr<Entity> e, float yMax, float xMax) 
{
	double xd = e->x - x;
	double yd;
	double zd = e->z - z;
	
	shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(e);
	if(mob != NULL)
	{
		yd = (y + getHeadHeight()) - (mob->y + mob->getHeadHeight());
	} 
	else 
	{
		yd = (e->bb->y0 + e->bb->y1) / 2 - (y + getHeadHeight());
	}

	double sd = Mth::sqrt(xd * xd + zd * zd);

	float yRotD = (float) (atan2(zd, xd) * 180 / PI) - 90;
	float xRotD = (float) -(atan2(yd, sd) * 180 / PI);
	xRot = -rotlerp(xRot, xRotD, xMax);
	yRot = rotlerp(yRot, yRotD, yMax);
}

bool Mob::isLookingAtAnEntity() 
{
	return lookingAt != NULL;
}

shared_ptr<Entity> Mob::getLookingAt() 
{
	return lookingAt;
}

float Mob::rotlerp(float a, float b, float max) 
{
	float diff = Mth::wrapDegrees(b - a);
	if (diff > max) 
	{
		diff = max;
	}
	if (diff < -max) 
	{
		diff = -max;
	}
	return a + diff;
}

bool Mob::canSpawn() 
{
	// 4J - altered to use special containsAnyLiquid variant
	return level->isUnobstructed(bb) && level->getCubes(shared_from_this(), bb)->empty() && !level->containsAnyLiquid_NoLoad(bb);
}

void Mob::outOfWorld() 
{
	hurt(DamageSource::outOfWorld, 4);
}

float Mob::getAttackAnim(float a) 
{
	float diff = attackAnim - oAttackAnim;
	if (diff < 0) diff += 1;
	return oAttackAnim + diff * a;
}


Vec3 *Mob::getPos(float a) 
{
	if (a == 1) 
	{
		return Vec3::newTemp(x, y, z);
	}
	double x = xo + (this->x - xo) * a;
	double y = yo + (this->y - yo) * a;
	double z = zo + (this->z - zo) * a;

	return Vec3::newTemp(x, y, z);
}

Vec3 *Mob::getLookAngle() 
{
	return getViewVector(1);
}

Vec3 *Mob::getViewVector(float a) 
{
	if (a == 1) 
	{
		float yCos = Mth::cos(-yRot * Mth::RAD_TO_GRAD - PI);
		float ySin = Mth::sin(-yRot * Mth::RAD_TO_GRAD - PI);
		float xCos = -Mth::cos(-xRot * Mth::RAD_TO_GRAD);
		float xSin = Mth::sin(-xRot * Mth::RAD_TO_GRAD);

		return Vec3::newTemp(ySin * xCos, xSin, yCos * xCos);
	}
	float xRot = xRotO + (this->xRot - xRotO) * a;
	float yRot = yRotO + (this->yRot - yRotO) * a;

	float yCos = Mth::cos(-yRot * Mth::RAD_TO_GRAD - PI);
	float ySin = Mth::sin(-yRot * Mth::RAD_TO_GRAD - PI);
	float xCos = -Mth::cos(-xRot * Mth::RAD_TO_GRAD);
	float xSin = Mth::sin(-xRot * Mth::RAD_TO_GRAD);

	return Vec3::newTemp(ySin * xCos, xSin, yCos * xCos);
}

float Mob::getSizeScale()
{
	return 1.0f;
}

float Mob::getHeadSizeScale()
{
	return 1.0f;
}

HitResult *Mob::pick(double range, float a) 
{
	Vec3 *from = getPos(a);
	Vec3 *b = getViewVector(a);
	Vec3 *to = from->add(b->x * range, b->y * range, b->z * range);
	return level->clip(from, to);
}

int Mob::getMaxSpawnClusterSize() 
{
	return 4;
}

shared_ptr<ItemInstance> Mob::getCarriedItem() 
{
	return nullptr;
}

shared_ptr<ItemInstance> Mob::getArmor(int pos)
{
	// 4J Stu - Not implemented yet
	return nullptr;
	//return equipment[pos + 1];
}

void Mob::handleEntityEvent(byte id) 
{
	if (id == EntityEvent::HURT) 
	{
		this->walkAnimSpeed = 1.5f;

		invulnerableTime = invulnerableDuration;
		hurtTime = hurtDuration = 10;
		hurtDir = 0;

		MemSect(31);
		// 4J-PB -added because villagers have no sounds
		int iHurtSound=getHurtSound();
		if(iHurtSound!=-1)
		{		
			level->playSound(shared_from_this(), iHurtSound, getSoundVolume(), (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);
		}
		MemSect(0);
		hurt(DamageSource::genericSource, 0);
	} 
	else if (id == EntityEvent::DEATH) 
	{
		MemSect(31);
		// 4J-PB -added because villagers have no sounds
		int iDeathSound=getDeathSound();
		if(iDeathSound!=-1)
		{		
			level->playSound(shared_from_this(), iDeathSound, getSoundVolume(), (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);
		}
		MemSect(0);
		health = 0;
		die(DamageSource::genericSource);
	} 
	else 
	{
		Entity::handleEntityEvent(id);
	}
}

bool Mob::isSleeping() 
{
	return false;
}

Icon *Mob::getItemInHandIcon(shared_ptr<ItemInstance> item, int layer) 
{
	return item->getIcon();
}

// 4J added so we can not render mobs before their chunks are loaded - to resolve bug 10327 :Gameplay: NPCs can spawn over chunks that have not yet been streamed and display jitter.
bool Mob::shouldRender(Vec3 *c)
{
	if( !level->reallyHasChunksAt( Mth::floor(bb->x0), Mth::floor(bb->y0), Mth::floor(bb->z0), Mth::floor(bb->x1), Mth::floor(bb->y1), Mth::floor(bb->z1)))
	{
		return false;
	}
	return Entity::shouldRender(c);
}

void Mob::tickEffects()
{
	bool removed = false;
	for(AUTO_VAR(it, activeEffects.begin()); it != activeEffects.end();)
	{
		MobEffectInstance *effect = it->second;
		removed = false;
		if (!effect->tick(dynamic_pointer_cast<Mob>(shared_from_this())))
		{
			if (!level->isClientSide)
			{
				it = activeEffects.erase( it );
				onEffectRemoved(effect);
				delete effect;
				removed = true;
			}
		}
		if(!removed)
		{
			++it;
		}
	}
	if (effectsDirty)
	{
		if (!level->isClientSide)
		{
			if (activeEffects.empty())
			{
				entityData->set(DATA_EFFECT_COLOR_ID, (int) 0);
				setInvisible(false);
				setWeakened(false);
			}
			else
			{
				vector<MobEffectInstance *> values;
				for(AUTO_VAR(it, activeEffects.begin()); it != activeEffects.end();++it)
				{
					values.push_back(it->second);
				}
				int colorValue = PotionBrewing::getColorValue(&values);
				values.clear();
				entityData->set(DATA_EFFECT_COLOR_ID, colorValue);
				setInvisible(hasEffect(MobEffect::invisibility->id));
				setWeakened(hasEffect(MobEffect::weakness->id));
			}
		}
		effectsDirty = false;
	}
	if (random->nextBoolean())
	{
		int colorValue = entityData->getInteger(DATA_EFFECT_COLOR_ID);
		if (colorValue > 0)
		{
			double red = (double) ((colorValue >> 16) & 0xff) / 255.0;
			double green = (double) ((colorValue >> 8) & 0xff) / 255.0;
			double blue = (double) ((colorValue >> 0) & 0xff) / 255.0;

			level->addParticle(eParticleType_mobSpell, x + (random->nextDouble() - 0.5) * bbWidth, y + random->nextDouble() * bbHeight - heightOffset, z + (random->nextDouble() - 0.5) * bbWidth, red, green, blue);
		}
	}
}

void Mob::removeAllEffects()
{
	//Iterator<Integer> effectIdIterator = activeEffects.keySet().iterator();
	//while (effectIdIterator.hasNext())
	for(AUTO_VAR(it, activeEffects.begin()); it != activeEffects.end(); )
	{
		//Integer effectId = effectIdIterator.next();
		MobEffectInstance *effect = it->second;//activeEffects.get(effectId);

		if (!level->isClientSide)
		{
			//effectIdIterator.remove();
			it = activeEffects.erase(it);
			onEffectRemoved(effect);
			delete effect;
		}
		else
		{
			++it;
		}
	}
}

vector<MobEffectInstance *> *Mob::getActiveEffects()
{
	vector<MobEffectInstance *> *active = new vector<MobEffectInstance *>();

	for(AUTO_VAR(it, activeEffects.begin()); it != activeEffects.end(); ++it)
	{
		active->push_back(it->second);
	}

	return active;
}

bool Mob::hasEffect(int id)
{
	return activeEffects.find(id) != activeEffects.end();;
}

bool Mob::hasEffect(MobEffect *effect)
{
	return activeEffects.find(effect->id) != activeEffects.end();
}

MobEffectInstance *Mob::getEffect(MobEffect *effect)
{
	MobEffectInstance *effectInst = NULL;

	AUTO_VAR(it, activeEffects.find(effect->id));
	if(it != activeEffects.end() ) effectInst = it->second;

	return effectInst;
}

void Mob::addEffect(MobEffectInstance *newEffect)
{
	if (!canBeAffected(newEffect))
	{
		return;
	}

	if (activeEffects.find(newEffect->getId()) != activeEffects.end() )
	{
		// replace effect and update
		MobEffectInstance *effectInst = activeEffects.find(newEffect->getId())->second;
		effectInst->update(newEffect);
		onEffectUpdated(effectInst);
	}
	else
	{
		activeEffects.insert( unordered_map<int, MobEffectInstance *>::value_type( newEffect->getId(), newEffect ) );
		onEffectAdded(newEffect);
	}
}

// 4J Added
void Mob::addEffectNoUpdate(MobEffectInstance *newEffect)
{
	if (!canBeAffected(newEffect))
	{
		return;
	}

	if (activeEffects.find(newEffect->getId()) != activeEffects.end() )
	{
		// replace effect and update
		MobEffectInstance *effectInst = activeEffects.find(newEffect->getId())->second;
		effectInst->update(newEffect);
	}
	else
	{
		activeEffects.insert( unordered_map<int, MobEffectInstance *>::value_type( newEffect->getId(), newEffect ) );
	}
}

bool Mob::canBeAffected(MobEffectInstance *newEffect)
{
	if (getMobType() == UNDEAD)
	{
		int id = newEffect->getId();
		if (id == MobEffect::regeneration->id || id == MobEffect::poison->id)
		{
			return false;
		}
	}

	return true;
}

bool Mob::isInvertedHealAndHarm()
{
	return getMobType() == UNDEAD;
}

void Mob::removeEffectNoUpdate(int effectId)
{
	AUTO_VAR(it, activeEffects.find(effectId));
	if (it != activeEffects.end())
	{
		MobEffectInstance *effect = it->second;
		if(effect != NULL)
		{
			delete effect;
		}
		activeEffects.erase(it);
	}
}

void Mob::removeEffect(int effectId)
{
	AUTO_VAR(it, activeEffects.find(effectId));
	if (it != activeEffects.end())
	{
		MobEffectInstance *effect = it->second;
		if(effect != NULL)
		{
			onEffectRemoved(effect);
			delete effect;
		}
		activeEffects.erase(it);
	}
}

void Mob::onEffectAdded(MobEffectInstance *effect)
{
	effectsDirty = true;
}

void Mob::onEffectUpdated(MobEffectInstance *effect)
{
	effectsDirty = true;
}

void Mob::onEffectRemoved(MobEffectInstance *effect)
{
	effectsDirty = true;
}

float Mob::getWalkingSpeedModifier()
{
	float speed = 1.0f;
	if (hasEffect(MobEffect::movementSpeed))
	{
		speed *= 1.0f + .2f * (getEffect(MobEffect::movementSpeed)->getAmplifier() + 1);
	}
	if (hasEffect(MobEffect::movementSlowdown))
	{
		speed *= 1.0f - .15f * (getEffect(MobEffect::movementSlowdown)->getAmplifier() + 1);
	}
	return speed;
}

void Mob::teleportTo(double x, double y, double z) 
{
	moveTo(x, y, z, yRot, xRot);
}

bool Mob::isBaby()
{
	return false;
}

MobType Mob::getMobType()
{
	return UNDEFINED;
}

void Mob::breakItem(shared_ptr<ItemInstance> itemInstance)
{
	level->playSound(shared_from_this(), eSoundType_RANDOM_BREAK, 0.8f, 0.8f + level->random->nextFloat() * 0.4f);

	for (int i = 0; i < 5; i++)
	{
		Vec3 *d = Vec3::newTemp((random->nextFloat() - 0.5) * 0.1, Math::random() * 0.1 + 0.1, 0);
		d->xRot(-xRot * PI / 180);
		d->yRot(-yRot * PI / 180);

		Vec3 *p = Vec3::newTemp((random->nextFloat() - 0.5) * 0.3, -random->nextFloat() * 0.6 - 0.3, 0.6);
		p->xRot(-xRot * PI / 180);
		p->yRot(-yRot * PI / 180);
		p = p->add(x, y + getHeadHeight(), z);
		level->addParticle(PARTICLE_ICONCRACK(itemInstance->getItem()->id,0), p->x, p->y, p->z, d->x, d->y + 0.05, d->z);
	}
}

bool Mob::isInvulnerable()
{
	// 4J-JEV: I have no idea what was going on here (it gets changed in a later java version).
	return invulnerableTime > 0; // invulnerableTime <= invulnerableTime / 2;
}

void Mob::setLevel(Level *level)
{
	Entity::setLevel(level);
	navigation->setLevel(level);
	goalSelector.setLevel(level);
	targetSelector.setLevel(level);
}

void Mob::finalizeMobSpawn()
{

}

bool Mob::canBeControlledByRider()
{
	return false;
}
