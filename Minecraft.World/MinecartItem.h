#pragma once
using namespace std;

#include "Item.h"
#include "DefaultDispenseItemBehavior.h"

class MinecartItem : public Item 
{
private:
	class MinecartDispenseBehavior : public DefaultDispenseItemBehavior
	{
	private:
		DefaultDispenseItemBehavior defaultDispenseItemBehavior;

	public:
		virtual shared_ptr<ItemInstance> execute(BlockSource *source, shared_ptr<ItemInstance> dispensed, eOUTCOME &outcome);

	protected:
		virtual void playSound(BlockSource *source);
	};

public:
	int type;

	MinecartItem(int id, int type);

	virtual bool useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
};