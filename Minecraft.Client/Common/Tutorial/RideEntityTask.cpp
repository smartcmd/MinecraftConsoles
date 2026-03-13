#include "stdafx.h"

#include <string>

#include "Minecraft.h"
#include "Tutorial.h"

#include "..\Minecraft.World\EntityHorse.h"

#include "RideEntityTask.h"

RideEntityTask::RideEntityTask(const int eType, Tutorial *tutorial, int descriptionId,
	bool enablePreCompletion, vector<TutorialConstraint *> *inConstraints, bool bShowMinimumTime, bool bAllowFade, bool bTaskReminders)
	: TutorialTask( tutorial, descriptionId, enablePreCompletion, inConstraints, bShowMinimumTime, bAllowFade, bTaskReminders ),
	m_eType( eType )
{
}

bool RideEntityTask::isCompleted()
{
	return bIsCompleted;
}

void RideEntityTask::onRideEntity(shared_ptr<Entity> entity)
{
	if (entity->instanceof(static_cast<eINSTANCEOF>(m_eType)))
	{
		bIsCompleted = true;
	}
}