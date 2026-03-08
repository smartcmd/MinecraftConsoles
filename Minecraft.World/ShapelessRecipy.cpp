// package net.minecraft.world.item.crafting;
// 
// import java.util.*;
// 
// import net.minecraft.world.inventory.CraftingContainer;
// import net.minecraft.world.item.ItemInstance;
#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.inventory.h"
#include "Tile.h"
#include "Recipy.h"
#include "Recipes.h"
#include "ShapelessRecipy.h"

ShapelessRecipy::ShapelessRecipy(ItemInstance *result, vector<ItemInstance *> *ingredients, _eGroupType egroup) :
	result(result),
	ingredients(ingredients),
	group(egroup)
{
}

const int ShapelessRecipy::getGroup() 
{	
	return group;
}

const ItemInstance *ShapelessRecipy::getResultItem() 
{
	return result;
}

bool ShapelessRecipy::matches(shared_ptr<CraftingContainer> craftSlots, Level *level) 
{
	vector <ItemInstance *> tempList = *ingredients;
	
	for (int y = 0; y < 3; y++) 
	{
		for (int x = 0; x < 3; x++) 
		{
			shared_ptr<ItemInstance> item = craftSlots->getItem(x, y);

			if (item) 
			{
				bool found = false;

				auto citEnd = ingredients->end();
				for ( ItemInstance *ingredient : *ingredients )
				{
					if (item->id == ingredient->id && (ingredient->getAuxValue() == Recipes::ANY_AUX_VALUE || item->getAuxValue() == ingredient->getAuxValue())) 
					{
						found = true;
						auto it = find(tempList.begin(), tempList.end(), ingredient);
						if(it != tempList.end() ) tempList.erase(it);
						break;
					}
				}

				if (!found) 
				{
					return false;
				}
			}
		}
	}

	return tempList.empty();
}

shared_ptr<ItemInstance> ShapelessRecipy::assemble(shared_ptr<CraftingContainer> craftSlots) 
{
	return result->copy();
}

int ShapelessRecipy::size() 
{
	return (int)ingredients->size();
}

// 4J-PB
bool ShapelessRecipy::reqs(int iRecipe) 
{
	vector <ItemInstance *> *tempList = new vector<ItemInstance *>;

	*tempList=*ingredients;

	//printf("ShapelessRecipy %d\n",iRecipe);

	int iCount=0;
	for ( auto ingredient = ingredients->begin(); ingredient != ingredients->end(); ingredient++)
	{
		//printf("\tIngredient %d is %d\n",iCount++,(*ingredient)->id);
		//if (item->id == (*ingredient)->id && ((*ingredient)->getAuxValue() == Recipes::ANY_AUX_VALUE || item->getAuxValue() == (*ingredient)->getAuxValue())) 
		tempList->erase(ingredient);
	}

	delete tempList;
	return false;
}

void ShapelessRecipy::reqs(INGREDIENTS_REQUIRED *pIngReq) 
{
	int iCount=0;
	bool bFound;
	int j;
	INGREDIENTS_REQUIRED TempIngReq;

	// shapeless doesn't have the 3x3 shape, but we'll just use this to store the ingredients anyway
	TempIngReq.iIngC=0;
	TempIngReq.iType = RECIPE_TYPE_2x2; // all the dyes can be made in a 2x2
	TempIngReq.uiGridA = new unsigned int [9];
	TempIngReq.iIngIDA= new int [3*3];
	TempIngReq.iIngValA = new int [3*3];
	TempIngReq.iIngAuxValA = new int [3*3];

	ZeroMemory(TempIngReq.iIngIDA,sizeof(int)*9);
	ZeroMemory(TempIngReq.iIngValA,sizeof(int)*9);
	memset(TempIngReq.iIngAuxValA,Recipes::ANY_AUX_VALUE,sizeof(int)*9);
	ZeroMemory(TempIngReq.uiGridA,sizeof(unsigned int)*9);

	for ( ItemInstance *expected : *ingredients )
	{
		if ( expected ) 
		{			
			int iAuxVal = expected->getAuxValue();
			TempIngReq.uiGridA[iCount++]=expected->id | iAuxVal<<24;
			// 4J-PB - put the ingredients in boxes 1,2,4,5 so we can see them in a 2x2 crafting screen
			if(iCount==2) iCount=3;
			bFound=false;
			for(j=0;j<TempIngReq.iIngC;j++)
			{
				if((TempIngReq.iIngIDA[j]==expected->id) && (iAuxVal == Recipes::ANY_AUX_VALUE || TempIngReq.iIngAuxValA[j] == iAuxVal))
				{
					bFound= true;
					break;
				}
			}
			if(bFound)
			{
				TempIngReq.iIngValA[j]++;
			}
			else
			{
				TempIngReq.iIngIDA[TempIngReq.iIngC]=expected->id;
				TempIngReq.iIngAuxValA[TempIngReq.iIngC]=iAuxVal;
				TempIngReq.iIngValA[TempIngReq.iIngC++]++;
			}
		}
	}
	pIngReq->iIngIDA = new int [TempIngReq.iIngC];
	pIngReq->iIngValA = new int [TempIngReq.iIngC];
	pIngReq->iIngAuxValA = new int [TempIngReq.iIngC];
	pIngReq->uiGridA = new unsigned int [9];

	pIngReq->pRecipy=this;

	for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		pIngReq->bCanMake[i]=false;
	}

	pIngReq->iIngC=TempIngReq.iIngC;
	pIngReq->iType=TempIngReq.iType;

	if(pIngReq->iIngC!=0)
	{
		memcpy(pIngReq->iIngIDA,TempIngReq.iIngIDA,sizeof(int)*TempIngReq.iIngC);
		memcpy(pIngReq->iIngValA,TempIngReq.iIngValA,sizeof(int)*TempIngReq.iIngC);
		memcpy(pIngReq->iIngAuxValA,TempIngReq.iIngAuxValA,sizeof(int)*TempIngReq.iIngC);
	}
	memcpy(pIngReq->uiGridA,TempIngReq.uiGridA,sizeof(unsigned int) *9);

	delete [] TempIngReq.iIngIDA;
	delete [] TempIngReq.iIngValA;
	delete [] TempIngReq.iIngAuxValA;
	delete [] TempIngReq.uiGridA;
}