#include "stdafx.h"
#include "VineTile.h"
#include "Material.h"
#include "JavaMath.h"
#include "Facing.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.stats.h"
#include "net.minecraft.world.level.biome.h"

VineTile::VineTile(int id) : Tile(id, Material::replaceable_plant, isSolidRender() )
{
	setTicking(true);
}

void VineTile::updateDefaultShape()
{
	setShape(0, 0, 0, 1, 1, 1);
}

int VineTile::getRenderShape()
{
	return SHAPE_VINE;
}

bool VineTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool VineTile::isCubeShaped()
{
	return false;
}

void VineTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	const float thickness = 1.0f / 16.0f;

	int facings = level->getData(x, y, z);

	float minX = 1;
	float minY = 1;
	float minZ = 1;
	float maxX = 0;
	float maxY = 0;
	float maxZ = 0;
	bool hasWall = facings > 0;

	if ((facings & VINE_WEST) != 0)
	{
		maxX = Math::_max(maxX, thickness);
		minX = 0;
		minY = 0;
		maxY = 1;
		minZ = 0;
		maxZ = 1;
		hasWall = true;
	}
	if ((facings & VINE_EAST) != 0)
	{
		minX = Math::_min(minX, 1 - thickness);
		maxX = 1;
		minY = 0;
		maxY = 1;
		minZ = 0;
		maxZ = 1;
		hasWall = true;
	}
	if ((facings & VINE_NORTH) != 0)
	{
		maxZ = Math::_max(maxZ, thickness);
		minZ = 0;
		minX = 0;
		maxX = 1;
		minY = 0;
		maxY = 1;
		hasWall = true;
	}
	if ((facings & VINE_SOUTH) != 0)
	{
		minZ = Math::_min(minZ, 1 - thickness);
		maxZ = 1;
		minX = 0;
		maxX = 1;
		minY = 0;
		maxY = 1;
		hasWall = true;
	}
	if (!hasWall && isAcceptableNeighbor(level->getTile(x, y + 1, z)))
	{
		minY = Math::_min(minY, 1 - thickness);
		maxY = 1;
		minX = 0;
		maxX = 1;
		minZ = 0;
		maxZ = 1;
	}
	setShape(minX, minY, minZ, maxX, maxY, maxZ);

}

AABB *VineTile::getAABB(Level *level, int x, int y, int z)
{
	return NULL;
}

bool VineTile::mayPlace(Level *level, int x, int y, int z, int face)
{
	switch (face)
	{
	default:
		return false;
	case Facing::UP:
		return isAcceptableNeighbor(level->getTile(x, y + 1, z));
	case Facing::NORTH:
		return isAcceptableNeighbor(level->getTile(x, y, z + 1));
	case Facing::SOUTH:
		return isAcceptableNeighbor(level->getTile(x, y, z - 1));
	case Facing::EAST:
		return isAcceptableNeighbor(level->getTile(x - 1, y, z));
	case Facing::WEST:
		return isAcceptableNeighbor(level->getTile(x + 1, y, z));
	}
}

bool VineTile::isAcceptableNeighbor(int id)
{
	if (id == 0) return false;
	Tile *tile = Tile::tiles[id];
	if (tile->isCubeShaped() && tile->material->blocksMotion()) return true;
	return false;
}

bool VineTile::updateSurvival(Level *level, int x, int y, int z)
{
	int facings = level->getData(x, y, z);
	int newFacings = facings;

	if (newFacings > 0)
	{
		for (int d = 0; d <= 3; d++)
		{
			int facing = 1 << d;
			if ((facings & facing) != 0)
			{
				if (!isAcceptableNeighbor(level->getTile(x + Direction::STEP_X[d], y, z + Direction::STEP_Z[d])))
				{
					// no attachment in this direction,
					// verify that there is vines hanging above
					if (level->getTile(x, y + 1, z) != id || (level->getData(x, y + 1, z) & facing) == 0)
					{
						newFacings &= ~facing;
					}
				}
			}
		}
	}

	if (newFacings == 0)
	{
		// the block will die unless it has a roof
		if (!isAcceptableNeighbor(level->getTile(x, y + 1, z)))
		{
			return false;
		}
	}
	if (newFacings != facings)
	{
		level->setData(x, y, z, newFacings, Tile::UPDATE_CLIENTS);
	}
	return true;

}

int VineTile::getColor() const
{
	return FoliageColor::getDefaultColor();
}

int VineTile::getColor(int auxData)
{
	return FoliageColor::getDefaultColor();
}

int VineTile::getColor(LevelSource *level, int x, int y, int z, int data)
{
	return getColor(level, x, y, z);
}

int VineTile::getColor(LevelSource *level, int x, int y, int z)
{
	return level->getBiome(x, z)->getFolageColor();
}

void VineTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (!level->isClientSide && !updateSurvival(level, x, y, z))
	{
		spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->removeTile(x, y, z);
	}
}

void VineTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (!level->isClientSide)
	{
		if (level->random->nextInt(4) == 0)
		{
			// 4J - Brought side spread check forward from 1.2.3
			int r = 4;
			int max = 5;
			bool noSideSpread = false;
			for (int xx = x - r; xx <= x + r; xx++)
			{
				for (int zz = z - r; zz <= z + r; zz++)
					for (int yy = y - 1; yy <= y + 1; yy++)
					{
						if (level->getTile(xx, yy, zz) == id && --max <= 0)
						{
							noSideSpread = true;
							goto testLoop;
						}
					}
testLoop:		if(noSideSpread) break;
			}

			int currentFacings = level->getData(x, y, z);
			int testFacing = level->random->nextInt(6);
			int testDirection = Direction::FACING_DIRECTION[testFacing];

			if (testFacing == Facing::UP && y < (Level::maxBuildHeight - 1) && level->isEmptyTile(x, y + 1, z))
			{
				// 4J - Brought side spread check forward from 1.2.3
				if (noSideSpread) return;

				// grow upwards, but only if there is something to cling to
				int spawnFacings = level->random->nextInt(16) & currentFacings;
				if (spawnFacings > 0)
				{
					for (int d = 0; d <= 3; d++)
					{
						if (!isAcceptableNeighbor(level->getTile(x + Direction::STEP_X[d], y + 1, z + Direction::STEP_Z[d])))
						{
							spawnFacings &= ~(1 << d);
						}
					}
					if (spawnFacings > 0)
					{
						level->setTileAndData(x, y + 1, z, id, spawnFacings, Tile::UPDATE_CLIENTS);
					}
				}
			}
			else if (testFacing >= Facing::NORTH && testFacing <= Facing::EAST && (currentFacings & (1 << testDirection)) == 0)
			{
				// 4J - Brought side spread check forward from 1.2.3
				if (noSideSpread) return;

				int edgeTile = level->getTile(x + Direction::STEP_X[testDirection], y, z + Direction::STEP_Z[testDirection]);

				if (edgeTile == 0 || Tile::tiles[edgeTile] == NULL)
				{
					// if the edge tile is air, we could possibly cling
					// to something
					int left = (testDirection + 1) & 3;
					int right = (testDirection + 3) & 3;

					// attempt to grow straight onto solid tiles
					if ((currentFacings & (1 << left)) != 0
						&& isAcceptableNeighbor(level->getTile(x + Direction::STEP_X[testDirection] + Direction::STEP_X[left], y, z + Direction::STEP_Z[testDirection] + Direction::STEP_Z[left])))
					{
						level->setTileAndData(x + Direction::STEP_X[testDirection], y, z + Direction::STEP_Z[testDirection], id, 1 << left, Tile::UPDATE_CLIENTS);
					}
					else if ((currentFacings & (1 << right)) != 0
						&& isAcceptableNeighbor(level->getTile(x + Direction::STEP_X[testDirection] + Direction::STEP_X[right], y, z + Direction::STEP_Z[testDirection] + Direction::STEP_Z[right])))
					{
						level->setTileAndData(x + Direction::STEP_X[testDirection], y, z + Direction::STEP_Z[testDirection], id, 1 << right, Tile::UPDATE_CLIENTS);
					}
					// attempt to grow around corners, but only if the
					// base tile is solid
					else if ((currentFacings & (1 << left)) != 0
						&& level->isEmptyTile(x + Direction::STEP_X[testDirection] + Direction::STEP_X[left], y, z + Direction::STEP_Z[testDirection] + Direction::STEP_Z[left])
						&& isAcceptableNeighbor(level->getTile(x + Direction::STEP_X[left], y, z + Direction::STEP_Z[left])))
					{
						level->setTileAndData(x + Direction::STEP_X[testDirection] + Direction::STEP_X[left], y, z + Direction::STEP_Z[testDirection] + Direction::STEP_Z[left], id,
							1 << ((testDirection + 2) & 3), Tile::UPDATE_CLIENTS);
					}
					else if ((currentFacings & (1 << right)) != 0
						&& level->isEmptyTile(x + Direction::STEP_X[testDirection] + Direction::STEP_X[right], y, z + Direction::STEP_Z[testDirection] + Direction::STEP_Z[right])
						&& isAcceptableNeighbor(level->getTile(x + Direction::STEP_X[right], y, z + Direction::STEP_Z[right])))
					{
						level->setTileAndData(x + Direction::STEP_X[testDirection] + Direction::STEP_X[right], y, z + Direction::STEP_Z[testDirection] + Direction::STEP_Z[right], id,
							1 << ((testDirection + 2) & 3), Tile::UPDATE_CLIENTS);
					}
					// attempt to grow onto the ceiling
					else if (isAcceptableNeighbor(level->getTile(x + Direction::STEP_X[testDirection], y + 1, z + Direction::STEP_Z[testDirection])))
					{
						level->setTileAndData(x + Direction::STEP_X[testDirection], y, z + Direction::STEP_Z[testDirection], id, 0, Tile::UPDATE_CLIENTS);
					}

				}
				else if (Tile::tiles[edgeTile]->material->isSolidBlocking() && Tile::tiles[edgeTile]->isCubeShaped())
				{
					// we have a wall that we can cling to
					level->setData(x, y, z, currentFacings | (1 << testDirection), Tile::UPDATE_CLIENTS);
				}
			}
			// growing downwards happens more often than the other
			// directions
			else if (y > 1)
			{
				int belowTile = level->getTile(x, y - 1, z);
				// grow downwards into air
				if (belowTile == 0)
				{
					int spawnFacings = level->random->nextInt(16) & currentFacings;
					if (spawnFacings > 0)
					{
						level->setTileAndData(x, y - 1, z, id, spawnFacings, Tile::UPDATE_CLIENTS);
					}
				}
				else if (belowTile == id)
				{
					int spawnFacings = level->random->nextInt(16) & currentFacings;
					int belowData = level->getData(x, y - 1, z);
					if (belowData != (belowData | spawnFacings))
					{
						level->setData(x, y - 1, z, belowData | spawnFacings, Tile::UPDATE_CLIENTS);
					}
				}
			}
		}
	}
}

int VineTile::getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue)
{
	int facings = 0;
	switch (face)
	{
	case Facing::NORTH:
		facings = VINE_SOUTH;
		break;
	case Facing::SOUTH:
		facings = VINE_NORTH;
		break;
	case Facing::WEST:
		facings = VINE_EAST;
		break;
	case Facing::EAST:
		facings = VINE_WEST;
		break;
	}
	if (facings != 0)
	{
		return facings;
	}
	return itemValue;
}

int VineTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return 0;
}

int VineTile::getResourceCount(Random *random)
{
	return 0;
}

void VineTile::playerDestroy(Level *level, shared_ptr<Player>player, int x, int y, int z, int data)
{
	if (!level->isClientSide && player->getSelectedItem() != NULL && player->getSelectedItem()->id == Item::shears->id)
	{
		player->awardStat(
			GenericStats::blocksMined(id),
			GenericStats::param_blocksMined(id,data,1)
			);

		// drop leaf block instead of sapling
		popResource(level, x, y, z, shared_ptr<ItemInstance>(new ItemInstance(Tile::vine, 1, 0)));
	}
	else
	{
		Tile::playerDestroy(level, player, x, y, z, data);
	}
}
