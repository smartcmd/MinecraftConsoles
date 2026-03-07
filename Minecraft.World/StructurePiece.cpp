#include "stdafx.h"
#include "net.minecraft.world.level.levelgen.structure.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.entity.h"
#include "WeighedTreasure.h"
#include "StructurePiece.h"
#include "BoundingBox.h"
#include "Direction.h"
#include "JavaMath.h"
#include "Facing.h"
#include "DoorItem.h"

/**
*
* A structure piece is a construction or room, located somewhere in the world
* with a given orientatino (out of Direction.java). Structure pieces have a
* bounding box that says where the piece is located and its bounds, and the
* orientation is used to translate local coordinates into world coordinates.
* <p>
* The default orientation is Direction.UNDEFINED, in which case no translation
* will occur. If the orientation is Direction::NORTH, coordinate (0, 0, 0) will
* be at (boundingBox.x0, boundingBox.y0, boundingBox.z1). In other words, (1,
* 1, 1) will be translated to (boundingBox.x0 + 1, boundingBox.y0 + 1,
* boundingBox.z1 - 1).
* <p>
* When using Direction::SOUTH, the x coordinate will be the same, and the z
* coordinate will be flipped. In other words, the bounding box is NOT rotated!
* It is only flipped along the z axis. Also note that the bounding box is in
* world coordinates, so the local drawing must never reach outside of this.
* <p>
* When using east and west coordinates, the local z coordinate will be swapped
* with the local x coordinate. For example, (0, 0, 0) is (boundingBox.z1,
* boundingBox.y0, boundingBox.z0), and (1, 1, 1) becomes (boundingBox.x1 - 1,
* boundingBox.y0 + 1, boundingBox.z0 + 1) when using Direction::WEST.
* <p>
* When-ever a structure piece is placing blocks, it is VERY IMPORTANT to always
* make sure that all getTile and setTile calls are within the chunk's bounding
* box. Failing to check this will cause the level generator to create new
* chunks, leading to infinite loops and other errors.
*/

StructurePiece::StructurePiece()
{
	boundingBox = NULL;
	orientation = 0;
	genDepth = 0;
	// for reflection
}

StructurePiece::StructurePiece( int genDepth )
{
	boundingBox = NULL;
	this->genDepth = genDepth;
	orientation = Direction::UNDEFINED;
}

StructurePiece::~StructurePiece()
{
	if(boundingBox != NULL) delete boundingBox;
}

CompoundTag *StructurePiece::createTag()
{
	CompoundTag *tag = new CompoundTag();

	tag->putString(L"id", StructureFeatureIO::getEncodeId(this));
	tag->put(L"BB", boundingBox->createTag(L"BB"));
	tag->putInt(L"O", orientation);
	tag->putInt(L"GD", genDepth);

	addAdditonalSaveData(tag);

	return tag;
}

void StructurePiece::load(Level *level, CompoundTag *tag)
{

	if (tag->contains(L"BB"))
	{
		boundingBox = new BoundingBox(tag->getIntArray(L"BB"));
	}
	orientation = tag->getInt(L"O");
	genDepth = tag->getInt(L"GD");

	readAdditonalSaveData(tag);
}

void StructurePiece::addChildren( StructurePiece* startPiece, list< StructurePiece* > *pieces, Random* random )
{
}

BoundingBox* StructurePiece::getBoundingBox()
{
	return boundingBox;
}

int StructurePiece::getGenDepth()
{
	return genDepth;
}

bool StructurePiece::isInChunk( ChunkPos* pos )
{
	int cx = ( pos->x << 4 );
	int cz = ( pos->z << 4 );

	return boundingBox->intersects( cx, cz, cx + 15, cz + 15 );
}

StructurePiece* StructurePiece::findCollisionPiece( list< StructurePiece* > *pieces, BoundingBox* box )
{
	for (auto& piece : *pieces)
	{
		if ( piece && piece->getBoundingBox() && piece->getBoundingBox()->intersects( box ) )
		{
			return piece;
		}
	}
	return NULL;
}

// 4J-PB - Added from 1.2.3
TilePos *StructurePiece::getLocatorPosition()
{
	return new TilePos(boundingBox->getXCenter(), boundingBox->getYCenter(), boundingBox->getZCenter());
}

bool StructurePiece::edgesLiquid( Level* level, BoundingBox* chunkBB )
{
	int x0 = Math::_max( boundingBox->x0 - 1, chunkBB->x0 );
	int y0 = Math::_max( boundingBox->y0 - 1, chunkBB->y0 );
	int z0 = Math::_max( boundingBox->z0 - 1, chunkBB->z0 );
	int x1 = Math::_min( boundingBox->x1 + 1, chunkBB->x1 );
	int y1 = Math::_min( boundingBox->y1 + 1, chunkBB->y1 );
	int z1 = Math::_min( boundingBox->z1 + 1, chunkBB->z1 );

	// roof and floor
	for ( int x = x0; x <= x1; x++ )
	{
		for ( int z = z0; z <= z1; z++ )
		{
			int tile = level->getTile( x, y0, z );
			if ( tile > 0 && Tile::tiles[tile]->material->isLiquid() )
			{
				return true;
			}
			tile = level->getTile( x, y1, z );
			if ( tile > 0 && Tile::tiles[tile]->material->isLiquid() )
			{
				return true;
			}
		}
	}
	// north and south
	for ( int x = x0; x <= x1; x++ )
	{
		for ( int y = y0; y <= y1; y++ )
		{
			int tile = level->getTile( x, y, z0 );
			if ( tile > 0 && Tile::tiles[tile]->material->isLiquid() )
			{
				return true;
			}
			tile = level->getTile( x, y, z1 );
			if ( tile > 0 && Tile::tiles[tile]->material->isLiquid() )
			{
				return true;
			}
		}
	}
	// east and west
	for ( int z = z0; z <= z1; z++ )
	{
		for ( int y = y0; y <= y1; y++ )
		{
			int tile = level->getTile( x0, y, z );
			if ( tile > 0 && Tile::tiles[tile]->material->isLiquid() )
			{
				return true;
			}
			tile = level->getTile( x1, y, z );
			if ( tile > 0 && Tile::tiles[tile]->material->isLiquid() )
			{
				return true;
			}
		}
	}
	return false;

}

int StructurePiece::getWorldX( int x, int z )
{
	switch ( orientation )
	{
	case Direction::NORTH:
	case Direction::SOUTH:
		return boundingBox->x0 + x;
	case Direction::WEST:
		return boundingBox->x1 - z;
	case Direction::EAST:
		return boundingBox->x0 + z;
	default:
		return x;
	}
}

int StructurePiece::getWorldY( int y )
{
	if ( orientation == Direction::UNDEFINED )
	{
		return y;
	}
	return y + boundingBox->y0;
}

int StructurePiece::getWorldZ( int x, int z )
{
	switch ( orientation )
	{
	case Direction::NORTH:
		return boundingBox->z1 - z;
	case Direction::SOUTH:
		return boundingBox->z0 + z;
	case Direction::WEST:
	case Direction::EAST:
		return boundingBox->z0 + x;
	default:
		return z;
	}
}

int StructurePiece::getOrientationData( int tile, int data )
{
	if ( tile == Tile::rail->id )
	{
		if ( orientation == Direction::WEST || orientation == Direction::EAST )
		{
			if ( data == BaseRailTile::DIR_FLAT_X )
			{
				return BaseRailTile::DIR_FLAT_Z;
			}
			else
			{
				return BaseRailTile::DIR_FLAT_X;
			}
		}
	}
	else if ( tile == Tile::door_wood_Id || tile == Tile::door_iron_Id )
	{
		if ( orientation == Direction::SOUTH )
		{
			if ( data == 0 )
			{
				return 2;
			}
			if ( data == 2 )
			{
				return 0;
			}
		}
		else if ( orientation == Direction::WEST )
		{
			// 0 = 1
			// 1 = 2
			// 2 = 3
			// 3 = 0
			return ( data + 1 ) & 3;
		}
		else if ( orientation == Direction::EAST )
		{
			// 0 = 3
			// 1 = 0
			// 2 = 1
			// 3 = 2
			return ( data + 3 ) & 3;
		}
	}
	else if ( tile == Tile::stairs_stone_Id || tile == Tile::stairs_wood_Id || tile == Tile::stairs_netherBricks_Id || tile == Tile::stairs_stoneBrick_Id || tile == Tile::stairs_sandstone_Id)
	{
		if ( orientation == Direction::SOUTH )
		{
			if ( data == 2 )
			{
				return 3;
			}
			if ( data == 3 )
			{
				return 2;
			}
		}
		else if ( orientation == Direction::WEST )
		{
			if ( data == 0 )
			{
				return 2;
			}
			if ( data == 1 )
			{
				return 3;
			}
			if ( data == 2 )
			{
				return 0;
			}
			if ( data == 3 )
			{
				return 1;
			}
		}
		else if ( orientation == Direction::EAST )
		{
			if ( data == 0 )
			{
				return 2;
			}
			if ( data == 1 )
			{
				return 3;
			}
			if ( data == 2 )
			{
				return 1;
			}
			if ( data == 3 )
			{
				return 0;
			}
		}
	}
	else if ( tile == Tile::ladder->id )
	{
		if ( orientation == Direction::SOUTH )
		{
			if ( data == Facing::NORTH )
			{
				return Facing::SOUTH;
			}
			if ( data == Facing::SOUTH )
			{
				return Facing::NORTH;
			}
		}
		else if ( orientation == Direction::WEST )
		{
			if ( data == Facing::NORTH )
			{
				return Facing::WEST;
			}
			if ( data == Facing::SOUTH )
			{
				return Facing::EAST;
			}
			if ( data == Facing::WEST )
			{
				return Facing::NORTH;
			}
			if ( data == Facing::EAST )
			{
				return Facing::SOUTH;
			}
		}
		else if ( orientation == Direction::EAST )
		{
			if ( data == Facing::NORTH )
			{
				return Facing::EAST;
			}
			if ( data == Facing::SOUTH )
			{
				return Facing::WEST;
			}
			if ( data == Facing::WEST )
			{
				return Facing::NORTH;
			}
			if ( data == Facing::EAST )
			{
				return Facing::SOUTH;
			}
		}

	}
	else if ( tile == Tile::button->id )
	{
		if ( orientation == Direction::SOUTH )
		{
			if ( data == 3 )
			{
				return 4;
			}
			if ( data == 4 )
			{
				return 3;
			}
		}
		else if ( orientation == Direction::WEST )
		{
			if ( data == 3 )
			{
				return 1;
			}
			if ( data == 4 )
			{
				return 2;
			}
			if ( data == 2 )
			{
				return 3;
			}
			if ( data == 1 )
			{
				return 4;
			}
		}
		else if ( orientation == Direction::EAST )
		{
			if ( data == 3 )
			{
				return 2;
			}
			if ( data == 4 )
			{
				return 1;
			}
			if ( data == 2 )
			{
				return 3;
			}
			if ( data == 1 )
			{
				return 4;
			}
		}
	}
	else if (tile == Tile::tripWireSource_Id || (Tile::tiles[tile] != NULL && dynamic_cast<DirectionalTile *>(Tile::tiles[tile])))
	{
		if (orientation == Direction::SOUTH)
		{
			if (data == Direction::SOUTH || data == Direction::NORTH)
			{
				return Direction::DIRECTION_OPPOSITE[data];
			}
		}
		else if (orientation == Direction::WEST)
		{
			if (data == Direction::NORTH)
			{
				return Direction::WEST;
			}
			if (data == Direction::SOUTH)
			{
				return Direction::EAST;
			}
			if (data == Direction::WEST)
			{
				return Direction::NORTH;
			}
			if (data == Direction::EAST)
			{
				return Direction::SOUTH;
			}
		}
		else if (orientation == Direction::EAST)
		{
			if (data == Direction::NORTH)
			{
				return Direction::EAST;
			}
			if (data == Direction::SOUTH)
			{
				return Direction::WEST;
			}
			if (data == Direction::WEST)
			{
				return Direction::NORTH;
			}
			if (data == Direction::EAST)
			{
				return Direction::SOUTH;
			}
		}
	}
	else if (tile == Tile::pistonBase_Id || tile == Tile::pistonStickyBase_Id || tile == Tile::lever_Id || tile == Tile::dispenser_Id)
	{
		if (orientation == Direction::SOUTH)
		{
			if (data == Facing::NORTH || data == Facing::SOUTH)
			{
				return Facing::OPPOSITE_FACING[data];
			}
		}
		else if (orientation == Direction::WEST)
		{
			if (data == Facing::NORTH)
			{
				return Facing::WEST;
			}
			if (data == Facing::SOUTH)
			{
				return Facing::EAST;
			}
			if (data == Facing::WEST)
			{
				return Facing::NORTH;
			}
			if (data == Facing::EAST)
			{
				return Facing::SOUTH;
			}
		} else if (orientation == Direction::EAST)
		{
			if (data == Facing::NORTH)
			{
				return Facing::EAST;
			}
			if (data == Facing::SOUTH)
			{
				return Facing::WEST;
			}
			if (data == Facing::WEST)
			{
				return Facing::NORTH;
			}
			if (data == Facing::EAST)
			{
				return Facing::SOUTH;
			}
		}
	}
	return data;

}

void StructurePiece::placeBlock( Level* level, int block, int data, int x, int y, int z, BoundingBox* chunkBB )
{
	int worldX = getWorldX( x, z );
	int worldY = getWorldY( y );
	int worldZ = getWorldZ( x, z );

	if ( !chunkBB->isInside( worldX, worldY, worldZ ) )
	{
		return;
	}

	// 4J Stu - We shouldn't be removing bedrock when generating things (eg in SuperFlat)
	if(worldY == 0) return;

	level->setTileAndData( worldX, worldY, worldZ, block, data, Tile::UPDATE_CLIENTS);
}


/**
* The purpose of this method is to wrap the getTile call on Level, in order
* to prevent the level from generating chunks that shouldn't be loaded yet.
* Returns 0 if the call is out of bounds.
*
* @param level
* @param x
* @param y
* @param z
* @param chunkPosition
* @return
*/
int StructurePiece::getBlock( Level* level, int x, int y, int z, BoundingBox* chunkBB )
{
	int worldX = getWorldX( x, z );
	int worldY = getWorldY( y );
	int worldZ = getWorldZ( x, z );

	if ( !chunkBB->isInside( worldX, worldY, worldZ ) )
	{
		return 0;
	}

	return level->getTile( worldX, worldY, worldZ );
}

void StructurePiece::generateAirBox(Level *level, BoundingBox *chunkBB, int x0, int y0, int z0, int x1, int y1, int z1)
{
	for (int y = y0; y <= y1; y++)
	{
		for (int x = x0; x <= x1; x++)
		{
			for (int z = z0; z <= z1; z++)
			{
				placeBlock(level, 0, 0, x, y, z, chunkBB);
			}
		}
	}
}

void StructurePiece::generateBox( Level* level, BoundingBox* chunkBB, int x0, int y0, int z0, int x1, int y1, int z1,
								 int edgeTile, int fillTile, bool skipAir )
{
	for ( int y = y0; y <= y1; y++ )
	{
		for ( int x = x0; x <= x1; x++ )
		{
			for ( int z = z0; z <= z1; z++ )
			{

				if ( skipAir && getBlock( level, x, y, z, chunkBB ) == 0 )
				{
					continue;
				}
				if ( y == y0 || y == y1 || x == x0 || x == x1 || z == z0 || z == z1 )
				{
					placeBlock( level, edgeTile, 0, x, y, z, chunkBB );
				}
				else
				{
					placeBlock( level, fillTile, 0, x, y, z, chunkBB );
				}

			}
		}
	}
}

void StructurePiece::generateBox(Level *level, BoundingBox *chunkBB, int x0, int y0, int z0, int x1, int y1, int z1, int edgeTile, int edgeData, int fillTile, int fillData, bool skipAir)
{
	for (int y = y0; y <= y1; y++)
	{
		for (int x = x0; x <= x1; x++)
		{
			for (int z = z0; z <= z1; z++)
			{

				if (skipAir && getBlock(level, x, y, z, chunkBB) == 0)
				{
					continue;
				}
				if (y == y0 || y == y1 || x == x0 || x == x1 || z == z0 || z == z1)
				{
					placeBlock(level, edgeTile, edgeData, x, y, z, chunkBB);
				}
				else
				{
					placeBlock(level, fillTile, fillData, x, y, z, chunkBB);
				}

			}
		}
	}
}

void StructurePiece::generateBox( Level* level, BoundingBox* chunkBB, BoundingBox* boxBB, int edgeTile, int fillTile,
								 bool skipAir )
{
	generateBox( level, chunkBB, boxBB->x0, boxBB->y0, boxBB->z0, boxBB->x1, boxBB->y1, boxBB->z1, edgeTile, fillTile,
		skipAir );
}

void StructurePiece::generateBox( Level* level, BoundingBox* chunkBB, int x0, int y0, int z0, int x1, int y1, int z1,
								 bool skipAir, Random* random, StructurePiece::BlockSelector* selector )
{
	for ( int y = y0; y <= y1; y++ )
	{
		for ( int x = x0; x <= x1; x++ )
		{
			for ( int z = z0; z <= z1; z++ )
			{

				if ( skipAir && getBlock( level, x, y, z, chunkBB ) == 0 )
				{
					continue;
				}
				selector->next( random, x, y, z, y == y0 || y == y1 || x == x0 || x == x1 || z == z0 || z == z1 );
				placeBlock( level, selector->getNextId(), selector->getNextData(), x, y, z, chunkBB );

			}
		}
	}
}

void StructurePiece::generateBox( Level* level, BoundingBox* chunkBB, BoundingBox* boxBB, bool skipAir, Random* random,
								 StructurePiece::BlockSelector* selector )
{
	generateBox( level, chunkBB, boxBB->x0, boxBB->y0, boxBB->z0, boxBB->x1, boxBB->y1, boxBB->z1, skipAir, random,
		selector );
}

void StructurePiece::generateMaybeBox( Level* level, BoundingBox* chunkBB, Random *random, float probability, int x0,
									  int y0, int z0, int x1, int y1, int z1, int edgeTile, int fillTile,
									  bool skipAir )
{
	for ( int y = y0; y <= y1; y++ )
	{
		for ( int x = x0; x <= x1; x++ )
		{
			for ( int z = z0; z <= z1; z++ )
			{

				if ( random->nextFloat() > probability )
				{
					continue;
				}
				if ( skipAir && getBlock( level, x, y, z, chunkBB ) == 0 )
				{
					continue;
				}
				if ( y == y0 || y == y1 || x == x0 || x == x1 || z == z0 || z == z1 )
				{
					placeBlock( level, edgeTile, 0, x, y, z, chunkBB );
				}
				else
				{
					placeBlock( level, fillTile, 0, x, y, z, chunkBB );
				}

			}
		}
	}
}

void StructurePiece::maybeGenerateBlock( Level* level, BoundingBox* chunkBB, Random *random, float probability, int x,
										int y, int z, int tile, int data )
{
	if ( random->nextFloat() < probability )
	{
		placeBlock( level, tile, data, x, y, z, chunkBB );
	}
}

void StructurePiece::generateUpperHalfSphere( Level* level, BoundingBox* chunkBB, int x0, int y0, int z0, int x1,
											 int y1, int z1, int fillTile, bool skipAir )
{
	float	diagX = (float)( x1 - x0 + 1 );
	float	diagY = (float)( y1 - y0 + 1 );
	float	diagZ = (float)( z1 - z0 + 1 );
	float	cx = x0 + diagX / 2;
	float	cz = z0 + diagZ / 2;

	for ( int y = y0; y <= y1; y++ )
	{
		float normalizedYDistance = ( float )( y - y0 ) / diagY;

		for ( int x = x0; x <= x1; x++ )
		{
			float normalizedXDistance = ( float )( x - cx ) / ( diagX * 0.5f );

			for ( int z = z0; z <= z1; z++ )
			{
				float normalizedZDistance = ( float )( z - cz ) / ( diagZ * 0.5f );

				if ( skipAir && getBlock( level, x, y, z, chunkBB ) == 0 )
				{
					continue;
				}

				float dist = ( normalizedXDistance * normalizedXDistance ) + ( normalizedYDistance *
					normalizedYDistance ) + ( normalizedZDistance * normalizedZDistance );

				if ( dist <= 1.05f )
				{
					placeBlock( level, fillTile, 0, x, y, z, chunkBB );
				}

			}
		}
	}

}

void StructurePiece::generateAirColumnUp( Level* level, int x, int startY, int z, BoundingBox* chunkBB )
{
	int worldX = getWorldX( x, z );
	int worldY = getWorldY( startY );
	int worldZ = getWorldZ( x, z );

	if ( !chunkBB->isInside( worldX, worldY, worldZ ) )
	{
		return;
	}

	while ( !level->isEmptyTile( worldX, worldY, worldZ ) && worldY < Level::maxBuildHeight - 1 )
	{
		level->setTileAndData( worldX, worldY, worldZ, 0, 0, Tile::UPDATE_CLIENTS);
		worldY++;
	}
}

void StructurePiece::fillColumnDown( Level* level, int tile, int tileData, int x, int startY, int z, BoundingBox* chunkBB )
{
	int worldX = getWorldX( x, z );
	int worldY = getWorldY( startY );
	int worldZ = getWorldZ( x, z );

	if ( !chunkBB->isInside( worldX, worldY, worldZ ) )
	{
		return;
	}

	while ( ( level->isEmptyTile( worldX, worldY, worldZ ) || level->getMaterial( worldX, worldY, worldZ )->isLiquid() ) && worldY > 1 )
	{
		level->setTileAndData( worldX, worldY, worldZ, tile, tileData, Tile::UPDATE_CLIENTS );
		worldY--;
	}
}

bool StructurePiece::createChest( Level* level, BoundingBox* chunkBB, Random* random, int x, int y, int z,
								 WeighedTreasureArray treasure, int numRolls )
{
	int worldX = getWorldX( x, z );
	int worldY = getWorldY( y );
	int worldZ = getWorldZ( x, z );

	if ( chunkBB->isInside( worldX, worldY, worldZ ) )
	{
		if ( level->getTile( worldX, worldY, worldZ ) != Tile::chest->id )
		{
			level->setTileAndData( worldX, worldY, worldZ, Tile::chest->id, 0, Tile::UPDATE_CLIENTS );
			shared_ptr<ChestTileEntity> chest = dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity( worldX, worldY, worldZ ));
			if ( chest != NULL ) WeighedTreasure::addChestItems( random, treasure, chest, numRolls );
			return true;
		}
	}
	return false;
}

bool StructurePiece::createDispenser(Level *level, BoundingBox *chunkBB, Random *random, int x, int y, int z, int facing, WeighedTreasureArray items, int numRolls)
{
	int worldX = getWorldX(x, z);
	int worldY = getWorldY(y);
	int worldZ = getWorldZ(x, z);

	if (chunkBB->isInside(worldX, worldY, worldZ))
	{
		if (level->getTile(worldX, worldY, worldZ) != Tile::dispenser_Id)
		{
			level->setTileAndData(worldX, worldY, worldZ, Tile::dispenser_Id, getOrientationData(Tile::dispenser_Id, facing), Tile::UPDATE_CLIENTS);
			shared_ptr<DispenserTileEntity> dispenser = dynamic_pointer_cast<DispenserTileEntity>(level->getTileEntity(worldX, worldY, worldZ));
			if (dispenser != NULL) WeighedTreasure::addDispenserItems(random, items, dispenser, numRolls);
			return true;
		}
	}
	return false;
}

void StructurePiece::createDoor( Level* level, BoundingBox* chunkBB, Random* random, int x, int y, int z,
								int orientation )
{
	int worldX = getWorldX( x, z );
	int worldY = getWorldY( y );
	int worldZ = getWorldZ( x, z );

	if ( chunkBB->isInside( worldX, worldY, worldZ ) )
	{
		DoorItem::place( level, worldX, worldY, worldZ, orientation, Tile::door_wood );
	}
}
