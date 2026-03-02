#pragma once

#include "EnchantmentCategory.h"

class DamageSource;
class Mob;

class Enchantment //implements Descriptive<Enchantment> {
{
public :
	//static Enchantment *enchantments[256];
	static EnchantmentArray enchantments;
	static vector<Enchantment *> validEnchantments;

	static const int FREQ_COMMON = 10;
	static const int FREQ_UNCOMMON = 5;
	static const int FREQ_RARE = 2;
	static const int FREQ_VERY_RARE = 1;

	// armor
	static Enchantment *allDamageProtection;
	static Enchantment *fireProtection;
	static Enchantment *fallProtection;
	static Enchantment *explosionProtection;
	static Enchantment *projectileProtection;
	static Enchantment *drownProtection;
	static Enchantment *waterWorker;
	static Enchantment *thorns;

	// weapon
	static Enchantment *damageBonus;
	static Enchantment *damageBonusUndead;
	static Enchantment *damageBonusArthropods;
	static Enchantment *knockback;
	static Enchantment *fireAspect;
	static Enchantment *lootBonus;

	// digger
	static Enchantment *diggingBonus;
	static Enchantment *untouching;
	static Enchantment *digDurability;
	static Enchantment *resourceBonus;

	// bows
	static Enchantment *arrowBonus;
	static Enchantment *arrowKnockback;
	static Enchantment *arrowFire;
	static Enchantment *arrowInfinite;

	const int id;

	static void staticCtor();

private:
	const int frequency;

public:
	const EnchantmentCategory *category;

protected:
	int descriptionId;

private:
	void _init(int id);

protected:
	Enchantment(int id, int frequency, const EnchantmentCategory *category);
	Enchantment(int id);

public:
	virtual int getFrequency();
	virtual int getMinLevel();
	virtual int getMaxLevel();
	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getDamageProtection(int level, DamageSource *source);
	virtual int getDamageBonus(int level, shared_ptr<Mob> target);
	virtual bool isCompatibleWith(Enchantment *other) const;
	virtual Enchantment *setDescriptionId(int id);
	virtual int getDescriptionId();
	virtual wstring getFullname(int level,wstring &unformatted); // 4J Stu added unformatted
	virtual bool canEnchant(shared_ptr<ItemInstance> item);

private:
	// 4J Added
	wstring getLevelString(int level);
};