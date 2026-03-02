#pragma once
using namespace std;

#include "Item.h"

class SaddleItem : public Item
{
public:
	SaddleItem(int id);

	virtual bool interactEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<Mob> mob);
	virtual bool hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<Mob> mob, shared_ptr<Mob> attacker);
};