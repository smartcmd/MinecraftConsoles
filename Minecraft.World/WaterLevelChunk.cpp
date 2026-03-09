#include "stdafx.h"
#include "Arrays.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "WaterLevelChunk.h"
#include "net.minecraft.world.level.biome.h"

WaterLevelChunk::WaterLevelChunk(Level *level, byteArray blocks, int x, int z): LevelChunk(level,blocks,x,z)
{
	dontSave = true;
	// Set this as fully post-processed, so we don't try and run post-processing on any edge chunks that will overlap into real chunks
	terrainPopulated = LevelChunk::sTerrainPopulatedAllNeighbours | LevelChunk::sTerrainPostPostProcessed;
}

bool WaterLevelChunk::isAt(int x, int z)
{
	return x == this->x && z == this->z;
}

void WaterLevelChunk::recalcBlockLights()
{
}

void WaterLevelChunk::recalcHeightmapOnly()
{
}

void WaterLevelChunk::recalcHeightmap()
{
}

void WaterLevelChunk::lightLava()
{
}

bool WaterLevelChunk::setTileAndData(int x, int y, int z, int _tile, int _data)
{
	return true;
}

bool WaterLevelChunk::setTile(int x, int y, int z, int _tile)
{
	return true;
}

bool WaterLevelChunk::setData(int x, int y, int z, int val, int mask, bool *maskedBitsChanged)
{
	*maskedBitsChanged = true;
	return true;
}

void WaterLevelChunk::setBrightness(LightLayer::variety layer, int x, int y, int z, int brightness)
{
}

void WaterLevelChunk::addEntity(shared_ptr<Entity> e)
{
}

void WaterLevelChunk::removeEntity(shared_ptr<Entity> e)
{
}

void WaterLevelChunk::removeEntity(shared_ptr<Entity> e, int yc)
{
}

void WaterLevelChunk::skyBrightnessChanged()
{
}

shared_ptr<TileEntity> WaterLevelChunk::getTileEntity(int x, int y, int z)
{
	return shared_ptr<TileEntity>();
}

void WaterLevelChunk::addTileEntity(shared_ptr<TileEntity> te)
{
}

void WaterLevelChunk::setTileEntity(int x, int y, int z, shared_ptr<TileEntity> tileEntity)
{
}

void WaterLevelChunk::removeTileEntity(int x, int y, int z)
{
}

void WaterLevelChunk::load()
{
}

void WaterLevelChunk::unload(bool unloadTileEntities)	// 4J - added parameter
{
}

bool WaterLevelChunk::containsPlayer()
{
	return false;
}

void WaterLevelChunk::markUnsaved()
{
}

void WaterLevelChunk::getEntities(shared_ptr<Entity> except, AABB bb, vector<shared_ptr<Entity> > &es)
{
}

void WaterLevelChunk::getEntitiesOfClass(const type_info& ec, AABB bb, vector<shared_ptr<Entity> > &es)
{
}

int WaterLevelChunk::countEntities()
{
	return 0;
}

bool WaterLevelChunk::shouldSave(bool force)
{
	return false;
}

void WaterLevelChunk::setBlocks(byteArray newBlocks, int sub)
{
}

int WaterLevelChunk::setBlocksAndData(byteArray data, int x0, int y0, int z0, int x1, int y1, int z1, int p, bool includeLighting/* = true*/)
{
    int xs = x1 - x0;
    int ys = y1 - y0;
    int zs = z1 - z0;

    int s = xs * ys * zs;

	if( includeLighting )
	{
	    return s + s / 2 * 3;
	}
	else
	{
		return s + s / 2;
	}
}

bool WaterLevelChunk::testSetBlocksAndData(byteArray data, int x0, int y0, int z0, int x1, int y1, int z1, int p)
{
	return false;
}

Random *WaterLevelChunk::getRandom(int64_t l)
{
	return new Random((level->getSeed() + x * x * 4987142 + x * 5947611 + z * z * 4392871l + z * 389711) ^ l);
}

void WaterLevelChunk::setLevelChunkBrightness(LightLayer::variety layer, int x, int y, int z, int brightness)
{
	LevelChunk::setBrightness(layer, x, y, z, brightness);
}

Biome *WaterLevelChunk::getBiome(int x, int z, BiomeSource *biomeSource)
{
	return nullptr;
}
