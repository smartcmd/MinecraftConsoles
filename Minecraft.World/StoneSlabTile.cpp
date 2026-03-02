#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.h"
#include "StoneSlabTile.h"


const unsigned int StoneSlabTile::SLAB_NAMES[SLAB_NAMES_LENGTH] = {	IDS_TILE_STONESLAB_STONE,
													IDS_TILE_STONESLAB_SAND,
													IDS_TILE_STONESLAB_WOOD,
													IDS_TILE_STONESLAB_COBBLE,
													IDS_TILE_STONESLAB_BRICK,
													IDS_TILE_STONESLAB_SMOOTHBRICK,
													IDS_TILE_STONESLAB_NETHERBRICK,
													IDS_TILE_STONESLAB_QUARTZ,
												};

StoneSlabTile::StoneSlabTile(int id, bool fullSize) : HalfSlabTile(id, fullSize, Material::stone)
{
}

Icon *StoneSlabTile::getTexture(int face, int data)
{
	int type = data & TYPE_MASK;
	if (fullSize && (data & TOP_SLOT_BIT) != 0)
	{
		face = Facing::UP;
	}
	switch(type)
	{
	case STONE_SLAB:
		if (face == Facing::UP || face == Facing::DOWN) return icon;
		return iconSide;
		break;	
	case SAND_SLAB:
		return Tile::sandStone->getTexture(face);
	case WOOD_SLAB:
		return Tile::wood->getTexture(face);
	case COBBLESTONE_SLAB:
		return Tile::stoneBrick->getTexture(face);
	case BRICK_SLAB:
		return Tile::redBrick->getTexture(face);
	case SMOOTHBRICK_SLAB:
		return Tile::stoneBrickSmooth->getTexture(face, SmoothStoneBrickTile::TYPE_DEFAULT);
	case NETHERBRICK_SLAB:
		return Tile::netherBrick->getTexture(Facing::UP);
	case QUARTZ_SLAB:
		return Tile::quartzBlock->getTexture(face);
	}
	
	return icon;
}

void StoneSlabTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(L"stoneslab_top");
	iconSide = iconRegister->registerIcon(L"stoneslab_side");
}

int StoneSlabTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Tile::stoneSlabHalf_Id;
}

unsigned int StoneSlabTile::getDescriptionId(int iData /*= -1*/)
{
	if(iData < 0 ) iData = 0;
	return StoneSlabTile::SLAB_NAMES[iData];
}

int StoneSlabTile::getAuxName(int auxValue) 
{
	if (auxValue < 0 || auxValue >= SLAB_NAMES_LENGTH) 
	{
		auxValue = 0;
	}
	return SLAB_NAMES[auxValue];//super.getDescriptionId() + "." + SLAB_NAMES[auxValue];
}

shared_ptr<ItemInstance> StoneSlabTile::getSilkTouchItemInstance(int data)
{
	return shared_ptr<ItemInstance>(new ItemInstance(Tile::stoneSlabHalf_Id, 2, data & TYPE_MASK));
}
