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
    __int64 xScale = random->nextLong();
    __int64 zScale = random->nextLong();

    for (int x = xOffs - r; x <= xOffs + r; x++)
	{
        for (int z = zOffs - r; z <= zOffs + r; z++)
		{
            __int64 xx = x * xScale;
            __int64 zz = z * zScale;
            random->setSeed(xx ^ zz ^ level->getSeed());
            addFeature(level, x, z, xOffs, zOffs, blocks);
        }
    }
}