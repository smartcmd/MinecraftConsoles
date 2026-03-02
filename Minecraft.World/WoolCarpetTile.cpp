#include "stdafx.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "SharedConstants.h"
#include "WoolCarpetTile.h"

WoolCarpetTile::WoolCarpetTile(int id) : Tile(id, Material::clothDecoration, isSolidRender() )
{
	setShape(0, 0, 0, 1, 1 / 16.0f, 1);
	setTicking(true);
	updateShape(0);
}

Icon *WoolCarpetTile::getTexture(int face, int data)
{
	return Tile::wool->getTexture(face, data);
}

AABB *WoolCarpetTile::getAABB(Level *level, int x, int y, int z)
{
	int height = 0;
	float offset = 1.0f / SharedConstants::WORLD_RESOLUTION;
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return AABB::newTemp(x + tls->xx0, y + tls->yy0, z + tls->zz0, x + tls->xx1, y + (height * offset), z + tls->zz1);
}

bool WoolCarpetTile::blocksLight()
{
	return false;
}

bool WoolCarpetTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool WoolCarpetTile::isCubeShaped()
{
	return false;
}

void WoolCarpetTile::updateDefaultShape()
{
	updateShape(0);
}

void WoolCarpetTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity)
{
	updateShape(level->getData(x, y, z));
}

void WoolCarpetTile::updateShape(int data)
{
	int height = 0;
	float o = 1 * (1 + height) / 16.0f;
	setShape(0, 0, 0, 1, o, 1);
}

bool WoolCarpetTile::mayPlace(Level *level, int x, int y, int z)
{
	return Tile::mayPlace(level, x, y, z) && canSurvive(level, x, y, z);
}

void WoolCarpetTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	checkCanSurvive(level, x, y, z);
}

bool WoolCarpetTile::checkCanSurvive(Level *level, int x, int y, int z)
{
	if (!canSurvive(level, x, y, z))
	{
		spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->removeTile(x, y, z);
		return false;
	}
	return true;
}

bool WoolCarpetTile::canSurvive(Level *level, int x, int y, int z)
{
	return !level->isEmptyTile(x, y - 1, z);
}

bool WoolCarpetTile::shouldRenderFace(LevelSource *level, int x, int y, int z, int face)
{
	if (face == 1) return true;
	return Tile::shouldRenderFace(level, x, y, z, face);
}

int WoolCarpetTile::getSpawnResourcesAuxValue(int data)
{
	return data;
}

int WoolCarpetTile::getTileDataForItemAuxValue(int auxValue)
{
	return (~auxValue & 0xf);
}

int WoolCarpetTile::getItemAuxValueForTileData(int data)
{
	return (~data & 0xf);
}

void WoolCarpetTile::registerIcons(IconRegister *iconRegister)
{
	// None, delegates to cloth tile
}
