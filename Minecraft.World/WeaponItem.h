#pragma once
using namespace std;

#include "Item.h"

class WeaponItem : public Item
{
private:
	float damage;
	const Tier *tier;

public:
	WeaponItem(int id, const Tier *tier);
	virtual float getTierDamage();
	virtual float getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile *tile);
	virtual bool hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker);
	virtual bool mineBlock(shared_ptr<ItemInstance> itemInstance, Level *level, int tile, int x, int y, int z, shared_ptr<LivingEntity> owner);
	virtual bool isHandEquipped();
	virtual UseAnim getUseAnimation(shared_ptr<ItemInstance> itemInstance);
	virtual int getUseDuration(shared_ptr<ItemInstance> itemInstance);
	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual bool canDestroySpecial(Tile *tile);
	virtual int getEnchantmentValue();

	const Tier *getTier();
	bool isValidRepairItem(shared_ptr<ItemInstance> source, shared_ptr<ItemInstance> repairItem);
	attrAttrModMap *getDefaultAttributeModifiers();
};