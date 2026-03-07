#include "stdafx.h"
#include "../Minecraft.Client/Minecraft.h"
#include "FoliageColor.h"

// 4J Stu - Don't use this any more
//intArray FoliageColor::pixels;
//
//void FoliageColor::init(intArray pixels)
//{
//	int *oldData = FoliageColor::pixels.data;
//	FoliageColor::pixels = pixels;
//	delete[] oldData;
//}
//
//int FoliageColor::get(double temp, double rain)
//{
//    rain *= temp;
//    int x = (int) ((1 - temp) * 255);
//    int y = (int) ((1 - rain) * 255);
//    int returnVal = pixels[y << 8 | x];
//	return returnVal;
//}

int FoliageColor::getEvergreenColor()
{
	return Minecraft::GetInstance()->getColourTable()->getColor(eMinecraftColour_Foliage_Evergreen);
	//return 0x619961;
}

int FoliageColor::getBirchColor()
{
	return Minecraft::GetInstance()->getColourTable()->getColor(eMinecraftColour_Foliage_Birch);
	//return 0x80a755;
}

int FoliageColor::getDefaultColor()
{
	return Minecraft::GetInstance()->getColourTable()->getColor(eMinecraftColour_Foliage_Default);
	//return 0x48b518;
}