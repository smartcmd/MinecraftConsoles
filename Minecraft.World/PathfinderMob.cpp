#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.pathfinder.h"
#include "net.minecraft.world.phys.h"
#include "SharedConstants.h"
#include "PathfinderMob.h"



PathfinderMob::PathfinderMob(Level *level) : Mob( level )
{
	path = NULL;
	attackTarget = nullptr;
	holdGround = false;
	fleeTime = 0;
}

bool PathfinderMob::shouldHoldGround()
{
	return false;
}

PathfinderMob::~PathfinderMob()
{
	delete path;
}

void PathfinderMob::serverAiStep()
{
	if (fleeTime > 0) fleeTime--;
	holdGround = shouldHoldGround();
	float maxDist = 16;

	if (attackTarget == NULL)
	{
		attackTarget = findAttackTarget();
		if (attackTarget != NULL)
		{
			setPath(level->findPath(shared_from_this(), attackTarget, maxDist, true, false, false, true)); // 4J - changed to setPath from path =
		}
	}
	else
	{
		if (attackTarget->isAlive())
		{
			float d = attackTarget->distanceTo(shared_from_this());
			if (canSee(attackTarget))
			{
				checkHurtTarget(attackTarget, d);
			}
		}
		else
		{
			attackTarget = nullptr;
		}
	}

	/*
	* if (holdGround) { xxa = 0; yya = 0; jumping = false; return; }
	*/

	// 4J - a few changes here so that we can call findRandomStrollLocation for a sub-set of things that it normally wouldn't be in the java game.
	// This is so that we can have entities wander around a little, in order that we can measure how far they wander and then determine (if they wander too far) that
	// they aren't enclosed. We don't want the extra network overhead of just having Everything wandering round all the time, so have put a management system in place
	// that selects a subset of entities which have had their flag set through the considerForExtraWandering method so that these can keep doing random strolling.

	if (!holdGround && (attackTarget != NULL && (path == NULL || random->nextInt(20) == 0)))
	{
		setPath(level->findPath(shared_from_this(), attackTarget, maxDist, true, false, false, true));// 4J - changed to setPath from path =
	}
	else if (!holdGround && ((path == NULL && (random->nextInt(180) == 0) || fleeTime > 0) || (random->nextInt(120) == 0 || fleeTime > 0)))
	{
		if(noActionTime < SharedConstants::TICKS_PER_SECOND * 5) 
		{
			findRandomStrollLocation();
		}
	}
	else if (!holdGround && (path == NULL ) )
	{
		if( ( noActionTime >= SharedConstants::TICKS_PER_SECOND * 5 ) && isExtraWanderingEnabled() )
		{
			// This entity wouldn't normally be randomly strolling. However, if our management system says that it should do, then do. Don't
			// bother waiting for random conditions to be met before picking a direction though as the point here is to see if it is possible to
			// stroll out of a given area and so waiting around is just wasting time
			findRandomStrollLocation(getWanderingQuadrant());
		}
	}

	// Consider this for extra strolling if it is protected against despawning. We aren't interested in ones that aren't protected as the whole point of this
	// extra wandering is to potentially transition from protected to not protected.
	considerForExtraWandering( isDespawnProtected() );

	int yFloor = Mth::floor(bb->y0 + 0.5f);

	bool inWater = isInWater();
	bool inLava = isInLava();
	xRot = 0;
	if (path == NULL || random->nextInt(100) == 0)
	{
		this->Mob::serverAiStep();
		setPath(NULL);// 4J - changed to setPath from path =
		return;
	}

	Vec3 *target = path->currentPos(shared_from_this());
	double r = bbWidth * 2;
	while (target != NULL && target->distanceToSqr(x, target->y, z) < r * r)
	{
		path->next();
		if (path->isDone())
		{
			target = NULL;
			setPath(NULL); // 4J - changed to setPath from path =
		}
		else target = path->currentPos(shared_from_this());
	}

	jumping = false;
	if (target != NULL)
	{
		double xd = target->x - x;
		double zd = target->z - z;
		double yd = target->y - yFloor;
		float yRotD = (float) (atan2(zd, xd) * 180 / PI) - 90;
		float rotDiff = Mth::wrapDegrees(yRotD - yRot);
		yya = runSpeed;
		if (rotDiff > MAX_TURN)
		{
			rotDiff = MAX_TURN;
		}
		if (rotDiff < -MAX_TURN)
		{
			rotDiff = -MAX_TURN;
		}
		yRot += rotDiff;

		if (holdGround)
		{
			if (attackTarget != NULL)
			{
				double xd2 = attackTarget->x - x;
				double zd2 = attackTarget->z - z;

				float oldyRot = yRot;
				yRot = (float) (atan2(zd2, xd2) * 180 / PI) - 90;

				rotDiff = ((oldyRot - yRot) + 90) * PI / 180;
				xxa = -Mth::sin(rotDiff) * yya * 1.0f;
				yya = Mth::cos(rotDiff) * yya * 1.0f;
			}
		}
		if (yd > 0)
		{
			jumping = true;
		}
	}

	if (attackTarget != NULL)
	{
		lookAt(attackTarget, 30, 30);
	}

	if (this->horizontalCollision && !isPathFinding()) jumping = true;
	if (random->nextFloat() < 0.8f && (inWater || inLava)) jumping = true;
}

void PathfinderMob::findRandomStrollLocation(int quadrant/*=-1*/)	// 4J - added quadrant
{
	bool hasBest = false;
	int xBest = -1;
	int yBest = -1;
	int zBest = -1;
	float best = -99999;
	for (int i = 0; i < 10; i++)
	{
		// 4J - added quadrant parameter to this method so that the caller can request that only stroll locations in one quadrant be found. If -1 is passed then
		// behaviour is the same as the java game
		int xt, zt;
		int yt = Mth::floor(y + random->nextInt(7) - 3);
		if( quadrant == -1 )
		{
			xt = Mth::floor(x + random->nextInt(13) - 6);
			zt = Mth::floor(z + random->nextInt(13) - 6);
		}
		else
		{
			int sx = ( ( quadrant & 1 ) ? -1 : 1 );
			int sz = ( ( quadrant & 2 ) ? -1 : 1 );
			xt = Mth::floor(x + random->nextInt(7) * sx);
			zt = Mth::floor(z + random->nextInt(7) * sz);
		}
		float value = getWalkTargetValue(xt, yt, zt);
		if (value > best)
		{
			best = value;
			xBest = xt;
			yBest = yt;
			zBest = zt;
			hasBest = true;
		}
	}
	if (hasBest)
	{
		setPath(level->findPath(shared_from_this(), xBest, yBest, zBest, 10, true, false, false, true)); // 4J - changed to setPath from path =
	}
}

void PathfinderMob::checkHurtTarget(shared_ptr<Entity> target, float d)
{
}

float PathfinderMob::getWalkTargetValue(int x, int y, int z)
{
	return 0;
}

shared_ptr<Entity> PathfinderMob::findAttackTarget()
{
	return shared_ptr<Entity>();
}


bool PathfinderMob::canSpawn()
{
	int xt = Mth::floor(x);
	int yt = Mth::floor(bb->y0);
	int zt = Mth::floor(z);
	return this->Mob::canSpawn() && getWalkTargetValue(xt, yt, zt) >= 0;
}

bool PathfinderMob::isPathFinding()
{
	return path != NULL;
}

void PathfinderMob::setPath(Path *path)
{
	delete this->path;
	this->path = path;
}

shared_ptr<Entity> PathfinderMob::getAttackTarget()
{
	return attackTarget;
}

void PathfinderMob::setAttackTarget(shared_ptr<Entity> attacker)
{
	attackTarget = attacker;
}

float PathfinderMob::getWalkingSpeedModifier()
{
	if (useNewAi()) return 1.0f;
	float speed = Mob::getWalkingSpeedModifier();
	if (fleeTime > 0) speed *= 2;
	return speed;
}

bool PathfinderMob::couldWander()
{
	return (noActionTime < SharedConstants::TICKS_PER_SECOND * 5) || ( isExtraWanderingEnabled() );
}