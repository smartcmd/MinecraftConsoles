#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.h"
#include "FenceTile.h"

FenceTile::FenceTile(int id, const wstring &texture, Material *material) : Tile( id, material, isSolidRender())
{
	this->texture = texture;
}

void FenceTile::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source)
{
	bool n = connectsTo(level, x, y, z - 1);
	bool s = connectsTo(level, x, y, z + 1);
	bool w = connectsTo(level, x - 1, y, z);
	bool e = connectsTo(level, x + 1, y, z);

	float west = 6.0f / 16.0f;
	float east = 10.0f / 16.0f;
	float north = 6.0f / 16.0f;
	float south = 10.0f / 16.0f;

	if (n)
	{
		north = 0;
	}
	if (s)
	{
		south = 1;
	}
	if (n || s)
	{
		setShape(west, 0, north, east, 1.5f, south);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
	}
	north = 6.0f / 16.0f;
	south = 10.0f / 16.0f;
	if (w)
	{
		west = 0;
	}
	if (e)
	{
		east = 1;
	}
	if (w || e || (!n && !s))
	{
		setShape(west, 0, north, east, 1.5f, south);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
	}

	if (n)
	{
		north = 0;
	}
	if (s)
	{
		south = 1;
	}

	setShape(west, 0, north, east, 1.0f, south);
}

void FenceTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	bool n = connectsTo(level, x, y, z - 1);
	bool s = connectsTo(level, x, y, z + 1);
	bool w = connectsTo(level, x - 1, y, z);
	bool e = connectsTo(level, x + 1, y, z);

	float west = 6.0f / 16.0f;
	float east = 10.0f / 16.0f;
	float north = 6.0f / 16.0f;
	float south = 10.0f / 16.0f;

	if (n)
	{
		north = 0;
	}
	if (s)
	{
		south = 1;
	}
	if (w)
	{
		west = 0;
	}
	if (e)
	{
		east = 1;
	}

	setShape(west, 0, north, east, 1.0f, south);
}

bool FenceTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool FenceTile::isCubeShaped()
{
	return false;
}

bool FenceTile::isPathfindable(LevelSource *level, int x, int y, int z)
{
	return false;
}

int FenceTile::getRenderShape()
{
	return Tile::SHAPE_FENCE;
}

bool FenceTile::connectsTo(LevelSource *level, int x, int y, int z)
{
	int tile = level->getTile(x, y, z);
	if (tile == id || tile == Tile::fenceGate_Id)
	{
		return true;
	}
	Tile *tileInstance = Tile::tiles[tile];
	if (tileInstance != NULL)
	{
		if (tileInstance->material->isSolidBlocking() && tileInstance->isCubeShaped())
		{
			return tileInstance->material != Material::vegetable;
		}
	}
	return false;
}

bool FenceTile::isFence(int tile)
{
	return tile == Tile::fence_Id || tile == Tile::netherFence_Id;
}

void FenceTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(texture);
}

bool FenceTile::shouldRenderFace(LevelSource *level, int x, int y, int z, int face)
{
	return true;
}

bool FenceTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly)
{
	if (level->isClientSide) return true;
	if (LeashItem::bindPlayerMobs(player, level, x, y, z))
	{
		return true;
	}
	return false;
}