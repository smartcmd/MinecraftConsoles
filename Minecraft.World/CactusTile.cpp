#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.h"
#include "net.minecraft.world.h"
#include "CactusTile.h"

CactusTile::CactusTile(int id) : Tile(id, Material::cactus,isSolidRender())
{
	setTicking(true);
	iconTop = nullptr;
	iconBottom = nullptr;
}

void CactusTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (level->isEmptyTile(x, y + 1, z))
	{ 
		int height = 1;
		while (level->getTile(x, y - height, z) == id)
		{
			height++;
		}
		if (height < 3)
		{
			int age = level->getData(x, y, z);
			if (age == 15)
			{
				level->setTileAndUpdate(x, y + 1, z, id);
				level->setData(x, y, z, 0, Tile::UPDATE_NONE);
				neighborChanged(level, x, y + 1, z, id);
			}
			else
			{
				level->setData(x, y, z, age + 1, Tile::UPDATE_NONE);
			}
		}
	}
}

AABB *CactusTile::getAABB(Level *level, int x, int y, int z)
{
	float r = 1 / 16.0f;
	return AABB::newTemp(x + r, y, z + r, x + 1 - r, y + 1 - r, z + 1 - r);

}

AABB *CactusTile::getTileAABB(Level *level, int x, int y, int z)
{
	float r = 1 / 16.0f;
	return AABB::newTemp(x + r, y, z + r, x + 1 - r, y + 1, z + 1 - r);
}

Icon *CactusTile::getTexture(int face, int data)
{
	if (face == Facing::UP) return iconTop;
	if (face == Facing::DOWN) return iconBottom;
	else return icon;
}

bool CactusTile::isCubeShaped()
{
	return false;
}

bool CactusTile::isSolidRender(bool isServerLevel)
{
	return false;
}

int CactusTile::getRenderShape()
{
	return Tile::SHAPE_CACTUS;
}

bool CactusTile::mayPlace(Level *level, int x, int y, int z)
{
	if (!Tile::mayPlace(level, x, y, z)) return false;

	return canSurvive(level, x, y, z);
}

void CactusTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (!canSurvive(level, x, y, z))
	{
		level->destroyTile(x, y, z, true);
	}
}

bool CactusTile::canSurvive(Level *level, int x, int y, int z)
{
	if (level->getMaterial(x - 1, y, z)->isSolid()) return false;
	if (level->getMaterial(x + 1, y, z)->isSolid()) return false;
	if (level->getMaterial(x, y, z - 1)->isSolid()) return false;
	if (level->getMaterial(x, y, z + 1)->isSolid()) return false;
	int below = level->getTile(x, y - 1, z);
	return below == Tile::cactus_Id || below == Tile::sand_Id;
}

void CactusTile::entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
	entity->hurt(DamageSource::cactus, 1);
}

void CactusTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(L"cactus_side");
	iconTop = iconRegister->registerIcon(L"cactus_top");
	iconBottom = iconRegister->registerIcon(L"cactus_bottom");
}

bool CactusTile::shouldTileTick(Level *level, int x,int y,int z)
{
	return level->isEmptyTile(x, y + 1, z);
}
