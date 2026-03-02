using namespace std;

#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "CoalItem.h"

CoalItem::CoalItem(int id) : Item( id )
{
	setStackedByData(true);
	setMaxDamage(0);
}

unsigned int CoalItem::getDescriptionId(shared_ptr<ItemInstance> instance) 
{
	if (instance->getAuxValue() == CHAR_COAL) 
	{
		return IDS_ITEM_CHARCOAL;
	}
	return IDS_ITEM_COAL;
}
