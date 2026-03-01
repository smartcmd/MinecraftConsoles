#include "stdafx.h"

#include "DispenseItemBehavior.h"

DispenseItemBehavior *DispenseItemBehavior::NOOP = new NoOpDispenseItemBehavior();

shared_ptr<ItemInstance> NoOpDispenseItemBehavior::dispense(BlockSource *source, shared_ptr<ItemInstance> dispensed)
{
	return dispensed;
}