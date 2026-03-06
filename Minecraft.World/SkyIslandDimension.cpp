#include "stdafx.h"
#include "SkyIslandDimension.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.levelgen.h"

void SkyIslandDimension::init()
{
    biomeSource = new FixedBiomeSource(Biome::sky, 0.5f, 0);
    id = 1;
}

ChunkSource *SkyIslandDimension::createRandomLevelSource() const
{
	return new SkyIslandRandomLevelSource(level, level->getSeed());
}

float SkyIslandDimension::getTimeOfDay(int64_t time, float a) const
{
	return 0.0f;
}

float *SkyIslandDimension::getSunriseColor(float td, float a)
{
	return NULL;
}

Vec3 *SkyIslandDimension::getFogColor(float td, float a) const
{
    int fogColor = 0x8080a0;
    float br = Mth::cos(td * PI * 2) * 2 + 0.5f;
    if (br < 0.0f) br = 0.0f;
    if (br > 1.0f) br = 1.0f;

    float r = ((fogColor >> 16) & 0xff) / 255.0f;
    float g = ((fogColor >> 8) & 0xff) / 255.0f;
    float b = ((fogColor) & 0xff) / 255.0f;
    r *= br * 0.94f + 0.06f;
    g *= br * 0.94f + 0.06f;
    b *= br * 0.91f + 0.09f;

    return Vec3::newTemp(r, g, b);
}

bool SkyIslandDimension::hasGround()
{
	return false;
}

float SkyIslandDimension::getCloudHeight()
{
	return 8;
}

bool SkyIslandDimension::isValidSpawn(int x, int z) const
{
    int topTile = level->getTopTile(x, z);

    if (topTile == 0) return false;

    return Tile::tiles[topTile]->material->blocksMotion();
}