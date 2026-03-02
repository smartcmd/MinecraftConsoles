#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.dimension.h"
#include "PortalForcer.h"

PortalForcer::PortalForcer()
{
	random = new Random();
}


void PortalForcer::force(Level *level, shared_ptr<Entity> e)
{
    if (level->dimension->id == 1)
	{
        int x = Mth::floor(e->x);
        int y = Mth::floor(e->y) - 1;
        int z = Mth::floor(e->z);

        int xa = 1;
        int za = 0;
        for (int b = -2; b <= 2; b++)
		{
            for (int s = -2; s <= 2; s++)
			{
                for (int h = -1; h < 3; h++)
				{
                    int xt = x + s * xa + b * za;
                    int yt = y + h;
                    int zt = z + s * za - b * xa;

                    bool border = h < 0;

                    level->setTile(xt, yt, zt, border ? Tile::obsidian_Id : 0);
                }
            }
        }

        e->moveTo(x, y, z, e->yRot, 0);
        e->xd = e->yd = e->zd = 0;

        return;
    }

	if (findPortal(level, e))
	{
		return;
	}

	createPortal(level, e);
	findPortal(level, e);
}


bool PortalForcer::findPortal(Level *level, shared_ptr<Entity> e)
{
	// 4J Stu - Decrease the range at which we search for a portal in the nether given our smaller nether
	int r = 16;//* 8;
	if(level->dimension->id == -1)
	{
		r *= 3;
	}
	else
	{
#ifdef __PSVITA__
		// AP poor little Vita takes 30 seconds to leave the Nether. This should help
		r *= 5;
#else
		r *= 8;
#endif
	}
	double closest = -1;
	int xTarget = 0;
	int yTarget = 0;
	int zTarget = 0;

	int xc = Mth::floor(e->x);
	int zc = Mth::floor(e->z);

	for (int x = xc - r; x <= xc + r; x++)
	{
		double xd = (x + 0.5) - e->x;
		for (int z = zc - r; z <= zc + r; z++)
		{
			double zd = (z + 0.5) - e->z;
			for (int y = level->getHeight() - 1; y >= 0; y--)
			{
				if (level->getTile(x, y, z) == Tile::portalTile_Id)
				{
					while (level->getTile(x, y - 1, z) == Tile::portalTile_Id)
					{
						y--;
					}

					double yd = (y + 0.5) - e->y;
					double dist = xd * xd + yd * yd + zd * zd;
					if (closest < 0 || dist < closest)
					{
						closest = dist;
						xTarget = x;
						yTarget = y;
						zTarget = z;
					}
				}
			}
		}
	}

	if (closest >= 0)
	{
		int x = xTarget;
		int y = yTarget;
		int z = zTarget;

		double xt = x + 0.5;
		double yt = y + 0.5;
		double zt = z + 0.5;

		if (level->getTile(x - 1, y, z) == Tile::portalTile_Id) xt -= 0.5;
		if (level->getTile(x + 1, y, z) == Tile::portalTile_Id) xt += 0.5;

		if (level->getTile(x, y, z - 1) == Tile::portalTile_Id) zt -= 0.5;
		if (level->getTile(x, y, z + 1) == Tile::portalTile_Id) zt += 0.5;

		e->moveTo(xt, yt, zt, e->yRot, 0);
		e->xd = e->yd = e->zd = 0;
		return true;
	}

	return false;
}


bool PortalForcer::createPortal(Level *level, shared_ptr<Entity> e)
{
	// 4J Stu - Increase the range at which we try and create a portal to stop creating them floating in mid air over lava
	int r = 16 * 3;
	double closest = -1;

	int xc = Mth::floor(e->x);
	int yc = Mth::floor(e->y);
	int zc = Mth::floor(e->z);

	// 4J Stu - Changes to stop Portals being created at the border of the nether inside the bedrock
	int XZSIZE = level->dimension->getXZSize() * 16; // XZSize is chunks, convert to blocks
	int XZOFFSET = (XZSIZE / 2) - 4; // Subtract 4 to stay away from the edges // TODO Make the 4 a constant in HellRandomLevelSource

	// Move the positions that we want to check away from the edge of the world	
	if( (xc - r) < -XZOFFSET )
	{
		app.DebugPrintf("Adjusting portal creation x due to being too close to the edge\n");
		xc -= ( (xc - r) + XZOFFSET);
	}
	else if ( (xc + r) >= XZOFFSET )
	{
		app.DebugPrintf("Adjusting portal creation x due to being too close to the edge\n");
		xc -= ( (xc + r) - XZOFFSET);
	}
	if( (zc - r) < -XZOFFSET )
	{
		app.DebugPrintf("Adjusting portal creation z due to being too close to the edge\n");
		zc -= ( (zc - r) + XZOFFSET);
	}
	else if ( (zc + r) >= XZOFFSET )
	{
		app.DebugPrintf("Adjusting portal creation z due to being too close to the edge\n");
		zc -= ( (zc + r) - XZOFFSET);
	}

	int xTarget = xc;
	int yTarget = yc;
	int zTarget = zc;
	int dirTarget = 0;

	int dirOffs = random->nextInt(4);

	{
		for (int x = xc - r; x <= xc + r; x++)
		{
			double xd = (x + 0.5) - e->x;
			for (int z = zc - r; z <= zc + r; z++)
			{
				double zd = (z + 0.5) - e->z;

				for (int y = level->getHeight() - 1; y >= 0; y--)
				{
					if (level->isEmptyTile(x, y, z))
					{
						while (y>0 && level->isEmptyTile(x, y - 1, z))
						{
							y--;
						}

						for (int dir = dirOffs; dir < dirOffs + 4; dir++)
						{
							int xa = dir % 2;
							int za = 1 - xa;

							if (dir % 4 >= 2)
							{
								xa = -xa;
								za = -za;
							}


							for (int b = 0; b < 3; b++)
							{
								for (int s = 0; s < 4; s++)
								{
									for (int h = -1; h < 4; h++)
									{
										int xt = x + (s - 1) * xa + b * za;
										int yt = y + h;
										int zt = z + (s - 1) * za - b * xa;

										// 4J Stu - Changes to stop Portals being created at the border of the nether inside the bedrock		
										if( ( xt < -XZOFFSET ) || ( xt >= XZOFFSET ) || ( zt < -XZOFFSET ) || ( zt >= XZOFFSET ) )
										{
											app.DebugPrintf("Skipping possible portal location as at least one block is too close to the edge\n");
											goto next_first;
										}

										if (h < 0 && !level->getMaterial(xt, yt, zt)->isSolid()) goto next_first;
										if (h >= 0 && !level->isEmptyTile(xt, yt, zt)) goto next_first;
									}
								}
							}

							double yd = (y + 0.5) - e->y;
							double dist = xd * xd + yd * yd + zd * zd;
							if (closest < 0 || dist < closest)
							{
								closest = dist;
								xTarget = x;
								yTarget = y;
								zTarget = z;
								dirTarget = dir % 4;
							}
						}
					}
					next_first: continue;
				}
			}
		}
	}
	if (closest < 0)
	{
		for (int x = xc - r; x <= xc + r; x++)
		{
			double xd = (x + 0.5) - e->x;
			for (int z = zc - r; z <= zc + r; z++)
			{
				double zd = (z + 0.5) - e->z;

				for (int y = level->getHeight() - 1; y >= 0; y--)
				{
					if (level->isEmptyTile(x, y, z))
					{
						while (y > 0 && level->isEmptyTile(x, y - 1, z))
						{
							y--;
						}

						for (int dir = dirOffs; dir < dirOffs + 2; dir++)
						{
							int xa = dir % 2;
							int za = 1 - xa;
							for (int s = 0; s < 4; s++)
							{
								for (int h = -1; h < 4; h++)
								{
									int xt = x + (s - 1) * xa;
									int yt = y + h;
									int zt = z + (s - 1) * za;

									// 4J Stu - Changes to stop Portals being created at the border of the nether inside the bedrock		
									if( ( xt < -XZOFFSET ) || ( xt >= XZOFFSET ) || ( zt < -XZOFFSET ) || ( zt >= XZOFFSET ) )
									{
										app.DebugPrintf("Skipping possible portal location as at least one block is too close to the edge\n");
										goto next_second;
									}

									if (h < 0 && !level->getMaterial(xt, yt, zt)->isSolid()) goto next_second;
									if (h >= 0 && !level->isEmptyTile(xt, yt, zt)) goto next_second;
								}
							}

							double yd = (y + 0.5) - e->y;
							double dist = xd * xd + yd * yd + zd * zd;
							if (closest < 0 || dist < closest)
							{
								closest = dist;
								xTarget = x;
								yTarget = y;
								zTarget = z;
								dirTarget = dir % 2;
							}
						}
					}
					next_second: continue;
				}
			}
		}
	}



	int dir = dirTarget;

	int x = xTarget;
	int y = yTarget;
	int z = zTarget;

	int xa = dir % 2;
	int za = 1 - xa;

	if (dir % 4 >= 2)
	{
		xa = -xa;
		za = -za;
	}


	if (closest < 0)
	{
		if (yTarget < 70) yTarget = 70;
		if (yTarget > level->getHeight() - 10) yTarget = level->getHeight() - 10;
		y = yTarget;

		for (int b = -1; b <= 1; b++)
		{
			for (int s = 1; s < 3; s++)
			{
				for (int h = -1; h < 3; h++)
				{
					int xt = x + (s - 1) * xa + b * za;
					int yt = y + h;
					int zt = z + (s - 1) * za - b * xa;

					bool border = h < 0;

					level->setTile(xt, yt, zt, border ? Tile::obsidian_Id : 0);
				}
			}
		}
	}

	for (int pass = 0; pass < 4; pass++)
	{
		level->noNeighborUpdate = true;
		for (int s = 0; s < 4; s++)
		{
			for (int h = -1; h < 4; h++)
			{
				int xt = x + (s - 1) * xa;
				int yt = y + h;
				int zt = z + (s - 1) * za;

				bool border = s == 0 || s == 3 || h == -1 || h == 3;
				level->setTile(xt, yt, zt, border ? Tile::obsidian_Id : Tile::portalTile_Id);
			}
		}
		level->noNeighborUpdate = false;

		for (int s = 0; s < 4; s++)
		{
			for (int h = -1; h < 4; h++)
			{
				int xt = x + (s - 1) * xa;
				int yt = y + h;
				int zt = z + (s - 1) * za;

				level->updateNeighborsAt(xt, yt, zt, level->getTile(xt, yt, zt));
			}
		}
	}

	return true;
}
