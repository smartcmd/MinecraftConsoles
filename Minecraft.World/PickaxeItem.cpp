#include "stdafx.h"
#include "net.minecraft.world.level.tile.h"
#include "PickaxeItem.h"

TileArray *PickaxeItem::diggables = NULL;

void PickaxeItem::staticCtor()
{
	PickaxeItem::diggables = new TileArray( PICKAXE_DIGGABLES);
	diggables->data[0] = Tile::stoneBrick;
	diggables->data[1] = Tile::stoneSlab;
	diggables->data[2] = Tile::stoneSlabHalf;
	diggables->data[3] = Tile::rock;
	diggables->data[4] = Tile::sandStone;
	diggables->data[5] = Tile::mossStone;
	diggables->data[6] = Tile::ironOre;
	diggables->data[7] = Tile::ironBlock;
	diggables->data[8] = Tile::coalOre;
	diggables->data[9] = Tile::goldBlock;
	diggables->data[10] = Tile::goldOre;
	diggables->data[11] = Tile::diamondOre;
	diggables->data[12] = Tile::diamondBlock;
	diggables->data[13] = Tile::ice;
	diggables->data[14] = Tile::hellRock;
	diggables->data[15] = Tile::lapisOre;
	diggables->data[16] = Tile::lapisBlock;
	// 4J - brought forward from 1.2.3
	diggables->data[17] = Tile::redStoneOre;
	diggables->data[18] = Tile::redStoneOre_lit;
	diggables->data[19] = Tile::rail;
	diggables->data[20] = Tile::detectorRail;
	diggables->data[21] = Tile::goldenRail;
}

PickaxeItem::PickaxeItem(int id, const Tier *tier) : DiggerItem(id, 2, tier, diggables)
{
}

bool PickaxeItem::canDestroySpecial(Tile *tile)
{
	if (tile == Tile::obsidian) return tier->getLevel() == 3;
	if (tile == Tile::diamondBlock || tile == Tile::diamondOre) return tier->getLevel() >= 2;
	if (tile == Tile::emeraldBlock || tile == Tile::emeraldOre) return tier->getLevel() >= 2;
	if (tile == Tile::goldBlock || tile == Tile::goldOre) return tier->getLevel() >= 2;
	if (tile == Tile::ironBlock || tile == Tile::ironOre) return tier->getLevel() >= 1;
	if (tile == Tile::lapisBlock || tile == Tile::lapisOre) return tier->getLevel() >= 1;
	if (tile == Tile::redStoneOre || tile == Tile::redStoneOre_lit) return tier->getLevel() >= 2;
	if (tile->material == Material::stone) return true;
	if (tile->material == Material::metal) return true;
	if (tile->material == Material::heavyMetal) return true;
	return false;
}

// 4J - brought forward from 1.2.3
float PickaxeItem::getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile *tile)
{
    if (tile != NULL && (tile->material == Material::metal || tile->material == Material::heavyMetal || tile->material == Material::stone))
	{
        return speed;
    }
    return DiggerItem::getDestroySpeed(itemInstance, tile);
}
