#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.pathfinder.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "PathNavigation.h"

PathNavigation::PathNavigation(Mob *mob, Level *level, float maxDist)
{
	this->mob = mob;
	this->level = level;
	this->maxDist = maxDist;

	path = NULL;
	speed = 0.0f;
	avoidSun = false;
	_tick = 0;
	lastStuckCheck = 0;
	lastStuckCheckPos = Vec3::newPermanent(0, 0, 0);
	_canPassDoors = true;
	_canOpenDoors = false;
	avoidWater = false;
	canFloat = false;
}

PathNavigation::~PathNavigation()
{
	if(path != NULL) delete path;
	delete lastStuckCheckPos;
}

void PathNavigation::setAvoidWater(bool avoidWater)
{
	this->avoidWater = avoidWater;
}

bool PathNavigation::getAvoidWater()
{
	return avoidWater;
}

void PathNavigation::setCanOpenDoors(bool canOpenDoors)
{
	this->_canOpenDoors = canOpenDoors;
}

bool PathNavigation::canPassDoors()
{
	return _canPassDoors;
}

void PathNavigation::setCanPassDoors(bool canPass)
{
	_canPassDoors = canPass;
}

bool PathNavigation::canOpenDoors()
{
	return _canOpenDoors;
}

void PathNavigation::setAvoidSun(bool avoidSun)
{
	this->avoidSun = avoidSun;
}

void PathNavigation::setSpeed(float speed)
{
	this->speed = speed;
}

void PathNavigation::setCanFloat(bool canFloat)
{
	this->canFloat = canFloat;
}

Path *PathNavigation::createPath(double x, double y, double z)
{
	if (!canUpdatePath()) return NULL;
	return level->findPath(mob->shared_from_this(), Mth::floor(x), (int) y, Mth::floor(z), maxDist, _canPassDoors, _canOpenDoors, avoidWater, canFloat);
}

bool PathNavigation::moveTo(double x, double y, double z, float speed)
{
	MemSect(52);
	Path *newPath = createPath(Mth::floor(x), (int) y, Mth::floor(z));
	MemSect(0);
	// No need to delete newPath here as this will be copied into the member variable path and the class can assume responsibility for it
	return moveTo(newPath, speed);
}

Path *PathNavigation::createPath(shared_ptr<Mob> target)
{
	if (!canUpdatePath()) return NULL;
	return level->findPath(mob->shared_from_this(), target, maxDist, _canPassDoors, _canOpenDoors, avoidWater, canFloat);
}

bool PathNavigation::moveTo(shared_ptr<Mob> target, float speed)
{
	MemSect(53);
	Path *newPath = createPath(target);
	MemSect(0);
	// No need to delete newPath here as this will be copied into the member variable path and the class can assume responsibility for it
	if (newPath != NULL) return moveTo(newPath, speed);
	else return false;
}

bool PathNavigation::moveTo(Path *newPath, float speed)
{
	if(newPath == NULL)
	{
		if(path != NULL) delete path;
		path = NULL;
		return false;
	}
	if(!newPath->sameAs(path))
	{
		if(path != NULL) delete path;
		path = newPath;
	}
	else
	{
		delete newPath;
	}
	if (avoidSun) trimPathFromSun();
	if (path->getSize() == 0) return false;

	this->speed = speed;
	Vec3 *mobPos = getTempMobPos();
	lastStuckCheck = _tick;
	lastStuckCheckPos->x = mobPos->x;
	lastStuckCheckPos->y = mobPos->y;
	lastStuckCheckPos->z = mobPos->z;
	return true;
}

Path *PathNavigation::getPath()
{
	return path;
}

void PathNavigation::tick()
{
	++_tick;
	if (isDone()) return;

	if (canUpdatePath()) updatePath();

	if (isDone()) return;
	Vec3 *target = path->currentPos(mob->shared_from_this());
	if (target == NULL) return;

	mob->getMoveControl()->setWantedPosition(target->x, target->y, target->z, speed);
}

void PathNavigation::updatePath()
{
	Vec3 *mobPos = getTempMobPos();

	// find first elevations in path
	int firstElevation = path->getSize();
	for (int i = path->getIndex(); path != NULL && i < path->getSize(); ++i)
	{
		if ((int) path->get(i)->y != (int) mobPos->y)
		{
			firstElevation = i;
			break;
		}
	}

	// remove those within way point radius (this is not optimal, should
	// check canWalkDirectly also) possibly only check next as well
	float waypointRadiusSqr = mob->bbWidth * mob->bbWidth;
	for (int i = path->getIndex(); i < firstElevation; ++i)
	{
		Vec3 *pathPos = path->getPos(mob->shared_from_this(), i);
		if (mobPos->distanceToSqr(pathPos) < waypointRadiusSqr)
		{
			path->setIndex(i + 1);
		}
	}

	// smooth remaining on same elevation
	int sx = (int) ceil(mob->bbWidth);
	int sy = (int) mob->bbHeight + 1;
	int sz = sx;
	for (int i = firstElevation - 1; i >= path->getIndex(); --i)
	{
		if (canMoveDirectly(mobPos, path->getPos(mob->shared_from_this(), i), sx, sy, sz))
		{
			path->setIndex(i);
			break;
		}
	}

	// stuck detection (probably pushed off path)
	if (_tick - lastStuckCheck > 100)
	{
		if (mobPos->distanceToSqr(lastStuckCheckPos) < 1.5 * 1.5) stop();
		lastStuckCheck = _tick;
		lastStuckCheckPos->x = mobPos->x;
		lastStuckCheckPos->y = mobPos->y;
		lastStuckCheckPos->z = mobPos->z;
	}
}

bool PathNavigation::isDone()
{
	return path == NULL || path->isDone();
}

void PathNavigation::stop()
{
	if(path != NULL) delete path;
	path = NULL;
}

Vec3 *PathNavigation::getTempMobPos()
{
	return Vec3::newTemp(mob->x, getSurfaceY(), mob->z);
}

int PathNavigation::getSurfaceY()
{
	if (!mob->isInWater() || !canFloat) return (int) (mob->bb->y0 + 0.5);

	int surface = (int) (mob->bb->y0);
	int tileId = level->getTile(Mth::floor(mob->x), surface, Mth::floor(mob->z));
	int steps = 0;
	while (tileId == Tile::water_Id || tileId == Tile::calmWater_Id)
	{
		++surface;
		tileId = level->getTile(Mth::floor(mob->x), surface, Mth::floor(mob->z));
		if (++steps > 16) return (int) (mob->bb->y0);
	}
	return surface;
}

bool PathNavigation::canUpdatePath()
{
	return mob->onGround || (canFloat && isInLiquid());
}

bool PathNavigation::isInLiquid()
{
	return mob->isInWater() || mob->isInLava();
}

void PathNavigation::trimPathFromSun()
{
	if (level->canSeeSky(Mth::floor(mob->x), (int) (mob->bb->y0 + 0.5), Mth::floor(mob->z))) return;

	for (int i = 0; i < path->getSize(); ++i)
	{
		Node *n = path->get(i);
		if (level->canSeeSky((int) n->x, (int) n->y, (int) n->z))
		{
			path->setSize(i - 1);
			return;
		}
	}
}

bool PathNavigation::canMoveDirectly(Vec3 *startPos, Vec3 *stopPos, int sx, int sy, int sz)
{

	int gridPosX = Mth::floor(startPos->x);
	int gridPosZ = Mth::floor(startPos->z);

	double dirX = stopPos->x - startPos->x;
	double dirZ = stopPos->z - startPos->z;
	double distSqr = dirX * dirX + dirZ * dirZ;
	if (distSqr < 0.00000001) return false;

	double nf = 1 / sqrt(distSqr);
	dirX *= nf;
	dirZ *= nf;

	sx += 2;
	sz += 2;
	if (!canWalkOn(gridPosX, (int) startPos->y, gridPosZ, sx, sy, sz, startPos, dirX, dirZ)) return false;
	sx -= 2;
	sz -= 2;

	double deltaX = 1 / abs(dirX);
	double deltaZ = 1 / abs(dirZ);

	double maxX = gridPosX * 1 - startPos->x;
	double maxZ = gridPosZ * 1 - startPos->z;
	if (dirX >= 0) maxX += 1;
	if (dirZ >= 0) maxZ += 1;
	maxX /= dirX;
	maxZ /= dirZ;

	int stepX = dirX < 0 ? -1 : 1;
	int stepZ = dirZ < 0 ? -1 : 1;
	int gridGoalX = Mth::floor(stopPos->x);
	int gridGoalZ = Mth::floor(stopPos->z);
	int currentDirX = gridGoalX - gridPosX;
	int currentDirZ = gridGoalZ - gridPosZ;
	while (currentDirX * stepX > 0 || currentDirZ * stepZ > 0)
	{
		if (maxX < maxZ)
		{
			maxX += deltaX;
			gridPosX += stepX;
			currentDirX = gridGoalX - gridPosX;
		}
		else
		{
			maxZ += deltaZ;
			gridPosZ += stepZ;
			currentDirZ = gridGoalZ - gridPosZ;
		}

		if (!canWalkOn(gridPosX, (int) startPos->y, gridPosZ, sx, sy, sz, startPos, dirX, dirZ)) return false;
	}
	return true;
}

bool PathNavigation::canWalkOn(int x, int y, int z, int sx, int sy, int sz, Vec3 *startPos, double goalDirX, double goalDirZ)
{

	int startX = x - sx / 2;
	int startZ = z - sz / 2;

	if (!canWalkAbove(startX, y, startZ, sx, sy, sz, startPos, goalDirX, goalDirZ)) return false;

	// lava or water or air under
	for (int xx = startX; xx < startX + sx; xx++)
	{
		for (int zz = startZ; zz < startZ + sz; zz++)
		{
			double dirX = xx + 0.5 - startPos->x;
			double dirZ = zz + 0.5 - startPos->z;
			if (dirX * goalDirX + dirZ * goalDirZ < 0) continue;
			int tile = level->getTile(xx, y - 1, zz);
			if (tile <= 0) return false;
			Material *m = Tile::tiles[tile]->material;
			if (m == Material::water && !mob->isInWater()) return false;
			if (m == Material::lava) return false;
		}
	}

	return true;
}

bool PathNavigation::canWalkAbove(int startX, int startY, int startZ, int sx, int sy, int sz, Vec3 *startPos, double goalDirX, double goalDirZ)
{

	for (int xx = startX; xx < startX + sx; xx++)
	{
		for (int yy = startY; yy < startY + sy; yy++)
		{
			for (int zz = startZ; zz < startZ + sz; zz++)
			{

				double dirX = xx + 0.5 - startPos->x;
				double dirZ = zz + 0.5 - startPos->z;
				if (dirX * goalDirX + dirZ * goalDirZ < 0) continue;
				int tile = level->getTile(xx, yy, zz);
				if (tile <= 0) continue;
				if (!Tile::tiles[tile]->isPathfindable(level, xx, yy, zz)) return false;
			}
		}
	}
	return true;
}

void PathNavigation::setLevel(Level *level)
{
	this->level = level;
}