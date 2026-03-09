#include "stdafx.h"
#include "MaterialColor.h"

MaterialColor **MaterialColor::colors;
		   
MaterialColor *MaterialColor::none = nullptr;
MaterialColor *MaterialColor::grass = nullptr;
MaterialColor *MaterialColor::sand = nullptr;
MaterialColor *MaterialColor::cloth = nullptr;
MaterialColor *MaterialColor::fire = nullptr;
MaterialColor *MaterialColor::ice = nullptr;
MaterialColor *MaterialColor::metal = nullptr;
MaterialColor *MaterialColor::plant = nullptr;
MaterialColor *MaterialColor::snow = nullptr;
MaterialColor *MaterialColor::clay = nullptr;
MaterialColor *MaterialColor::dirt = nullptr;
MaterialColor *MaterialColor::stone = nullptr;
MaterialColor *MaterialColor::water = nullptr;
MaterialColor *MaterialColor::wood = nullptr;

void MaterialColor::staticCtor()
{
	MaterialColor::colors = new MaterialColor *[16];

	MaterialColor::none = new MaterialColor(0, eMinecraftColour_Material_None);
	MaterialColor::grass = new MaterialColor(1, eMinecraftColour_Material_Grass);
	MaterialColor::sand = new MaterialColor(2, eMinecraftColour_Material_Sand);
	MaterialColor::cloth = new MaterialColor(3, eMinecraftColour_Material_Cloth);
	MaterialColor::fire = new MaterialColor(4, eMinecraftColour_Material_Fire);
	MaterialColor::ice = new MaterialColor(5, eMinecraftColour_Material_Ice);
	MaterialColor::metal = new MaterialColor(6, eMinecraftColour_Material_Metal);
	MaterialColor::plant = new MaterialColor(7, eMinecraftColour_Material_Plant);
	MaterialColor::snow = new MaterialColor(8, eMinecraftColour_Material_Snow);
	MaterialColor::clay = new MaterialColor(9, eMinecraftColour_Material_Clay);
	MaterialColor::dirt = new MaterialColor(10, eMinecraftColour_Material_Dirt);
	MaterialColor::stone = new MaterialColor(11, eMinecraftColour_Material_Stone);
	MaterialColor::water = new MaterialColor(12, eMinecraftColour_Material_Water);
	MaterialColor::wood = new MaterialColor(13, eMinecraftColour_Material_Wood);
}

MaterialColor::MaterialColor(int id, eMinecraftColour col)
{
	this->id = id;
	this->col = col;
	colors[id] = this;
}