#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.levelgen.h"
#include "net.minecraft.world.level.levelgen.feature.h"
#include "net.minecraft.world.level.levelgen.structure.h"
#include "net.minecraft.world.level.levelgen.synth.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.storage.h"
#include "FlatLevelSource.h"


//FlatLevelSource::villageFeature = new VillageFeature(1);

FlatLevelSource::FlatLevelSource(Level *level, __int64 seed, bool generateStructures) 
{
	m_XZSize = level->getLevelData()->getXZSize();

	this->level = level;
	this->generateStructures = generateStructures;
	this->random = new Random(seed);
	this->pprandom = new Random(seed);		// 4J - added, so that we can have a separate random for doing post-processing in parallel with creation

	villageFeature = new VillageFeature(0,m_XZSize);


}

FlatLevelSource::~FlatLevelSource()
{
	delete random;
	delete pprandom;
	delete villageFeature;
}

void FlatLevelSource::prepareHeights(byteArray blocks) 
{
	int height = blocks.length / (16 * 16);

	for (int xc = 0; xc < 16; xc++) 
	{
		for (int zc = 0; zc < 16; zc++) 
		{
			for (int yc = 0; yc < height; yc++) 
			{
				int block = 0;
				if (yc == 0) 
				{
					block = Tile::unbreakable_Id;
				} 
				else if (yc <= 2) 
				{
					block = Tile::dirt_Id;
				} 
				else if (yc == 3) 
				{
					block = Tile::grass_Id;
				}
				blocks[xc << 11 | zc << 7 | yc] = (byte) block;
			}
		}
	}
}

LevelChunk *FlatLevelSource::create(int x, int z) 
{
	return getChunk(x, z);
}

LevelChunk *FlatLevelSource::getChunk(int xOffs, int zOffs) 
{
	// 4J - now allocating this with a physical alloc & bypassing general memory management so that it will get cleanly freed
	int chunksSize = Level::genDepth * 16  * 16;
	byte *tileData = (byte *)XPhysicalAlloc(chunksSize, MAXULONG_PTR, 4096, PAGE_READWRITE);
	XMemSet128(tileData,0,chunksSize);
	byteArray blocks = byteArray(tileData,chunksSize);
//	byteArray blocks = byteArray(16 * level->depth * 16);
	prepareHeights(blocks);

//	LevelChunk *levelChunk = new LevelChunk(level, blocks, xOffs, zOffs);		// 4J - moved below
	//        double[] temperatures = level.getBiomeSource().temperatures;


	if (generateStructures) 
	{
		villageFeature->apply(this, level, xOffs, zOffs, blocks);
	}

	// 4J - this now creates compressed block data from the blocks array passed in, so moved it until after the blocks are actually finalised. We also
	// now need to free the passed in blocks as the LevelChunk doesn't use the passed in allocation anymore.
	LevelChunk *levelChunk = new LevelChunk(level, blocks, xOffs, zOffs);
	XPhysicalFree(tileData);

	levelChunk->recalcHeightmap();

	return levelChunk;
}


bool FlatLevelSource::hasChunk(int x, int y) 
{
	return true;
}

void FlatLevelSource::postProcess(ChunkSource *parent, int xt, int zt) 
{
	// 4J - changed from random to pprandom so we can run in parallel with getChunk etc.
	pprandom->setSeed(level->getSeed());
	__int64 xScale = pprandom->nextLong() / 2 * 2 + 1;
	__int64 zScale = pprandom->nextLong() / 2 * 2 + 1;
	pprandom->setSeed(((xt * xScale) + (zt * zScale)) ^ level->getSeed());

	if (generateStructures) 
	{
		villageFeature->postProcess(level, pprandom, xt, zt);
	}
	
	app.processSchematics(parent->getChunk(xt,zt));
}

bool FlatLevelSource::save(bool force, ProgressListener *progressListener) 
{
	return true;
}

bool FlatLevelSource::tick() 
{
	return false;
}

bool FlatLevelSource::shouldSave() 
{
	return true;
}

wstring FlatLevelSource::gatherStats() 
{
	return L"FlatLevelSource";
}

vector<Biome::MobSpawnerData *> *FlatLevelSource::getMobsAt(MobCategory *mobCategory, int x, int y, int z)
{
 	Biome *biome = level->getBiome(x, z);
 	if (biome == NULL) 
 	{
 		return NULL;
 	}
 	return biome->getMobs(mobCategory);
}

TilePos *FlatLevelSource::findNearestMapFeature(Level *level, const wstring& featureName, int x, int y, int z)
{
	return NULL;
}
