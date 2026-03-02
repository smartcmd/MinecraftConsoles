#pragma once
#include <vector>

class MerchantRecipe;
class CompoundTag;
class ItemInstance;
class DataOutputStream;
class DataInputStream;

class MerchantRecipeList
{
private:
	std::vector<MerchantRecipe *> m_recipes;

public:
	MerchantRecipeList();
	MerchantRecipeList(CompoundTag *tag);
	~MerchantRecipeList();

	MerchantRecipe *getRecipeFor(shared_ptr<ItemInstance> buyA, shared_ptr<ItemInstance> buyB, int selectionHint);
	bool addIfNewOrBetter(MerchantRecipe *recipe); // 4J Added bool return
	MerchantRecipe *getMatchingRecipeFor(shared_ptr<ItemInstance> buy, shared_ptr<ItemInstance> buyB, shared_ptr<ItemInstance> sell);
	void writeToStream(DataOutputStream *stream);
	static MerchantRecipeList *createFromStream(DataInputStream *stream);
	void load(CompoundTag *tag);
	CompoundTag *createTag();

	void push_back(MerchantRecipe *recipe);
	MerchantRecipe *at(size_t index);
	std::vector<MerchantRecipe *>::iterator begin();
	std::vector<MerchantRecipe *>::iterator end();
	std::vector<MerchantRecipe *>::iterator erase(std::vector<MerchantRecipe *>::iterator it);
	size_t size();
	bool empty();
};