#include "stdafx.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "Tutorial.h"
#include "LookAtTileHint.h"


LookAtTileHint::LookAtTileHint(eTutorial_Hint id, Tutorial *tutorial, int tiles[], unsigned int tilesLength, int iconOverride /*= -1*/, int iData /* = -1 */, int iDataOverride  /*= -1*/)
	: TutorialHint(id, tutorial, -1, e_Hint_LookAtTile)
{
	m_iTilesCount = tilesLength;

	m_iTiles= new int [m_iTilesCount];
	for(unsigned int i=0;i<m_iTilesCount;i++)
	{
		m_iTiles[i]=tiles[i];
	}

	m_iconOverride = iconOverride;
	m_iData = iData;
	m_iDataOverride = iDataOverride;
}

bool LookAtTileHint::onLookAt(int id,int iData)
{
	if(id > 0 && id < 256 && (m_iData == -1 || m_iData == iData) )
	{
		bool itemFound = false;
		for(unsigned int i=0;i<m_iTilesCount;i++)
		{
			if(id == m_iTiles[i])
			{
				itemFound = true;
				break;
			}
		}
		if(itemFound)
		{
			// Display hint
			Tutorial::PopupMessageDetails *message = new Tutorial::PopupMessageDetails();
			message->m_delay = true;
			if( m_iconOverride >= 0 )
			{
				message->m_icon = m_iconOverride;
			}
			else if(m_iconOverride == -2)
			{
				message->m_icon = TUTORIAL_NO_ICON;
			}
			else
			{
				message->m_icon = id;
			}

			// 4J-JEV: Moved to keep data override even if we're overriding the icon as well.
			message->m_iAuxVal = (m_iDataOverride > -1) ? m_iDataOverride : iData;

			message->m_messageId = Item::items[id]->getUseDescriptionId();
			message->m_titleId = Item::items[id]->getDescriptionId(message->m_iAuxVal);
			return m_tutorial->setMessage(this, message);
		}
	}
	return false;
}