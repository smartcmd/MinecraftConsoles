#pragma once
using namespace std;
#include "TutorialEnum.h"

class Level;
class Tutorial;
class TutorialConstraint;
class MobEffect;

// A class that represents each individual task in the tutorial.
//
// Members:
// enablePreCompletion	- If this is true, then the player can complete this task out of sequence.
//						 This stops us asking them to do things they have already done
// constraints			- A list of constraints which can be activated (as a whole).
//						 If they are active, then the constraints are removed when the task is completed
// areConstraintsEnabled- A flag which records whether or not we have added the constraints to the tutorial
class TutorialTask
{
protected:
	int descriptionId;
	int m_promptId;
	Tutorial *tutorial;
	bool enablePreCompletion;
	bool bHasBeenActivated;
	bool m_bAllowFade;
	bool m_bTaskReminders;
	bool m_bShowMinimumTime;
	
protected:
	bool bIsCompleted;
	bool m_bShownForMinimumTime;
	vector<TutorialConstraint *> constraints;
	bool areConstraintsEnabled;
public:
	TutorialTask(Tutorial *tutorial, int descriptionId, bool enablePreCompletion, vector<TutorialConstraint *> *inConstraints, bool bShowMinimumTime=false,  bool bAllowFade=true, bool m_bTaskReminders=true );
	virtual ~TutorialTask();

	virtual int getDescriptionId() { return descriptionId; }
	virtual int getPromptId() { return m_promptId; }

	virtual bool isCompleted() = 0;
	virtual eTutorial_CompletionAction getCompletionAction() { return e_Tutorial_Completion_None; }
	virtual bool isPreCompletionEnabled() { return enablePreCompletion; }
	virtual void taskCompleted();
	virtual void enableConstraints(bool enable, bool delayRemove = false);	
	virtual void setAsCurrentTask(bool active = true);

	virtual void setShownForMinimumTime() { m_bShownForMinimumTime = true; }
	virtual bool hasBeenActivated() { return bHasBeenActivated; }
	virtual bool AllowFade()		{ return m_bAllowFade;}
	bool TaskReminders()	{ return m_bTaskReminders;}
	virtual bool ShowMinimumTime()	{ return m_bShowMinimumTime;}

	virtual void useItemOn(Level *level, shared_ptr<ItemInstance> item, int x, int y, int z, bool bTestUseOnly=false) { }
	virtual void useItem(shared_ptr<ItemInstance> item,bool bTestUseOnly=false) { }
	virtual void completeUsingItem(shared_ptr<ItemInstance> item) { }
	virtual void handleUIInput(int iAction) { }
	virtual void onCrafted(shared_ptr<ItemInstance> item) { }
	virtual void onTake(shared_ptr<ItemInstance> item, unsigned int invItemCountAnyAux, unsigned int invItemCountThisAux) { }
	virtual void onStateChange(eTutorial_State newState) { }
	virtual void onEffectChanged(MobEffect *effect, bool bRemoved=false) { }
};