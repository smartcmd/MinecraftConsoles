#pragma once
using namespace std;
#include "Tutorial.h"
#include "TutorialTask.h"

class StateChangeTask : public TutorialTask
{
private:
	eTutorial_State m_state;
public:
	StateChangeTask(eTutorial_State state,
		Tutorial *tutorial, int descriptionId = -1, bool enablePreCompletion = false, vector<TutorialConstraint *> *inConstraints = nullptr,
		bool bShowMinimumTime=false,  bool bAllowFade=true, bool m_bTaskReminders=true ) :
	TutorialTask(tutorial, descriptionId, enablePreCompletion, inConstraints, bShowMinimumTime,  bAllowFade, m_bTaskReminders ),
		m_state( state )
	{}

	virtual bool isCompleted() { return bIsCompleted; }

	virtual void onStateChange(eTutorial_State newState)
	{
		if(newState == m_state)
		{
			bIsCompleted = true;
		}
	}
};