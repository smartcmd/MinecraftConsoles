using namespace std;

#include "stdafx.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.item.h"
#include "Facing.h"
#include "Material.h"
#include "GenericStats.h"
#include "DoorItem.h"

DoorItem::DoorItem(int id, Material *material) :  Item(id)
{
	this->material = material;
	maxStackSize = 1;
}

bool DoorItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly) 
{
	if (face != Facing::UP) return false;
	y++;

	Tile *tile;

	if (material == Material::wood) tile = Tile::door_wood;
	else tile = Tile::door_iron;

	if (!player->mayBuild(x, y, z) || !player->mayBuild(x, y + 1, z)) return false;
	if (!tile->mayPlace(level, x, y, z)) return false;

	// 4J-PB - Adding a test only version to allow tooltips to be displayed
	if(bTestUseOnOnly) return true;

	// 4J-JEV: Hook for durango 'BlockPlaced' event.
	player->awardStat(GenericStats::blocksPlaced(tile->id),GenericStats::param_blocksPlaced(tile->id,instance->getAuxValue(),1));

	int dir = Mth::floor(((player->yRot + 180) * 4) / 360 - 0.5) & 3;
	place(level, x, y, z, dir, tile);

	instance->count--;
	return true;
}

void DoorItem::place(Level *level, int x, int y, int z, int dir, Tile *tile)
{

	int xra = 0;
	int zra = 0;
	if (dir == 0) zra = +1;
	if (dir == 1) xra = -1;
	if (dir == 2) zra = -1;
	if (dir == 3) xra = +1;


	int solidLeft = (level->isSolidBlockingTile(x - xra, y, z - zra) ? 1 : 0) + (level->isSolidBlockingTile(x - xra, y + 1, z - zra) ? 1 : 0);
	int solidRight = (level->isSolidBlockingTile(x + xra, y, z + zra) ? 1 : 0) + (level->isSolidBlockingTile(x + xra, y + 1, z + zra) ? 1 : 0);

	bool doorLeft = (level->getTile(x - xra, y, z - zra) == tile->id) || (level->getTile(x - xra, y + 1, z - zra) == tile->id);
	bool doorRight = (level->getTile(x + xra, y, z + zra) == tile->id) || (level->getTile(x + xra, y + 1, z + zra) == tile->id);

	bool flip = false;
	if (doorLeft && !doorRight) flip = true;
	else if (solidRight > solidLeft) flip = true;

	level->noNeighborUpdate = true;
	level->setTileAndData(x, y, z, tile->id, dir);
	level->setTileAndData(x, y + 1, z, tile->id, 8 | (flip ? 1 : 0));
	level->noNeighborUpdate = false;
	level->updateNeighborsAt(x, y, z, tile->id);
	level->updateNeighborsAt(x, y + 1, z, tile->id);
}

