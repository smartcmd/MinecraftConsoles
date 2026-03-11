#include "stdafx.h"
#include "CaveFeature.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
using namespace std;

bool CaveFeature::place(Level *level, Random *random, int x, int y, int z)
{
	float dir = random->nextFloat() * PI;
	double rd = 8;

	double x0 = x + 8 + Mth::sin(dir) * rd;
	double x1 = x + 8 - Mth::sin(dir) * rd;
	double z0 = z + 8 + Mth::cos(dir) * rd;
	double z1 = z + 8 - Mth::cos(dir) * rd;

	double y0 = y + random->nextInt(8) + 2;
	double y1 = y + random->nextInt(8) + 2;

	double radius = random->nextDouble() * 4 + 2;
	double fuss = random->nextDouble() * 0.6;

	int64_t seed = random->nextLong();
	random->setSeed(seed);
	vector<TilePos *> toRemove;

	for (int d = 0; d <= 16; d++)
	{
		double xx = x0 + (x1 - x0) * d / 16;
		double yy = y0 + (y1 - y0) * d / 16;
		double zz = z0 + (z1 - z0) * d / 16;

		double ss = random->nextDouble();
		double r = (Mth::sin(d / 16.0f * PI) * radius + 1) * ss + 1;
		double hr = (Mth::sin(d / 16.0f * PI) * radius + 1) * ss + 1;

		// 4J Stu Added to stop cave features generating areas previously place by game rule generation
		if(app.getLevelGenerationOptions() != nullptr)
		{
			LevelGenerationOptions *levelGenOptions = app.getLevelGenerationOptions();
			bool intersects = levelGenOptions->checkIntersects((xx - r / 2), (yy - hr / 2), (zz - r / 2), (xx + r / 2), (yy + hr / 2), (zz + r / 2));
			if(intersects)
			{
				//app.DebugPrintf("Skipping cave feature generation as it overlaps a game rule structure\n");
				return false;
			}
		}

		for (int x2 = static_cast<int>(xx - r / 2); x2 <= static_cast<int>(xx + r / 2); x2++)
			for (int y2 = static_cast<int>(yy - hr / 2); y2 <= static_cast<int>(yy + hr / 2); y2++)
				for (int z2 = static_cast<int>(zz - r / 2); z2 <= static_cast<int>(zz + r / 2); z2++)
				{
					double xd = ((x2 + 0.5) - xx) / (r / 2);
					double yd = ((y2 + 0.5) - yy) / (hr / 2);
					double zd = ((z2 + 0.5) - zz) / (r / 2);
					if (xd * xd + yd * yd + zd * zd < random->nextDouble() * fuss + (1 - fuss))
					{
						if (!level->isEmptyTile(x2, y2, z2))
						{
							for (int x3 = (x2 - 2); x3 <= (x2 + 1); x3++)
								for (int y3 = (y2 - 1); y3 <= (y2 + 1); y3++)
									for (int z3 = (z2 - 1); z3 <= (z2 + 1); z3++)
									{
										if (x3 <= x || z3 <= z || x3 >= x + 16 - 1 || z3 >= z + 16 - 1) return false;
										if (level->getMaterial(x3, y3, z3)->isLiquid()) return false;
									}
							toRemove.push_back(new TilePos(x2, y2, z2));
						}
					}
				}
	}

	for ( auto& p : toRemove )
	{
		level->setTileAndData(p->x, p->y, p->z, 0, 0, Tile::UPDATE_CLIENTS);
	}

	for ( auto& p : toRemove )
	{
		if (level->getTile(p->x, p->y - 1, p->z) == Tile::dirt_Id && level->getDaytimeRawBrightness(p->x, p->y, p->z) > 8)
		{
			level->setTileAndData(p->x, p->y - 1, p->z, Tile::grass_Id, 0, Tile::UPDATE_CLIENTS);
		}
		delete p;
	}

	return true;
}