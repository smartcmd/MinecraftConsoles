#pragma once

class MerchantRecipe
{
private:
	shared_ptr<ItemInstance> buyA;
	shared_ptr<ItemInstance> buyB;
	shared_ptr<ItemInstance> sell;
	int uses;
	int maxUses;

	void _init(shared_ptr<ItemInstance> buyA, shared_ptr<ItemInstance> buyB, shared_ptr<ItemInstance> sell);

public:
	MerchantRecipe(CompoundTag *tag);
	MerchantRecipe(shared_ptr<ItemInstance> buyA, shared_ptr<ItemInstance> buyB, shared_ptr<ItemInstance> sell, int uses = 0, int maxUses = 7);
	MerchantRecipe(shared_ptr<ItemInstance> buy, shared_ptr<ItemInstance> sell);
	MerchantRecipe(shared_ptr<ItemInstance> buy, Item *sell);
	MerchantRecipe(shared_ptr<ItemInstance> buy, Tile *sell);

	shared_ptr<ItemInstance> getBuyAItem();
	shared_ptr<ItemInstance> getBuyBItem();
	bool hasSecondaryBuyItem();
	shared_ptr<ItemInstance> getSellItem();
	bool isSame(MerchantRecipe *other);
	bool isSameSameButBetter(MerchantRecipe *other);
	int getUses();
	int getMaxUses();
	void increaseUses();
	void increaseMaxUses(int amount);
	bool isDeprecated();
	void enforceDeprecated();
	void load(CompoundTag *tag);
	CompoundTag *createTag();
};