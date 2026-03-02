#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.h"
#include "StairTile.h"

int StairTile::DEAD_SPACES[8][2] = {
            {2, 6}, {3, 7}, {2, 3}, {6, 7},
            {0, 4}, {1, 5}, {0, 1}, {4, 5}
    };

StairTile::StairTile(int id, Tile *base,int basedata) : Tile(id, base->material, isSolidRender())
{
	this->base = base;
	this->basedata = basedata;
	isClipping = false;
	clipStep = 0;
	setDestroyTime(base->destroySpeed);
	setExplodeable(base->explosionResistance / 3);
	setSoundType(base->soundType);
	setLightBlock(255);
}

void StairTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	if (isClipping)
	{
		setShape(0.5f * (clipStep % 2), 0.5f * (clipStep / 2 % 2), 0.5f * (clipStep / 4 % 2), 0.5f + 0.5f * (clipStep % 2), 0.5f + 0.5f * (clipStep / 2 % 2), 0.5f + 0.5f * (clipStep / 4 % 2));
	}
	else
	{
		setShape(0, 0, 0, 1, 1, 1);
	}
}

bool StairTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool StairTile::isCubeShaped()
{
	return false;
}

int StairTile::getRenderShape()
{
	return Tile::SHAPE_STAIRS;
}

void StairTile::setBaseShape(LevelSource *level, int x, int y, int z)
{
	int data = level->getData(x, y, z);

	if ((data & UPSIDEDOWN_BIT) != 0)
	{
		setShape(0, .5f, 0, 1, 1, 1);
	}
	else
	{
		setShape(0, 0, 0, 1, .5f, 1);
	}
}

bool StairTile::isStairs(int id)
{
	StairTile *st = dynamic_cast<StairTile *>(Tile::tiles[id]);
	return id > 0 && st != NULL; //Tile::tiles[id] instanceof StairTile;
}

bool StairTile::isLockAttached(LevelSource *level, int x, int y, int z, int data)
{
	int lockTile = level->getTile(x, y, z);
	if (isStairs(lockTile) && level->getData(x, y, z) == data)
	{
		return true;
	}

	return false;
}

bool StairTile::setStepShape(LevelSource *level, int x, int y, int z)
{
	int data = level->getData(x, y, z);
	int dir = data & 0x3;

	float bottom = 0.5f;
	float top = 1.0f;

	if ((data & UPSIDEDOWN_BIT) != 0)
	{
		bottom = 0;
		top = .5f;
	}

	float west = 0;
	float east = 1;
	float north = 0;
	float south = .5f;

	bool checkInnerPiece = true;

	if (dir == DIR_EAST)
	{
		west = .5f;
		south = 1;

		int backTile = level->getTile(x + 1, y, z);
		int backData = level->getData(x + 1, y, z);
		if (isStairs(backTile) && ((data & UPSIDEDOWN_BIT) == (backData & UPSIDEDOWN_BIT)))
		{
			int backDir = backData & 0x3;
			if (backDir == DIR_NORTH && !isLockAttached(level, x, y, z + 1, data))
			{
				south = .5f;
				checkInnerPiece = false;
			}
			else if (backDir == DIR_SOUTH && !isLockAttached(level, x, y, z - 1, data))
			{
				north = .5f;
				checkInnerPiece = false;
			}
		}
	}
	else if (dir == DIR_WEST)
	{
		east = .5f;
		south = 1;

		int backTile = level->getTile(x - 1, y, z);
		int backData = level->getData(x - 1, y, z);
		if (isStairs(backTile) && ((data & UPSIDEDOWN_BIT) == (backData & UPSIDEDOWN_BIT)))
		{
			int backDir = backData & 0x3;
			if (backDir == DIR_NORTH && !isLockAttached(level, x, y, z + 1, data))
			{
				south = .5f;
				checkInnerPiece = false;
			}
			else if (backDir == DIR_SOUTH && !isLockAttached(level, x, y, z - 1, data))
			{
				north = .5f;
				checkInnerPiece = false;
			}
		}
	}
	else if (dir == DIR_SOUTH)
	{
		north = .5f;
		south = 1;

		int backTile = level->getTile(x, y, z + 1);
		int backData = level->getData(x, y, z + 1);
		if (isStairs(backTile) && ((data & UPSIDEDOWN_BIT) == (backData & UPSIDEDOWN_BIT)))
		{
			int backDir = backData & 0x3;
			if (backDir == DIR_WEST && !isLockAttached(level, x + 1, y, z, data))
			{
				east = .5f;
				checkInnerPiece = false;
			}
			else if (backDir == DIR_EAST && !isLockAttached(level, x - 1, y, z, data))
			{
				west = .5f;
				checkInnerPiece = false;
			}
		}
	}
	else if (dir == DIR_NORTH)
	{
		int backTile = level->getTile(x, y, z - 1);
		int backData = level->getData(x, y, z - 1);
		if (isStairs(backTile) && ((data & UPSIDEDOWN_BIT) == (backData & UPSIDEDOWN_BIT)))
		{
			int backDir = backData & 0x3;
			if (backDir == DIR_WEST && !isLockAttached(level, x + 1, y, z, data))
			{
				east = .5f;
				checkInnerPiece = false;
			}
			else if (backDir == DIR_EAST && !isLockAttached(level, x - 1, y, z, data))
			{
				west = .5f;
				checkInnerPiece = false;
			}
		}
	}

	setShape(west, bottom, north, east, top, south);
	return checkInnerPiece;
}

/*
* This method adds an extra 1/8 block if the stairs can attach as an
* "inner corner."
*/
bool StairTile::setInnerPieceShape(LevelSource *level, int x, int y, int z)
{
	int data = level->getData(x, y, z);
	int dir = data & 0x3;

	float bottom = 0.5f;
	float top = 1.0f;

	if ((data & UPSIDEDOWN_BIT) != 0)
	{
		bottom = 0;
		top = .5f;
	}

	float west = 0;
	float east = .5f;
	float north = .5f;
	float south = 1.0f;

	bool hasInnerPiece = false;

	if (dir == DIR_EAST)
	{
		int frontTile = level->getTile(x - 1, y, z);
		int frontData = level->getData(x - 1, y, z);
		if (isStairs(frontTile) && ((data & UPSIDEDOWN_BIT) == (frontData & UPSIDEDOWN_BIT)))
		{
			int frontDir = frontData & 0x3;
			if (frontDir == DIR_NORTH && !isLockAttached(level, x, y, z - 1, data))
			{
				north = 0;
				south = .5f;
				hasInnerPiece = true;
			}
			else if (frontDir == DIR_SOUTH && !isLockAttached(level, x, y, z + 1, data))
			{
				north = .5f;
				south = 1;
				hasInnerPiece = true;
			}
		}
	}
	else if (dir == DIR_WEST)
	{
		int frontTile = level->getTile(x + 1, y, z);
		int frontData = level->getData(x + 1, y, z);
		if (isStairs(frontTile) && ((data & UPSIDEDOWN_BIT) == (frontData & UPSIDEDOWN_BIT)))
		{
			west = .5f;
			east = 1.0f;
			int frontDir = frontData & 0x3;
			if (frontDir == DIR_NORTH && !isLockAttached(level, x, y, z - 1, data))
			{
				north = 0;
				south = .5f;
				hasInnerPiece = true;
			}
			else if (frontDir == DIR_SOUTH && !isLockAttached(level, x, y, z + 1, data))
			{
				north = .5f;
				south = 1;
				hasInnerPiece = true;
			}
		}
	}
	else if (dir == DIR_SOUTH)
	{
		int frontTile = level->getTile(x, y, z - 1);
		int frontData = level->getData(x, y, z - 1);
		if (isStairs(frontTile) && ((data & UPSIDEDOWN_BIT) == (frontData & UPSIDEDOWN_BIT)))
		{
			north = 0;
			south = .5f;

			int frontDir = frontData & 0x3;
			if (frontDir == DIR_WEST && !isLockAttached(level, x - 1, y, z, data))
			{
				hasInnerPiece = true;
			}
			else if (frontDir == DIR_EAST && !isLockAttached(level, x + 1, y, z, data))
			{
				west = .5f;
				east = 1.0f;
				hasInnerPiece = true;
			}
		}
	}
	else if (dir == DIR_NORTH)
	{
		int frontTile = level->getTile(x, y, z + 1);
		int frontData = level->getData(x, y, z + 1);
		if (isStairs(frontTile) && ((data & UPSIDEDOWN_BIT) == (frontData & UPSIDEDOWN_BIT)))
		{
			int frontDir = frontData & 0x3;
			if (frontDir == DIR_WEST && !isLockAttached(level, x - 1, y, z, data))
			{
				hasInnerPiece = true;
			}
			else if (frontDir == DIR_EAST && !isLockAttached(level, x + 1, y, z, data))
			{
				west = .5f;
				east = 1.0f;
				hasInnerPiece = true;
			}
		}
	}

	if (hasInnerPiece)
	{
		setShape(west, bottom, north, east, top, south);
	}
	return hasInnerPiece;
}

void StairTile::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source)
{
	setBaseShape(level, x, y, z);
	Tile::addAABBs(level, x, y, z, box, boxes, source);

	bool checkInnerPiece = setStepShape(level, x, y, z);
	Tile::addAABBs(level, x, y, z, box, boxes, source);

	if (checkInnerPiece)
	{
		if (setInnerPieceShape(level, x, y, z))
		{
			Tile::addAABBs(level, x, y, z, box, boxes, source);
		}
	}

	//int data = level->getData(x, y, z);
	//int dir = data & 0x3;
	//float lowerPieceY0 = 0;
	//float lowerPieceY1 = 0.5f;
	//float upperPieceY0 = 0.5f;
	//float upperPieceY1 = 1;

	//if ((data & UPSIDEDOWN_BIT) != 0)
	//{
	//	lowerPieceY0 = .5f;
	//	lowerPieceY1 = 1;
	//	upperPieceY0 = 0;
	//	upperPieceY1 = .5f;
	//}

	//setShape(0, lowerPieceY0, 0, 1, lowerPieceY1, 1);
	//Tile::addAABBs(level, x, y, z, box, boxes);

	//if (dir == 0)
	//{
	//	setShape(0.5f, upperPieceY0, 0, 1, upperPieceY1, 1);
	//	Tile::addAABBs(level, x, y, z, box, boxes);
	//}
	//else if (dir == 1)
	//{
	//	setShape(0, upperPieceY0, 0, .5f, upperPieceY1, 1);
	//	Tile::addAABBs(level, x, y, z, box, boxes);
	//}
	//else if (dir == 2)
	//{
	//	setShape(0, upperPieceY0, 0.5f, 1, upperPieceY1, 1);
	//	Tile::addAABBs(level, x, y, z, box, boxes);
	//}
	//else if (dir == 3)
	//{
	//	setShape(0, upperPieceY0, 0, 1, upperPieceY1, .5f);
	//	Tile::addAABBs(level, x, y, z, box, boxes);
	//}

	setShape(0, 0, 0, 1, 1, 1);
}

/** DELEGATES: **/


void StairTile::addLights(Level *level, int x, int y, int z)
{
	base->addLights(level, x, y, z);
}

void StairTile::animateTick(Level *level, int x, int y, int z, Random *random)
{
	base->animateTick(level, x, y, z, random);
}

void StairTile::attack(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	base->attack(level, x, y, z, player);
}

void StairTile::destroy(Level *level, int x, int y, int z, int data)
{
	base->destroy(level, x, y, z, data);
}

// 4J - brought forward from 1.8.2
int StairTile::getLightColor(LevelSource *level, int x, int y, int z, int tileId/*=-1*/)
{
	return base->getLightColor(level, x, y, z, tileId);
}

float StairTile::getBrightness(LevelSource *level, int x, int y, int z)
{
	return base->getBrightness(level, x, y, z);
}

float StairTile::getExplosionResistance(shared_ptr<Entity> source)
{
	return base->getExplosionResistance(source);
}

int StairTile::getRenderLayer()
{
	return base->getRenderLayer();
}

Icon *StairTile::getTexture(int face, int data)
{
	return base->getTexture(face, basedata);
}

int StairTile::getTickDelay()
{
	return base->getTickDelay();
}

AABB *StairTile::getTileAABB(Level *level, int x, int y, int z)
{
	return base->getTileAABB(level, x, y, z);
}

void StairTile::handleEntityInside(Level *level, int x, int y, int z, shared_ptr<Entity> e, Vec3 *current)
{
	base->handleEntityInside(level, x, y, z, e, current);
}

bool StairTile::mayPick()
{
	return base->mayPick();
}

bool StairTile::mayPick(int data, bool liquid)
{
	return base->mayPick(data, liquid);
}

bool StairTile::mayPlace(Level *level, int x, int y, int z)
{
	return base->mayPlace(level, x, y, z);
}

void StairTile::onPlace(Level *level, int x, int y, int z)
{
	neighborChanged(level, x, y, z, 0);
	base->onPlace(level, x, y, z);
}

void StairTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	base->onRemove(level, x, y, z, id, data);
}

void StairTile::prepareRender(Level *level, int x, int y, int z)
{
	base->prepareRender(level, x, y, z);
}

void StairTile::stepOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
	base->stepOn(level, x, y, z, entity);
}

void StairTile::tick(Level *level, int x, int y, int z, Random *random)
{
	base->tick(level, x, y, z, random);
}

// 4J-HEG - Removed this to prevent weird tooltips (place steak on stairs!?)
//// 4J-PB - Adding a TestUse for tooltip display
//bool StairTile::TestUse()
//{
//	return true;
//}

bool StairTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if (soundOnly) return false;
	return base->use(level, x, y, z, player, 0, 0, 0, 0);
}

void StairTile::wasExploded(Level *level, int x, int y, int z)
{
	base->wasExploded(level, x, y, z);
}

void StairTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by)
{
	int dir = ( Mth::floor(by->yRot * 4 / (360) + 0.5) ) & 3;
	int usd = level->getData(x, y, z) & UPSIDEDOWN_BIT;

	if (dir == 0) level->setData(x, y, z, DIR_SOUTH | usd);
	if (dir == 1) level->setData(x, y, z, DIR_WEST | usd);
	if (dir == 2) level->setData(x, y, z, DIR_NORTH | usd);
	if (dir == 3) level->setData(x, y, z, DIR_EAST | usd);
}

int StairTile::getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue)
{
	if (face == Facing::DOWN || (face != Facing::UP && clickY > 0.5))
	{
		return itemValue | UPSIDEDOWN_BIT;
	}
	return itemValue;
}

HitResult *StairTile::clip(Level *level, int xt, int yt, int zt, Vec3 *a, Vec3 *b)
{
	HitResult *results[8];
	for(unsigned int i = 0; i < 8; ++i)
	{
		results[i] = NULL;
	}
	int data = level->getData(xt, yt, zt);
	int dir = data & 0x3;
	bool upsideDown = (data & UPSIDEDOWN_BIT) == UPSIDEDOWN_BIT;
	int *deadSpaces = DEAD_SPACES[dir + (upsideDown ? 4 : 0)];

	isClipping = true;
	for (int i = 0; i < 8; i++)
	{
		clipStep = i;

		for(unsigned int j = 0; j < DEAD_SPACE_COLUMN_COUNT; ++j)
		{
			if (deadSpaces[j] == i) continue;
		}

		results[i] = Tile::clip(level, xt, yt, zt, a, b);
	}

	for(unsigned int j = 0; j < DEAD_SPACE_COLUMN_COUNT; ++j)
	{
		results[deadSpaces[j]] = NULL;
	}

	HitResult *closest = NULL;
	double closestDist = 0;

	for (unsigned int i = 0; i < 8; ++i)
	{
		HitResult *result = results[i];
		if (result != NULL)
		{
			double dist = result->pos->distanceToSqr(b);

			if (dist > closestDist)
			{
				closest = result;
				closestDist = dist;
			}
		}
	}

	return closest;
}

void StairTile::registerIcons(IconRegister *iconRegister)
{
	// None
}
