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
	int attackDamage;

protected:
	const Tier *tier;

	DiggerItem(int id, int attackDamage, const Tier *tier, TileArray *tiles);

public:
	virtual float getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile *tile);
	virtual bool hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<Mob> mob, shared_ptr<Mob> attacker);
	virtual bool mineBlock(shared_ptr<ItemInstance> itemInstance, Level *level, int tile, int x, int y, int z, shared_ptr<Mob> owner);
	virtual int getAttackDamage(shared_ptr<Entity> entity);
	virtual bool isHandEquipped();
	virtual int getEnchantmentValue();

	const Tier *getTier();
	bool isValidRepairItem(shared_ptr<ItemInstance> source, shared_ptr<ItemInstance> repairItem);
};