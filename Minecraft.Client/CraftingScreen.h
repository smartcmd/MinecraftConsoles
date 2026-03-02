#pragma once
#include "AbstractContainerScreen.h"
class Inventory;
class Level;

class CraftingScreen : public AbstractContainerScreen
{
public:
	CraftingScreen(shared_ptr<Inventory> inventory, Level *level, int x, int y, int z);
	virtual void removed();
protected:
	virtual void renderLabels();
	virtual void renderBg(float a);
};