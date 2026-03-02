#pragma once

#include "TutorialHint.h"

class DiggerItem;
class Level;

class DiggerItemHint : public TutorialHint
{
private:
	int *m_iItems;
	unsigned int m_iItemsCount;

public:
	DiggerItemHint(eTutorial_Hint id, Tutorial *tutorial, int descriptionId, int items[], unsigned int itemsLength);
	virtual int startDestroyBlock(shared_ptr<ItemInstance> item, Tile *tile);
	virtual int attack(shared_ptr<ItemInstance> item, shared_ptr<Entity> entity);
};