#include "stdafx.h"
#include "BonusChestFeature.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "WeighedTreasure.h"
#include "StructurePiece.h"

BonusChestFeature::BonusChestFeature(WeighedTreasureArray treasureList, int numRolls)	: treasureList(treasureList), numRolls(numRolls)
{
}

// 4J - original virtual method
bool BonusChestFeature::place(Level *level, Random *random, int x, int y, int z)
{
	return place(level, random, x, y, z, false );
}

// 4J - added force parameter - trying to keep this as similar as possible to the original algorithm, but would also like it to definitely place a
// chest as it doesn't necessarily find somewhere in the original java. This method is called multple times for different x,y,z round the
// spawn point and force signifies that this is the last time this will be called. In this case, just place the chest exactly where the input
// parameters requested we place it (we know this will be one tile above the top solid block of a randomn column), and then do our best to place
// any surrounding torches where appropriate.

bool BonusChestFeature::place(Level *level, Random *random, int x, int y, int z, bool force)
{
	//Will only spawn a bonus chest if the world is new and has never been saved.
	if (level->isNew)
	{
		if( !force )
		{
			int t = 0;
			while (((t = level->getTile(x, y, z)) == 0 || t == Tile::leaves_Id) && y > 1)
			y--;

			if (y < 1)
			{
				return false;
			}
			y++;
		}

	for (int i = 0; i < 4; i++)
	{
		int x2, y2, z2;

		if( force )
		{
			x2 = x;
			y2 = y - 1;	// 4J - the position passed in is actually two above the top solid block, as the calling function adds 1 to getTopSolidBlock, and that actually returns the block above anyway.
			// this would explain why there is a while loop above here (not used in force mode) to move the y back down again, shouldn't really be needed if 1 wasn't added to the getTopSolidBlock return value.
			z2 = z;
		}
		else
		{
			x2 = x + random->nextInt(4) - random->nextInt(4);
			y2 = y + random->nextInt(3) - random->nextInt(3);
			z2 = z + random->nextInt(4) - random->nextInt(4);
		}

		if (force || ( level->isEmptyTile(x2, y2, z2) && level->isTopSolidBlocking(x2, y2 - 1, z2)))
		{
			level->setTileAndData(x2, y2, z2, Tile::chest_Id, 0, Tile::UPDATE_CLIENTS);
			shared_ptr<ChestTileEntity> chest = dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(x2, y2, z2));
			if (chest != nullptr)
			{
				WeighedTreasure::addChestItems(random, treasureList, chest, numRolls);
				chest->isBonusChest = true;	// 4J added
			}
			if (level->isEmptyTile(x2 - 1, y2, z2) && level->isTopSolidBlocking(x2 - 1, y2 - 1, z2))
			{
				level->setTileAndData(x2 - 1, y2, z2, Tile::torch_Id, 0, Tile::UPDATE_CLIENTS);
			}
			if (level->isEmptyTile(x2 + 1, y2, z2) && level->isTopSolidBlocking(x2 - 1, y2 - 1, z2))
			{
				level->setTileAndData(x2 + 1, y2, z2, Tile::torch_Id, 0, Tile::UPDATE_CLIENTS);
			}
			if (level->isEmptyTile(x2, y2, z2 - 1) && level->isTopSolidBlocking(x2 - 1, y2 - 1, z2))
			{
				level->setTileAndData(x2, y2, z2 - 1, Tile::torch_Id, 0, Tile::UPDATE_CLIENTS);
			}
			if (level->isEmptyTile(x2, y2, z2 + 1) && level->isTopSolidBlocking(x2 - 1, y2 - 1, z2))
			{
				level->setTileAndData(x2, y2, z2 + 1, Tile::torch_Id, 0, Tile::UPDATE_CLIENTS);
			}
			return true;
		}
	}

	return false;

	}
}
