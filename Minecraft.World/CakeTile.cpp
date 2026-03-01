#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.food.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.h"
#include "net.minecraft.h"
#include "CakeTile.h"


CakeTile::CakeTile(int id) : Tile(id, Material::cake,isSolidRender())
{
	setTicking(true);

	iconTop = NULL;
	iconBottom = NULL;
	iconInner = NULL;
}

void CakeTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	int d = level->getData(x, y, z);
	float r = 1 / 16.0f;
	float r2 = (1 + d * 2) / 16.0f;
	float h = 8 / 16.0f;
	this->setShape(r2, 0, r, 1 - r, h, 1 - r);
}

void CakeTile::updateDefaultShape()
{
	float r = 1 / 16.0f;
	float h = 8 / 16.0f;
	this->setShape(r, 0, r, 1 - r, h, 1 - r);
}

AABB *CakeTile::getAABB(Level *level, int x, int y, int z)
{
	int d = level->getData(x, y, z);
	float r = 1 / 16.0f;
	float r2 = (1 + d * 2) / 16.0f;
	float h = 8 / 16.0f;
	return AABB::newTemp(x + r2, y, z + r, x + 1 - r, y + h - r, z + 1 - r);
}

AABB *CakeTile::getTileAABB(Level *level, int x, int y, int z)
{
	int d = level->getData(x, y, z);
	float r = 1 / 16.0f;
	float r2 = (1 + d * 2) / 16.0f;
	float h = 8 / 16.0f;
	return AABB::newTemp(x + r2, y, z + r, x + 1 - r, y + h, z + 1 - r);
}

Icon *CakeTile::getTexture(int face, int data)
{
	if (face == Facing::UP) return iconTop;
	if (face == Facing::DOWN) return iconBottom;
	if (data > 0 && face == Facing::WEST) return iconInner;
	return icon;
}

void CakeTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(L"cake_side");
	iconInner = iconRegister->registerIcon(L"cake_inner");
	iconTop = iconRegister->registerIcon(L"cake_top");
	iconBottom = iconRegister->registerIcon(L"cake_bottom");
}

bool CakeTile::isCubeShaped()
{
	return false;
}

bool CakeTile::isSolidRender(bool isServerLevel)
{
	return false;
}

// 4J-PB - Adding a TestUse for tooltip display
bool CakeTile::TestUse()
{
	return true;
}

bool CakeTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if( soundOnly ) return false;
	eat(level, x, y, z, player);
	return true;
}

void CakeTile::attack(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	eat(level, x, y, z, player);
}

void CakeTile::eat(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	if (player->canEat(false))
	{
		player->getFoodData()->eat(2, FoodConstants::FOOD_SATURATION_POOR);

		int d = level->getData(x, y, z) + 1;
		if (d >= 6)
		{
			level->removeTile(x, y, z);
		}
		else
		{
			level->setData(x, y, z, d, Tile::UPDATE_CLIENTS);
		}
	}
}

bool CakeTile::mayPlace(Level *level, int x, int y, int z)
{
	if (!Tile::mayPlace(level, x, y, z)) return false;

	return canSurvive(level, x, y, z);
}

void CakeTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (!canSurvive(level, x, y, z))
	{
		level->removeTile(x, y, z);
	}
}

bool CakeTile::canSurvive(Level *level, int x, int y, int z)
{
	return level->getMaterial(x, y - 1, z)->isSolid();
}

int CakeTile::getResourceCount(Random *random)
{
	return 0;
}

int CakeTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return 0;
}

int CakeTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Item::cake_Id;
}