#pragma once

#include "Item.h"

class Mob;

class DiggerItem : public Item
{
private:
	TileArray *tiles;
protected:
	float speed;
private:
	float attackDamage;

protected:
	const Tier *tier;

	DiggerItem(int id, float attackDamage, const Tier *tier, TileArray *tiles);

public:
	virtual float getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile *tile);
	virtual bool hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker);
	virtual bool mineBlock(shared_ptr<ItemInstance> itemInstance, Level *level, int tile, int x, int y, int z, shared_ptr<LivingEntity> owner);
	virtual bool isHandEquipped();
	virtual int getEnchantmentValue();

	const Tier *getTier();
	bool isValidRepairItem(shared_ptr<ItemInstance> source, shared_ptr<ItemInstance> repairItem);
	virtual attrAttrModMap *getDefaultAttributeModifiers();
};