#pragma once
using namespace std;

#include "Item.h"

class SaddleItem : public Item
{
public:
	SaddleItem(int id);

	virtual bool interactEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, shared_ptr<LivingEntity> mob);
	virtual bool hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker);
};