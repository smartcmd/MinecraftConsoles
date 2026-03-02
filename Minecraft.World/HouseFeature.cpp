#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "HouseFeature.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.item.h"

bool HouseFeature::place(Level *level, Random *random, int x, int y, int z)
{
    while (y > 0 && !level->getMaterial(x, y - 1, z)->blocksMotion())
        y--;

	int w = random->nextInt(7) + 7;
	int h = 4 + random->nextInt(3) / 2;
	int d = random->nextInt(7) + 7;

    int x0 = x - w / 2;
    int y0 = y;
    int z0 = z - d / 2;

    int doorSide = random->nextInt(4);
    if (doorSide < 2) d += 2;
    else w += 2;

    for (int xx = x0; xx < x0 + w; xx++)
	{
        for (int zz = z0; zz < z0 + d; zz++)
		{
            Material *m = level->getMaterial(xx, y - 1, zz);
            if (!m->blocksMotion() || m == Material::ice) return false;

            bool ok = false;
            if (doorSide == 0 && xx < x0 + 2) ok = true;
            if (doorSide == 1 && xx > x0 + w - 1 - 2) ok = true;
            if (doorSide == 2 && zz < z0 + 2) ok = true;
            if (doorSide == 3 && zz > z0 + d - 1 - 2) ok = true;
            int t = level->getTile(xx, y, zz);
            if (ok)
			{
                if (t != 0) return false;
            }
			else
			{
                if (t == Tile::stoneBrick_Id || t == Tile::mossStone_Id) return false;
			}

        }
    }

    if (doorSide == 0)
	{
        x0++;
        w--;
    }
	else if (doorSide == 1)
	{
        w--;
    }
	else if (doorSide == 2)
	{
        z0++;
        d--;
    }
	else if (doorSide == 3)
	{
        d--;
    }

    int xx0 = x0;
    int xx1 = x0 + w - 1;
    int zz0 = z0;
    int zz1 = z0 + d - 1;
    if (doorSide >= 2)
	{
		xx0++;
		xx1--;
    }
	else
	{
		zz0++;
		zz1--;
    }
    for (int xx = x0; xx < x0 + w; xx++)
	{
        for (int zz = z0; zz < z0 + d; zz++)
		{
            int ho = h;

            int d1 = zz - z0;
            int d2 = (z0 + d - 1) - zz;
            if (doorSide < 2)
			{
                d1 = xx - x0;
                d2 = (x0 + w - 1) - xx;
            }

            if (d2 < d1) d1 = d2;
            h += d1;
            for (int yy = y0 - 1; yy < y0 + h; yy++)
			{
                int material = -1;
                if (yy == y0 + h - 1)
				{
                    material = Tile::wood_Id;
                }
				else if (xx >= xx0 && xx <= xx1 && zz >= zz0 && zz <= zz1)
				{
                    material = 0;
                    if (yy == y0 - 1 || yy == y0 + h - 1 || xx == xx0 || zz == zz0 || xx == xx1 || zz == zz1)
					{
                        if (yy <= y0 + random->nextInt(3)) material = Tile::mossStone_Id;
                        else material = Tile::stoneBrick_Id;
                    }
                }

                if (material >= 0)
				{
                    level->setTileNoUpdate(xx, yy, zz, material);
                }
            }
            h = ho;
        }
    }
    {
        int xx = x0 + random->nextInt(w - 4) + 2;
        int zz = z0 + random->nextInt(d - 4) + 2;
        if (doorSide == 0) xx = x0;
        if (doorSide == 1) xx = x0 + w - 1;
        if (doorSide == 2) zz = z0;
        if (doorSide == 3) zz = z0 + d - 1;
        level->setTileNoUpdate(xx, y0, zz, 0);
        level->setTileNoUpdate(xx, y0 + 1, zz, 0);

        int dir = 0;
        if (doorSide == 0) dir = 0;
        if (doorSide == 2) dir = 1;
        if (doorSide == 1) dir = 2;
        if (doorSide == 3) dir = 3;

        DoorItem::place(level, xx, y0, zz, dir, Tile::door_wood);
    }

    for (int i = 0; i < (w * 2 + d * 2) * 3; i++)
	{
        int xx = x0 + random->nextInt(w - 4) + 2;
        int zz = z0 + random->nextInt(d - 4) + 2;
        int side = random->nextInt(4);

        if (side == 0) xx = xx0;
        if (side == 1) xx = xx1;
        if (side == 2) zz = zz0;
        if (side == 3) zz = zz1;

        if (level->isSolidBlockingTile(xx, y0 + 1, zz))
		{
            int count = 0;
            if (level->isSolidBlockingTile(xx - 1, y0 + 1, zz) && level->isSolidBlockingTile(xx + 1, y0 + 1, zz)) count++;
            if (level->isSolidBlockingTile(xx, y0 + 1, zz - 1) && level->isSolidBlockingTile(xx, y0 + 1, zz + 1)) count++;
            if (count == 1) {
                level->setTileNoUpdate(xx, y0 + 1, zz, Tile::glass_Id);
            }
        }
    }

    int ww = xx1 - xx0;
    int dd = zz1 - zz0;
    for (int i = 0; i < (ww * 2 + dd * 2); i++)
	{
        int xx = xx0 + random->nextInt(ww - 1) + 1;
        int zz = zz0 + random->nextInt(dd - 1) + 1;
        int yy = y0;

        if (level->getTile(xx, yy + 2, zz) == 0)
		{
            int count = 0;
            if (level->isSolidBlockingTile(xx - 1, yy + 2, zz)) count++;
            if (level->isSolidBlockingTile(xx + 1, yy + 2, zz)) count++;
            if (level->isSolidBlockingTile(xx, yy + 2, zz - 1)) count++;
            if (level->isSolidBlockingTile(xx, yy + 2, zz + 1)) count++;
            if (count == 1)
			{
                level->setTileNoUpdate(xx, y0 + 2, zz, Tile::torch_Id);
            }
        }
    }

    shared_ptr<PigZombie>(pz) = shared_ptr<PigZombie>(new PigZombie(level));
    pz->moveTo(x0 + w / 2.0 + 0.5, y0 + 0.5, z0 + d / 2.0 + 0.5, 0, 0);
    level->addEntity(pz);

    return true;

}