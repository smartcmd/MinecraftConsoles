#pragma once
#include "AbstractContainerScreen.h"
class DispenserTileEntity;
class Inventory;

class TrapScreen : public AbstractContainerScreen
{
public:
	TrapScreen(shared_ptr<Inventory> inventory, shared_ptr<DispenserTileEntity> trap);
protected:
	virtual void renderLabels();
    virtual void renderBg(float a);
};