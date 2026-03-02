#pragma once
#include "AbstractContainerScreen.h"

class FurnaceTileEntity;
class Inventory;

class FurnaceScreen : public AbstractContainerScreen
{
private:
	shared_ptr<FurnaceTileEntity> furnace;

public:
	FurnaceScreen(shared_ptr<Inventory> inventory, shared_ptr<FurnaceTileEntity> furnace);
protected:
	virtual void renderLabels();
	virtual void renderBg(float a);
};