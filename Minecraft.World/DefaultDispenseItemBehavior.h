#pragma once
#include "DispenseItemBehavior.h"

class FacingEnum;
class Position;

class DefaultDispenseItemBehavior : public DispenseItemBehavior
{
protected:
	enum eOUTCOME
	{
		// Item has special behaviour that was executed successfully.
		ACTIVATED_ITEM = 0,

		// Item was dispenced onto the ground as a pickup.
		DISPENCED_ITEM = 1,
		
		// Execution failed, the item was left unaffected.
		LEFT_ITEM = 2,
	};

public:
	DefaultDispenseItemBehavior() {};
	virtual ~DefaultDispenseItemBehavior() {};
	virtual shared_ptr<ItemInstance> dispense(BlockSource *source, shared_ptr<ItemInstance> dispensed);

protected:
	// 4J-JEV: Added value used to play FAILED sound effect upon reaching spawn limits.
	virtual shared_ptr<ItemInstance> execute(BlockSource *source, shared_ptr<ItemInstance> dispensed, eOUTCOME &outcome);

public:
	static void spawnItem(Level *world, shared_ptr<ItemInstance> item, int accuracy, FacingEnum *facing, Position *position);

protected:
	virtual void playSound(BlockSource *source, eOUTCOME outcome);
	virtual void playAnimation(BlockSource *source, FacingEnum *facing, eOUTCOME outcome);

private:
	virtual int getLevelEventDataFrom(FacingEnum *facing);
};