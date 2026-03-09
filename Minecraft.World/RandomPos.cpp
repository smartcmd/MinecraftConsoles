#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.phys.h"
#include "RandomPos.h"

Vec3 *RandomPos::tempDir = Vec3::newPermanent(0, 0, 0);

Vec3 *RandomPos::getPos(shared_ptr<PathfinderMob> mob, int xzDist, int yDist, int quadrant/*=-1*/)		// 4J - added quadrant
{
	return generateRandomPos(mob, xzDist, yDist, nullptr, quadrant);
}

Vec3 *RandomPos::getPosTowards(shared_ptr<PathfinderMob> mob, int xzDist, int yDist, Vec3 *towardsPos)
{
	tempDir->x = towardsPos->x - mob->x;
	tempDir->y = towardsPos->y - mob->y;
	tempDir->z = towardsPos->z - mob->z;
	return generateRandomPos(mob, xzDist, yDist, tempDir);
}

Vec3 *RandomPos::getPosAvoid(shared_ptr<PathfinderMob> mob, int xzDist, int yDist, Vec3 *avoidPos)
{
	tempDir->x = mob->x - avoidPos->x;
	tempDir->y = mob->y - avoidPos->y;
	tempDir->z = mob->z - avoidPos->z;
	return generateRandomPos(mob, xzDist, yDist, tempDir);
}

Vec3 *RandomPos::generateRandomPos(shared_ptr<PathfinderMob> mob, int xzDist, int yDist, Vec3 *dir, int quadrant/*=-1*/)		// 4J - added quadrant
{
	Random *random = mob->getRandom();
	bool hasBest = false;
	int xBest = 0, yBest = 0, zBest = 0;
	float best = -99999;

	// 4J Stu - restrict is a reserved keyword
	bool bRestrict;
	if (mob->hasRestriction())
	{
		double restDist = mob->getRestrictCenter()->distSqr(Mth::floor(mob->x), Mth::floor(mob->y), Mth::floor(mob->z)) + 4;
		double radius = mob->getRestrictRadius() + xzDist;
		bRestrict = restDist < radius * radius;
	}
	else bRestrict = false;

	for (int i = 0; i < 10; i++)
	{
		int xt, yt, zt;
		// 4J - added quadrant here so that we can choose to select positions only within the one quadrant. Passing a parameter of -1 will
		// lead to normal java behaviour
		if( quadrant == -1 )
		{
			xt = random->nextInt(2 * xzDist) - xzDist;
			zt = random->nextInt(2 * xzDist) - xzDist;
		}
		else
		{
			int sx = ( ( quadrant & 1 ) ? -1 : 1 );
			int sz = ( ( quadrant & 2 ) ? -1 : 1 );
			xt = random->nextInt(xzDist) * sx;
			zt = random->nextInt(xzDist) * sz;
		}
		yt = random->nextInt(2 * yDist) - yDist;

		if (dir != nullptr && xt * dir->x + zt * dir->z < 0) continue;

		xt += Mth::floor(mob->x);
		yt += Mth::floor(mob->y);
		zt += Mth::floor(mob->z);

		if (bRestrict && !mob->isWithinRestriction(xt, yt, zt)) continue;
		float value = mob->getWalkTargetValue(xt, yt, zt);
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
		return Vec3::newTemp(xBest, yBest, zBest);
	}

	return nullptr;
}