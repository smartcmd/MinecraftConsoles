#pragma once

#include "TileEntityRenderer.h"

class ChestModel;

class ChestRenderer : public TileEntityRenderer
{
private:
	ChestModel *chestModel;
	ChestModel *largeChestModel;

public:
	ChestRenderer();	
	~ChestRenderer();

	void render(shared_ptr<TileEntity>  _chest, double x, double y, double z, float a, bool setColor, float alpha=1.0f, bool useCompiled = true); // 4J added setColor param
};
