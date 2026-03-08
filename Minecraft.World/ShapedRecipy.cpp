// package net.minecraft.world.item.crafting;
// 
// import net.minecraft.world.inventory.CraftingContainer;
// import net.minecraft.world.item.ItemInstance;

#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.inventory.h"
#include "Tile.h"
#include "Recipy.h"
#include "Recipes.h"
#include "ShapedRecipy.h"

// 4J-PB - for new crafting - Adding group to define type of item that the recipe produces
ShapedRecipy::ShapedRecipy(int width, int height, ItemInstance **recipeItems, ItemInstance *result, int iGroup) 
	: resultId(result->id)
{
		this->width = width;
		this->height = height;
		this->recipeItems = recipeItems;
		this->result = result;
		this->group = iGroup;
		_keepTag = false;
}

const int ShapedRecipy::getGroup() 
{
	return group;
}

const ItemInstance *ShapedRecipy::getResultItem() 
{
	return result;
}

bool ShapedRecipy::matches(shared_ptr<CraftingContainer> craftSlots, Level *level) 
{
	for (int xOffs = 0; xOffs <= (3 - width); xOffs++) 
	{
		for (int yOffs = 0; yOffs <= (3 - height); yOffs++) 
		{
			if (matches(craftSlots, xOffs, yOffs, true)) return true;
			if (matches(craftSlots, xOffs, yOffs, false)) return true;
		}
	}
	return false;
}

bool ShapedRecipy::matches(shared_ptr<CraftingContainer> craftSlots, int xOffs, int yOffs, bool xFlip) 
{
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			int xs = x - xOffs;
			int ys = y - yOffs;
			ItemInstance *expected = NULL;
			if (xs >= 0 && ys >= 0 && xs < width && ys < height) 
			{
				if (xFlip) expected = recipeItems[(width - xs - 1) + ys * width];
				else expected = recipeItems[xs + ys * width];
			}
			shared_ptr<ItemInstance> item = craftSlots->getItem(x, y);
			if (item == NULL && expected == NULL) 
			{
				continue;
			}
			if ((item == NULL && expected != NULL) || (item != NULL && expected == NULL)) 
			{
				return false;
			}
			if (expected->id != item->id) 
			{
				return false;
			}
			if (expected->getAuxValue() != Recipes::ANY_AUX_VALUE && expected->getAuxValue() != item->getAuxValue()) 
			{
				return false;
			}
		}
	}
	return true;
}

shared_ptr<ItemInstance> ShapedRecipy::assemble(shared_ptr<CraftingContainer> craftSlots) 
{
	shared_ptr<ItemInstance> result = getResultItem()->copy();

	if (_keepTag && craftSlots != NULL)
	{
		for (int i = 0; i < craftSlots->getContainerSize(); i++)
		{
			shared_ptr<ItemInstance> item = craftSlots->getItem(i);

			if (item != NULL && item->hasTag())
			{
				result->setTag((CompoundTag *) item->tag->copy());
			}
		}
	}

	return result;
}

int ShapedRecipy::size() 
{
	return width * height;
}

// 4J-PB
bool ShapedRecipy::reqs(int iRecipe) 
{
	app.DebugPrintf("ShapedRecipy %d\n",iRecipe);
	int iCount=0;
	for (int x = 0; x < 3; x++) 
	{
		for (int y = 0; y < 3; y++) 
		{
			if (x < width && y < height) 
			{
				ItemInstance *expected = recipeItems[x+y*width];
				if (expected!=NULL) 
				{			
					//printf("\tIngredient %d is %d\n",iCount++,expected->id);
				}
			}
		}
	}



	return false;
}

void ShapedRecipy::reqs(INGREDIENTS_REQUIRED *pIngReq) 
{
	//printf("ShapedRecipy %d\n",iRecipe);

	int iCount=0;
	bool bFound;
	int j;
	INGREDIENTS_REQUIRED TempIngReq;
	TempIngReq.iIngC=0;
	TempIngReq.iType = ((width>2) ||(height>2))?RECIPE_TYPE_3x3:RECIPE_TYPE_2x2; // 3x3
	// 3x3
	TempIngReq.uiGridA = new unsigned int [9];
	TempIngReq.iIngIDA= new int [9];
	TempIngReq.iIngValA = new int [9];
	TempIngReq.iIngAuxValA = new int [9];

	ZeroMemory(TempIngReq.iIngIDA,sizeof(int)*9);
	ZeroMemory(TempIngReq.iIngValA,sizeof(int)*9);
	memset(TempIngReq.iIngAuxValA,Recipes::ANY_AUX_VALUE,sizeof(int)*9);
	ZeroMemory(TempIngReq.uiGridA,sizeof(unsigned int)*9);

	for (int x = 0; x < 3; x++) 
	{
		for (int y = 0; y < 3; y++) 
		{
			if (x < width && y < height) 
			{
				ItemInstance *expected = recipeItems[x+y*width];

				if (expected!=NULL) 
				{			
					int iAuxVal = expected->getAuxValue();
					TempIngReq.uiGridA[x+y*3]=expected->id | iAuxVal<<24;

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
					//printf("\tIngredient %d is %d\n",iCount++,expected->id);
				}

			}
		}
	}
	pIngReq->iIngIDA= new int [TempIngReq.iIngC];
	pIngReq->iIngValA= new int [TempIngReq.iIngC];
	pIngReq->iIngAuxValA = new int [TempIngReq.iIngC];
	pIngReq->uiGridA = new unsigned int [9];

	pIngReq->iIngC=TempIngReq.iIngC;
	pIngReq->iType=TempIngReq.iType;

	pIngReq->pRecipy=this;

	for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		pIngReq->bCanMake[i]=false;
	}

	for(j=0;j<9;j++)
	{
		pIngReq->uiGridA[j]=TempIngReq.uiGridA[j];
	}

	if(pIngReq->iIngC!=0)
	{
		memcpy(pIngReq->iIngIDA,TempIngReq.iIngIDA,sizeof(int)*TempIngReq.iIngC);
		memcpy(pIngReq->iIngValA,TempIngReq.iIngValA,sizeof(int)*TempIngReq.iIngC);
		memcpy(pIngReq->iIngAuxValA,TempIngReq.iIngAuxValA,sizeof(int)*TempIngReq.iIngC);
	}
	memcpy(pIngReq->uiGridA,TempIngReq.uiGridA,sizeof(unsigned int)*9);

	delete [] TempIngReq.iIngIDA;
	delete [] TempIngReq.iIngValA;
	delete [] TempIngReq.iIngAuxValA;
	delete [] TempIngReq.uiGridA;
}

ShapedRecipy *ShapedRecipy::keepTag()
{
	_keepTag = true;
	return this;
}