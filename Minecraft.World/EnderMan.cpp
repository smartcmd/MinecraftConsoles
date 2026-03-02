#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.phys.h"
#include "com.mojang.nbt.h"
#include "..\Minecraft.Client\Textures.h"
#include "EnderMan.h"



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
	MAY_TAKE[Tile::mushroom1_Id] = true;
	MAY_TAKE[Tile::mushroom2_Id] = true;
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

	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that the derived version of the function is called
	health = getMaxHealth();

	// 4J initialisors
	teleportTime = 0;
	aggroTime = 0;

	this->textureIdx = TN_MOB_ENDERMAN; // 4J was "/mob/enderman.png";
	runSpeed = 0.2f;
	attackDamage = 7;
	setSize(0.6f, 2.9f);
	footSize = 1;
}

int EnderMan::getMaxHealth()
{
	return 40;
}

// Brought forward from 1.2.3
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
	if (this->isInWaterOrRain()) hurt(DamageSource::drown, 1);

	runSpeed = attackTarget != NULL ? 6.5f : 0.3f;

	if (!level->isClientSide)
	{
		if (getCarryingTile() == 0)
		{
			if (random->nextInt(20) == 0)
			{
				int xt = Mth::floor(x - 2 + random->nextDouble() * 4);
				int yt = Mth::floor(y + random->nextDouble() * 3);
				int zt = Mth::floor(z - 2 + random->nextDouble() * 4);
				int t = level->getTile(xt, yt, zt);
				//if (t > 0 && Tile::tiles[t]->isCubeShaped())
				if(EnderMan::MAY_TAKE[t]) // 4J - Brought forward from 1.2.3
				{
					setCarryingTile(level->getTile(xt, yt, zt));
					setCarryingData(level->getData(xt, yt, zt));
					level->setTile(xt, yt, zt, 0);
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
					level->setTileAndData(xt, yt, zt, getCarryingTile(), getCarryingData());
					setCarryingTile(0);
				}
			}
		}
	}

	// 4J - Brought forward particles from 1.2.3
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
			if (level->canSeeSky(Mth::floor(x), Mth::floor(y), Mth::floor(z)) && random->nextFloat() * 30 < (br - 0.4f) * 2)
			{
				// 4J - Brought forward behaviour change from 1.2.3
				//onFire = 20 * 15;
				attackTarget = nullptr;
				setCreepy(false);
				teleport();
			}
		}
	}
	// 4J Brought forward behaviour change from 1.2.3
	if (isInWaterOrRain() || isOnFire())
	{
		attackTarget = nullptr;
		setCreepy(false);
		teleport();
	}
	jumping = false;
	if (attackTarget != NULL)
	{
		this->lookAt(attackTarget, 100, 100);
	}

	if (!level->isClientSide && isAlive())
	{
		if (attackTarget != NULL)
		{
			if ( dynamic_pointer_cast<Player>(attackTarget) != NULL && isLookingAtMe(dynamic_pointer_cast<Player>(attackTarget)))
			{
				xxa = yya = 0;
				runSpeed = 0;
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

			// 4J - Brought forward particle change from 1.2.3
			//level->addParticle(eParticleType_largesmoke, _x, _y, _z, xa, ya, za);
			level->addParticle(eParticleType_ender, _x, _y, _z, xa, ya, za);
		}
		// 4J - moved sounds forward from 1.2.3
		level->playSound(xo, yo, zo, eSoundType_MOB_ENDERMEN_PORTAL, 1, 1);
		level->playSound(shared_from_this(), eSoundType_MOB_ENDERMEN_PORTAL, 1, 1);
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
	// 4J - brought sound change forward from 1.2.3
	return eSoundType_MOB_ENDERMEN_IDLE;
}

int EnderMan::getHurtSound()
{
	// 4J - brought sound change forward from 1.2.3
	return eSoundType_MOB_ENDERMEN_HIT;
}

int EnderMan::getDeathSound()
{
	// 4J - brought sound change forward from 1.2.3
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

bool EnderMan::hurt(DamageSource *source, int damage)
{
	if (dynamic_cast<IndirectEntityDamageSource *>(source) != NULL)
	{
		for (int i = 0; i < 64; i++)
		{
			if (teleport()) return true;
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