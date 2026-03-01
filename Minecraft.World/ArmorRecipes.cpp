//package net.minecraft.world.item.crafting;

//import net.minecraft.world.item.*;
//import net.minecraft.world.level.tile.Tile;
#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "Tile.h"
#include "Recipy.h"
#include "Recipes.h"
#include "ArmorRecipes.h"

// 4J-PB - adding "" on the end of these so we can detect it
wstring ArmorRecipes::shapes[][4] = 
{
	{L"XXX", //
	L"X X",L""},//

	{L"X X", //
	L"XXX",//
	L"XXX",L""},//

	{L"XXX", //
	L"X X",//
	L"X X",L""},//

	{L"X X",//
	L"X X",L""},//
};

/*
ArmorRecipes::map[5] = 
{
	{Item::leather, Tile::fire, Item::ironIngot, Item::diamond, Item::goldIngot}, 
	{Item::helmet_cloth, Item::helmet_chain, Item::helmet_iron, Item::helmet_diamond, Item::helmet_gold}, 
	{Item::chestplate_cloth, Item::chestplate_chain, Item::chestplate_iron, Item::chestplate_diamond, Item::chestplate_gold}, 
	{Item::leggings_cloth, Item::leggings_chain, Item::leggings_iron, Item::leggings_diamond, Item::leggings_gold}, 
	{Item::boots_cloth, Item::boots_chain, Item::boots_iron, Item::boots_diamond, Item::boots_gold}, 
};
*/

void ArmorRecipes::_init()
{
	map = new vector <Object *> [MAX_ARMOUR_RECIPES];

	// 4J-PB - removing the chain armour, since we show all possible recipes in the xbox game, and it's not one you can make
	ADD_OBJECT(map[0],Item::leather);
//	ADD_OBJECT(map[0],Tile::fire);
	ADD_OBJECT(map[0],Item::ironIngot);
	ADD_OBJECT(map[0],Item::diamond);
	ADD_OBJECT(map[0],Item::goldIngot);

	ADD_OBJECT(map[1],Item::helmet_leather);
//	ADD_OBJECT(map[1],Item::helmet_chain);
	ADD_OBJECT(map[1],Item::helmet_iron);
	ADD_OBJECT(map[1],Item::helmet_diamond);
	ADD_OBJECT(map[1],Item::helmet_gold);

	ADD_OBJECT(map[2],Item::chestplate_leather);
//	ADD_OBJECT(map[2],Item::chestplate_chain);
	ADD_OBJECT(map[2],Item::chestplate_iron);
	ADD_OBJECT(map[2],Item::chestplate_diamond);
	ADD_OBJECT(map[2],Item::chestplate_gold);

	ADD_OBJECT(map[3],Item::leggings_leather);
//	ADD_OBJECT(map[3],Item::leggings_chain);
	ADD_OBJECT(map[3],Item::leggings_iron);
	ADD_OBJECT(map[3],Item::leggings_diamond);
	ADD_OBJECT(map[3],Item::leggings_gold);

	ADD_OBJECT(map[4],Item::boots_leather);
//	ADD_OBJECT(map[4],Item::boots_chain);
	ADD_OBJECT(map[4],Item::boots_iron);
	ADD_OBJECT(map[4],Item::boots_diamond);
	ADD_OBJECT(map[4],Item::boots_gold);
}

// 4J-PB added for quick equip in the inventory
ArmorRecipes::_eArmorType ArmorRecipes::GetArmorType(int iId) 
{
	switch(iId)
	{
	case Item::helmet_leather_Id:	
	case Item::helmet_chain_Id:	
	case Item::helmet_iron_Id:		
	case Item::helmet_diamond_Id:						
	case Item::helmet_gold_Id:	
		return eArmorType_Helmet;
		break;

	case Item::chestplate_leather_Id:
	case Item::chestplate_chain_Id:
	case Item::chestplate_iron_Id:	
	case Item::chestplate_diamond_Id:
	case Item::chestplate_gold_Id:
		return eArmorType_Chestplate;
		break;

	case Item::leggings_leather_Id:
	case Item::leggings_chain_Id:	
	case Item::leggings_iron_Id:
	case Item::leggings_diamond_Id:
	case Item::leggings_gold_Id:
		return eArmorType_Leggings;
		break;

	case Item::boots_leather_Id:		
	case Item::boots_chain_Id:	
	case Item::boots_iron_Id:
	case Item::boots_diamond_Id:
	case Item::boots_gold_Id:
		return eArmorType_Boots;
		break;
	}

	return eArmorType_None;
}

void ArmorRecipes::addRecipes(Recipes *r) 
{
	wchar_t wchTypes[5];
	wchTypes[4]=0;

	for (unsigned int m = 0; m < map[0].size(); m++) 
	{
		Object *pObjMaterial = map[0].at(m);

		for (int t=0; t<MAX_ARMOUR_RECIPES-1; t++) 
		{
			Item *target = map[t+1].at(m)->item;

			wchTypes[0]=L'w';
			wchTypes[1]=L'c';
			wchTypes[3]=L'g';
			if(pObjMaterial->GetType()==eType_TILE)
			{
				wchTypes[2]=L't';
				r->addShapedRecipy(new ItemInstance(target), 
					wchTypes,
					shapes[t], 

					L'X', pObjMaterial->tile,
					L'A');
			}
			else
			{
				// must be Item
				wchTypes[2]=L'i';
				r->addShapedRecipy(new ItemInstance(target), 
					wchTypes,
					shapes[t], 

					L'X', pObjMaterial->item,
					L'A');
			}
		}
	}
}

