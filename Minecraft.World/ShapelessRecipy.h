#pragma once

class ShapelessRecipy : public Recipy 
{
private:
	_eGroupType group;
	const ItemInstance *result;
	vector<ItemInstance *> *ingredients;

public:
	ShapelessRecipy(ItemInstance *result, vector<ItemInstance *> *ingredients, _eGroupType egroup=Recipy::eGroupType_Decoration);

	virtual const ItemInstance *getResultItem();
	virtual const int getGroup();
	virtual bool matches(shared_ptr<CraftingContainer> craftSlots, Level *level);
	virtual shared_ptr<ItemInstance> assemble(shared_ptr<CraftingContainer> craftSlots);
	virtual int size();

	// 4J-PB - to return the items required to make a recipe
	virtual bool reqs(int iRecipe);
	virtual void reqs(INGREDIENTS_REQUIRED *pIngReq); 

};
