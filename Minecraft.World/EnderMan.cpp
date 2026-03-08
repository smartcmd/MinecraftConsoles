#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.phys.h"
#include "com.mojang.nbt.h"
#include "../Minecraft.Client/Textures.h"
#include "EnderMan.h"

AttributeModifier *EnderMan::SPEED_MODIFIER_ATTACKING = (new AttributeModifier(eModifierId_MOB_ENDERMAN_ATTACKSPEED, 6.2f, AttributeModifier::OPERATION_ADDITION))->setSerialize(false);

bool EnderMan::MAY_TAKE[256];

void EnderMan::staticCtor()
{
	ZeroMemory(MAY_TAKE, sizeof(bool) * 256);
	MAY_TAKE[Tile::grass_Id] = true;
	MAY_TAKE[Tile::dirt_Id] = true;
	MAY_TAKE[Tile::sand_Id] = true;
	MAY_TAKE[Tile::gravel_Id] = true;
	MAY_TAKE[Tile::flower_Id] = true;
	MAY_TAKE[Tile::rose_Id] = true;
	MAY_TAKE[Tile::mushroom_brown_Id] = true;
	MAY_TAKE[Tile::mushroom_red_Id] = true;
	MAY_TAKE[Tile::tnt_Id] = true;
	MAY_TAKE[Tile::cactus_Id] = true;
	MAY_TAKE[Tile::clay_Id] = true;
	MAY_TAKE[Tile::pumpkin_Id] = true;
	MAY_TAKE[Tile::melon_Id] = true;
	MAY_TAKE[Tile::mycel_Id] = true;
}

EnderMan::EnderMan(Level *level) : Monster( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	// Brought forward from 1.2.3
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	// 4J initialisors
	teleportTime = 0;
	aggroTime = 0;
	lastAttackTarget = nullptr;
	aggroedByPlayer = false;

	setSize(0.6f, 2.9f);
	footSize = 1;
}

void EnderMan::registerAttributes()
{
	Monster::registerAttributes();

	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(40);
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.3f);
	getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(7);
}

void EnderMan::defineSynchedData()
{
	Monster::defineSynchedData();

	entityData->define(DATA_CARRY_ITEM_ID, (byte) 0);
	entityData->define(DATA_CARRY_ITEM_DATA, (byte) 0);
	entityData->define(DATA_CREEPY, (byte) 0);
}

void EnderMan::addAdditonalSaveData(CompoundTag *tag)
{
	Monster::addAdditonalSaveData(tag);
	tag->putShort(L"carried", (short) getCarryingTile());
	tag->putShort(L"carriedData", (short) getCarryingData());
}

void EnderMan::readAdditionalSaveData(CompoundTag *tag)
{
	Monster::readAdditionalSaveData(tag);
	setCarryingTile(tag->getShort(L"carried"));
	setCarryingData(tag->getShort(L"carryingData"));
}

shared_ptr<Entity> EnderMan::findAttackTarget()
{
#ifndef _FINAL_BUILD
	if(app.GetMobsDontAttackEnabled())
	{
		return shared_ptr<Player>();
	}
#endif

	shared_ptr<Player> player = level->getNearestAttackablePlayer(shared_from_this(), 64);
	if (player != NULL)
	{
		if (isLookingAtMe(player))
		{
			aggroedByPlayer = true;
			if (aggroTime == 0) level->playEntitySound(player, eSoundType_MOB_ENDERMAN_STARE, 1, 1);
			if (aggroTime++ == 5)
			{
				aggroTime = 0;
				setCreepy(true);
				return player;
			}
		}
		else
		{
			aggroTime = 0;
		}
	}
	return nullptr;
}

bool EnderMan::isLookingAtMe(shared_ptr<Player> player)
{
	shared_ptr<ItemInstance> helmet = player->inventory->armor[3];
	if (helmet != NULL && helmet->id == Tile::pumpkin_Id) return false;

	Vec3 *look = player->getViewVector(1)->normalize();
	Vec3 *dir = Vec3::newTemp(x - player->x, (bb->y0 + bbHeight / 2) - (player->y + player->getHeadHeight()), z - player->z);
	double dist = dir->length();
	dir = dir->normalize();
	double dot = look->dot(dir);
	if (dot > 1 - 0.025 / dist)
	{
		return player->canSee(shared_from_this());
	}
	return false;
}

void EnderMan::aiStep()
{
	if (isInWaterOrRain()) hurt(DamageSource::drown, 1);

	if (lastAttackTarget != attackTarget)
	{
		AttributeInstance *speed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
		speed->removeModifier(SPEED_MODIFIER_ATTACKING);

		if (attackTarget != NULL)
		{
			speed->addModifier(new AttributeModifier(*SPEED_MODIFIER_ATTACKING));
		}
	}

	lastAttackTarget = attackTarget;

	if (!level->isClientSide)
	{
		if (level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
		{
			if (getCarryingTile() == 0)
			{
				if (random->nextInt(20) == 0)
				{
					int xt = Mth::floor(x - 2 + random->nextDouble() * 4);
					int yt = Mth::floor(y + random->nextDouble() * 3);
					int zt = Mth::floor(z - 2 + random->nextDouble() * 4);
					int t = level->getTile(xt, yt, zt);
					if(MAY_TAKE[t])
					{
						setCarryingTile(level->getTile(xt, yt, zt));
						setCarryingData(level->getData(xt, yt, zt));
						level->setTileAndUpdate(xt, yt, zt, 0);
					}
				}
			}
			else
			{
				if (random->nextInt(2000) == 0)
				{
					int xt = Mth::floor(x - 1 + random->nextDouble() * 2);
					int yt = Mth::floor(y + random->nextDouble() * 2);
					int zt = Mth::floor(z - 1 + random->nextDouble() * 2);
					int t = level->getTile(xt, yt, zt);
					int bt = level->getTile(xt, yt - 1, zt);
					if (t == 0 && bt > 0 && Tile::tiles[bt]->isCubeShaped())
					{
						level->setTileAndData(xt, yt, zt, getCarryingTile(), getCarryingData(), Tile::UPDATE_ALL);
						setCarryingTile(0);
					}
				}
			}
		}
	}


	for (int i = 0; i < 2; i++)
	{
		level->addParticle(eParticleType_ender, x + (random->nextDouble() - 0.5) * bbWidth, y + random->nextDouble() * bbHeight - 0.25f, z + (random->nextDouble() - 0.5) * bbWidth,
			(random->nextDouble() - 0.5) * 2, -random->nextDouble(), (random->nextDouble() - 0.5) * 2);
	}

	if (level->isDay() && !level->isClientSide)
	{
		float br = getBrightness(1);
		if (br > 0.5f)
		{
			if (level->canSeeSky(Mth::floor(x), (int)floor( y + 0.5 ), Mth::floor(z)) && random->nextFloat() * 30 < (br - 0.4f) * 2)
			{
				attackTarget = nullptr;
				setCreepy(false);
				aggroedByPlayer = false;
				teleport();
			}
		}
	}

	if (isInWaterOrRain() || isOnFire())
	{
		attackTarget = nullptr;
		setCreepy(false);
		aggroedByPlayer = false;
		teleport();
	}

	if (isCreepy() && !aggroedByPlayer && random->nextInt(100) == 0)
	{
		setCreepy(false);
	}

	jumping = false;
	if (attackTarget != NULL)
	{
		lookAt(attackTarget, 100, 100);
	}

	if (!level->isClientSide && isAlive())
	{
		if (attackTarget != NULL)
		{
			if ( attackTarget->instanceof(eTYPE_PLAYER) && isLookingAtMe(dynamic_pointer_cast<Player>(attackTarget)))
			{
				if (attackTarget->distanceToSqr(shared_from_this()) < 4 * 4)
				{
					teleport();
				}
				teleportTime = 0;
			}
			else if (attackTarget->distanceToSqr(shared_from_this()) > 16 * 16)
			{
				if (teleportTime++ >= 30)
				{
					if (teleportTowards(attackTarget))
					{
						teleportTime = 0;
					}
				}
			}
		}
		else
		{
			setCreepy(false);
			teleportTime = 0;
		}
	}

	Monster::aiStep();
}

bool EnderMan::teleport()
{
	double xx = x + (random->nextDouble() - 0.5) * 64;
	double yy = y + (random->nextInt(64) - 32);
	double zz = z + (random->nextDouble() - 0.5) * 64;
	return teleport(xx, yy, zz);
}

bool EnderMan::teleportTowards(shared_ptr<Entity> e)
{
	Vec3 *dir = Vec3::newTemp(x - e->x, bb->y0 + bbHeight / 2 - e->y + e->getHeadHeight(), z - e->z);
	dir = dir->normalize();
	double d = 16;
	double xx = x + (random->nextDouble() - 0.5) * 8 - dir->x * d;
	double yy = y + (random->nextInt(16) - 8) - dir->y * d;
	double zz = z + (random->nextDouble() - 0.5) * 8 - dir->z * d;
	return teleport(xx, yy, zz);
}

bool EnderMan::teleport(double xx, double yy, double zz)
{
	double xo = x;
	double yo = y;
	double zo = z;

	x = xx;
	y = yy;
	z = zz;
	bool ok = false;
	int xt = Mth::floor(x);
	int yt = Mth::floor(y);
	int zt = Mth::floor(z);

	if (level->hasChunkAt(xt, yt, zt))
	{
		bool landed = false;
		while (!landed && yt > 0)
		{
			int t = level->getTile(xt, yt - 1, zt);
			if (t == 0 || !(Tile::tiles[t]->material->blocksMotion()))
			{
				y--;
				yt--;
			}
			else
			{
				landed = true;
			}
		}
		if (landed)
		{
			setPos(x, y, z);
			if (level->getCubes(shared_from_this(), bb)->empty() && !level->containsAnyLiquid(bb))
			{
				ok = true;
			}
		}
	}


	if (ok)
	{
		int count = 128;
		for (int i = 0; i < count; i++)
		{
			double d = i / (count - 1.0);
			float xa = (random->nextFloat() - 0.5f) * 0.2f;
			float ya = (random->nextFloat() - 0.5f) * 0.2f;
			float za = (random->nextFloat() - 0.5f) * 0.2f;

			double _x = xo + (x - xo) * d + (random->nextDouble() - 0.5) * bbWidth * 2;
			double _y = yo + (y - yo) * d + random->nextDouble() * bbHeight;
			double _z = zo + (z - zo) * d + (random->nextDouble() - 0.5) * bbWidth * 2;

			level->addParticle(eParticleType_ender, _x, _y, _z, xa, ya, za);
		}
		level->playSound(xo, yo, zo, eSoundType_MOB_ENDERMEN_PORTAL, 1, 1);
		playSound(eSoundType_MOB_ENDERMEN_PORTAL, 1, 1);
		return true;
	}
	else
	{
		setPos(xo, yo, zo);
		return false;
	}
}

int EnderMan::getAmbientSound()
{
	return isCreepy()? eSoundType_MOB_ENDERMAN_SCREAM : eSoundType_MOB_ENDERMEN_IDLE;
}

int EnderMan::getHurtSound()
{
	return eSoundType_MOB_ENDERMEN_HIT;
}

int EnderMan::getDeathSound()
{
	return eSoundType_MOB_ENDERMEN_DEATH;
}

int EnderMan::getDeathLoot()
{
	return Item::enderPearl_Id;
}

void EnderMan::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	int loot = getDeathLoot();
	if (loot > 0)
	{
		int count = random->nextInt(2 + playerBonusLevel);
		for (int i = 0; i < count; i++)
			spawnAtLocation(loot, 1);
	}
}

// 4J Brought forward from 1.2.3 to help fix Enderman behaviour
void EnderMan::setCarryingTile(int carryingTile)
{
	entityData->set(DATA_CARRY_ITEM_ID, (byte) (carryingTile & 0xff));
}

int EnderMan::getCarryingTile()
{
	return entityData->getByte(DATA_CARRY_ITEM_ID);
}

void EnderMan::setCarryingData(int carryingData)
{
	entityData->set(DATA_CARRY_ITEM_DATA, (byte) (carryingData & 0xff));
}

int EnderMan::getCarryingData()
{
	return entityData->getByte(DATA_CARRY_ITEM_DATA);
}

bool EnderMan::hurt(DamageSource *source, float damage)
{
	if (isInvulnerable()) return false;
	setCreepy(true);

	if ( dynamic_cast<EntityDamageSource *>(source) != NULL && source->getEntity()->instanceof(eTYPE_PLAYER))
	{
		aggroedByPlayer = true;
	}

	if (dynamic_cast<IndirectEntityDamageSource *>(source) != NULL)
	{
		aggroedByPlayer = false;
		for (int i = 0; i < 64; i++)
		{
			if (teleport())
			{
				return true;
			}
		}
		return false;
	}
	return Monster::hurt(source, damage);
}

bool EnderMan::isCreepy()
{
	return entityData->getByte(DATA_CREEPY) > 0;
}

void EnderMan::setCreepy(bool creepy)
{
	entityData->set(DATA_CREEPY, (byte)(creepy ? 1 : 0));
}