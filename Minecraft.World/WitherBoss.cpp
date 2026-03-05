#include "stdafx.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "Mth.h"

#include "SoundTypes.h"

#include "WitherBoss.h"

bool LivingEntitySelector::matches(shared_ptr<Entity> entity) const
{
	if ( entity->instanceof(eTYPE_LIVINGENTITY) )
	{
		return dynamic_pointer_cast<LivingEntity>(entity)->getMobType() != UNDEAD;
	}
	else
	{
		return false;
	}
}

EntitySelector *WitherBoss::livingEntitySelector = new LivingEntitySelector(); 

WitherBoss::WitherBoss(Level *level) : Monster(level)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	for(unsigned int i = 0; i < 2; ++i)
	{	
		xRotHeads[i] = 0.0f;
		yRotHeads[i] = 0.0f;
		xRotOHeads[i] = 0.0f;
		yRotOHeads[i] = 0.0f;
		nextHeadUpdate[i] = 0;
		idleHeadUpdates[i] = 0;
	}
	destroyBlocksTick = 0;

	setSize(.9f, 4);

	//        noPhysics = true;
	fireImmune = true;

	//        noCulling = true;

	getNavigation()->setCanFloat(true);

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(2, new RangedAttackGoal(this, this, 1.0, SharedConstants::TICKS_PER_SECOND * 2, 20));

	goalSelector.addGoal(5, new RandomStrollGoal(this, 1.0));
	goalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 8));
	goalSelector.addGoal(7, new RandomLookAroundGoal(this));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, false));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Mob), 0, false, false, livingEntitySelector));

	xpReward = Enemy::XP_REWARD_BOSS;
}

void WitherBoss::defineSynchedData()
{
	Monster::defineSynchedData();

	entityData->define(DATA_TARGET_A, (int)0);
	entityData->define(DATA_TARGET_B, (int)0);
	entityData->define(DATA_TARGET_C, (int)0);
	entityData->define(DATA_ID_INV, (int)0);
}

void WitherBoss::addAdditonalSaveData(CompoundTag *entityTag)
{
	Monster::addAdditonalSaveData(entityTag);

	entityTag->putInt(L"Invul", getInvulnerableTicks());
}

void WitherBoss::readAdditionalSaveData(CompoundTag *tag)
{
	Monster::readAdditionalSaveData(tag);

	setInvulnerableTicks(tag->getInt(L"Invul"));
}

float WitherBoss::getShadowHeightOffs()
{
	return bbHeight / 8;
}

int WitherBoss::getAmbientSound()
{
	return eSoundType_MOB_WITHER_IDLE; //"mob.wither.idle";
}

int WitherBoss::getHurtSound()
{
	return eSoundType_MOB_WITHER_HURT; //"mob.wither.hurt";
}

int WitherBoss::getDeathSound()
{
	return eSoundType_MOB_WITHER_DEATH; //"mob.wither.death";
}

void WitherBoss::aiStep()
{
	yd *= 0.6f;

	if (!level->isClientSide && getAlternativeTarget(0) > 0)
	{
		shared_ptr<Entity> e = level->getEntity(getAlternativeTarget(0));
		if (e != NULL)
		{
			if ((y < e->y) || (!isPowered() && y < (e->y + 5)))
			{
				if (yd < 0)
				{
					yd = 0;
				}
				yd += (.5f - yd) * .6f;
			}

			double xdist = e->x - x;
			double zdist = e->z - z;
			double distSqr = xdist * xdist + zdist * zdist;
			if (distSqr > 9)
			{
				double sd = Mth::sqrt(distSqr);
				xd += ((xdist / sd) * .5f - xd) * .6f;
				zd += ((zdist / sd) * .5f - zd) * .6f;
			}
		}
	}
	if ((xd * xd + zd * zd) > .05f)
	{
		yRot = (float) atan2(zd, xd) * Mth::RADDEG - 90;
	}
	Monster::aiStep();


	for (int i = 0; i < 2; i++)
	{
		yRotOHeads[i] = yRotHeads[i];
		xRotOHeads[i] = xRotHeads[i];
	}

	for (int i = 0; i < 2; i++)
	{
		int entityId = getAlternativeTarget(i + 1);
		shared_ptr<Entity> e = nullptr;
		if (entityId > 0)
		{
			e = level->getEntity(entityId);
		}
		if (e != NULL)
		{
			double hx = getHeadX(i + 1);
			double hy = getHeadY(i + 1);
			double hz = getHeadZ(i + 1);

			double xd = e->x - hx;
			double yd = e->y + e->getHeadHeight() - hy;
			double zd = e->z - hz;
			double sd = Mth::sqrt(xd * xd + zd * zd);

			float yRotD = (float) (atan2(zd, xd) * 180 / PI) - 90;
			float xRotD = (float) -(atan2(yd, sd) * 180 / PI);
			xRotHeads[i] = rotlerp(xRotHeads[i], xRotD, 40);
			yRotHeads[i] = rotlerp(yRotHeads[i], yRotD, 10);


		}
		else
		{
			yRotHeads[i] = rotlerp(yRotHeads[i], yBodyRot, 10);
		}
	}
	bool _isPowered = isPowered();
	for (int i = 0; i < 3; i++)
	{
		double hx = getHeadX(i);
		double hy = getHeadY(i);
		double hz = getHeadZ(i);

		level->addParticle(eParticleType_smoke, hx + random->nextGaussian() * .3f, hy + random->nextGaussian() * .3f, hz + random->nextGaussian() * .3f, 0, 0, 0);
		if (_isPowered && level->random->nextInt(4) == 0)
		{
			level->addParticle(eParticleType_mobSpell, hx + random->nextGaussian() * .3f, hy + random->nextGaussian() * .3f, hz + random->nextGaussian() * .3f, .7f, .7f, .5f);
		}
	}
	if (getInvulnerableTicks() > 0)
	{
		for (int i = 0; i < 3; i++)
		{
			level->addParticle(eParticleType_mobSpell, x + random->nextGaussian() * 1.0f, y + random->nextFloat() * 3.3f, z + random->nextGaussian() * 1.0f, .7f, .7f, .9f);
		}
	}
}

void WitherBoss::newServerAiStep()
{
	if (getInvulnerableTicks() > 0)
	{
		int newCount = getInvulnerableTicks() - 1;

		if (newCount <= 0)
		{
			level->explode(shared_from_this(), x, y + getHeadHeight(), z, 7, false, level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING));
			level->globalLevelEvent(LevelEvent::SOUND_WITHER_BOSS_SPAWN, (int) x, (int) y, (int) z, 0);
		}

		setInvulnerableTicks(newCount);
		if (tickCount % 10 == 0)
		{
			heal(10);
		}

		return;
	}

	Monster::newServerAiStep();

	for (int i = 1; i < 3; i++)
	{
		if (tickCount >= nextHeadUpdate[i - 1])
		{
			nextHeadUpdate[i - 1] = tickCount + SharedConstants::TICKS_PER_SECOND / 2 + random->nextInt(SharedConstants::TICKS_PER_SECOND / 2);

			if (level->difficulty >= Difficulty::NORMAL && idleHeadUpdates[i - 1]++ > 15)
			{
				float hrange = 10;
				float vrange = 5;
				double xt = Mth::nextDouble(random, x - hrange, x + hrange);
				double yt = Mth::nextDouble(random, y - vrange, y + vrange);
				double zt = Mth::nextDouble(random, z - hrange, z + hrange);
				performRangedAttack(i + 1, xt, yt, zt, true);
				idleHeadUpdates[i - 1] = 0;
			}

			int headTarget = getAlternativeTarget(i);
			if (headTarget > 0)
			{
				shared_ptr<Entity> current = level->getEntity(headTarget);

				// 4J: Added check for instance of living entity, had a problem with IDs being recycled to other entities
				if (current == NULL || !current->instanceof(eTYPE_LIVINGENTITY) || !current->isAlive() || distanceToSqr(current) > 30 * 30 || !canSee(current))
				{
					setAlternativeTarget(i, 0);
				}
				else
				{
					performRangedAttack(i + 1, dynamic_pointer_cast<LivingEntity>(current) );
					nextHeadUpdate[i - 1] = tickCount + SharedConstants::TICKS_PER_SECOND * 2 + random->nextInt(SharedConstants::TICKS_PER_SECOND);
					idleHeadUpdates[i - 1] = 0;
				}
			}
			else
			{
				vector<shared_ptr<Entity> > *entities = level->getEntitiesOfClass(typeid(LivingEntity), bb->grow(20, 8, 20), livingEntitySelector);
				// randomly try to find a target 10 times
				for (int attempt = 0; attempt < 10 && !entities->empty(); attempt++)
				{
					int randomIndex = random->nextInt(entities->size());
					shared_ptr<LivingEntity> selected = dynamic_pointer_cast<LivingEntity>( entities->at(randomIndex) );

					if (selected != shared_from_this() && selected->isAlive() && canSee(selected))
					{
						if ( selected->instanceof(eTYPE_PLAYER) )
						{
							if (!dynamic_pointer_cast<Player>(selected)->abilities.invulnerable)
							{
								assert(selected->instanceof(eTYPE_LIVINGENTITY));
								setAlternativeTarget(i, selected->entityId);
							}
							break;
						}
						else
						{
							assert(selected->instanceof(eTYPE_LIVINGENTITY));
							setAlternativeTarget(i, selected->entityId);
							break;
						}
					}
					// don't pick this again
					entities->erase(entities->begin() + randomIndex);
				}
				delete entities;
			}
		}
	}
	if (getTarget() != NULL)
	{
		assert(getTarget()->instanceof(eTYPE_LIVINGENTITY));
		setAlternativeTarget(0, getTarget()->entityId);
	}
	else
	{
		setAlternativeTarget(0, 0);
	}

	if (destroyBlocksTick > 0)
	{
		destroyBlocksTick--;

		if (destroyBlocksTick == 0 && level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
		{
			// destroy all blocks that are within 1 range, counting from
			// feet and 3 blocks up

			int feet = Mth::floor(y);
			int ox = Mth::floor(x);
			int oz = Mth::floor(z);
			bool destroyed = false;

			for (int xStep = -1; xStep <= 1; xStep++)
			{
				for (int zStep = -1; zStep <= 1; zStep++)
				{
					for (int yStep = 0; yStep <= 3; yStep++)
					{
						int tx = ox + xStep;
						int ty = feet + yStep;
						int tz = oz + zStep;
						int tile = level->getTile(tx, ty, tz);
						if (tile > 0 && tile != Tile::unbreakable_Id && tile != Tile::endPortalTile_Id && tile != Tile::endPortalFrameTile_Id)
						{
							destroyed = level->destroyTile(tx, ty, tz, true) || destroyed;
						}
					}
				}
			}
			if (destroyed)
			{
				level->levelEvent(nullptr, LevelEvent::SOUND_ZOMBIE_DOOR_CRASH, (int) x, (int) y, (int) z, 0);
			}
		}
	}

	if ((tickCount % (SharedConstants::TICKS_PER_SECOND)) == 0)
	{
		heal(1);
	}
}

void WitherBoss::makeInvulnerable()
{
	setInvulnerableTicks(SharedConstants::TICKS_PER_SECOND * 11);
	setHealth(getMaxHealth() / 3);
}

void WitherBoss::makeStuckInWeb()
{
}

int WitherBoss::getArmorValue()
{
	return 4;
}

double WitherBoss::getHeadX(int index)
{
	if (index <= 0)
	{
		return x;
	}
	float headAngle = (yBodyRot + 180 * (index - 1)) / 180.0f * PI;
	float cos = Mth::cos(headAngle);
	return x + cos * 1.3;
}

double WitherBoss::getHeadY(int index)
{
	if (index <= 0)
	{
		return y + 3;
	}
	else
	{
		return y + 2.2;
	}
}

double WitherBoss::getHeadZ(int index)
{
	if (index <= 0)
	{
		return z;
	}
	float headAngle = (yBodyRot + 180 * (index - 1)) / 180.0f * PI;
	float sin = Mth::sin(headAngle);
	return z + sin * 1.3;
}

float WitherBoss::rotlerp(float a, float b, float max)
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

void WitherBoss::performRangedAttack(int head, shared_ptr<LivingEntity> target)
{
	performRangedAttack(head, target->x, target->y + target->getHeadHeight() * .5, target->z, head == 0 && random->nextFloat() < 0.001f);
}

void WitherBoss::performRangedAttack(int head, double tx, double ty, double tz, bool dangerous)
{
	level->levelEvent(nullptr, LevelEvent::SOUND_WITHER_BOSS_SHOOT, (int) x, (int) y, (int) z, 0);

	double hx = getHeadX(head);
	double hy = getHeadY(head);
	double hz = getHeadZ(head);

	double xd = tx - hx;
	double yd = ty - hy;
	double zd = tz - hz;

	shared_ptr<WitherSkull> ie = shared_ptr<WitherSkull>( new WitherSkull(level, dynamic_pointer_cast<LivingEntity>(shared_from_this()), xd, yd, zd) );
	if (dangerous) ie->setDangerous(true);
	ie->y = hy;
	ie->x = hx;
	ie->z = hz;
	level->addEntity(ie);
}

void WitherBoss::performRangedAttack(shared_ptr<LivingEntity> target, float power)
{
	performRangedAttack(0, target);
}

bool WitherBoss::hurt(DamageSource *source, float dmg)
{
	if (isInvulnerable()) return false;
	if (source == DamageSource::drown) return false;
	if (getInvulnerableTicks() > 0)
	{
		return false;
	}

	if (isPowered())
	{
		shared_ptr<Entity> directEntity = source->getDirectEntity();
		if (directEntity != NULL && directEntity->GetType() == eTYPE_ARROW)
		{
			return false;
		}
	}

	shared_ptr<Entity> sourceEntity = source->getEntity();
	if (sourceEntity != NULL)
	{
		if ( sourceEntity->instanceof(eTYPE_PLAYER) )
		{
		}
		else if ( sourceEntity->instanceof(eTYPE_LIVINGENTITY) && dynamic_pointer_cast<LivingEntity>(sourceEntity)->getMobType() == getMobType())
		{
			// can't be harmed by other undead
			return false;
		}
	}
	if (destroyBlocksTick <= 0)
	{
		destroyBlocksTick = SharedConstants::TICKS_PER_SECOND;
	}

	for (int i = 0; i < IDLE_HEAD_UPDATES_SIZE; i++)
	{
		idleHeadUpdates[i] += 3;
	}

	return Monster::hurt(source, dmg);
}

void WitherBoss::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	spawnAtLocation(Item::netherStar_Id, 1);
}

void WitherBoss::checkDespawn()
{
	noActionTime = 0;
}

int WitherBoss::getLightColor(float a)
{
	return SharedConstants::FULLBRIGHT_LIGHTVALUE;
}

bool WitherBoss::isPickable()
{
	return !removed;
}

void WitherBoss::causeFallDamage(float distance)
{
}

void WitherBoss::addEffect(MobEffectInstance *newEffect)
{
	// do nothing
}

bool WitherBoss::useNewAi()
{
	return true;
}

void WitherBoss::registerAttributes()
{
	Monster::registerAttributes();

	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(300);
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.6f);

	// 4J Stu - Don't make it so far!
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(40);
}

float WitherBoss::getHeadYRot(int i)
{
	return yRotHeads[i];
}

float WitherBoss::getHeadXRot(int i)
{
	return xRotHeads[i];
}

int WitherBoss::getInvulnerableTicks()
{
	return entityData->getInteger(DATA_ID_INV);
}

void WitherBoss::setInvulnerableTicks(int invulnerableTicks)
{
	entityData->set(DATA_ID_INV, invulnerableTicks);
}

int WitherBoss::getAlternativeTarget(int headIndex)
{
	return entityData->getInteger(DATA_TARGET_A + headIndex);
}

void WitherBoss::setAlternativeTarget(int headIndex, int entityId)
{
	entityData->set(DATA_TARGET_A + headIndex, entityId);
}

bool WitherBoss::isPowered()
{
	return getHealth() <= getMaxHealth() / 2;
}

MobType WitherBoss::getMobType()
{
	return UNDEAD;
}

void WitherBoss::ride(shared_ptr<Entity> e)
{
	riding = nullptr;
}
