#include "stdafx.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.h"
#include "Tutorial.h"
#include "DiggerItemHint.h"


DiggerItemHint::DiggerItemHint(eTutorial_Hint id, Tutorial *tutorial, int descriptionId, int items[], unsigned int itemsLength)
	: TutorialHint(id, tutorial, descriptionId, e_Hint_DiggerItem)
{
	m_iItemsCount = itemsLength;

	m_iItems= new int [m_iItemsCount];
	for(unsigned int i=0;i<m_iItemsCount;i++)
	{
		m_iItems[i]=items[i];
	}
	tutorial->addMessage(IDS_TUTORIAL_HINT_ATTACK_WITH_TOOL, true);
}

int DiggerItemHint::startDestroyBlock(shared_ptr<ItemInstance> item, Tile *tile)
{
	if(item != NULL)
	{
		bool itemFound = false;
		for(unsigned int i=0;i<m_iItemsCount;i++)
		{
			if(item->id == m_iItems[i])
			{
				itemFound = true;
				break;
			}
		}
		if(itemFound)
		{
			float speed = item->getDestroySpeed(tile);
			if(speed == 1)
			{
				// Display hint
				return m_descriptionId;
			}
		}
	}
	return -1;
}

int DiggerItemHint::attack(shared_ptr<ItemInstance> item, shared_ptr<Entity> entity)
{
	if(item != NULL)
	{
		bool itemFound = false;
		for(unsigned int i=0;i<m_iItemsCount;i++)
		{
			if(item->id == m_iItems[i])
			{
				itemFound = true;
				break;
			}
		}
		if(itemFound)
		{
			// It's also possible that we could hit TileEntities (eg falling sand) so don't want to give this hint then
			if( entity->instanceof(eTYPE_MOB) )
			{
				return IDS_TUTORIAL_HINT_ATTACK_WITH_TOOL;
			}
			else
			{
				return -1;
			}
		}
	}
	return -1;
}