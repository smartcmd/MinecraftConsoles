#pragma once

#include "Item.h"

class EnchantmentInstance;

class EnchantedBookItem : public Item
{
public:
	static const wstring TAG_STORED_ENCHANTMENTS;

	EnchantedBookItem(int id);

	bool isFoil(shared_ptr<ItemInstance> itemInstance);
	bool isEnchantable(shared_ptr<ItemInstance> itemInstance);
	const Rarity *getRarity(shared_ptr<ItemInstance> itemInstance);
	ListTag<CompoundTag> *getEnchantments(shared_ptr<ItemInstance> item);
	void appendHoverText(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, vector<wstring> *lines, bool advanced, vector<wstring> &unformattedStrings);
	void addEnchantment(shared_ptr<ItemInstance> item, EnchantmentInstance *enchantment);
	shared_ptr<ItemInstance> createForEnchantment(EnchantmentInstance *enchant);
	void createForEnchantment(Enchantment *enchant, vector<shared_ptr<ItemInstance> > *items);
	shared_ptr<ItemInstance> createForRandomLoot(Random *random);
	WeighedTreasure *createForRandomTreasure(Random *random);
	WeighedTreasure *createForRandomTreasure(Random *random, int minCount, int maxCount, int weight);
};