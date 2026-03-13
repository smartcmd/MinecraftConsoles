#include "stdafx.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "Tutorial.h"
#include "TakeItemHint.h"


TakeItemHint::TakeItemHint(eTutorial_Hint id, Tutorial *tutorial, int items[], unsigned int itemsLength)
	: TutorialHint(id, tutorial, -1, e_Hint_TakeItem)
{
	m_iItemsCount = itemsLength;

	m_iItems= new int [m_iItemsCount];
	for(unsigned int i=0;i<m_iItemsCount;i++)
	{
		m_iItems[i]=items[i];
	}
}

bool TakeItemHint::onTake(shared_ptr<ItemInstance> item)
{
	if(item != nullptr)
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
			// Display hint
			Tutorial::PopupMessageDetails *message = new Tutorial::PopupMessageDetails();
			message->m_messageId = item->getUseDescriptionId();
			message->m_titleId = item->getDescriptionId();
			message->m_icon = item->id;
			message->m_delay = true;
			return m_tutorial->setMessage(this, message);
		}
	}
	return false;
}