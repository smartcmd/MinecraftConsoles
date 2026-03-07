#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "Tile.h"
#include "FurnaceRecipes.h"

FurnaceRecipes *FurnaceRecipes::instance = NULL;

void FurnaceRecipes::staticCtor()
{
	FurnaceRecipes::instance = new FurnaceRecipes();
}

FurnaceRecipes *FurnaceRecipes::getInstance()
{
	return instance;
}

FurnaceRecipes::FurnaceRecipes()
{
	addFurnaceRecipy(Tile::ironOre_Id, new ItemInstance(Item::ironIngot), .7f);
	addFurnaceRecipy(Tile::goldOre_Id, new ItemInstance(Item::goldIngot), 1);
	addFurnaceRecipy(Tile::diamondOre_Id, new ItemInstance(Item::diamond), 1);
	addFurnaceRecipy(Tile::sand_Id, new ItemInstance(Tile::glass), .1f);
	addFurnaceRecipy(Item::porkChop_raw_Id, new ItemInstance(Item::porkChop_cooked), .35f);
	addFurnaceRecipy(Item::beef_raw_Id, new ItemInstance(Item::beef_cooked), .35f);
	addFurnaceRecipy(Item::chicken_raw_Id, new ItemInstance(Item::chicken_cooked), .35f);
	addFurnaceRecipy(Item::fish_raw_Id, new ItemInstance(Item::fish_cooked), .35f);
	addFurnaceRecipy(Tile::cobblestone_Id, new ItemInstance(Tile::stone), .1f);
	addFurnaceRecipy(Item::clay_Id, new ItemInstance(Item::brick), .3f);
	addFurnaceRecipy(Tile::clay_Id, new ItemInstance(Tile::clayHardened), .35f);
	addFurnaceRecipy(Tile::cactus_Id, new ItemInstance(Item::dye_powder, 1, DyePowderItem::GREEN), .2f);
	addFurnaceRecipy(Tile::treeTrunk_Id, new ItemInstance(Item::coal, 1, CoalItem::CHAR_COAL), .15f);
	addFurnaceRecipy(Tile::emeraldOre_Id, new ItemInstance(Item::emerald), 1);
	addFurnaceRecipy(Item::potato_Id, new ItemInstance(Item::potatoBaked), .35f);
	addFurnaceRecipy(Tile::netherRack_Id, new ItemInstance(Item::netherbrick), .1f);

	// special silk touch related recipes:
	addFurnaceRecipy(Tile::coalOre_Id, new ItemInstance(Item::coal), .1f);
	addFurnaceRecipy(Tile::redStoneOre_Id, new ItemInstance(Item::redStone), .7f);
	addFurnaceRecipy(Tile::lapisOre_Id, new ItemInstance(Item::dye_powder, 1, DyePowderItem::BLUE), .2f);
	addFurnaceRecipy(Tile::netherQuartz_Id, new ItemInstance(Item::netherQuartz), .2f);


}

void FurnaceRecipes::addFurnaceRecipy(int itemId, ItemInstance *result, float value)
{
	//recipies->put(itemId, result);
	recipies[itemId]=result;
	recipeValue[result->id] = value;
}

bool FurnaceRecipes::isFurnaceItem(int itemId)
{
    auto it = recipies.find(itemId);
    return it != recipies.end();
}

ItemInstance *FurnaceRecipes::getResult(int itemId)
{
    auto it = recipies.find(itemId);
    if(it != recipies.end())
	{
		return it->second;
	}
	return NULL;
}

unordered_map<int, ItemInstance *> *FurnaceRecipes::getRecipies()
{
	return &recipies;
}

float FurnaceRecipes::getRecipeValue(int itemId)
{
    auto it = recipeValue.find(itemId);
    if (it != recipeValue.end())
	{
		return it->second;
	}
	return 0.0f;
}