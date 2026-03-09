#pragma once

#include "Recipy.h"

class FireworksRecipe : public Recipy
{
private:
	//shared_ptr<ItemInstance> resultItem;

		// 4J added so we can have separate contexts and rleBuf for different threads
	class ThreadStorage
	{
	public:
		shared_ptr<ItemInstance> resultItem;
		ThreadStorage();
	};
	static DWORD tlsIdx;
	static ThreadStorage *tlsDefault;

	void setResultItem(shared_ptr<ItemInstance> item);
public:
	// Each new thread that needs to use Compression will need to call one of the following 2 functions, to either create its own
	// local storage, or share the default storage already allocated by the main thread
	static void CreateNewThreadStorage();
	static void UseDefaultThreadStorage();
	static void ReleaseThreadStorage();

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