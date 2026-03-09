#pragma once
using namespace std;

#include "TutorialTask.h"

class Level;

// 4J-JEV: Tasks that involve riding an entity.
class RideEntityTask : public TutorialTask
{
protected:
	const int m_eType;

public:
	RideEntityTask(const int eTYPE, Tutorial *tutorial, int descriptionId,
		bool enablePreCompletion = false, vector<TutorialConstraint *> *inConstraints = nullptr,
		bool bShowMinimumTime = false, bool bAllowFade = true, bool bTaskReminders = true );

	virtual bool isCompleted();

	virtual void onRideEntity(shared_ptr<Entity> entity);
};