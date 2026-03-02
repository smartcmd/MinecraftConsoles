#pragma once
using namespace std;

#include "TutorialTask.h"

class Level;

// 4J Stu - Tasks that involve placing a tile
class UseItemTask : public TutorialTask
{
private:
	const int itemId;
	bool completed;

public:
	UseItemTask(const int itemId, Tutorial *tutorial, int descriptionId,
		bool enablePreCompletion = false, vector<TutorialConstraint *> *inConstraints = NULL, bool bShowMinimumTime = false, bool bAllowFade = true, bool bTaskReminders = true );
	virtual bool isCompleted();
	virtual void useItem(shared_ptr<ItemInstance> item, bool bTestUseOnly=false);
};