#include "stdafx.h"
#include "net.minecraft.world.item.enchantment.h"
#include "Enchantment.h"

//Enchantment *Enchantment::enchantments[256];
EnchantmentArray Enchantment::enchantments = EnchantmentArray( 256 );
vector<Enchantment *> Enchantment::validEnchantments;

Enchantment *Enchantment::allDamageProtection = NULL;
Enchantment *Enchantment::fireProtection = NULL;
Enchantment *Enchantment::fallProtection = NULL;
Enchantment *Enchantment::explosionProtection = NULL;
Enchantment *Enchantment::projectileProtection = NULL;
Enchantment *Enchantment::drownProtection = NULL;
Enchantment *Enchantment::waterWorker = NULL;
Enchantment *Enchantment::thorns = NULL;

// weapon
Enchantment *Enchantment::damageBonus = NULL;
Enchantment *Enchantment::damageBonusUndead = NULL;
Enchantment *Enchantment::damageBonusArthropods = NULL;
Enchantment *Enchantment::knockback = NULL;
Enchantment *Enchantment::fireAspect = NULL;
Enchantment *Enchantment::lootBonus = NULL;

// digger
Enchantment *Enchantment::diggingBonus = NULL;
Enchantment *Enchantment::untouching = NULL;
Enchantment *Enchantment::digDurability = NULL;
Enchantment *Enchantment::resourceBonus = NULL;

// bows
Enchantment *Enchantment::arrowBonus = NULL;
Enchantment *Enchantment::arrowKnockback = NULL;
Enchantment *Enchantment::arrowFire = NULL;
Enchantment *Enchantment::arrowInfinite = NULL;

void Enchantment::staticCtor()
{
	allDamageProtection = new ProtectionEnchantment(0, FREQ_COMMON, ProtectionEnchantment::ALL);
	fireProtection = new ProtectionEnchantment(1, FREQ_UNCOMMON, ProtectionEnchantment::FIRE);
	fallProtection = new ProtectionEnchantment(2, FREQ_UNCOMMON, ProtectionEnchantment::FALL);
	explosionProtection = new ProtectionEnchantment(3, FREQ_RARE, ProtectionEnchantment::EXPLOSION);
	projectileProtection = new ProtectionEnchantment(4, FREQ_UNCOMMON, ProtectionEnchantment::PROJECTILE);
	drownProtection = new OxygenEnchantment(5, FREQ_RARE);
	waterWorker = new WaterWorkerEnchantment(6, FREQ_RARE);
	thorns = new ThornsEnchantment(7, FREQ_VERY_RARE);

	// weapon
	damageBonus = new DamageEnchantment(16, FREQ_COMMON, DamageEnchantment::ALL);
	damageBonusUndead = new DamageEnchantment(17, FREQ_UNCOMMON, DamageEnchantment::UNDEAD);
	damageBonusArthropods = new DamageEnchantment(18, FREQ_UNCOMMON, DamageEnchantment::ARTHROPODS);
	knockback = new KnockbackEnchantment(19, FREQ_UNCOMMON);
	fireAspect = new FireAspectEnchantment(20, FREQ_RARE);
	lootBonus = new LootBonusEnchantment(21, FREQ_RARE, EnchantmentCategory::weapon);

	// digger
	diggingBonus = new DiggingEnchantment(32, FREQ_COMMON);
	untouching = new UntouchingEnchantment(33, FREQ_VERY_RARE);
	digDurability = new DigDurabilityEnchantment(34, FREQ_UNCOMMON);
	resourceBonus = new LootBonusEnchantment(35, FREQ_RARE, EnchantmentCategory::digger);

	// bows
	arrowBonus = new ArrowDamageEnchantment(48, FREQ_COMMON);
	arrowKnockback = new ArrowKnockbackEnchantment(49, FREQ_RARE);
	arrowFire = new ArrowFireEnchantment(50, FREQ_RARE);
	arrowInfinite = new ArrowInfiniteEnchantment(51, FREQ_VERY_RARE);

	for(unsigned int i = 0; i < 256; ++i)
	{
		Enchantment *enchantment = enchantments[i];
		if (enchantment != NULL)
		{
			validEnchantments.push_back(enchantment);
		}
	}
}

void Enchantment::_init(int id)
{
	if (enchantments[id] != NULL)
	{
		app.DebugPrintf("Duplicate enchantment id!");
#ifndef _CONTENT_PACKAGE
		__debugbreak();
#endif
		//throw new IllegalArgumentException("Duplicate enchantment id!");
	}
	enchantments[id] = this;
}

Enchantment::Enchantment(int id, int frequency, const EnchantmentCategory *category) : id(id), frequency(frequency), category(category)
{
	_init(id);
}

Enchantment::Enchantment(int id) : id(id), frequency(FREQ_COMMON), category(EnchantmentCategory::all)
{
	_init(id);
}

int Enchantment::getFrequency()
{
	return frequency;
}

int Enchantment::getMinLevel()
{
	return 1;
}

int Enchantment::getMaxLevel()
{
	return 1;
}

int Enchantment::getMinCost(int level)
{
	return 1 + level * 10;
}

int Enchantment::getMaxCost(int level)
{
	return getMinCost(level) + 5;
}

int Enchantment::getDamageProtection(int level, DamageSource *source)
{
	return 0;
}

int Enchantment::getDamageBonus(int level, shared_ptr<Mob> target)
{
	return 0;
}

bool Enchantment::isCompatibleWith(Enchantment *other) const
{
	return this != other;
}

Enchantment *Enchantment::setDescriptionId(int id)
{
	descriptionId = id;
	return this;
}

int Enchantment::getDescriptionId()
{
	return descriptionId;
}

wstring Enchantment::getFullname(int level,wstring &unformatted)
{
	wchar_t formatted[256];
	swprintf(formatted,256,L"%ls %ls",app.GetString( getDescriptionId() ), getLevelString(level).c_str());
	unformatted = formatted;	
	swprintf(formatted,256,L"<font color=\"#%08x\">%ls</font>",app.GetHTMLColour(eHTMLColor_f),unformatted.c_str());
	return formatted;
}

bool Enchantment::canEnchant(shared_ptr<ItemInstance> item)
{
	return category->canEnchant(item->getItem());
}

// 4J Added
wstring Enchantment::getLevelString(int level)
{
	int stringId = IDS_ENCHANTMENT_LEVEL_1;
	switch(level)
	{
	case 2:
		stringId = IDS_ENCHANTMENT_LEVEL_2;
		break;
	case 3:
		stringId = IDS_ENCHANTMENT_LEVEL_3;
		break;
	case 4:
		stringId = IDS_ENCHANTMENT_LEVEL_4;
		break;
	case 5:
		stringId = IDS_ENCHANTMENT_LEVEL_5;
		break;
	case 6:
		stringId = IDS_ENCHANTMENT_LEVEL_6;
		break;
	case 7:
		stringId = IDS_ENCHANTMENT_LEVEL_7;
		break;
	case 8:
		stringId = IDS_ENCHANTMENT_LEVEL_8;
		break;
	case 9:
		stringId = IDS_ENCHANTMENT_LEVEL_9;
		break;
	case 10:
		stringId = IDS_ENCHANTMENT_LEVEL_10;
		break;
	};
	return app.GetString(stringId); //I18n.get("enchantment.level." + level);
}