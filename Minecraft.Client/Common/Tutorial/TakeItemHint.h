#pragma once
using namespace std;

#include "TutorialHint.h"

class ItemInstance;

class TakeItemHint : public TutorialHint
{
private:
	int *m_iItems;
	unsigned int m_iItemsCount;

public:
	TakeItemHint(eTutorial_Hint id, Tutorial *tutorial, int items[], unsigned int itemsLength);
	~TakeItemHint();

	virtual bool onTake( shared_ptr<ItemInstance> item );
};