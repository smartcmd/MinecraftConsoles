#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "CanyonFeature.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.biome.h"

void CanyonFeature::addTunnel(int64_t seed, int xOffs, int zOffs, byteArray blocks, double xCave, double yCave, double zCave, float thickness, float yRot, float xRot, int step, int dist, double yScale)
{
	MemSect(49);
	Random *random = new Random(seed);
	MemSect(0);
	double xMid = xOffs * 16 + 8;
	double zMid = zOffs * 16 + 8;

	float yRota = 0;
	float xRota = 0;
	//        int dist = CAVE_RADIUS * 16 - 16;
	//        if (step>0) dist = step*2;

	if (dist <= 0)
	{
		int max = radius * 16 - 16;
		dist = max - random->nextInt(max / 4);
	}
	bool singleStep = false;

	if (step == -1)
	{
		step = dist / 2;
		singleStep = true;
	}

	float f = 1;
	for (int i = 0; i < Level::genDepth; i++)
	{
		if (i == 0 || random->nextInt(3) == 0)
		{
			f = 1 + (random->nextFloat() * random->nextFloat()) * 1.0f;
		}
		rs[i] = f * f;
	}

	for (; step < dist; step++)
	{
		double rad = 1.5 + (Mth::sin(step * PI / dist) * thickness) * 1;
		double yRad = rad * yScale;

		rad *= (random->nextFloat() * 0.25 + 0.75);
		yRad *= (random->nextFloat() * 0.25 + 0.75);

		float xc = Mth::cos(xRot);
		float xs = Mth::sin(xRot);
		xCave += Mth::cos(yRot) * xc;
		yCave += xs;
		zCave += Mth::sin(yRot) * xc;

		xRot *= 0.7f;

		xRot += xRota * 0.05f;
		yRot += yRota * 0.05f;

		xRota *= 0.80f;
		yRota *= 0.50f;
		xRota += (random->nextFloat() - random->nextFloat()) * random->nextFloat() * 2;
		yRota += (random->nextFloat() - random->nextFloat()) * random->nextFloat() * 4;

		if (!singleStep && random->nextInt(4) == 0) continue;

		{
			double xd = xCave - xMid;
			double zd = zCave - zMid;
			double remaining = dist - step;
			double rr = (thickness + 2) + 16;
			if (xd * xd + zd * zd - (remaining * remaining) > rr * rr)
			{
				delete random;
				return;
			}
		}

		if (xCave < xMid - 16 - rad * 2 || zCave < zMid - 16 - rad * 2 || xCave > xMid + 16 + rad * 2 || zCave > zMid + 16 + rad * 2) continue;

		int x0 = Mth::floor(xCave - rad) - xOffs * 16 - 1;
		int x1 = Mth::floor(xCave + rad) - xOffs * 16 + 1;

		int y0 = Mth::floor(yCave - yRad) - 1;
		int y1 = Mth::floor(yCave + yRad) + 1;

		int z0 = Mth::floor(zCave - rad) - zOffs * 16 - 1;
		int z1 = Mth::floor(zCave + rad) - zOffs * 16 + 1;

		if (x0 < 0) x0 = 0;
		if (x1 > 16) x1 = 16;

		if (y0 < 1) y0 = 1;
		if (y1 > Level::genDepth - 8) y1 = Level::genDepth - 8;

		if (z0 < 0) z0 = 0;
		if (z1 > 16) z1 = 16;

		bool detectedWater = false;
		for (int xx = x0; !detectedWater && xx < x1; xx++)
		{
			for (int zz = z0; !detectedWater && zz < z1; zz++)
			{
				for (int yy = y1 + 1; !detectedWater && yy >= y0 - 1; yy--)
				{
					int p = (xx * 16 + zz) * Level::genDepth + yy;
					if (yy < 0 || yy >= Level::genDepth) continue;
					if (blocks[p] == Tile::water_Id || blocks[p] == Tile::calmWater_Id)
					{
						detectedWater = true;
					}
					if (yy != y0 - 1 && xx != x0 && xx != x1 - 1 && zz != z0 && zz != z1 - 1)
					{
						yy = y0;
					}
				}
			}
		}
		if (detectedWater) continue;

		for (int xx = x0; xx < x1; xx++)
		{
			double xd = ((xx + xOffs * 16 + 0.5) - xCave) / rad;
			for (int zz = z0; zz < z1; zz++)
			{
				double zd = ((zz + zOffs * 16 + 0.5) - zCave) / rad;
				int p = (xx * 16 + zz) * Level::genDepth + y1;
				bool hasGrass = false;
				if (xd * xd + zd * zd < 1)
				{
					for (int yy = y1 - 1; yy >= y0; yy--)
					{
						double yd = (yy + 0.5 - yCave) / yRad;
						if ((xd * xd + zd * zd) * rs[yy] + (yd * yd / 6) < 1)
						{
							int block = blocks[p];
							if (block == Tile::grass_Id) hasGrass = true;
							if (block == Tile::stone_Id || block == Tile::dirt_Id || block == Tile::grass_Id)
							{
								if (yy < 10)
								{
									blocks[p] = (byte) Tile::lava_Id;
								}
								else
								{
									blocks[p] = (byte) 0;
									if (hasGrass && blocks[p - 1] == Tile::dirt_Id) blocks[p - 1] = (byte) level->getBiome(xx + xOffs * 16, zz + zOffs * 16)->topMaterial;
								}
							}
						}
						p--;
					}
				}
			}
		}
		if (singleStep) break;
	}
	delete random;
}

void CanyonFeature::addFeature(Level *level, int x, int z, int xOffs, int zOffs, byteArray blocks)
{
	if (random->nextInt(50) != 0) return;

	double xCave = x * 16 + random->nextInt(16);
	double yCave = random->nextInt(random->nextInt(40) + 8) + 20;
	double zCave = z * 16 + random->nextInt(16);

	int tunnels = 1;

	for (int i = 0; i < tunnels; i++)
	{
		float yRot = random->nextFloat() * PI * 2;
		float xRot = ((random->nextFloat() - 0.5f) * 2) / 8;
		float thickness = (random->nextFloat() * 2 + random->nextFloat()) * 2;

		addTunnel(random->nextLong(), xOffs, zOffs, blocks, xCave, yCave, zCave, thickness, yRot, xRot, 0, 0, 3.0);

		// 4J Add to feature list
		app.AddTerrainFeaturePosition(eTerrainFeature_Ravine,(int)(xCave/16.0),(int)(yCave/16.0));
	}

}