#include "stdafx.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.crafting.h"
#include "ArmorDyeRecipe.h"

bool ArmorDyeRecipe::matches(shared_ptr<CraftingContainer> craftSlots, Level *level)
{
	shared_ptr<ItemInstance> target = nullptr;
	vector<shared_ptr<ItemInstance> > dyes;

	for (int slot = 0; slot < craftSlots->getContainerSize(); slot++)
	{
		shared_ptr<ItemInstance> item = craftSlots->getItem(slot);
		if (item == NULL) continue;

		ArmorItem *armor = dynamic_cast<ArmorItem *>(item->getItem());
		if (armor)
		{
			if (armor->getMaterial() == ArmorItem::ArmorMaterial::CLOTH && target == NULL)
			{
				target = item;
			}
			else
			{
				return false;
			}
		}
		else if (item->id == Item::dye_powder_Id)
		{
			dyes.push_back(item);
		}
		else
		{
			return false;
		}
	}

	return target != NULL && !dyes.empty();
}

shared_ptr<ItemInstance> ArmorDyeRecipe::assembleDyedArmor(shared_ptr<CraftingContainer> craftSlots)
{
	shared_ptr<ItemInstance> target = nullptr;
	int colorTotals[3] = {0,0,0};
	int intensityTotal = 0;
	int colourCounts = 0;
	ArmorItem *armor = NULL;

	if(craftSlots != NULL)
	{
		for (int slot = 0; slot < craftSlots->getContainerSize(); slot++)
		{
			shared_ptr<ItemInstance> item = craftSlots->getItem(slot);
			if (item == NULL) continue;

			armor = dynamic_cast<ArmorItem *>(item->getItem());
			if (armor)
			{
				if (armor->getMaterial() == ArmorItem::ArmorMaterial::CLOTH && target == NULL)
				{
					target = item->copy();
					target->count = 1;

					if (armor->hasCustomColor(item))
					{
						int color = armor->getColor(target);
						float red = (float) ((color >> 16) & 0xFF) / 0xFF;
						float green = (float) ((color >> 8) & 0xFF) / 0xFF;
						float blue = (float) (color & 0xFF) / 0xFF;

						intensityTotal += max(red, max(green, blue)) * 0xFF;

						colorTotals[0] += red * 0xFF;
						colorTotals[1] += green * 0xFF;
						colorTotals[2] += blue * 0xFF;
						colourCounts++;
					}
				}
				else
				{
					return nullptr;
				}
			}
			else if (item->id == Item::dye_powder_Id)
			{
				int tileData = ColoredTile::getTileDataForItemAuxValue(item->getAuxValue());
				int red = (int) (Sheep::COLOR[tileData][0] * 0xFF);
				int green = (int) (Sheep::COLOR[tileData][1] * 0xFF);
				int blue = (int) (Sheep::COLOR[tileData][2] * 0xFF);

				intensityTotal += max(red, max(green, blue));

				colorTotals[0] += red;
				colorTotals[1] += green;
				colorTotals[2] += blue;
				colourCounts++;
			}
			else
			{
				return nullptr;
			}
		}
	}

	if (armor == NULL) return nullptr;

	int red = (colorTotals[0] / colourCounts);
	int green = (colorTotals[1] / colourCounts);
	int blue = (colorTotals[2] / colourCounts);

	float averageIntensity = (float) intensityTotal / colourCounts;
	float resultIntensity = (float) max(red, max(green, blue));
	//        System.out.println(averageIntensity + ", " + resultIntensity);

	red = (int) ((float) red * averageIntensity / resultIntensity);
	green = (int) ((float) green * averageIntensity / resultIntensity);
	blue = (int) ((float) blue * averageIntensity / resultIntensity);

	int rgb = red;
	rgb = (rgb << 8) + green;
	rgb = (rgb << 8) + blue;

	armor->setColor(target, rgb);
	return target;
}

shared_ptr<ItemInstance> ArmorDyeRecipe::assemble(shared_ptr<CraftingContainer> craftSlots)
{
	return ArmorDyeRecipe::assembleDyedArmor(craftSlots);
}

int ArmorDyeRecipe::size()
{
	return 10;
}

const ItemInstance *ArmorDyeRecipe::getResultItem()
{
	return NULL;
}

const int ArmorDyeRecipe::getGroup()
{
	return ShapedRecipy::eGroupType_Armour;
}

// 4J-PB
bool ArmorDyeRecipe::reqs(int iRecipe)
{
	return false;
}

void ArmorDyeRecipe::reqs(INGREDIENTS_REQUIRED *pIngReq)
{
	//int iCount=0;
	//bool bFound;
	//int j;
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

#if 0
	for ( ItemInstance *expected : *ingredients )
	{
		if (expected!=NULL) 
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
#endif

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
