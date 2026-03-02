#include "stdafx.h"
#include "PickupTask.h"

void PickupTask::onTake(shared_ptr<ItemInstance> item, unsigned int invItemCountAnyAux, unsigned int invItemCountThisAux)
{
	if(item->id == m_itemId)
	{
		if(m_auxValue == -1 && invItemCountAnyAux >= m_quantity)
		{
			bIsCompleted = true;
		}
		else if( m_auxValue == item->getAuxValue() && invItemCountThisAux >= m_quantity)
		{
			bIsCompleted = true;
		}
	}
}