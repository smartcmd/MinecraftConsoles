#pragma once
#include "TileEntityRenderer.h"

class ChestModel;

class ChestRenderer : public TileEntityRenderer
{
private:
	static ResourceLocation CHEST_LARGE_TRAP_LOCATION;
	//static ResourceLocation CHEST_LARGE_XMAS_LOCATION;
	static ResourceLocation CHEST_LARGE_LOCATION;
	static ResourceLocation CHEST_TRAP_LOCATION;
	//static ResourceLocation CHEST_XMAS_LOCATION;
	static ResourceLocation CHEST_LOCATION;

	ChestModel *chestModel;
	ChestModel *largeChestModel;
	boolean xmasTextures;

public:
	ChestRenderer();	
	~ChestRenderer();

	void render(shared_ptr<TileEntity>  _chest, double x, double y, double z, float a, bool setColor, float alpha=1.0f, bool useCompiled = true); // 4J added setColor param
};
