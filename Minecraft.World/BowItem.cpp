#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.h"
#include "BowItem.h"
#include "SoundTypes.h"

const wstring BowItem::TEXTURE_PULL[] = {L"bow_pull_0", L"bow_pull_1", L"bow_pull_2"};

BowItem::BowItem(int id) : Item( id )
{
	maxStackSize = 1;
	setMaxDamage(384);

	icons = nullptr;
}

void BowItem::releaseUsing(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, int durationLeft)
{
	bool infiniteArrows = player->abilities.instabuild || EnchantmentHelper::getEnchantmentLevel(Enchantment::arrowInfinite->id, itemInstance) > 0;

	if (infiniteArrows || player->inventory->hasResource(Item::arrow_Id))
	{
		int timeHeld = getUseDuration(itemInstance) - durationLeft;
		float pow = timeHeld / static_cast<float>(MAX_DRAW_DURATION);
		pow = ((pow * pow) + pow * 2) / 3;
		if (pow < 0.1) return;
		if (pow > 1) pow = 1;

		shared_ptr<Arrow> arrow = std::make_shared<Arrow>(level, player, pow * 2.0f);
		if (pow == 1) arrow->setCritArrow(true);
		int damageBonus = EnchantmentHelper::getEnchantmentLevel(Enchantment::arrowBonus->id, itemInstance);
		if (damageBonus > 0)
		{
			arrow->setBaseDamage(arrow->getBaseDamage() + static_cast<double>(damageBonus) * .5 + .5);
		}
		int knockbackBonus = EnchantmentHelper::getEnchantmentLevel(Enchantment::arrowKnockback->id, itemInstance);
		if (knockbackBonus > 0)
		{
			arrow->setKnockback(knockbackBonus);
		}
		if (EnchantmentHelper::getEnchantmentLevel(Enchantment::arrowFire->id, itemInstance) > 0)
		{
			arrow->setOnFire(100);
		}
		itemInstance->hurtAndBreak(1, player);

		level->playEntitySound(player, eSoundType_RANDOM_BOW, 1.0f, 1 / (random->nextFloat() * 0.4f + 1.2f) + pow * 0.5f);

		if (infiniteArrows)
		{
			arrow->pickup = Arrow::PICKUP_CREATIVE_ONLY;
		}
		else
		{
			player->inventory->removeResource(Item::arrow_Id);
		}
		if (!level->isClientSide) level->addEntity(arrow);
	}
}

shared_ptr<ItemInstance> BowItem::useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	return instance;
}

int BowItem::getUseDuration(shared_ptr<ItemInstance> itemInstance)
{
	return 20 * 60 * 60;
}

UseAnim BowItem::getUseAnimation(shared_ptr<ItemInstance> itemInstance)
{
	return UseAnim_bow;
}

shared_ptr<ItemInstance> BowItem::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	if (player->abilities.instabuild || player->inventory->hasResource(Item::arrow_Id))
	{
		player->startUsingItem(instance, getUseDuration(instance));
	}
	return instance;
}

int BowItem::getEnchantmentValue()
{
	return 1;
}

void BowItem::registerIcons(IconRegister *iconRegister)
{
	Item::registerIcons(iconRegister);

	icons = new Icon *[BOW_ICONS_COUNT];

	for (int i = 0; i < BOW_ICONS_COUNT; i++)
	{
		icons[i] = iconRegister->registerIcon(TEXTURE_PULL[i]);
	}
}

Icon *BowItem::getDrawnIcon(int amount)
{
	return icons[amount];
}