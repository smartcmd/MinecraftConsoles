#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "Tile.h"
#include "Recipy.h"
#include "Recipes.h"
#include "ToolRecipies.h"

// 4J-PB - adding "" on the end of these so we can detect it
wstring ToolRecipies::shapes[][4] = 
{
	{L"XXX", //
	L" # ",//
	L" # "},//

	{L"X",//
	L"#",//
	L"#"},//

	{L"XX",//
	L"X#",//
	L" #"},//

	{L"XX",//
	L" #",//
	L" #"},//
};

void ToolRecipies::_init()
{
	map = new vector <Object *> [MAX_TOOL_RECIPES];

	ADD_OBJECT(map[0],Tile::wood);
	ADD_OBJECT(map[0],Tile::cobblestone);
	ADD_OBJECT(map[0],Item::ironIngot);
	ADD_OBJECT(map[0],Item::diamond);
	ADD_OBJECT(map[0],Item::goldIngot);

	ADD_OBJECT(map[1],Item::pickAxe_wood);
	ADD_OBJECT(map[1],Item::pickAxe_stone);
	ADD_OBJECT(map[1],Item::pickAxe_iron);
	ADD_OBJECT(map[1],Item::pickAxe_diamond);
	ADD_OBJECT(map[1],Item::pickAxe_gold);

	ADD_OBJECT(map[2],Item::shovel_wood);
	ADD_OBJECT(map[2],Item::shovel_stone);
	ADD_OBJECT(map[2],Item::shovel_iron);
	ADD_OBJECT(map[2],Item::shovel_diamond);
	ADD_OBJECT(map[2],Item::shovel_gold);

	ADD_OBJECT(map[3],Item::hatchet_wood);
	ADD_OBJECT(map[3],Item::hatchet_stone);
	ADD_OBJECT(map[3],Item::hatchet_iron);
	ADD_OBJECT(map[3],Item::hatchet_diamond);
	ADD_OBJECT(map[3],Item::hatchet_gold);

	ADD_OBJECT(map[4],Item::hoe_wood);
	ADD_OBJECT(map[4],Item::hoe_stone);
	ADD_OBJECT(map[4],Item::hoe_iron);
	ADD_OBJECT(map[4],Item::hoe_diamond);
	ADD_OBJECT(map[4],Item::hoe_gold);
}

void ToolRecipies::addRecipes(Recipes *r) 
{
	wchar_t wchTypes[7];
	wchTypes[6]=0;

	for (unsigned int m = 0; m < map[0].size(); m++) 
	{
		Object *pObjMaterial = map[0].at(m);

		for (int t=0; t<MAX_TOOL_RECIPES-1; t++) 
		{
			Item *target = map[t+1].at(m)->item;

			wchTypes[0]=L'w';
			wchTypes[1]=L'c';
			wchTypes[2]=L'i';
			wchTypes[3]=L'c';
			wchTypes[5]=L'g';
			if(pObjMaterial->GetType()==eType_TILE)
			{
				wchTypes[4]=L't';
				r->addShapedRecipy(new ItemInstance(target), 
					wchTypes,
					shapes[t], 

					L'#', Item::stick,
					L'X', pObjMaterial->tile,
					L'T');
			}
			else
			{
				// must be Item
				wchTypes[4]=L'i';
				r->addShapedRecipy(new ItemInstance(target), 
					wchTypes,
					shapes[t], 

					L'#', Item::stick,
					L'X', pObjMaterial->item,
					L'T');
			}
		}
	}
	r->addShapedRecipy(new ItemInstance((Item *)Item::shears), 
		L"sscig",
        L" #", //
        L"# ", //
		L'#', Item::ironIngot,
		L'T'
		);
}