#include "stdafx.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "TorchTile.h"

TorchTile::TorchTile(int id) : Tile(id, Material::decoration,isSolidRender())
{
	this->setTicking(true);
}

AABB *TorchTile::getAABB(Level *level, int x, int y, int z)
{
	return nullptr;
}

AABB *TorchTile::getTileAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	return Tile::getTileAABB(level, x, y, z);
}

void TorchTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	setShape(level->getData(x, y, z));
}

void TorchTile::setShape(int data)
{
	int dir = data & 7;

	float r = 0.15f;
	if (dir == 1)
	{
		setShape(0, 0.2f, 0.5f - r, r * 2, 0.8f, 0.5f + r);
	}
	else if (dir == 2)
	{
		setShape(1 - r * 2, 0.2f, 0.5f - r, 1, 0.8f, 0.5f + r);
	}
	else if (dir == 3)
	{
		setShape(0.5f - r, 0.2f, 0, 0.5f + r, 0.8f, r * 2);
	}
	else if (dir == 4)
	{
		setShape(0.5f - r, 0.2f, 1 - r * 2, 0.5f + r, 0.8f, 1);
	}
	else
	{
		r = 0.1f;
		setShape(0.5f - r, 0.0f, 0.5f - r, 0.5f + r, 0.6f, 0.5f + r);
	}
}

bool TorchTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool TorchTile::isCubeShaped()
{
	return false;
}

int TorchTile::getRenderShape()
{
	return Tile::SHAPE_TORCH;
}

bool TorchTile::isConnection(Level *level, int x, int y, int z)
{
	if (level->isTopSolidBlocking(x, y, z))
	{
		return true;
	}
	int tile = level->getTile(x, y, z);
	if  (tile == Tile::fence_Id || tile == Tile::netherFence_Id 
		|| tile == Tile::glass_Id || tile == Tile::cobbleWall_Id)
	{
		return true;
	}
	return false;
}

bool TorchTile::mayPlace(Level *level, int x, int y, int z)
{
	if (level->isSolidBlockingTileInLoadedChunk(x - 1, y, z, true))
	{
		return true;
	}
	else if (level->isSolidBlockingTileInLoadedChunk(x + 1, y, z, true))
	{
		return true;
	}
	else if (level->isSolidBlockingTileInLoadedChunk(x, y, z - 1, true))
	{
		return true;
	}
	else if (level->isSolidBlockingTileInLoadedChunk(x, y, z + 1, true))
	{
		return true;
	}
	else if (isConnection(level, x, y - 1, z))
	{
		return true;
	}
	return false;
}

int TorchTile::getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue)
{
	int dir = itemValue;

	if (face == 1 && isConnection(level, x, y - 1, z)) dir = 5;
	if (face == 2 && level->isSolidBlockingTileInLoadedChunk(x, y, z + 1, true)) dir = 4;
	if (face == 3 && level->isSolidBlockingTileInLoadedChunk(x, y, z - 1, true)) dir = 3;
	if (face == 4 && level->isSolidBlockingTileInLoadedChunk(x + 1, y, z, true)) dir = 2;
	if (face == 5 && level->isSolidBlockingTileInLoadedChunk(x - 1, y, z, true)) dir = 1;

	return dir;
}

void TorchTile::tick(Level *level, int x, int y, int z, Random *random)
{
	Tile::tick(level, x, y, z, random);
	if (level->getData(x, y, z) == 0) onPlace(level, x, y, z);
}

void TorchTile::onPlace(Level *level, int x, int y, int z)
{
	if(level->getData(x,y,z) == 0)
	{
		if (level->isSolidBlockingTileInLoadedChunk(x - 1, y, z, true))
		{
			level->setData(x, y, z, 1, Tile::UPDATE_CLIENTS);
		}
		else if (level->isSolidBlockingTileInLoadedChunk(x + 1, y, z, true))
		{
			level->setData(x, y, z, 2, Tile::UPDATE_CLIENTS);
		}
		else if (level->isSolidBlockingTileInLoadedChunk(x, y, z - 1, true))
		{
			level->setData(x, y, z, 3, Tile::UPDATE_CLIENTS);
		}
		else if (level->isSolidBlockingTileInLoadedChunk(x, y, z + 1, true))
		{
			level->setData(x, y, z, 4, Tile::UPDATE_CLIENTS);
		}
		else if (isConnection(level, x, y - 1, z))
		{
			level->setData(x, y, z, 5, Tile::UPDATE_CLIENTS);
		}
	}
	checkCanSurvive(level, x, y, z);
}

void TorchTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	checkDoPop(level, x, y, z, type);
}

bool TorchTile::checkDoPop(Level *level, int x, int y, int z, int type)
{
	if (checkCanSurvive(level, x, y, z))
	{
		int dir = level->getData(x, y, z);
		bool replace = false;

		if (!level->isSolidBlockingTileInLoadedChunk(x - 1, y, z, true) && dir == 1) replace = true;
		if (!level->isSolidBlockingTileInLoadedChunk(x + 1, y, z, true) && dir == 2) replace = true;
		if (!level->isSolidBlockingTileInLoadedChunk(x, y, z - 1, true) && dir == 3) replace = true;
		if (!level->isSolidBlockingTileInLoadedChunk(x, y, z + 1, true) && dir == 4) replace = true;
		if (!isConnection(level, x, y - 1, z) && dir == 5) replace = true;

		if (replace)
		{
			spawnResources(level, x, y, z, level->getData(x, y, z), 0);
			level->removeTile(x, y, z);
			return true;
		}
	}
	else
	{
		return true;
	}
	return false;
}

bool TorchTile::checkCanSurvive(Level *level, int x, int y, int z)
{
	if (!mayPlace(level, x, y, z))
	{
		if (level->getTile(x, y, z) == id)
		{
			this->spawnResources(level, x, y, z, level->getData(x, y, z), 0);
			level->removeTile(x, y, z);
		}
		return false;
	}
	return true;
}

HitResult *TorchTile::clip(Level *level, int x, int y, int z, Vec3 *a, Vec3 *b)
{
	setShape(level->getData(x, y, z));

	return Tile::clip(level, x, y, z, a, b);
}

void TorchTile::animateTick(Level *level, int xt, int yt, int zt, Random *random)
{
	int dir = level->getData(xt, yt, zt);
	double x = xt + 0.5f;
	double y = yt + 0.7f;
	double z = zt + 0.5f;
	double h = 0.22f;
	double r = 0.27f;
	if (dir == 1)
	{
		level->addParticle(eParticleType_smoke, x - r, y + h, z, 0, 0, 0);
		level->addParticle(eParticleType_flame, x - r, y + h, z, 0, 0, 0);
	}
	else if (dir == 2)
	{
		level->addParticle(eParticleType_smoke, x + r, y + h, z, 0, 0, 0);
		level->addParticle(eParticleType_flame, x + r, y + h, z, 0, 0, 0);
	}
	else if (dir == 3)
	{
		level->addParticle(eParticleType_smoke, x, y + h, z - r, 0, 0, 0);
		level->addParticle(eParticleType_flame, x, y + h, z - r, 0, 0, 0);
	}
	else if (dir == 4)
	{
		level->addParticle(eParticleType_smoke, x, y + h, z + r, 0, 0, 0);
		level->addParticle(eParticleType_flame, x, y + h, z + r, 0, 0, 0);
	}
	else
	{
		level->addParticle(eParticleType_smoke, x, y, z, 0, 0, 0);
		level->addParticle(eParticleType_flame, x, y, z, 0, 0, 0);
	}
}

bool TorchTile::shouldTileTick(Level *level, int x,int y,int z)
{
	return level->getData(x, y, z) == 0;
}
