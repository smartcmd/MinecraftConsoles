#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.pathfinder.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "PathNavigation.h"

PathNavigation::PathNavigation(Mob *mob, Level *level)
{
	this->mob = mob;
	this->level = level;
	dist = mob->getAttribute(SharedMonsterAttributes::FOLLOW_RANGE);

	path = nullptr;
	speedModifier = 0.0;
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
	if(path != nullptr) delete path;
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

void PathNavigation::setSpeedModifier(double speedModifier)
{
	this->speedModifier = speedModifier;
}

void PathNavigation::setCanFloat(bool canFloat)
{
	this->canFloat = canFloat;
}

float PathNavigation::getMaxDist()
{
	return static_cast<float>(dist->getValue());
}

Path *PathNavigation::createPath(double x, double y, double z)
{
	if (!canUpdatePath()) return nullptr;
	return level->findPath(mob->shared_from_this(), Mth::floor(x), static_cast<int>(y), Mth::floor(z), getMaxDist(), _canPassDoors, _canOpenDoors, avoidWater, canFloat);
}

bool PathNavigation::moveTo(double x, double y, double z, double speedModifier)
{
	MemSect(52);
	Path *newPath = createPath(Mth::floor(x), static_cast<int>(y), Mth::floor(z));
	MemSect(0);
	// No need to delete newPath here as this will be copied into the member variable path and the class can assume responsibility for it
	return moveTo(newPath, speedModifier);
}

Path *PathNavigation::createPath(shared_ptr<Entity> target)
{
	if (!canUpdatePath()) return nullptr;
	return level->findPath(mob->shared_from_this(), target, getMaxDist(), _canPassDoors, _canOpenDoors, avoidWater, canFloat);
}

bool PathNavigation::moveTo(shared_ptr<Entity> target, double speedModifier)
{
	MemSect(53);
	Path *newPath = createPath(target);
	MemSect(0);
	// No need to delete newPath here as this will be copied into the member variable path and the class can assume responsibility for it
	if (newPath != nullptr) return moveTo(newPath, speedModifier);
	else return false;
}

bool PathNavigation::moveTo(Path *newPath, double speedModifier)
{
	if(newPath == nullptr)
	{
		if(path != nullptr) delete path;
		path = nullptr;
		return false;
	}
	if(!newPath->sameAs(path))
	{
		if(path != nullptr) delete path;
		path = newPath;
	}
	else
	{
		delete newPath;
	}
	if (avoidSun) trimPathFromSun();
	if (path->getSize() == 0) return false;

	this->speedModifier = speedModifier;
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
	if (target == nullptr) return;

	mob->getMoveControl()->setWantedPosition(target->x, target->y, target->z, speedModifier);
}

void PathNavigation::updatePath()
{
	Vec3 *mobPos = getTempMobPos();

	// find first elevations in path
	int firstElevation = path->getSize();
	for (int i = path->getIndex(); path != nullptr && i < path->getSize(); ++i)
	{
		if (static_cast<int>(path->get(i)->y) != static_cast<int>(mobPos->y))
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
	int sx = static_cast<int>(ceil(mob->bbWidth));
	int sy = static_cast<int>(mob->bbHeight) + 1;
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
	return path == nullptr || path->isDone();
}

void PathNavigation::stop()
{
	if(path != nullptr) delete path;
	path = nullptr;
}

Vec3 *PathNavigation::getTempMobPos()
{
	return Vec3::newTemp(mob->x, getSurfaceY(), mob->z);
}

int PathNavigation::getSurfaceY()
{
	if (!mob->isInWater() || !canFloat) return static_cast<int>(mob->bb->y0 + 0.5);

	int surface = static_cast<int>(mob->bb->y0);
	int tileId = level->getTile(Mth::floor(mob->x), surface, Mth::floor(mob->z));
	int steps = 0;
	while (tileId == Tile::water_Id || tileId == Tile::calmWater_Id)
	{
		++surface;
		tileId = level->getTile(Mth::floor(mob->x), surface, Mth::floor(mob->z));
		if (++steps > 16) return static_cast<int>(mob->bb->y0);
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
	if (level->canSeeSky(Mth::floor(mob->x), static_cast<int>(mob->bb->y0 + 0.5), Mth::floor(mob->z))) return;

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
	if (!canWalkOn(gridPosX, static_cast<int>(startPos->y), gridPosZ, sx, sy, sz, startPos, dirX, dirZ)) return false;
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

		if (!canWalkOn(gridPosX, static_cast<int>(startPos->y), gridPosZ, sx, sy, sz, startPos, dirX, dirZ)) return false;
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