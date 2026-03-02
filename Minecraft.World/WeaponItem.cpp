#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.level.tile.h"
#include "WeaponItem.h"

WeaponItem::WeaponItem(int id, const Tier *tier) : Item(id), tier( tier )
{
	maxStackSize = 1;
	setMaxDamage(tier->getUses());

	damage = 4 + tier->getAttackDamageBonus();
}

float WeaponItem::getTierDamage()
{
	return tier->getAttackDamageBonus();
}

float WeaponItem::getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile *tile)
{
	if (tile->id == Tile::web_Id)
	{
		// swords can quickly cut web
		return 15;
	}
	// this change modifies which tiles the swords can destroy in creative
	// mode (>1 == yes)
	Material *material = tile->material;
	if (material == Material::plant || material == Material::replaceable_plant || material == Material::coral || material == Material::leaves || material == Material::vegetable)
	{
		return 1.5f;
	}
	return 1.0f;
}

bool WeaponItem::hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker) 
{
	itemInstance->hurtAndBreak(1, attacker);
	return true;
}

bool WeaponItem::mineBlock(shared_ptr<ItemInstance> itemInstance, Level *level, int tile, int x, int y, int z, shared_ptr<LivingEntity> owner)
{
	// Don't damage weapons if the tile can be destroyed in one hit.
	if (Tile::tiles[tile]->getDestroySpeed(level, x, y, z) != 0.0) itemInstance->hurtAndBreak(2, owner);
	return true;
}

bool WeaponItem::isHandEquipped()
{
	return true;
}

UseAnim WeaponItem::getUseAnimation(shared_ptr<ItemInstance> itemInstance)
{
	return UseAnim_block;
}    

int WeaponItem::getUseDuration(shared_ptr<ItemInstance> itemInstance)
{
	return 20 * 60 * 60; // Block for a maximum of one hour!
}

shared_ptr<ItemInstance> WeaponItem::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	player->startUsingItem(instance, getUseDuration(instance));
	return instance;
}

bool WeaponItem::canDestroySpecial(Tile *tile)
{
	return tile->id == Tile::web_Id;
}

int WeaponItem::getEnchantmentValue()
{
	return tier->getEnchantmentValue();
}

const Item::Tier *WeaponItem::getTier()
{
	return tier;
}

bool WeaponItem::isValidRepairItem(shared_ptr<ItemInstance> source, shared_ptr<ItemInstance> repairItem)
{
	if (tier->getTierItemId() == repairItem->id)
	{
		return true;
	}
	return Item::isValidRepairItem(source, repairItem);
}

attrAttrModMap *WeaponItem::getDefaultAttributeModifiers()
{
	attrAttrModMap *result = Item::getDefaultAttributeModifiers();

	result->insert(attrAttrModMap::value_type( SharedMonsterAttributes::ATTACK_DAMAGE->getId(), new AttributeModifier(eModifierId_ITEM_BASEDAMAGE, damage, AttributeModifier::OPERATION_ADDITION) ) );

	return result;
}