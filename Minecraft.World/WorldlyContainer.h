#pragma once

#include "Container.h"

class WorldlyContainer : public Container
{
public:
	virtual intArray getSlotsForFace(int face) = 0;
	virtual bool canPlaceItemThroughFace(int slot, shared_ptr<ItemInstance> item, int face) = 0;
	virtual bool canTakeItemThroughFace(int slot, shared_ptr<ItemInstance> item, int face) = 0;
};