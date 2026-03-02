#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.redstone.h"
#include "net.minecraft.h"
#include "LeverTile.h"

LeverTile::LeverTile(int id) : Tile(id, Material::decoration,isSolidRender())
{
}

AABB *LeverTile::getAABB(Level *level, int x, int y, int z)
{
	return NULL;
}

bool LeverTile::blocksLight()
{
	return false;
}

bool LeverTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool LeverTile::isCubeShaped()
{
	return false;
}

int LeverTile::getRenderShape()
{
	return Tile::SHAPE_LEVER;
}

bool LeverTile::mayPlace(Level *level, int x, int y, int z, int face)
{
	if (face == Facing::DOWN && level->isSolidBlockingTile(x, y + 1, z)) return true;
	if (face == Facing::UP && level->isTopSolidBlocking(x, y - 1, z)) return true;
	if (face == Facing::NORTH && level->isSolidBlockingTile(x, y, z + 1)) return true;
	if (face == Facing::SOUTH && level->isSolidBlockingTile(x, y, z - 1)) return true;
	if (face == Facing::WEST && level->isSolidBlockingTile(x + 1, y, z)) return true;
	if (face == Facing::EAST && level->isSolidBlockingTile(x - 1, y, z)) return true;
	return false;
}

bool LeverTile::mayPlace(Level *level, int x, int y, int z)
{
	if (level->isSolidBlockingTile(x - 1, y, z))
	{
		return true;
	} else if (level->isSolidBlockingTile(x + 1, y, z))
	{
		return true;
	} else if (level->isSolidBlockingTile(x, y, z - 1))
	{
		return true;
	} else if (level->isSolidBlockingTile(x, y, z + 1))
	{
		return true;
	} else if (level->isTopSolidBlocking(x, y - 1, z))
	{
		return true;
	}
	else if (level->isSolidBlockingTile(x, y + 1, z))
	{
		return true;
	}
	return false;
}

int LeverTile::getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue)
{
	int dir = itemValue;

	int oldFlip = dir & 8;
	dir &= 7;

	dir = -1;

	if (face == Facing::DOWN && level->isSolidBlockingTile(x, y + 1, z)) dir = 0;
	if (face == Facing::UP && level->isTopSolidBlocking(x, y - 1, z)) dir = 5;
	if (face == Facing::NORTH && level->isSolidBlockingTile(x, y, z + 1)) dir = 4;
	if (face == Facing::SOUTH && level->isSolidBlockingTile(x, y, z - 1)) dir = 3;
	if (face == Facing::WEST && level->isSolidBlockingTile(x + 1, y, z)) dir = 2;
	if (face == Facing::EAST && level->isSolidBlockingTile(x - 1, y, z)) dir = 1;

	return dir + oldFlip;
}

void LeverTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	int data = level->getData(x, y, z);
	int dir = data & 7;
	int flip = data & 8;

	if (dir == getLeverFacing(Facing::UP))
	{
		if ((Mth::floor(by->yRot * 4 / (360) + 0.5) & 1) == 0)
		{
			level->setData(x, y, z, 5 | flip, Tile::UPDATE_CLIENTS);
		}
		else
		{
			level->setData(x, y, z, 6 | flip, Tile::UPDATE_CLIENTS);
		}
	}
	else if (dir == getLeverFacing(Facing::DOWN))
	{
		if ((Mth::floor(by->yRot * 4 / (360) + 0.5) & 1) == 0)
		{
			level->setData(x, y, z, 7 | flip, Tile::UPDATE_CLIENTS);
		}
		else
		{
			level->setData(x, y, z, 0 | flip, Tile::UPDATE_CLIENTS);
		}
	}
}

int LeverTile::getLeverFacing(int facing)
{
	switch (facing)
	{
	case Facing::DOWN:
		return 0;
	case Facing::UP:
		return 5;
	case Facing::NORTH:
		return 4;
	case Facing::SOUTH:
		return 3;
	case Facing::WEST:
		return 2;
	case Facing::EAST:
		return 1;
	}
	return -1;
}

void LeverTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (checkCanSurvive(level, x, y, z))
	{
		int dir = level->getData(x, y, z) & 7;
		bool replace = false;

		if (!level->isSolidBlockingTile(x - 1, y, z) && dir == 1) replace = true;
		if (!level->isSolidBlockingTile(x + 1, y, z) && dir == 2) replace = true;
		if (!level->isSolidBlockingTile(x, y, z - 1) && dir == 3) replace = true;
		if (!level->isSolidBlockingTile(x, y, z + 1) && dir == 4) replace = true;
		if (!level->isTopSolidBlocking(x, y - 1, z) && dir == 5) replace = true;
		if (!level->isTopSolidBlocking(x, y - 1, z) && dir == 6) replace = true;
		if (!level->isSolidBlockingTile(x, y + 1, z) && dir == 0) replace = true;
		if (!level->isSolidBlockingTile(x, y + 1, z) && dir == 7) replace = true;

		if (replace)
		{
			spawnResources(level, x, y, z, level->getData(x, y, z), 0);
			level->removeTile(x, y, z);
		}
	}

}

bool LeverTile::checkCanSurvive(Level *level, int x, int y, int z)
{
	if (!mayPlace(level, x, y, z))
	{
		spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->removeTile(x, y, z);
		return false;
	}
	return true;
}

void LeverTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	int dir = level->getData(x, y, z) & 7;
	float r = 3 / 16.0f;
	if (dir == 1)
	{
		setShape(0, 0.2f, 0.5f - r, r * 2, 0.8f, 0.5f + r);
	}
	else if (dir == 2)
	{
		setShape(1 - r * 2, 0.2f, 0.5f - r, 1, 0.8f, 0.5f + r);
	}
	else if (dir == 3)
	{
		setShape(0.5f - r, 0.2f, 0, 0.5f + r, 0.8f, r * 2);
	}
	else if (dir == 4)
	{
		setShape(0.5f - r, 0.2f, 1 - r * 2, 0.5f + r, 0.8f, 1);
	}
	else if (dir == 5 || dir == 6)
	{
		r = 4 / 16.0f;
		setShape(0.5f - r, 0.0f, 0.5f - r, 0.5f + r, 0.6f, 0.5f + r);
	}
	else if (dir == 0 || dir == 7)
	{
		r = 4 / 16.0f;
		setShape(0.5f - r, 0.4f, 0.5f - r, 0.5f + r, 1.0f, 0.5f + r);
	}
}

// 4J-PB - Adding a TestUse for tooltip display
bool LeverTile::TestUse()
{
	return true;
}

bool LeverTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if( soundOnly )
	{
		// 4J - added - just do enough to play the sound
		int data = level->getData(x, y, z);
		int dir = data & 7;
		int open = 8 - (data & 8);
		level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_RANDOM_CLICK, 0.3f, open > 0 ? 0.6f : 0.5f);
		return false;
	}
	if (level->isClientSide)
	{
		// 4J - added stuff to play sound in this case too
		int data = level->getData(x, y, z);
		int dir = data & 7;
		int open = 8 - (data & 8);
		level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_RANDOM_CLICK, 0.3f, open > 0 ? 0.6f : 0.5f);

		return true;
	}
	int data = level->getData(x, y, z);
	int dir = data & 7;
	int open = 8 - (data & 8);

	level->setData(x, y, z, dir + open, Tile::UPDATE_ALL);
	level->setTilesDirty(x, y, z, x, y, z);

	level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_RANDOM_CLICK, 0.3f, open > 0 ? 0.6f : 0.5f);

	level->updateNeighborsAt(x, y, z, id);
	if (dir == 1)
	{
		level->updateNeighborsAt(x - 1, y, z, id);
	}
	else if (dir == 2)
	{
		level->updateNeighborsAt(x + 1, y, z, id);
	}
	else if (dir == 3)
	{
		level->updateNeighborsAt(x, y, z - 1, id);
	}
	else if (dir == 4)
	{
		level->updateNeighborsAt(x, y, z + 1, id);
	}
	else if (dir == 5 || dir == 6)
	{
		level->updateNeighborsAt(x, y - 1, z, id);
	}
	else if (dir == 0 || dir == 7)
	{
		level->updateNeighborsAt(x, y + 1, z, id);
	}

	return true;
}

void LeverTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	if ((data & 8) > 0)
	{
		level->updateNeighborsAt(x, y, z, this->id);
		int dir = data & 7;
		if (dir == 1)
		{
			level->updateNeighborsAt(x - 1, y, z, this->id);
		}
		else if (dir == 2)
		{
			level->updateNeighborsAt(x + 1, y, z, this->id);
		}
		else if (dir == 3)
		{
			level->updateNeighborsAt(x, y, z - 1, this->id);
		}
		else if (dir == 4)
		{
			level->updateNeighborsAt(x, y, z + 1, this->id);
		}
		else if (dir == 5 || dir == 6)
		{
			level->updateNeighborsAt(x, y - 1, z, this->id);
		}
		else if (dir == 0 || dir == 7)
		{
			level->updateNeighborsAt(x, y + 1, z, this->id);
		}
	}
	Tile::onRemove(level, x, y, z, id, data);
}

int LeverTile::getSignal(LevelSource *level, int x, int y, int z, int dir)
{
	return (level->getData(x, y, z) & 8) > 0 ? Redstone::SIGNAL_MAX : Redstone::SIGNAL_NONE;
}

int LeverTile::getDirectSignal(LevelSource *level, int x, int y, int z, int dir)
{
	int data = level->getData(x, y, z);
	if ((data & 8) == 0) return Redstone::SIGNAL_NONE;
	int myDir = data & 7;

	if (myDir == 0 && dir == 0) return Redstone::SIGNAL_MAX;
	if (myDir == 7 && dir == 0) return Redstone::SIGNAL_MAX;
	if (myDir == 6 && dir == 1) return Redstone::SIGNAL_MAX;
	if (myDir == 5 && dir == 1) return Redstone::SIGNAL_MAX;
	if (myDir == 4 && dir == 2) return Redstone::SIGNAL_MAX;
	if (myDir == 3 && dir == 3) return Redstone::SIGNAL_MAX;
	if (myDir == 2 && dir == 4) return Redstone::SIGNAL_MAX;
	if (myDir == 1 && dir == 5) return Redstone::SIGNAL_MAX;

	return Redstone::SIGNAL_NONE;
}

bool LeverTile::isSignalSource()
{
	return true;
}
