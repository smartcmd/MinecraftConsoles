#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "LargeFeature.h"

const wstring LargeFeature::STRONGHOLD = L"StrongHold";

LargeFeature::LargeFeature()
{
	radius = 8;
	random = new Random();
}

LargeFeature::~LargeFeature()
{
	delete random;
}

void LargeFeature::apply(ChunkSource *ChunkSource, Level *level, int xOffs, int zOffs, byteArray blocks)
{
    int r = radius;
	this->level = level;

    random->setSeed(level->getSeed());
    int64_t xScale = random->nextLong();
    int64_t zScale = random->nextLong();

    for (int x = xOffs - r; x <= xOffs + r; x++)
	{
        for (int z = zOffs - r; z <= zOffs + r; z++)
		{
            int64_t xx = x * xScale;
            int64_t zz = z * zScale;
            random->setSeed(xx ^ zz ^ level->getSeed());
            addFeature(level, x, z, xOffs, zOffs, blocks);
        }
    }
}