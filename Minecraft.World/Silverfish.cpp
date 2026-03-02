#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.h"
#include "..\Minecraft.Client\Textures.h"
#include "Silverfish.h"
#include "SoundTypes.h"



Silverfish::Silverfish(Level *level) : Monster( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that the derived version of the function is called
	health = getMaxHealth();

	this->textureIdx = TN_MOB_SILVERFISH;// 4J was "/mob/silverfish.png";
	this->setSize(0.3f, 0.7f);
	runSpeed = 0.6f;

	// 4J - Brought forward damage from 1.2.3
	attackDamage = 1;
}

int Silverfish::getMaxHealth()
{
	return 8;
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


bool Silverfish::hurt(DamageSource *source, int dmg)
{
	if (lookForFriends <= 0 && (dynamic_cast<EntityDamageSource *>(source) != NULL || source == DamageSource::magic))
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
		DamageSource *damageSource = DamageSource::mobAttack( dynamic_pointer_cast<Mob>( shared_from_this() ) );
		target->hurt(damageSource, attackDamage);
		delete damageSource;
	}

}

void Silverfish::playStepSound(int xt, int yt, int zt, int t)
{
	level->playSound(shared_from_this(), eSoundType_MOB_SILVERFISH_STEP, 1, 1);
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
							level->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, baseX + xOff, baseY + yOff, baseZ + zOff,
								Tile::monsterStoneEgg_Id + (level->getData(baseX + xOff, baseY + yOff, baseZ + zOff) << Tile::TILE_NUM_SHIFT));
							level->setTile(baseX + xOff, baseY + yOff, baseZ + zOff, 0);
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

	if (attackTarget == NULL && !isPathFinding())
	{
		// if the silverfish isn't doing anything special, it will merge
		// with any rock tile it is nearby
		int tileX = Mth::floor(x), tileY = Mth::floor(y + .5f), tileZ = Mth::floor(z);
		int facing = random->nextInt(6);

		int tile = level->getTile(tileX + Facing::STEP_X[facing], tileY + Facing::STEP_Y[facing], tileZ + Facing::STEP_Z[facing]);
		if (StoneMonsterTile::isCompatibleHostBlock(tile))
		{
			level->setTileAndData(tileX + Facing::STEP_X[facing], tileY + Facing::STEP_Y[facing], tileZ + Facing::STEP_Z[facing], Tile::monsterStoneEgg_Id, StoneMonsterTile::getDataForHostBlock(tile));
			spawnAnim();
			remove();
		}
		else
		{
			findRandomStrollLocation();
		}

	}
	else if (attackTarget != NULL && !isPathFinding())
	{
		attackTarget = nullptr;
	}
}

float Silverfish::getWalkTargetValue(int x, int y, int z)
{
	// silverfish LOVES stone =)
	if (level->getTile(x, y - 1, z) == Tile::rock_Id) return 10;
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
		return nearestPlayer == NULL;
	}
	return false;
}

MobType Silverfish::getMobType()
{
	return ARTHROPOD;
}