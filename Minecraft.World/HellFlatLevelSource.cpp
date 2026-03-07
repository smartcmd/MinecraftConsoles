#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.storage.h"
#include "HellFlatLevelSource.h"

HellFlatLevelSource::HellFlatLevelSource(Level *level, int64_t seed)
{
	int xzSize = level->getLevelData()->getXZSize();
	int hellScale = level->getLevelData()->getHellScale();
	m_XZSize = ceil(static_cast<float>(xzSize) / hellScale);

	this->level = level;

	random = new Random(seed);
	pprandom = new Random(seed);
}

HellFlatLevelSource::~HellFlatLevelSource()
{
	delete  random;
	delete pprandom;
}

void HellFlatLevelSource::prepareHeights(int xOffs, int zOffs, byteArray blocks)
{
	int height = blocks.length / (16 * 16);

	for (int xc = 0; xc < 16; xc++)
	{
		for (int zc = 0; zc < 16; zc++)
		{
			for (int yc = 0; yc < height; yc++)
			{
				int block = 0;
				if ( (yc <= 6) || ( yc >= 121 ) )
				{
					block = Tile::netherRack_Id;
				}

				blocks[xc << 11 | zc << 7 | yc] = static_cast<byte>(block);
			}
		}
	}
}

void HellFlatLevelSource::buildSurfaces(int xOffs, int zOffs, byteArray blocks)
{
	for (int x = 0; x < 16; x++)
	{
		for (int z = 0; z < 16; z++)
		{
			for (int y = Level::genDepthMinusOne; y >= 0; y--)
			{
				int offs = (z * 16 + x) * Level::genDepth + y;

				// 4J Build walls around the level
				bool blockSet = false;
				if(xOffs <= -(m_XZSize/2))
				{
					if( z - random->nextInt( 4 ) <= 0 || xOffs < -(m_XZSize/2) )
					{
						blocks[offs] = static_cast<byte>(Tile::unbreakable_Id);
						blockSet = true;
					}
				}
				if(zOffs <= -(m_XZSize/2))
				{
					if( x - random->nextInt( 4 ) <= 0 || zOffs < -(m_XZSize/2))
					{
						blocks[offs] = static_cast<byte>(Tile::unbreakable_Id);
						blockSet = true;
					}
				}
				if(xOffs >= (m_XZSize/2)-1)
				{
					if( z + random->nextInt(4) >= 15 || xOffs > (m_XZSize/2))
					{
						blocks[offs] = static_cast<byte>(Tile::unbreakable_Id);
						blockSet = true;
					}
				}
				if(zOffs >= (m_XZSize/2)-1)
				{
					if( x + random->nextInt(4) >= 15 || zOffs > (m_XZSize/2) )
					{
						blocks[offs] = static_cast<byte>(Tile::unbreakable_Id);
						blockSet = true;
					}
				}
				if( blockSet ) continue;
				// End 4J Extra to build walls around the level

				if (y >= Level::genDepthMinusOne - random->nextInt(5))
				{
					blocks[offs] = static_cast<byte>(Tile::unbreakable_Id);
				}
				else if (y <= 0 + random->nextInt(5))
				{
					blocks[offs] = static_cast<byte>(Tile::unbreakable_Id);
				}
			}
		}
	}
}

LevelChunk *HellFlatLevelSource::create(int x, int z)
{
	return getChunk(x,z);
}

LevelChunk *HellFlatLevelSource::getChunk(int xOffs, int zOffs)
{
	random->setSeed(xOffs * 341873128712l + zOffs * 132897987541l);

	// 4J - now allocating this with a physical alloc & bypassing general memory management so that it will get cleanly freed
	int chunksSize = Level::genDepth * 16  * 16;
	byte *tileData = static_cast<byte *>(XPhysicalAlloc(chunksSize, MAXULONG_PTR, 4096, PAGE_READWRITE));
	XMemSet128(tileData,0,chunksSize);
	byteArray blocks = byteArray(tileData,chunksSize);
	//    byteArray blocks = byteArray(16 * level->depth * 16);

	prepareHeights(xOffs, zOffs, blocks);
	buildSurfaces(xOffs, zOffs, blocks);

	//    caveFeature->apply(this, level, xOffs, zOffs, blocks);
	// townFeature.apply(this, level, xOffs, zOffs, blocks);
	// addCaves(xOffs, zOffs, blocks);
	// addTowns(xOffs, zOffs, blocks);

	// 4J - this now creates compressed block data from the blocks array passed in, so needs to be after data is finalised.
	// Also now need to free the passed in blocks as the LevelChunk doesn't use the passed in allocation anymore.
	LevelChunk *levelChunk = new LevelChunk(level, blocks, xOffs, zOffs);
	XPhysicalFree(tileData);
	return levelChunk;
}

// 4J - removed & moved into its own method from getChunk, so we can call recalcHeightmap after the chunk is added into the cache. Without
// doing this, then loads of the lightgaps() calls will fail to add any lights, because adding a light checks if the cache has this chunk in.
// lightgaps also does light 1 block into the neighbouring chunks, and maybe that is somehow enough to get lighting to propagate round the world,
// but this just doesn't seem right - this isn't a new fault in the 360 version, have checked that java does the same.
void HellFlatLevelSource::lightChunk(LevelChunk *lc)
{
	lc->recalcHeightmap();
}

bool HellFlatLevelSource::hasChunk(int x, int y)
{
	return true;
}

void HellFlatLevelSource::postProcess(ChunkSource *parent, int xt, int zt)
{
	HeavyTile::instaFall = true;
	int xo = xt * 16;
	int zo = zt * 16;

	// 4J - added. The original java didn't do any setting of the random seed here. We'll be running our postProcess in parallel with getChunk etc. so
	// we need to use a separate random - have used the same initialisation code as used in RandomLevelSource::postProcess to make sure this random value
	// is consistent for each world generation. Also changed all uses of random here to pprandom.
	pprandom->setSeed(level->getSeed());
	int64_t xScale = pprandom->nextLong() / 2 * 2 + 1;
	int64_t zScale = pprandom->nextLong() / 2 * 2 + 1;
	pprandom->setSeed(((xt * xScale) + (zt * zScale)) ^ level->getSeed());

	int count = pprandom->nextInt(pprandom->nextInt(10) + 1) + 1;

	for (int i = 0; i < count; i++)
	{
		int x = xo + pprandom->nextInt(16) + 8;
		int y = pprandom->nextInt(Level::genDepth - 8) + 4;
		int z = zo + pprandom->nextInt(16) + 8;
		HellFireFeature().place(level, pprandom, x, y, z);
	}

	count = pprandom->nextInt(pprandom->nextInt(10) + 1);
	for (int i = 0; i < count; i++)
	{
		int x = xo + pprandom->nextInt(16) + 8;
		int y = pprandom->nextInt(Level::genDepth - 8) + 4;
		int z = zo + pprandom->nextInt(16) + 8;
		LightGemFeature().place(level, pprandom, x, y, z);
	}

	HeavyTile::instaFall = false;

	app.processSchematics(parent->getChunk(xt,zt));

}

bool HellFlatLevelSource::save(bool force, ProgressListener *progressListener)
{
	return true;
}

bool HellFlatLevelSource::tick()
{
	return false;
}

bool HellFlatLevelSource::shouldSave()
{
	return true;
}

wstring HellFlatLevelSource::gatherStats()
{
	return L"HellFlatLevelSource";
}

vector<Biome::MobSpawnerData *> *HellFlatLevelSource::getMobsAt(MobCategory *mobCategory, int x, int y, int z)
{
	Biome *biome = level->getBiome(x, z);
	if (biome == nullptr)
	{
		return nullptr;
	}
	return biome->getMobs(mobCategory);
}

TilePos *HellFlatLevelSource::findNearestMapFeature(Level *level, const wstring& featureName, int x, int y, int z)
{
	return nullptr;
}

void HellFlatLevelSource::recreateLogicStructuresForChunk(int chunkX, int chunkZ)
{
}