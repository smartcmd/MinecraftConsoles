#pragma once

class ItemInstance;
class Inventory;
class DamageSource;
class Enchantment;
class EnchantmentInstance;

class EnchantmentHelper
{
private:
	static Random random;

public:
	static int getEnchantmentLevel(int enchantmentId, shared_ptr<ItemInstance> piece);
	static unordered_map<int, int> *getEnchantments(shared_ptr<ItemInstance> item);
	static void setEnchantments(unordered_map<int, int> *enchantments, shared_ptr<ItemInstance> item);

	static int getEnchantmentLevel(int enchantmentId, ItemInstanceArray inventory);

private:


	class EnchantmentIterationMethod
	{
	public:
		virtual void doEnchantment(Enchantment *enchantment, int level) = 0;
	};

	static void runIterationOnItem(EnchantmentIterationMethod &method, shared_ptr<ItemInstance> piece);
	static void runIterationOnInventory(EnchantmentIterationMethod &method, ItemInstanceArray inventory);

	class GetDamageProtectionIteration : public EnchantmentIterationMethod
	{
	public:
		int sum;
		DamageSource *source;

		virtual void doEnchantment(Enchantment *enchantment, int level);
	};

	static GetDamageProtectionIteration getDamageProtectionIteration;

	/**
	* Fetches the protection value for enchanted items.
	* 
	* @param inventory
	* @param source
	* @return
	*/
public:
	static int getDamageProtection(ItemInstanceArray armor, DamageSource *source);

private:
	class GetDamageBonusIteration : public EnchantmentIterationMethod
	{
	public:
		float sum;
		shared_ptr<LivingEntity> target;

		virtual void doEnchantment(Enchantment *enchantment, int level);
	};

	static GetDamageBonusIteration getDamageBonusIteration;

	/**
	* 
	* @param inventory
	* @param target
	* @return
	*/
public:
	static float getDamageBonus(shared_ptr<LivingEntity> source, shared_ptr<LivingEntity> target);
	static int getKnockbackBonus(shared_ptr<LivingEntity> source, shared_ptr<LivingEntity> target);
	static int getFireAspect(shared_ptr<LivingEntity> source);
	static int getOxygenBonus(shared_ptr<LivingEntity> source);
	static int getDiggingBonus(shared_ptr<LivingEntity> source);
	static int getDigDurability(shared_ptr<LivingEntity> source);
	static bool hasSilkTouch(shared_ptr<LivingEntity> source);
	static int getDiggingLootBonus(shared_ptr<LivingEntity> source);
	static int getKillingLootBonus(shared_ptr<LivingEntity> source);
	static bool hasWaterWorkerBonus(shared_ptr<LivingEntity> source);
	static int getArmorThorns(shared_ptr<LivingEntity> source);
	static shared_ptr<ItemInstance> getRandomItemWith(Enchantment *enchantment, shared_ptr<LivingEntity> source);

	/**
	* 
	* @param random
	* @param slot
	*            The table slot, 0-2
	* @param bookcases
	*            How many book cases that are found around the table.
	* @param itemInstance
	*            Which item that is being enchanted.
	* @return The enchantment cost, 0 means unchantable, 50 is max.
	*/
	static int getEnchantmentCost(Random *random, int slot, int bookcases, shared_ptr<ItemInstance> itemInstance);

	static shared_ptr<ItemInstance> enchantItem(Random *random, shared_ptr<ItemInstance> itemInstance, int enchantmentCost);

	/**
	* 
	* @param random
	* @param itemInstance
	* @param enchantmentCost
	* @return
	*/
	static vector<EnchantmentInstance *> *selectEnchantment(Random *random, shared_ptr<ItemInstance> itemInstance, int enchantmentCost);
	static unordered_map<int, EnchantmentInstance *> *getAvailableEnchantmentResults(int value, shared_ptr<ItemInstance> itemInstance);
};