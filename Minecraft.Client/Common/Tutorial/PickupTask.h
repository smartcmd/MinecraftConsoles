#pragma once
using namespace std;
#include "TutorialTask.h"

class ItemInstance;

class PickupTask : public TutorialTask
{
public:
	PickupTask( int itemId, unsigned int  quantity, int auxValue,
		Tutorial *tutorial, int descriptionId, bool enablePreCompletion = true, vector<TutorialConstraint *> *inConstraints = nullptr,
		bool bShowMinimumTime=false,  bool bAllowFade=true, bool m_bTaskReminders=true )
		: TutorialTask(tutorial, descriptionId, enablePreCompletion, inConstraints, bShowMinimumTime,  bAllowFade, m_bTaskReminders ),
		m_itemId( itemId),
		m_quantity( quantity ),
		m_auxValue( auxValue )
	{}

	virtual bool isCompleted() { return bIsCompleted; }
	virtual void onTake(shared_ptr<ItemInstance> item, unsigned int invItemCountAnyAux, unsigned int invItemCountThisAux);

private:
	int m_itemId;
	unsigned int m_quantity;
	int m_auxValue;
};