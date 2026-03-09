#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.levelgen.structure.h"
#include "ScatteredFeaturePieces.h"
#include "RandomScatteredLargeFeature.h"

const wstring RandomScatteredLargeFeature::OPTION_SPACING = L"distance";
vector<Biome *> RandomScatteredLargeFeature::allowedBiomes;

void RandomScatteredLargeFeature::staticCtor()
{
	allowedBiomes.push_back( Biome::desert );
	allowedBiomes.push_back( Biome::desertHills );
	allowedBiomes.push_back( Biome::jungle );
	allowedBiomes.push_back( Biome::jungleHills );
	allowedBiomes.push_back( Biome::swampland );
}

void RandomScatteredLargeFeature::_init()
{
	spacing = 32;
	minSeparation = 8;

	swamphutEnemies.push_back(new Biome::MobSpawnerData(eTYPE_WITCH, 1, 1, 1));
}

RandomScatteredLargeFeature::RandomScatteredLargeFeature()
{
	_init();
}

RandomScatteredLargeFeature::RandomScatteredLargeFeature(unordered_map<wstring, wstring> options)
{
	_init();

	for(auto& option : options)
	{
		if (option.first.compare(OPTION_SPACING) == 0)
		{
			spacing = Mth::getInt(option.second, spacing, minSeparation + 1);
		}
	}
}

wstring RandomScatteredLargeFeature::getFeatureName()
{
	return L"Temple";
}

bool RandomScatteredLargeFeature::isFeatureChunk(int x, int z, bool bIsSuperflat)
{
	int xx = x;
	int zz = z;
	if (x < 0) x -= spacing - 1;
	if (z < 0) z -= spacing - 1;

	int xCenterFeatureChunk = x / spacing;
	int zCenterFeatureChunk = z / spacing;
	Random *r = level->getRandomFor(xCenterFeatureChunk, zCenterFeatureChunk, 14357617);
	xCenterFeatureChunk *= spacing;
	zCenterFeatureChunk *= spacing;
	xCenterFeatureChunk += r->nextInt(spacing - minSeparation);
	zCenterFeatureChunk += r->nextInt(spacing - minSeparation);
	x = xx;
	z = zz;

	bool forcePlacement = false;
	LevelGenerationOptions *levelGenOptions = app.getLevelGenerationOptions();
	if( levelGenOptions != nullptr )
	{
		forcePlacement = levelGenOptions->isFeatureChunk(x,z,eFeature_Temples);
	}

	if (forcePlacement || (x == xCenterFeatureChunk && z == zCenterFeatureChunk))
	{
		Biome *biome = level->getBiomeSource()->getBiome(x * 16 + 8, z * 16 + 8);
		for ( const auto& a : allowedBiomes)
		{
			if (biome == a)
			{
				return true;
			}
		}
	}

	return false;

}

StructureStart *RandomScatteredLargeFeature::createStructureStart(int x, int z)
{
	return new ScatteredFeatureStart(level, random, x, z);
}

RandomScatteredLargeFeature::ScatteredFeatureStart::ScatteredFeatureStart()
{
	// for reflection
}

RandomScatteredLargeFeature::ScatteredFeatureStart::ScatteredFeatureStart(Level *level, Random *random, int chunkX, int chunkZ) : StructureStart(chunkX, chunkZ)
{
	Biome *biome = level->getBiome(chunkX * 16 + 8, chunkZ * 16 + 8);
	if (biome == Biome::jungle || biome == Biome::jungleHills)
	{
		ScatteredFeaturePieces::JunglePyramidPiece *startRoom = new ScatteredFeaturePieces::JunglePyramidPiece(random, chunkX * 16, chunkZ * 16);
		pieces.push_back(startRoom);
	}
	else if (biome == Biome::swampland)
	{
		ScatteredFeaturePieces::SwamplandHut *startRoom = new ScatteredFeaturePieces::SwamplandHut(random, chunkX * 16, chunkZ * 16);
		pieces.push_back(startRoom);
	}
	else
	{
		ScatteredFeaturePieces::DesertPyramidPiece *startRoom = new ScatteredFeaturePieces::DesertPyramidPiece(random, chunkX * 16, chunkZ * 16);
		pieces.push_back(startRoom);
	}

	calculateBoundingBox();
}

bool RandomScatteredLargeFeature::isSwamphut(int cellX, int cellY, int cellZ)
{
	StructureStart *structureAt = getStructureAt(cellX, cellY, cellZ);
	if (structureAt == nullptr || !( dynamic_cast<ScatteredFeatureStart *>( structureAt ) ) || structureAt->pieces.empty())
	{
		return false;
	}
	StructurePiece *first = nullptr;
    auto it = structureAt->pieces.begin();
    if(it != structureAt->pieces.end() ) first = *it;
	return dynamic_cast<ScatteredFeaturePieces::SwamplandHut *>(first) != nullptr;
}

vector<Biome::MobSpawnerData *> *RandomScatteredLargeFeature::getSwamphutEnemies()
{
	return &swamphutEnemies;
}