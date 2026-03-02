#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.h"
#include "DiggerItem.h"

DiggerItem::DiggerItem(int id, float attackDamage, const Tier *tier, TileArray *tiles) : Item( id ), tier( tier )
{
	//this->tier = tier;
	this->tiles = tiles;
	maxStackSize = 1;
	setMaxDamage(tier->getUses());
	this->speed = tier->getSpeed();
	this->attackDamage = attackDamage + tier->getAttackDamageBonus();
}

float DiggerItem::getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile *tile)
{
	for (unsigned int i = 0; i < tiles->length; i++)
		if ( (*tiles)[i] == tile) return speed;
	return 1;
}

bool DiggerItem::hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker)
{
	itemInstance->hurtAndBreak(2, attacker);
	return true;
}

bool DiggerItem::mineBlock(shared_ptr<ItemInstance> itemInstance, Level *level, int tile, int x, int y, int z, shared_ptr<LivingEntity> owner)
{
	// Don't damage tools if the tile can be destroyed in one hit.
	if (Tile::tiles[tile]->getDestroySpeed(level, x, y, z) != 0.0) itemInstance->hurtAndBreak(1, owner);
	return true;
}

bool DiggerItem::isHandEquipped()
{
	return true;
}

int DiggerItem::getEnchantmentValue()
{
	return tier->getEnchantmentValue();
}

const Item::Tier *DiggerItem::getTier()
{
	return tier;
}

bool DiggerItem::isValidRepairItem(shared_ptr<ItemInstance> source, shared_ptr<ItemInstance> repairItem)
{
	if (tier->getTierItemId() == repairItem->id)
	{
		return true;
	}
	return Item::isValidRepairItem(source, repairItem);
}

attrAttrModMap *DiggerItem::getDefaultAttributeModifiers()
{
	attrAttrModMap *result = Item::getDefaultAttributeModifiers();

	(*result)[SharedMonsterAttributes::ATTACK_DAMAGE->getId()] = new AttributeModifier(eModifierId_ITEM_BASEDAMAGE, attackDamage, AttributeModifier::OPERATION_ADDITION);

	return result;
}