#pragma once

class ShapedRecipy : public Recipy 
{
private:
	int width, height, group;
	ItemInstance **recipeItems;
	ItemInstance *result;
	bool _keepTag;
public:
	const int resultId;

public:
	ShapedRecipy(int width, int height, ItemInstance **recipeItems, ItemInstance *result, int iGroup=Recipy::eGroupType_Decoration);

	virtual const ItemInstance *getResultItem();
	virtual const int getGroup();
	virtual bool matches(shared_ptr<CraftingContainer> craftSlots, Level *level);

private:
	bool matches(shared_ptr<CraftingContainer> craftSlots, int xOffs, int yOffs, bool xFlip);

public:
	virtual shared_ptr<ItemInstance> assemble(shared_ptr<CraftingContainer> craftSlots);
	virtual int size();
	ShapedRecipy *keepTag();

	// 4J-PB - to return the items required to make a recipe
	virtual bool reqs(int iRecipe);
	virtual void reqs(INGREDIENTS_REQUIRED *pIngReq);
};

