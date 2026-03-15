#pragma once

#include "Recipy.h"

class FireworksRecipe : public Recipy
{
private:
	shared_ptr<ItemInstance> resultItem;

	void setResultItem(shared_ptr<ItemInstance> item);
public:
	FireworksRecipe();

	bool matches(shared_ptr<CraftingContainer> craftSlots, Level *level);
	shared_ptr<ItemInstance> assemble(shared_ptr<CraftingContainer> craftSlots);
	int size();
	const ItemInstance *getResultItem();

	
	virtual const int getGroup() { return 0; }		

	// 4J-PB
	virtual bool reqs(int iRecipe) { return false; };
	virtual void reqs(INGREDIENTS_REQUIRED *pIngReq) {};

	// 4J Added
	static void updatePossibleRecipes(shared_ptr<CraftingContainer> craftSlots, bool *firework, bool *charge, bool *fade);
	static bool isValidIngredient(shared_ptr<ItemInstance> item, bool firework, bool charge, bool fade);
};