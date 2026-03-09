#include "stdafx.h"
#include "net.minecraft.stats.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.h"
#include "TopSnowTile.h"

const int TopSnowTile::MAX_HEIGHT = 6;
const int TopSnowTile::HEIGHT_MASK = 7; // max 8 steps

TopSnowTile::TopSnowTile(int id) : Tile(id, Material::topSnow,isSolidRender())
{
	setShape(0, 0, 0, 1, 2 / 16.0f, 1);
	setTicking(true);
	updateShape(0);
}

void TopSnowTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(L"snow");
}

AABB *TopSnowTile::getAABB(Level *level, int x, int y, int z)
{
	int height = level->getData(x, y, z) & HEIGHT_MASK;
	float offset = 2.0f / SharedConstants::WORLD_RESOLUTION;
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
	return AABB::newTemp(x + tls->xx0, y + tls->yy0, z + tls->zz0, x + tls->xx1, y + (height * offset), z + tls->zz1);
}

float TopSnowTile::getHeight(Level *level, int x, int y, int z)
{
	int height = level->getData(x, y, z) & HEIGHT_MASK;
	return 2 * (1 + height) / 16.0f;
}

bool TopSnowTile::blocksLight()
{
	return false;
}

bool TopSnowTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool TopSnowTile::isCubeShaped()
{
	return false;
}

void TopSnowTile::updateDefaultShape()
{
	updateShape(0);
}

void TopSnowTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	updateShape(level->getData(x, y, z));
}

void TopSnowTile::updateShape(int data)
{
	int height = data & HEIGHT_MASK;
	float o = 2 * (1 + height) / 16.0f;
	setShape(0, 0, 0, 1, o, 1);
}

bool TopSnowTile::mayPlace(Level *level, int x, int y, int z)
{
	int t = level->getTile(x, y - 1, z);
	if (t == 0) return false;
	if (t == id && (level->getData(x, y - 1, z) & HEIGHT_MASK) == MAX_HEIGHT + 1) return true;
	// 4J Stu - Assume when placing that this is the server level and we don't care how it's going to be rendered
	// Fix for #9407 - Gameplay: Destroying a block of snow on top of trees, removes any adjacent snow.
	if (t != Tile::leaves_Id && !Tile::tiles[t]->isSolidRender(true)) return false;
	return level->getMaterial(x, y - 1, z)->blocksMotion();
}

void TopSnowTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	checkCanSurvive(level, x, y, z);
}

bool TopSnowTile::checkCanSurvive(Level *level, int x, int y, int z)
{
	if (!mayPlace(level, x, y, z))
	{
		spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->removeTile(x, y, z);
		return false;
	}
	return true;
}

void TopSnowTile::playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data)
{
	int type = Item::snowBall->id;
	int height = data & HEIGHT_MASK;
	popResource(level, x, y, z, std::make_shared<ItemInstance>(type, height + 1, 0));
	level->removeTile(x, y, z);
}

int TopSnowTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::snowBall->id;
}

int TopSnowTile::getResourceCount(Random *random)
{
	return 0;
}

void TopSnowTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (level->getBrightness(LightLayer::Block, x, y, z) > 11)
	{
		spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->removeTile(x, y, z);
	}
}

bool TopSnowTile::shouldRenderFace(LevelSource *level, int x, int y, int z, int face)
{
	if (face == 1) return true;
	// 4J - don't render faces if neighbouring tiles are also TopSnowTile with at least the same height as this one
	// Otherwise we get horrible artifacts from the non-manifold geometry created. Fixes bug #8506
	if ( ( level->getTile(x,y,z) == Tile::topSnow_Id ) && ( face >= 2 ) )
	{
		int h0 = level->getData(x,y,z) & HEIGHT_MASK;
		int xx = x;
		int yy = y;
		int zz = z;
		// Work out coords of tile who's face we're considering (rather than it's neighbour which is passed in here as x,y,z already
		// offsetting by the face direction)
		switch(face)
		{
		case 2:
			zz += 1;
			break;
		case 3:
			zz -= 1;
			break;
		case 4:
			xx += 1;
			break;
		case 5:
			xx -= 1;
			break;
		default:
			break;
		}
		int h1 = level->getData(xx,yy,zz) & HEIGHT_MASK;
		if( h0 >= h1 ) return false;
	}
	return Tile::shouldRenderFace(level, x, y, z, face);
}

bool TopSnowTile::shouldTileTick(Level *level, int x,int y,int z)
{
	return level->getBrightness(LightLayer::Block, x, y, z) > 11;
}
