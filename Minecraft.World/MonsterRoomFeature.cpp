#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.item.h"
#include "WeighedTreasure.h"
#include "MonsterRoomFeature.h"

WeighedTreasure *MonsterRoomFeature::monsterRoomTreasure[MonsterRoomFeature::TREASURE_ITEMS_COUNT] = 
{
		new WeighedTreasure(Item::saddle_Id, 0, 1, 1, 10),
		new WeighedTreasure(Item::ironIngot_Id, 0, 1, 4, 10),
		new WeighedTreasure(Item::bread_Id, 0, 1, 1, 10),
		new WeighedTreasure(Item::wheat_Id, 0, 1, 4, 10),
		new WeighedTreasure(Item::gunpowder_Id, 0, 1, 4, 10),
		new WeighedTreasure(Item::string_Id, 0, 1, 4, 10),
		new WeighedTreasure(Item::bucket_empty_Id, 0, 1, 1, 10),
		new WeighedTreasure(Item::apple_gold_Id, 0, 1, 1, 1),
		new WeighedTreasure(Item::redStone_Id, 0, 1, 4, 10),
		new WeighedTreasure(Item::record_01_Id, 0, 1, 1, 10),
		new WeighedTreasure(Item::record_02_Id, 0, 1, 1, 10),
		new WeighedTreasure(Item::nameTag_Id, 0, 1, 1, 10),
		new WeighedTreasure(Item::horseArmorGold_Id, 0, 1, 1, 2),
		new WeighedTreasure(Item::horseArmorMetal_Id, 0, 1, 1, 5),
		new WeighedTreasure(Item::horseArmorDiamond_Id, 0, 1, 1, 1),
};

bool MonsterRoomFeature::place(Level *level, Random *random, int x, int y, int z)
{
	int hr = 3;
	int xr = random->nextInt(2) + 2;
	int zr = random->nextInt(2) + 2;

	int holeCount = 0;
	for (int xx = x - xr - 1; xx <= x + xr + 1; xx++)
	{
		for (int yy = y - 1; yy <= y + hr + 1; yy++)
		{
			for (int zz = z - zr - 1; zz <= z + zr + 1; zz++)
			{
				Material *m = level->getMaterial(xx, yy, zz);
				if (yy == y - 1 && !m->isSolid()) return false;
				if (yy == y + hr + 1 && !m->isSolid()) return false;

				if (xx == x - xr - 1 || xx == x + xr + 1 || zz == z - zr - 1 || zz == z + zr + 1)
				{
					if (yy == y && level->isEmptyTile(xx, yy, zz) && level->isEmptyTile(xx, yy + 1, zz))
					{
						holeCount++;
					}
				}

			}
		}
	}

	if (holeCount < 1 || holeCount > 5) return false;

	for (int xx = x - xr - 1; xx <= x + xr + 1; xx++)
	{
		for (int yy = y + hr; yy >= y - 1; yy--)
		{
			for (int zz = z - zr - 1; zz <= z + zr + 1; zz++)
			{

				if (xx == x - xr - 1 || yy == y - 1 || zz == z - zr - 1 || xx == x + xr + 1 || yy == y + hr + 1 || zz == z + zr + 1)
				{
					if (yy >= 0 && !level->getMaterial(xx, yy - 1, zz)->isSolid())
					{
						level->removeTile(xx, yy, zz);
					}
					else if (level->getMaterial(xx, yy, zz)->isSolid())
					{
						if (yy == y - 1 && random->nextInt(4) != 0)
						{
							level->setTileAndData(xx, yy, zz, Tile::mossyCobblestone_Id, 0, Tile::UPDATE_CLIENTS);
						}
						else
						{
							level->setTileAndData(xx, yy, zz, Tile::cobblestone_Id, 0, Tile::UPDATE_CLIENTS);
						}
					}
				} else
				{
					level->removeTile(xx, yy, zz);
				}
			}
		}
	}

	for (int cc = 0; cc < 2; cc++)
	{
		for (int i = 0; i < 3; i++)
		{
			int xc = x + random->nextInt(xr * 2 + 1) - xr;
			int yc = y;
			int zc = z + random->nextInt(zr * 2 + 1) - zr;
			if (!level->isEmptyTile(xc, yc, zc)) continue;

			int count = 0;
			if (level->getMaterial(xc - 1, yc, zc)->isSolid()) count++;
			if (level->getMaterial(xc + 1, yc, zc)->isSolid()) count++;
			if (level->getMaterial(xc, yc, zc - 1)->isSolid()) count++;
			if (level->getMaterial(xc, yc, zc + 1)->isSolid()) count++;

			if (count != 1) continue;

			level->setTileAndData(xc, yc, zc, Tile::chest_Id, 0, Tile::UPDATE_CLIENTS);
			WeighedTreasureArray wrapperArray(monsterRoomTreasure, TREASURE_ITEMS_COUNT);
			WeighedTreasureArray treasure = WeighedTreasure::addToTreasure(wrapperArray, Item::enchantedBook->createForRandomTreasure(random));
			shared_ptr<ChestTileEntity> chest = dynamic_pointer_cast<ChestTileEntity >( level->getTileEntity(xc, yc, zc) );
			if (chest != nullptr )
			{
				WeighedTreasure::addChestItems(random, treasure, chest, 8);
			}

			break;
		}
	}


	level->setTileAndData(x, y, z, Tile::mobSpawner_Id, 0, Tile::UPDATE_CLIENTS);
	shared_ptr<MobSpawnerTileEntity> entity = dynamic_pointer_cast<MobSpawnerTileEntity>( level->getTileEntity(x, y, z) );
	if( entity != nullptr )
	{
		entity->getSpawner()->setEntityId(randomEntityId(random));
	}

	return true;

}

wstring MonsterRoomFeature::randomEntityId(Random *random)
{
	int id = random->nextInt(4);
	if (id == 0) return wstring(L"Skeleton");
	if (id == 1) return wstring(L"Zombie");
	if (id == 2) return wstring(L"Zombie");
	if (id == 3) return wstring(L"Spider");
	return wstring(L"");
}