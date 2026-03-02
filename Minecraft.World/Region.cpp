#include "stdafx.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.chunk.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.redstone.h"
#include "Material.h"
#include "Level.h"

#include "Region.h"


Region::~Region()
{
	for(unsigned int i = 0; i < chunks->length; ++i)
	{
		LevelChunkArray *lca = (*chunks)[i];
		delete [] lca->data;
		delete lca;
	}
	delete [] chunks->data;
	delete chunks;

	// AP - added a caching system for Chunk::rebuild to take advantage of
	if( CachedTiles )
	{
		free(CachedTiles);
	}
}

Region::Region(Level *level, int x1, int y1, int z1, int x2, int y2, int z2, int r)
{
	this->level = level;

	xc1 = (x1 - r) >> 4;
	zc1 = (z1 - r) >> 4;
	int xc2 = (x2 + r) >> 4;
	int zc2 = (z2 + r) >> 4;

	chunks = new LevelChunk2DArray(xc2 - xc1 + 1, zc2 - zc1 + 1);

	allEmpty = true;
	for (int xc = xc1; xc <= xc2; xc++)
	{
		for (int zc = zc1; zc <= zc2; zc++)
		{
			LevelChunk *chunk = level->getChunk(xc, zc);
			if(chunk != NULL)
			{
				LevelChunkArray *lca = (*chunks)[xc - xc1];
				lca->data[zc - zc1] = chunk;
			}
		}
	}
	for (int xc = (x1 >> 4); xc <= (x2 >> 4); xc++)
	{
		for (int zc = (z1 >> 4); zc <= (z2 >> 4); zc++)
		{
			LevelChunkArray *lca = (*chunks)[xc - xc1];
			LevelChunk *chunk = lca->data[zc - zc1];
			if (chunk != NULL)
			{
				if (!chunk->isYSpaceEmpty(y1, y2))
				{
					allEmpty = false;
				}
			}
		}
	}

	// AP - added a caching system for Chunk::rebuild to take advantage of
	xcCached = -1;
	zcCached = -1;
	CachedTiles = NULL;
}

bool Region::isAllEmpty()
{
	return allEmpty;
}

int Region::getTile(int x, int y, int z)
{
	if (y < 0) return 0;
	if (y >= Level::maxBuildHeight) return 0;

	int xc = (x >> 4);
	int zc = (z >> 4);

#ifdef __PSVITA__
	// AP - added a caching system for Chunk::rebuild to take advantage of
	if( CachedTiles && xc == xcCached && zc == zcCached )
	{
		unsigned char* Tiles = CachedTiles;
		Tiles += y;
		if(y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
		{
			Tiles += Level::COMPRESSED_CHUNK_SECTION_TILES - Level::COMPRESSED_CHUNK_SECTION_HEIGHT;
		}

		return Tiles[ ( (x & 15) << 11 ) | ( (z & 15) << 7 ) ];
	}
#endif

	xc -= xc1;
	zc -= zc1;

	if (xc < 0 || xc >= (int)chunks->length || zc < 0 || zc >= (int)(*chunks)[xc]->length)
	{
		return 0;
	}

	LevelChunk *lc = (*chunks)[xc]->data[zc];
	if (lc == NULL) return 0;

	return lc->getTile(x & 15, y, z & 15);
}

// AP - added a caching system for Chunk::rebuild to take advantage of
void Region::setCachedTiles(unsigned char *tiles, int xc, int zc)
{
	xcCached = xc;
	zcCached = zc;
	int size = 16 * 16 * Level::maxBuildHeight;
	if( CachedTiles == NULL )
	{
		CachedTiles = (unsigned char *) malloc(size);
	}
	memcpy(CachedTiles, tiles, size);
}

LevelChunk* Region::getLevelChunk(int x, int y, int z)
{
	if (y < 0) return 0;
	if (y >= Level::maxBuildHeight) return NULL;

	int xc = (x >> 4) - xc1;
	int zc = (z >> 4) - zc1;

	if (xc < 0 || xc >= (int)chunks->length || zc < 0 || zc >= (int)(*chunks)[xc]->length)
	{
		return NULL;
	}

	LevelChunk *lc = (*chunks)[xc]->data[zc];
	return lc;
}



shared_ptr<TileEntity> Region::getTileEntity(int x, int y, int z)
{
	int xc = (x >> 4) - xc1;
	int zc = (z >> 4) - zc1;

	return (*chunks)[xc]->data[zc]->getTileEntity(x & 15, y, z & 15);
}

int Region::getLightColor(int x, int y, int z, int emitt, int tileId/*=-1*/)
{
	int s = getBrightnessPropagate(LightLayer::Sky, x, y, z, tileId);
	int b = getBrightnessPropagate(LightLayer::Block, x, y, z, tileId);
	if (b < emitt) b = emitt;
	return s << 20 | b << 4;
}

float Region::getBrightness(int x, int y, int z, int emitt)
{
	int n = getRawBrightness(x, y, z);
	if (n < emitt) n = emitt;
	return level->dimension->brightnessRamp[n];
}


float Region::getBrightness(int x, int y, int z)
{
	return level->dimension->brightnessRamp[getRawBrightness(x, y, z)];
}


int Region::getRawBrightness(int x, int y, int z)
{
	return getRawBrightness(x, y, z, true);
}


int Region::getRawBrightness(int x, int y, int z, bool propagate)
{
	if (x < -Level::MAX_LEVEL_SIZE || z < -Level::MAX_LEVEL_SIZE || x >= Level::MAX_LEVEL_SIZE || z > Level::MAX_LEVEL_SIZE)
	{
		return Level::MAX_BRIGHTNESS;
	}

	if (propagate)
	{
		int id = getTile(x, y, z);
		switch(id)
		{
		case Tile::stoneSlabHalf_Id:
		case Tile::woodSlabHalf_Id:
		case Tile::farmland_Id:
		case Tile::stairs_stone_Id:
		case Tile::stairs_wood_Id:
			{
				int br = getRawBrightness(x, y + 1, z, false);
				int br1 = getRawBrightness(x + 1, y, z, false);
				int br2 = getRawBrightness(x - 1, y, z, false);
				int br3 = getRawBrightness(x, y, z + 1, false);
				int br4 = getRawBrightness(x, y, z - 1, false);
				if (br1 > br) br = br1;
				if (br2 > br) br = br2;
				if (br3 > br) br = br3;
				if (br4 > br) br = br4;
				return br;
			}
			break;
		}
	}

	if (y < 0) return 0;
	if (y >= Level::maxBuildHeight)
	{
		int br = Level::MAX_BRIGHTNESS - level->skyDarken;
		if (br < 0) br = 0;
		return br;
	}

	int xc = (x >> 4) - xc1;
	int zc = (z >> 4) - zc1;

	return (*chunks)[xc]->data[zc]->getRawBrightness(x & 15, y, z & 15, level->skyDarken);
}


int Region::getData(int x, int y, int z)
{
	if (y < 0) return 0;
	if (y >= Level::maxBuildHeight) return 0;
	int xc = (x >> 4) - xc1;
	int zc = (z >> 4) - zc1;

	return (*chunks)[xc]->data[zc]->getData(x & 15, y, z & 15);
}

Material *Region::getMaterial(int x, int y, int z)
{
	int t = getTile(x, y, z);
	if (t == 0) return Material::air;
	return Tile::tiles[t]->material;
}


BiomeSource *Region::getBiomeSource()
{
	return level->getBiomeSource();
}

Biome *Region::getBiome(int x, int z)
{
	return level->getBiome(x, z);
}

bool Region::isSolidRenderTile(int x, int y, int z)
{
	Tile *tile = Tile::tiles[getTile(x, y, z)];
	if (tile == NULL) return false;

	// 4J - addition here to make rendering big blocks of leaves more efficient. Normally leaves never consider themselves as solid, so
	// blocks of leaves will have all sides of each block completely visible. Changing to consider as solid if this block is surrounded by
	// other leaves (or solid things). This is paired with another change in Tile::getTexture which makes such solid tiles actually visibly solid (these
	// textures exist already for non-fancy graphics). Note: this tile-specific code is here rather than making some new virtual method in the tiles,
	// for the sake of efficiency - I don't imagine we'll be doing much more of this sort of thing
	if( tile->id == Tile::leaves_Id )
	{
		int axo[6] = { 1,-1, 0, 0, 0, 0};
		int ayo[6] = { 0, 0, 1,-1, 0, 0};
		int azo[6] = { 0, 0, 0, 0, 1,-1};
		for( int i = 0; i < 6; i++ )
		{
			int t = getTile(x + axo[i], y + ayo[i] , z + azo[i]);
			if( ( t != Tile::leaves_Id ) && ( ( Tile::tiles[t] == NULL ) || !Tile::tiles[t]->isSolidRender() ) )
			{
				return false;
			}
		}

		return true;
	}

	return tile->isSolidRender();
}


bool Region::isSolidBlockingTile(int x, int y, int z)
{
	Tile *tile = Tile::tiles[getTile(x, y, z)];
	if (tile == NULL) return false;
	return tile->material->blocksMotion() && tile->isCubeShaped();
}

bool Region::isTopSolidBlocking(int x, int y, int z)
{
	Tile *tile = Tile::tiles[getTile(x, y, z)];
	return level->isTopSolidBlocking(tile, getData(x, y, z));
}

bool Region::isEmptyTile(int x, int y, int z)
{
	Tile *tile = Tile::tiles[getTile(x, y, z)];
	return (tile == NULL);
}


// 4J - brought forward from 1.8.2
int Region::getBrightnessPropagate(LightLayer::variety layer, int x, int y, int z, int tileId)
{
	if (y < 0) y = 0;
	if (y >= Level::maxBuildHeight) y = Level::maxBuildHeight - 1;
	if (y < 0 || y >= Level::maxBuildHeight || x < -Level::MAX_LEVEL_SIZE || z < -Level::MAX_LEVEL_SIZE || x >= Level::MAX_LEVEL_SIZE || z > Level::MAX_LEVEL_SIZE)
	{
		// 4J Stu - The java LightLayer was an enum class type with a member "surrounding" which is what we
		// were returning here. Surrounding has the same value as the enum value in our C++ code, so just cast
		// it to an int
		return (int)layer;
	}
	if (layer == LightLayer::Sky && level->dimension->hasCeiling)
	{
		return 0;
	}

	int id = tileId > -1 ? tileId : getTile(x, y, z);
	if (Tile::propagate[id])
	{
		int br = getBrightness(layer, x, y + 1, z); if( br == 15 ) return 15;
		int br1 = getBrightness(layer, x + 1, y, z); if( br1 == 15 ) return 15;
		int br2 = getBrightness(layer, x - 1, y, z); if( br2 == 15 ) return 15;
		int br3 = getBrightness(layer, x, y, z + 1); if( br3 == 15 ) return 15;
		int br4 = getBrightness(layer, x, y, z - 1); if( br4 == 15 ) return 15;
		if (br1 > br) br = br1;
		if (br2 > br) br = br2;
		if (br3 > br) br = br3;
		if (br4 > br) br = br4;
		return br;
	}

	int xc = (x >> 4) - xc1;
	int zc = (z >> 4) - zc1;

	return (*chunks)[xc]->data[zc]->getBrightness(layer, x & 15, y, z & 15);
}

// 4J - brought forward from 1.8.2
int Region::getBrightness(LightLayer::variety layer, int x, int y, int z)
{
	if (y < 0) y = 0;
	if (y >= Level::maxBuildHeight) y = Level::maxBuildHeight - 1;
	if (y < 0 || y >= Level::maxBuildHeight || x < -Level::MAX_LEVEL_SIZE || z < -Level::MAX_LEVEL_SIZE || x >= Level::MAX_LEVEL_SIZE || z > Level::MAX_LEVEL_SIZE)
	{
		// 4J Stu - The java LightLayer was an enum class type with a member "surrounding" which is what we
		// were returning here. Surrounding has the same value as the enum value in our C++ code, so just cast
		// it to an int
		return (int)layer;
	}
	int xc = (x >> 4) - xc1;
	int zc = (z >> 4) - zc1;

	return (*chunks)[xc]->data[zc]->getBrightness(layer, x & 15, y, z & 15);
}

int Region::getMaxBuildHeight()
{
	return Level::maxBuildHeight;
}

int Region::getDirectSignal(int x, int y, int z, int dir)
{
	int t = getTile(x, y, z);
	if (t == 0) return Redstone::SIGNAL_NONE;
	return Tile::tiles[t]->getDirectSignal(this, x, y, z, dir);
}