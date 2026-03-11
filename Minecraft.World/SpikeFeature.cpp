#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.boss.enderdragon.h"
#include "net.minecraft.world.level.tile.h"
#include "SpikeFeature.h"

SpikeFeature::SpikeFeature(int tile)
{
	this->tile = tile;
	//m_iIndex=0;
}

bool SpikeFeature::place(Level *level, Random *random, int x, int y, int z)
{
	if (!(level->isEmptyTile(x, y, z) && level->getTile(x, y - 1, z) == tile))
	{
		return false;
	}
	int hh = random->nextInt(32) + 6;
	int r = random->nextInt(4) + 1;

	for (int xx = x - r; xx <= x + r; xx++)
		for (int zz = z - r; zz <= z + r; zz++)
		{
			int xd = xx - x;
			int zd = zz - z;
			if (xd * xd + zd * zd <= r * r + 1)
			{
				if (level->getTile(xx, y - 1, zz) != tile) 
				{
					return false;
				}
			}
		}

		for (int yy = y; yy < y + hh; yy++)
		{
			if (yy < Level::genDepth)
			{
				for (int xx = x - r; xx <= x + r; xx++)
					for (int zz = z - r; zz <= z + r; zz++)
					{
						int xd = xx - x;
						int zd = zz - z;
						if (xd * xd + zd * zd <= r * r + 1)
						{
							level->setTileAndData(xx, yy, zz, Tile::obsidian_Id, 0, Tile::UPDATE_CLIENTS);
						}
					}
			} else break;
		}

		shared_ptr<EnderCrystal> enderCrystal = std::make_shared<EnderCrystal>(level);
		enderCrystal->moveTo(x + 0.5f, y + hh, z + 0.5f, random->nextFloat() * 360, 0);
		level->addEntity(enderCrystal);
		level->setTileAndData(x, y + hh, z, Tile::unbreakable_Id, 0, Tile::UPDATE_CLIENTS);

		return true;
}

bool SpikeFeature::placeWithIndex(Level *level, Random *random, int x, int y, int z,int iIndex, int iRadius)
{
	app.DebugPrintf("Spike - %d,%d,%d - index %d\n",x,y,z,iIndex);

	int hh = 12 + (iIndex*3);

	// fill any tiles below the spike

	for (int xx = x - iRadius; xx <= x + iRadius; xx++)
	{
		for (int zz = z - iRadius; zz <= z + iRadius; zz++)
		{
			int xd = xx - x;
			int zd = zz - z;
			if (xd * xd + zd * zd <= iRadius * iRadius + 1)
			{
				int iTileBelow=1;

				while((y-iTileBelow>-10) && level->getTile(xx, y - iTileBelow, zz) != tile)
				{
					if(level->isEmptyTile(xx, y - iTileBelow, zz))
					{
						// empty tile
						level->setTileAndData(xx, y - iTileBelow, zz, Tile::obsidian_Id, 0, Tile::UPDATE_CLIENTS);
					}
					else
					{
						level->setTileAndData(xx, y - iTileBelow, zz, Tile::obsidian_Id, 0, Tile::UPDATE_CLIENTS);
					}
					iTileBelow++;
				}
			}
		}
	}

	for (int yy = y; yy < y + hh; yy++)
	{
		if (yy < Level::genDepth)
		{
			for (int xx = x - iRadius; xx <= x + iRadius; xx++)
			{
				for (int zz = z - iRadius; zz <= z + iRadius; zz++)
				{
					int xd = xx - x;
					int zd = zz - z;
					int iVal = xd * xd + zd * zd;
					if ( iVal <= iRadius * iRadius + 1)
					{
						//level->setTile(xx, yy, zz, Tile::obsidian_Id);
						placeBlock(level, xx, yy, zz, Tile::obsidian_Id, 0);
					}
				}
			}
		} 
		else 
		{
			app.DebugPrintf("Breaking out of spike feature\n");
			break;
		}
	}

	// cap the last spikes with a fence to stop lucky arrows hitting the crystal

	if(iIndex>5)
	{
		for (int yy = y; yy < y + hh; yy++)
		{
			if (yy < Level::genDepth)
			{
				for (int xx = x - 2; xx <= x + 2; xx++)
				{
					for (int zz = z - 2; zz <= z + 2; zz++)
					{
						int xd = xx - x;
						int zd = zz - z;
						int iVal = xd * xd + zd * zd;
						if ( iVal >= 2 * 2)
						{
							if(yy==(y + hh - 1))
							{
								placeBlock(level, xx, y + hh, zz, Tile::ironFence_Id, 0);
								placeBlock(level, xx, y + hh +1, zz, Tile::ironFence_Id, 0);
								placeBlock(level, xx, y + hh +2, zz, Tile::ironFence_Id, 0);
							}
						}
					}
				}
			} 
			else 
			{
				app.DebugPrintf("Breaking out of spike feature\n");
				break;
			}
		}

		// and cap off the top
		int yy =  y + hh + 3;

		if (yy < Level::genDepth)
		{
			for (int xx = x - 2; xx <= x + 2; xx++)
			{
				for (int zz = z - 2; zz <= z + 2; zz++)
				{
					placeBlock(level, xx, yy, zz, Tile::ironFence_Id, 0);
				}
			}
		}
	}

	shared_ptr<EnderCrystal> enderCrystal = std::make_shared<EnderCrystal>(level);
	enderCrystal->moveTo(x + 0.5f, y + hh, z + 0.5f, random->nextFloat() * 360, 0);
	level->addEntity(enderCrystal);
	placeBlock(level, x, y + hh, z, Tile::unbreakable_Id, 0);
	//level->setTile(x, y + hh, z, Tile::unbreakable_Id);

	return true;
}

