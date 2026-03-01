#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.boss.enderdragon.h"
#include "net.minecraft.world.level.tile.h"
#include "EndPodiumFeature.h"

EndPodiumFeature::EndPodiumFeature(int tile)
{
	this->tile = tile;
	//m_iIndex=0;
}

bool EndPodiumFeature::place(Level *level, Random *random, int x, int y, int z)
{ 
	// spawn Exit portal
	int r = 4;

	for (int yy = y - 1; yy <= y + 32; yy++)
	{
		for (int xx = x - r; xx <= x + r; xx++)
		{
			for (int zz = z - r; zz <= z + r; zz++)
			{
				double xd = xx - x;
				double zd = zz - z;
				double d = sqrt(xd * xd + zd * zd);
				if (d <= r - 0.5)
				{
					if (yy < y)
					{
						if (d > r - 1 - 0.5)
						{
						}
						else
						{
							//level->setTile(xx, yy, zz, Tile::unbreakable_Id);
							placeBlock(level, xx, yy, zz, Tile::unbreakable_Id, 0);
						}
					}
					else if (yy > y)
					{
						//level->setTile(xx, yy, zz, 0);
						placeBlock(level, xx, yy, zz, 0, 0);
					}
					else
					{
						if (d > r - 1 - 0.5)
						{
							//level->setTile(xx, yy, zz, Tile::unbreakable_Id);
							placeBlock(level, xx, yy, zz, Tile::unbreakable_Id, 0);
						}
					}
				}
			}
		}
	}

	placeBlock(level,x, y + 0, z,	 Tile::unbreakable_Id, 0);
	placeBlock(level,x, y + 1, z,	 Tile::unbreakable_Id, 0);
	placeBlock(level,x, y + 2, z,	 Tile::unbreakable_Id, 0);
	placeBlock(level,x - 1, y + 2, z, Tile::torch_Id, 0);
	placeBlock(level,x + 1, y + 2, z, Tile::torch_Id, 0);
	placeBlock(level,x, y + 2, z - 1, Tile::torch_Id, 0);
	placeBlock(level,x, y + 2, z + 1, Tile::torch_Id, 0);
	placeBlock(level,x, y + 3, z,	 Tile::unbreakable_Id, 0);
	//placeBlock(level,x, y + 4, z,	 Tile::dragonEgg_Id, 0);

	// 4J-PB - The podium can be floating with nothing under it, so put some whiteStone under it if this is the case
	for (int yy = y - 5; yy < y - 1; yy++)
	{
		for (int xx = x - (r - 1); xx <= x + (r - 1); xx++)
		{
			for (int zz = z - (r - 1); zz <= z + (r - 1); zz++)
			{
				if(level->isEmptyTile(xx,yy,zz))
				{
					placeBlock(level, xx, yy, zz, Tile::endStone_Id, 0);
				}
			}
		}
	}
	

    return true;
}

