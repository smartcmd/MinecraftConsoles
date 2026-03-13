#include "stdafx.h"

#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "AreaHint.h"
#include "../../../Minecraft.World/AABB.h"
#include "Tutorial.h"

AreaHint::AreaHint(eTutorial_Hint id, Tutorial *tutorial, eTutorial_State displayState, eTutorial_State completeState,
	int descriptionId, double x0, double y0, double z0, double x1, double y1, double z1, bool allowFade /*= false*/, bool contains /*= true*/ )
	: TutorialHint( id, tutorial, descriptionId, e_Hint_Area, allowFade )
{
	area = AABB::newPermanent(x0, y0, z0, x1, y1, z1);

	this->contains = contains;

	m_displayState = displayState;
	m_completeState = completeState;
}

AreaHint::~AreaHint()
{
	delete area;
}

int AreaHint::tick()
{	
	Minecraft *minecraft = Minecraft::GetInstance();
	if( (m_displayState == e_Tutorial_State_Any || m_tutorial->getCurrentState() == m_displayState) &&
		m_hintNeeded &&
		area->contains( minecraft->player->getPos(1) ) == contains )
	{
		if( m_completeState == e_Tutorial_State_None ) 
		{
			m_hintNeeded = false;
		}
		else if ( m_tutorial->isStateCompleted( m_completeState ) )
		{
			m_hintNeeded = false;
			return -1;
		}

		return m_descriptionId;
	}
	else
	{
		return -1;
	}
}