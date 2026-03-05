#include "stdafx.h"
#include "Tutorial.h"
#include "AreaTask.h"

AreaTask::AreaTask(eTutorial_State state, Tutorial *tutorial, vector<TutorialConstraint *> *inConstraints, int descriptionId, EAreaTaskCompletionStates completionState)
	: TutorialTask( tutorial, descriptionId, false, inConstraints, false, false, false )
{
	m_tutorialState = state;
	if(m_tutorialState == e_Tutorial_State_Gameplay)
	{
		enableConstraints(true);
	}
	m_completionState = completionState;
}

bool AreaTask::isCompleted()
{
	if(bIsCompleted) return true;

	bool complete = false;
	switch(m_completionState)
	{
	case eAreaTaskCompletion_CompleteOnConstraintsSatisfied:
		{
			bool allSatisfied = true;
			for( auto& constraint : constraints )
			{
				if(!constraint->isConstraintSatisfied(tutorial->getPad()))
				{
					allSatisfied = false;
					break;
				}
			}
			complete = allSatisfied;
		}
		break;
	case eAreaTaskCompletion_CompleteOnActivation:
		complete = bHasBeenActivated;
		break;
	};
	bIsCompleted = complete;
	return complete;
}

void AreaTask::setAsCurrentTask(bool active)
{
	TutorialTask::setAsCurrentTask(active);

	if(m_completionState == eAreaTaskCompletion_CompleteOnConstraintsSatisfied)
	{
		enableConstraints(active);
	}
}

void AreaTask::onStateChange(eTutorial_State newState)
{
	if(m_completionState == eAreaTaskCompletion_CompleteOnActivation)
	{
		if(m_tutorialState == newState)
		{
			enableConstraints(true);
		}
		else if(m_tutorialState != e_Tutorial_State_Gameplay)
		{
			//enableConstraints(false);
		}
	}
}