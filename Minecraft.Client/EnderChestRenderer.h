#pragma once
#include "TileEntityRenderer.h"
#include "ChestModel.h"

class EnderChestRenderer : public TileEntityRenderer
{
private:
	static ResourceLocation ENDER_CHEST_LOCATION; 
	ChestModel chestModel;

public:
	void render(shared_ptr<TileEntity>  _chest, double x, double y, double z, float a, bool setColor, float alpha=1.0f, bool useCompiled = true); // 4J added setColor param
};
