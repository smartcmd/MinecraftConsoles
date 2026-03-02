#pragma once

#include "Recipy.h"

class ArmorDyeRecipe : public Recipy
{
public:
	bool matches(shared_ptr<CraftingContainer> craftSlots, Level *level);

	// 4J Stu - Made static as we use this in a different way from the Java (but needs to be a different name otherwise Orbis compiler complains
	static shared_ptr<ItemInstance> assembleDyedArmor(shared_ptr<CraftingContainer> craftSlots);
	shared_ptr<ItemInstance> assemble(shared_ptr<CraftingContainer> craftSlots);

	int size();
	const ItemInstance *getResultItem();

	
	virtual const int getGroup();		

	// 4J-PB
	virtual bool requires(int iRecipe);
	virtual void requires(INGREDIENTS_REQUIRED *pIngReq);
};