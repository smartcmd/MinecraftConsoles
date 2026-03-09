#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.h"
#include "..\Minecraft.Client\Textures.h"
#include "Silverfish.h"
#include "SoundTypes.h"



Silverfish::Silverfish(Level *level) : Monster( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	setSize(0.3f, 0.7f);
}

void Silverfish::registerAttributes()
{
	Monster::registerAttributes();

	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(8);
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.6f);
	getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(1);
}

bool Silverfish::makeStepSound()
{
	return false;
}

shared_ptr<Entity> Silverfish::findAttackTarget()
{
#ifndef _FINAL_BUILD
	if(app.GetMobsDontAttackEnabled())
	{
		return shared_ptr<Player>();
	}
#endif

	double maxDist = 8;
	return level->getNearestAttackablePlayer(shared_from_this(), maxDist);
}

int Silverfish::getAmbientSound()
{
	// 4J - brought sound change forward from 1.2.3
	return eSoundType_MOB_SILVERFISH_AMBIENT;
}

int Silverfish::getHurtSound()
{
	// 4J - brought sound change forward from 1.2.3
	return eSoundType_MOB_SILVERFISH_HURT;
}

int Silverfish::getDeathSound()
{
	// 4J - brought sound change forward from 1.2.3
	return eSoundType_MOB_SILVERFISH_DEATH;
}


bool Silverfish::hurt(DamageSource *source, float dmg)
{
	if (isInvulnerable()) return false;
	if (lookForFriends <= 0 && (dynamic_cast<EntityDamageSource *>(source) != nullptr || source == DamageSource::magic))
	{
		// look for friends
		lookForFriends = 20;
	}
	return Monster::hurt(source, dmg);
}

void Silverfish::checkHurtTarget(shared_ptr<Entity> target, float d)
{

	//        super.checkHurtTarget(target, d);
	if (attackTime <= 0 && d < 1.2f && target->bb->y1 > bb->y0 && target->bb->y0 < bb->y1)
	{
		attackTime = 20;
		doHurtTarget(target);
	}

}

void Silverfish::playStepSound(int xt, int yt, int zt, int t)
{
	playSound(eSoundType_MOB_SILVERFISH_STEP, 0.15f, 1);
}

int Silverfish::getDeathLoot()
{
	return 0;
}

void Silverfish::tick()
{
	// rotate the whole body to the same angle as the head
	yBodyRot = yRot;

	Monster::tick();
}

void Silverfish::serverAiStep()
{
	Monster::serverAiStep();

	if (level->isClientSide)
	{
		return;
	}

	if (lookForFriends > 0)
	{
		lookForFriends--;
		if (lookForFriends == 0)
		{
			// see if there are any friendly monster eggs nearby
			int baseX = Mth::floor(x);
			int baseY = Mth::floor(y);
			int baseZ = Mth::floor(z);
			bool doBreak = false;

			for (int yOff = 0; !doBreak && yOff <= 5 && yOff >= -5; yOff = (yOff <= 0) ? 1 - yOff : 0 - yOff)
			{
				for (int xOff = 0; !doBreak && xOff <= 10 && xOff >= -10; xOff = (xOff <= 0) ? 1 - xOff : 0 - xOff)
				{
					for (int zOff = 0; !doBreak && zOff <= 10 && zOff >= -10; zOff = (zOff <= 0) ? 1 - zOff : 0 - zOff)
					{
						int tile = level->getTile(baseX + xOff, baseY + yOff, baseZ + zOff);
						if (tile == Tile::monsterStoneEgg_Id)
						{
							if (!level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
							{
								int data = level->getData(baseX + xOff, baseY + yOff, baseZ + zOff);

								Tile *restoreTile = Tile::stone;
								if (data == StoneMonsterTile::HOST_COBBLE)
								{
									restoreTile = Tile::cobblestone;
								}
								if (data == StoneMonsterTile::HOST_STONEBRICK)
								{
									restoreTile = Tile::stoneBrick;
								}

								level->setTileAndData(baseX + xOff, baseY + yOff, baseZ + zOff, restoreTile->id, 0, Tile::UPDATE_ALL);
							}
							else
							{
								level->destroyTile(baseX + xOff, baseY + yOff, baseZ + zOff, false);
							}
							Tile::monsterStoneEgg->destroy(level, baseX + xOff, baseY + yOff, baseZ + zOff, 0);

							if (random->nextBoolean())
							{
								doBreak = true;
								break;
							}
						}
					}
				}
			}
		}
	}

	if (attackTarget == nullptr && !isPathFinding())
	{
		// if the silverfish isn't doing anything special, it will merge
		// with any rock tile it is nearby
		int tileX = Mth::floor(x), tileY = Mth::floor(y + .5f), tileZ = Mth::floor(z);
		int facing = random->nextInt(6);

		int tile = level->getTile(tileX + Facing::STEP_X[facing], tileY + Facing::STEP_Y[facing], tileZ + Facing::STEP_Z[facing]);
		if (StoneMonsterTile::isCompatibleHostBlock(tile))
		{
			level->setTileAndData(tileX + Facing::STEP_X[facing], tileY + Facing::STEP_Y[facing], tileZ + Facing::STEP_Z[facing], Tile::monsterStoneEgg_Id, StoneMonsterTile::getDataForHostBlock(tile), Tile::UPDATE_ALL);
			spawnAnim();
			remove();
		}
		else
		{
			findRandomStrollLocation();
		}

	}
	else if (attackTarget != nullptr && !isPathFinding())
	{
		attackTarget = nullptr;
	}
}

float Silverfish::getWalkTargetValue(int x, int y, int z)
{
	// silverfish LOVES stone =)
	if (level->getTile(x, y - 1, z) == Tile::stone_Id) return 10;
	return Monster::getWalkTargetValue(x, y, z);
}

bool Silverfish::isDarkEnoughToSpawn()
{
	return true;
}

bool Silverfish::canSpawn()
{
	if (Monster::canSpawn())
	{
		shared_ptr<Player> nearestPlayer = level->getNearestPlayer(shared_from_this(), 5.0);
		return nearestPlayer == nullptr;
	}
	return false;
}

MobType Silverfish::getMobType()
{
	return ARTHROPOD;
}