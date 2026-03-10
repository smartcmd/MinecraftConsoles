#include "stdafx.h"
#include "Tutorial.h"
#include "FullTutorialActiveTask.h"

FullTutorialActiveTask::FullTutorialActiveTask(Tutorial *tutorial, eTutorial_CompletionAction completeAction /*= e_Tutorial_Completion_None*/)
	: TutorialTask( tutorial, -1, false, nullptr, false, false, false )
{
	m_completeAction = completeAction;
}

bool FullTutorialActiveTask::isCompleted()
{
	return bHasBeenActivated;
}

eTutorial_CompletionAction FullTutorialActiveTask::getCompletionAction()
{
	if( tutorial->m_fullTutorialComplete )
	{
		return m_completeAction;
	}
	else
	{
		return e_Tutorial_Completion_None;
	}
}