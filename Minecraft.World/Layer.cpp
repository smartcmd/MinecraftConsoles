#include "stdafx.h"
#include "net.minecraft.world.level.newbiome.layer.h"
#include "net.minecraft.world.level.h"
#include "BiomeOverrideLayer.h"

#ifdef __PSVITA__
// AP - this is used to perform fast 64bit divides of known values
#include "..\Minecraft.Client\PSVita\PSVitaExtras\libdivide.h"

libdivide::divider<long long> fast_d2(2);
libdivide::divider<long long> fast_d3(3);
libdivide::divider<long long> fast_d4(4);
libdivide::divider<long long> fast_d5(5);
libdivide::divider<long long> fast_d6(6);
libdivide::divider<long long> fast_d7(7);
libdivide::divider<long long> fast_d10(10);
#endif

LayerArray Layer::getDefaultLayers(int64_t seed, LevelType *levelType)
{
	// 4J - Some changes moved here from 1.2.3. Temperature & downfall layers are no longer created & returned, and a debug layer is isn't.
	// For reference with regard to future merging, things NOT brought forward from the 1.2.3 version are new layer types that we
	// don't have yet (shores, swamprivers, region hills etc.)
	shared_ptr<Layer>islandLayer = std::make_shared<IslandLayer>(1);
	islandLayer = std::make_shared<FuzzyZoomLayer>(2000, islandLayer);
	islandLayer = std::make_shared<AddIslandLayer>(1, islandLayer);
	islandLayer = std::make_shared<ZoomLayer>(2001, islandLayer);
	islandLayer = std::make_shared<AddIslandLayer>(2, islandLayer);
	islandLayer = std::make_shared<AddSnowLayer>(2, islandLayer);
	islandLayer = std::make_shared<ZoomLayer>(2002, islandLayer);
	islandLayer = std::make_shared<AddIslandLayer>(3, islandLayer);
	islandLayer = std::make_shared<ZoomLayer>(2003, islandLayer);
	islandLayer = std::make_shared<AddIslandLayer>(4, islandLayer);
//	islandLayer = shared_ptr<Layer>(new AddMushroomIslandLayer(5, islandLayer));		// 4J - old position of mushroom island layer

	int zoomLevel = 4;
	if (levelType == LevelType::lvl_largeBiomes)
	{
		zoomLevel = 6;
	}

	shared_ptr<Layer> riverLayer = islandLayer;
	riverLayer = ZoomLayer::zoom(1000, riverLayer, 0);
	riverLayer = std::make_shared<RiverInitLayer>(100, riverLayer);
	riverLayer = ZoomLayer::zoom(1000, riverLayer, zoomLevel + 2);
	riverLayer = std::make_shared<RiverLayer>(1, riverLayer);
	riverLayer = std::make_shared<SmoothLayer>(1000, riverLayer);

	shared_ptr<Layer> biomeLayer = islandLayer;
	biomeLayer = ZoomLayer::zoom(1000, biomeLayer, 0);
	biomeLayer = std::make_shared<BiomeInitLayer>(200, biomeLayer, levelType);

	biomeLayer = ZoomLayer::zoom(1000, biomeLayer, 2);
	biomeLayer = std::make_shared<RegionHillsLayer>(1000, biomeLayer);

	for (int i = 0; i < zoomLevel; i++)
	{
		biomeLayer = std::make_shared<ZoomLayer>(1000 + i, biomeLayer);

	if (i == 0) biomeLayer = std::make_shared<AddIslandLayer>(3, biomeLayer);

		if (i == 0)
		{
			// 4J - moved mushroom islands to here. This skips 3 zooms that the old location of the add was, making them about 1/8 of the original size. Adding
			// them at this scale actually lets us place them near enough other land, if we add them at the same scale as java then they have to be too far out to see for
			// the scale of our maps
			biomeLayer = std::make_shared<AddMushroomIslandLayer>(5, biomeLayer);
		}

		if (i == 1 )
		{
			// 4J - now expand mushroom islands up again. This does a simple region grow to add a new mushroom island element when any of the neighbours are also mushroom islands.
			// This helps make the islands into nice compact shapes of the type that are actually likely to be able to make an island out of the sea in a small space. Also
			// helps the shore layer from doing too much damage in shrinking the islands we are making
			biomeLayer = std::make_shared<GrowMushroomIslandLayer>(5, biomeLayer);
			// Note - this reduces the size of mushroom islands by turning their edges into shores. We are doing this at i == 1 rather than i == 0 as the original does
			biomeLayer = std::make_shared<ShoreLayer>(1000, biomeLayer);

			biomeLayer = std::make_shared<SwampRiversLayer>(1000, biomeLayer);
		}
	}

	biomeLayer = std::make_shared<SmoothLayer>(1000, biomeLayer);

	biomeLayer = std::make_shared<RiverMixerLayer>(100, biomeLayer, riverLayer);

#ifndef _CONTENT_PACKAGE
#ifdef _BIOME_OVERRIDE
	if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_EnableBiomeOverride))
	{
		biomeLayer = std::make_shared<BiomeOverrideLayer>(1);
	}
#endif
#endif

	shared_ptr<Layer> debugLayer = biomeLayer;

	shared_ptr<Layer>zoomedLayer = std::make_shared<VoronoiZoom>(10, biomeLayer);

	biomeLayer->init(seed);
	zoomedLayer->init(seed);

	LayerArray result(3);
	result[0] = biomeLayer;
	result[1] = zoomedLayer;
	result[2] = debugLayer;
	return result;
}

Layer::Layer(int64_t seedMixup)
{
	parent = nullptr;

	this->seedMixup = seedMixup;
	this->seedMixup *= this->seedMixup * 6364136223846793005l + 1442695040888963407l;
	this->seedMixup += seedMixup;
	this->seedMixup *= this->seedMixup * 6364136223846793005l + 1442695040888963407l;
	this->seedMixup += seedMixup;
	this->seedMixup *= this->seedMixup * 6364136223846793005l + 1442695040888963407l;
	this->seedMixup += seedMixup;
}

void Layer::init(int64_t seed)
{
	this->seed = seed;
	if (parent != nullptr) parent->init(seed);
	this->seed *= this->seed * 6364136223846793005l + 1442695040888963407l;
	this->seed += seedMixup;
	this->seed *= this->seed * 6364136223846793005l + 1442695040888963407l;
	this->seed += seedMixup;
	this->seed *= this->seed * 6364136223846793005l + 1442695040888963407l;
	this->seed += seedMixup;
}

void Layer::initRandom(int64_t x, int64_t y)
{
	rval = seed;
	rval *= rval * 6364136223846793005l + 1442695040888963407l;
	rval += x;
	rval *= rval * 6364136223846793005l + 1442695040888963407l;
	rval += y;
	rval *= rval * 6364136223846793005l + 1442695040888963407l;
	rval += x;
	rval *= rval * 6364136223846793005l + 1442695040888963407l;
	rval += y;
}

int Layer::nextRandom(int max)
{
#ifdef __PSVITA__
	// AP - 64bit mods are very slow on Vita. Replaced with a divide/mult for general case and a fast divide library for specific numbers
	// todo - this can sometimes yield a different number to the original. There's a strange bug sometimes with Vita where if the line
	// "result = (int) ((rval >> 24) % max);" is done twice in a row 'result' will not be the same. Need to speak to Sony about that
	// Also need to compare level against a different platform using the same seed
	int result;
	long long temp = rval;
	temp >>= 24;
	if( max == 2 )
	{
		result = temp-(temp/fast_d2)*2;
	}
	else if( max == 3 )
	{
		result = temp-(temp/fast_d3)*3;
	}
	else if( max == 4 )
	{
		result = temp-(temp/fast_d4)*4;
	}
	else if( max == 5 )
	{
		result = temp-(temp/fast_d5)*5;
	}
	else if( max == 6 )
	{
		result = temp-(temp/fast_d6)*6;
	}
	else if( max == 7 )
	{
		result = temp-(temp/fast_d7)*7;
	}
	else if( max == 10 )
	{
		result = temp-(temp/fast_d10)*10;
	}
	else
	{
		result = temp-(temp/max)*max;
	}
#else

	int result = static_cast<int>((rval >> 24) % max);
#endif

	if (result < 0) result += max;
	rval *= rval * 6364136223846793005l + 1442695040888963407l;
	rval += seed;
	return result;
}
