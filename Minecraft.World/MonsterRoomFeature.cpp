#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.item.h"
#include "MonsterRoomFeature.h"

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
						level->setTile(xx, yy, zz, 0);
					} else if (level->getMaterial(xx, yy, zz)->isSolid())
					{
						if (yy == y - 1 && random->nextInt(4) != 0)
						{
							level->setTile(xx, yy, zz, Tile::mossStone_Id);
						} else {
							level->setTile(xx, yy, zz, Tile::stoneBrick_Id);
						}
					}
				} else
				{
					level->setTile(xx, yy, zz, 0);
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

			level->setTile(xc, yc, zc, Tile::chest_Id);
			shared_ptr<ChestTileEntity> chest = dynamic_pointer_cast<ChestTileEntity >( level->getTileEntity(xc, yc, zc) );
			if (chest != NULL )
			{
				for (int j = 0; j < 8; j++)
				{
					shared_ptr<ItemInstance> item = randomItem(random);
					if (item != NULL) chest->setItem(random->nextInt(chest->getContainerSize()), item);
				}
			}

			break;
		}
	}


	level->setTile(x, y, z, Tile::mobSpawner_Id);
	shared_ptr<MobSpawnerTileEntity> entity = dynamic_pointer_cast<MobSpawnerTileEntity>( level->getTileEntity(x, y, z) );
	if( entity != NULL )
	{
		entity->setEntityId(randomEntityId(random));
	}

	return true;

}

shared_ptr<ItemInstance> MonsterRoomFeature::randomItem(Random *random)
{
	int type = random->nextInt(12);
	if (type == 0) return shared_ptr<ItemInstance>( new ItemInstance(Item::saddle) );
	if (type == 1) return shared_ptr<ItemInstance>( new ItemInstance(Item::ironIngot, random->nextInt(4) + 1) );
	if (type == 2) return shared_ptr<ItemInstance>( new ItemInstance(Item::bread) );
	if (type == 3) return shared_ptr<ItemInstance>( new ItemInstance(Item::wheat, random->nextInt(4) + 1) );
	if (type == 4) return shared_ptr<ItemInstance>( new ItemInstance(Item::sulphur, random->nextInt(4) + 1) );
	if (type == 5) return shared_ptr<ItemInstance>( new ItemInstance(Item::string, random->nextInt(4) + 1) );
	if (type == 6) return shared_ptr<ItemInstance>( new ItemInstance(Item::bucket_empty) );
	if (type == 7 && random->nextInt(100) == 0) return shared_ptr<ItemInstance>( new ItemInstance(Item::apple_gold) );
	if (type == 8 && random->nextInt(2) == 0) return shared_ptr<ItemInstance>( new ItemInstance(Item::redStone, random->nextInt(4) + 1) );
	if (type == 9 && random->nextInt(10) == 0) return shared_ptr<ItemInstance>( new ItemInstance( Item::items[Item::record_01->id + random->nextInt(2)]) );
	if (type == 10) return shared_ptr<ItemInstance>( new ItemInstance(Item::dye_powder, 1, DyePowderItem::BROWN) );
	if (type == 11) return Item::enchantedBook->createForRandomLoot(random);

	return shared_ptr<ItemInstance>();
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