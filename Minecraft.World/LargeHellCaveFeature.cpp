#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "LargeHellCaveFeature.h"
#include "net.minecraft.world.level.tile.h"

void LargeHellCaveFeature::addRoom(int64_t seed, int xOffs, int zOffs, byteArray blocks, double xRoom, double yRoom, double zRoom)
{
	addTunnel(seed, xOffs, zOffs, blocks, xRoom, yRoom, zRoom, 1 + random->nextFloat() * 6, 0, 0, -1, -1, 0.5);
}

void LargeHellCaveFeature::addTunnel(int64_t seed, int xOffs, int zOffs, byteArray blocks, double xCave, double yCave, double zCave, float thickness, float yRot, float xRot, int step, int dist, double yScale)
{
	double xMid = xOffs * 16 + 8;
	double zMid = zOffs * 16 + 8;

	float yRota = 0;
	float xRota = 0;
	Random random(seed);

	if (dist <= 0)
	{
		int max = radius * 16 - 16;
		dist = max - random.nextInt(max / 4);
	}
	bool singleStep = false;

	if (step == -1)
	{
		step = dist / 2;
		singleStep = true;
	}


	int splitPoint = random.nextInt(dist / 2) + dist / 4;
	bool steep = random.nextInt(6) == 0;

	for (; step < dist; step++)
	{
		double rad = 1.5 + (Mth::sin(step * PI / dist) * thickness) * 1;
		double yRad = rad * yScale;

		float xc = Mth::cos(xRot);
		float xs = Mth::sin(xRot);
		xCave += Mth::cos(yRot) * xc;
		yCave += xs;
		zCave += Mth::sin(yRot) * xc;

		if (steep)
		{
			xRot *= 0.92f;
		}
		else
		{
			xRot *= 0.7f;
		}
		xRot += xRota * 0.1f;
		yRot += yRota * 0.1f;

		xRota *= 0.90f;
		yRota *= 0.75f;
		xRota += (random.nextFloat() - random.nextFloat()) * random.nextFloat() * 2;
		yRota += (random.nextFloat() - random.nextFloat()) * random.nextFloat() * 4;


		if (!singleStep && step == splitPoint && thickness > 1)
		{
			addTunnel(random.nextLong(), xOffs, zOffs, blocks, xCave, yCave, zCave, random.nextFloat() * 0.5f + 0.5f, yRot - PI / 2, xRot / 3, step, dist, 1.0);
			addTunnel(random.nextLong(), xOffs, zOffs, blocks, xCave, yCave, zCave, random.nextFloat() * 0.5f + 0.5f, yRot + PI / 2, xRot / 3, step, dist, 1.0);
			return;
		}
		if (!singleStep && random.nextInt(4) == 0) continue;

		{
			double xd = xCave - xMid;
			double zd = zCave - zMid;
			double remaining = dist - step;
			double rr = (thickness + 2) + 16;
			if (xd * xd + zd * zd - (remaining * remaining) > rr * rr)
			{
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
					if (blocks[p] == Tile::lava_Id || blocks[p] == Tile::calmLava_Id)
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
				for (int yy = y1 - 1; yy >= y0; yy--)
				{
					double yd = (yy + 0.5 - yCave) / yRad;
					if (yd > -0.7 && xd * xd + yd * yd + zd * zd < 1)
					{
						int block = blocks[p];
						if (block == Tile::netherRack_Id || block == Tile::dirt_Id || block == Tile::grass_Id)
						{
							blocks[p] = static_cast<byte>(0);
						}
					}
					p--;
				}
			}
		}
		if (singleStep) break;
	}
}


void LargeHellCaveFeature::addFeature(Level *level, int x, int z, int xOffs, int zOffs, byteArray blocks)
{
	int caves = random->nextInt(random->nextInt(random->nextInt(10) + 1) + 1);
	if (random->nextInt(5) != 0) caves = 0;

	for (int cave = 0; cave < caves; cave++)
	{
		double xCave = x * 16 + random->nextInt(16);
		double yCave = random->nextInt(Level::genDepth);
		double zCave = z * 16 + random->nextInt(16);

		int tunnels = 1;
		if (random->nextInt(4) == 0)
		{
			addRoom(random->nextLong(), xOffs, zOffs, blocks, xCave, yCave, zCave);
			tunnels += random->nextInt(4);
		}

		for (int i = 0; i < tunnels; i++)
		{

			float yRot = random->nextFloat() * PI * 2;
			float xRot = ((random->nextFloat() - 0.5f) * 2) / 8;
			float thickness = random->nextFloat() * 2 + random->nextFloat();

			addTunnel(random->nextLong(), xOffs, zOffs, blocks, xCave, yCave, zCave, thickness*2, yRot, xRot, 0, 0, 0.5);
		}
	}
}


