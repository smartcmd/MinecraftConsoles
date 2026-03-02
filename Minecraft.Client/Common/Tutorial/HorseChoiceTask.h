#pragma once
using namespace std;

#include "ChoiceTask.h"


// Same as choice task, but switches description based on horse type.
class HorseChoiceTask : public ChoiceTask
{
protected:
	int m_eHorseType;

	int m_iDescHorse, m_iDescDonkey, m_iDescMule;

public:
	HorseChoiceTask(Tutorial *tutorial, int iDescHorse, int iDescDonkey, int iDescMule, int iPromptId = -1,
		bool requiresUserInput = false, int iConfirmMapping = 0, int iCancelMapping = 0, 
		eTutorial_CompletionAction cancelAction = e_Tutorial_Completion_None, ETelemetryChallenges telemetryEvent = eTelemetryChallenges_Unknown);

	virtual int getDescriptionId();

	virtual void onLookAtEntity(shared_ptr<Entity> entity);
};