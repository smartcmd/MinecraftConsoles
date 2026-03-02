#include "stdafx.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "Tutorial.h"
#include "TutorialHint.h"
#include "..\..\Minecraft.h"
#include "..\..\MultiplayerLocalPlayer.h"

TutorialHint::TutorialHint(eTutorial_Hint id, Tutorial *tutorial, int descriptionId, eHintType type, bool allowFade /*= true*/) 
	: m_id( id ), m_tutorial(tutorial), m_descriptionId( descriptionId ), m_type( type ), m_counter( 0 ),
	m_lastTile( NULL ), m_hintNeeded( true ), m_allowFade(allowFade)
{
	tutorial->addMessage(descriptionId, type != e_Hint_NoIngredients);
}

int TutorialHint::startDestroyBlock(shared_ptr<ItemInstance> item, Tile *tile)
{
	int returnVal = -1;
	switch(m_type)
	{
	case e_Hint_HoldToMine:
		if( tile == m_lastTile && m_hintNeeded )
		{
			++m_counter;
			if(m_counter > TUTORIAL_HINT_MAX_MINE_REPEATS)
			{
				returnVal = m_descriptionId;
			}
		}
		else
		{
			m_counter = 0;
		}
		m_lastTile = tile;
		break;
	default:
		break;
	}

	return returnVal;
}

int TutorialHint::destroyBlock(Tile *tile)
{
	int returnVal = -1;
	switch(m_type)
	{
	case e_Hint_HoldToMine:
		if(tile == m_lastTile && m_counter > 0)
		{
			m_hintNeeded = false;
		}
		break;
	default:
		break;
	}

	return returnVal;
}

int TutorialHint::attack(shared_ptr<ItemInstance> item, shared_ptr<Entity> entity)
{
	/*
	switch(m_type)
	{
	default:
		return -1;
	}
	*/
	return -1;
}

int TutorialHint::createItemSelected(shared_ptr<ItemInstance> item, bool canMake)
{
	int returnVal = -1;
	switch(m_type)
	{
	case e_Hint_NoIngredients:
		if(!canMake)
			returnVal = m_descriptionId;
		break;
	default:
		break;
	}
	return returnVal;
}

int TutorialHint::itemDamaged(shared_ptr<ItemInstance> item)
{
	int returnVal = -1;
	switch(m_type)
	{
	case e_Hint_ToolDamaged:
			returnVal = m_descriptionId;
		break;
	default:
		break;
	}
	return returnVal;
}

bool TutorialHint::onTake( shared_ptr<ItemInstance> item )
{
	return false;
}

bool TutorialHint::onLookAt(int id, int iData)
{
	return false;
}

bool TutorialHint::onLookAtEntity(eINSTANCEOF type)
{
	return false;
}

int TutorialHint::tick()
{
	int returnVal = -1;
	switch(m_type)
	{
	case e_Hint_SwimUp:
		if( Minecraft::GetInstance()->localplayers[m_tutorial->getPad()]->isUnderLiquid(Material::water) ) returnVal = m_descriptionId; 
		break;
	}
	return returnVal;
}