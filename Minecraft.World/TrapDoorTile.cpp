#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.h"
#include "TrapDoorTile.h"


TrapDoorTile::TrapDoorTile(int id, Material *material) : Tile(id, material,isSolidRender())
{
	float r = 0.5f;
	float h = 1.0f;
	setShape(0.5f - r, 0, 0.5f - r, 0.5f + r, h, 0.5f + r);
}

bool TrapDoorTile::blocksLight()
{
	return false;
}


bool TrapDoorTile::isSolidRender(bool isServerLevel)
{
	return false;
}


bool TrapDoorTile::isCubeShaped()
{
	return false;
}

bool TrapDoorTile::isPathfindable(LevelSource *level, int x, int y, int z)
{
	return !isOpen(level->getData(x, y, z));
}

int TrapDoorTile::getRenderShape()
{
	return Tile::SHAPE_BLOCK;
}


AABB *TrapDoorTile::getTileAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	return Tile::getTileAABB(level, x, y, z);
}


AABB *TrapDoorTile::getAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	return Tile::getAABB(level, x, y, z);
}


void TrapDoorTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	setShape(level->getData(x, y, z));
}


void TrapDoorTile::updateDefaultShape()
{
	float r = 3 / 16.0f;
	setShape(0, 0.5f - r / 2, 0, 1, 0.5f + r / 2, 1);
}


void TrapDoorTile::setShape(int data)
{

	float r = 3 / 16.0f;
	Tile::setShape(0, 0, 0, 1, r, 1);
	if (isOpen(data))
	{
		if ((data & 3) == 0) setShape(0, 0, 1 - r, 1, 1, 1);
		if ((data & 3) == 1) setShape(0, 0, 0, 1, 1, r);
		if ((data & 3) == 2) setShape(1 - r, 0, 0, 1, 1, 1);
		if ((data & 3) == 3) setShape(0, 0, 0, r, 1, 1);
	}
}


void TrapDoorTile::attack(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	use(level, x, y, z, player, 0, 0, 0, 0);
}

// 4J-PB - Adding a TestUse for tooltip display
bool TrapDoorTile::TestUse()
{
	return true;
}

bool TrapDoorTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if (material == Material::metal) return true;

	if (soundOnly)
	{
		// 4J - added - just do enough to play the sound
		level->levelEvent(player, LevelEvent::SOUND_OPEN_DOOR, x, y, z, 0);
		return false;
	}

	int dir = level->getData(x, y, z);
	level->setData(x, y, z, dir ^ 4);

	level->levelEvent(player, LevelEvent::SOUND_OPEN_DOOR, x, y, z, 0);
	return true;
}


void TrapDoorTile::setOpen(Level *level, int x, int y, int z, bool shouldOpen)
{
	int dir = level->getData(x, y, z);

	bool wasOpen = (dir & 4) > 0;
	if (wasOpen == shouldOpen) return;

	level->setData(x, y, z, dir ^ 4);

	level->levelEvent(nullptr, LevelEvent::SOUND_OPEN_DOOR, x, y, z, 0);
}

void TrapDoorTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (level->isClientSide) return;

	int data = level->getData(x, y, z);
	int xt = x;
	int zt = z;
	if ((data & 3) == 0) zt++;
	if ((data & 3) == 1) zt--;
	if ((data & 3) == 2) xt++;
	if ((data & 3) == 3) xt--;


	if (!attachesTo(level->getTile(xt, y, zt)))
	{
		level->setTile(x, y, z, 0);
		spawnResources(level, x, y, z, data, 0);
	}

	bool signal = level->hasNeighborSignal(x, y, z);
	if( signal || ((type > 0 && Tile::tiles[type]->isSignalSource())) )
	{
		setOpen(level, x, y, z, signal);
	}
}

HitResult *TrapDoorTile::clip(Level *level, int xt, int yt, int zt, Vec3 *a, Vec3 *b)
{
	updateShape(level, xt, yt, zt);
	return Tile::clip(level, xt, yt, zt, a, b);
}

int TrapDoorTile::getDir(int dir)
{
	if ((dir & 4) == 0)
	{
		return ((dir - 1) & 3);
	}
	else
	{
		return (dir & 3);
	}
}

int TrapDoorTile::getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue)
{
	int dir = 0;
	if (face == 2) dir = 0;
	if (face == 3) dir = 1;
	if (face == 4) dir = 2;
	if (face == 5) dir = 3;
	if (face != Facing::UP && face != Facing::DOWN && clickY > 0.5f) dir |= TOP_MASK;
	return dir;
}

bool TrapDoorTile::mayPlace(Level *level, int x, int y, int z, int face)
{
	if (face == 0) return false;
	if (face == 1) return false;
	if (face == 2) z++;
	if (face == 3) z--;
	if (face == 4) x++;
	if (face == 5) x--;

	return attachesTo(level->getTile(x, y, z));
}

bool TrapDoorTile::isOpen(int data)
{
	return (data & 4) != 0;
}

bool TrapDoorTile::attachesTo(int id)
{
	if (id <= 0)
	{
		return false;
	}
	Tile *tile = Tile::tiles[id];

	return tile != NULL && (tile->material->isSolidBlocking() && tile->isCubeShaped()) || tile == Tile::lightGem || (dynamic_cast<HalfSlabTile *>(tile) != NULL) || (dynamic_cast<StairTile *>(tile) != NULL);
}