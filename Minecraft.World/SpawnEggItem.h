#pragma once

#include "Item.h"

class SpawnEggItem : public Item
{
private:
	static const int SPAWN_COUNT = 1;

	Icon *overlay;

public:

	enum _eSpawnResult
	{
		eSpawnResult_OK=0,
		eSpawnResult_FailTooManyPigsCowsSheepCats,
		eSpawnResult_FailTooManyChickens,
		eSpawnResult_FailTooManySquid,
		eSpawnResult_FailTooManyBats,
		eSpawnResult_FailTooManyWolves,
		eSpawnResult_FailTooManyMooshrooms,
		eSpawnResult_FailTooManyAnimals,
		eSpawnResult_FailTooManyMonsters,
		eSpawnResult_FailTooManyVillagers,
		eSpawnResult_FailCantSpawnInPeaceful,
	};

	SpawnEggItem(int id);

	virtual wstring getHoverName(shared_ptr<ItemInstance> itemInstance);
	virtual int getColor(shared_ptr<ItemInstance> item, int spriteLayer);
	virtual bool hasMultipleSpriteLayers();
	virtual Icon *getLayerIcon(int auxValue, int spriteLayer);
	virtual bool useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);

	static shared_ptr<Entity> spawnMobAt(Level *level, int mobId, double x, double y, double z, int *piResult); // 4J Added piResult param

	// 4J-PB added for dispenser
	static shared_ptr<Entity> canSpawn(int iAuxVal, Level *level, int *piResult);

	// 4J: Added for neatness
	static void DisplaySpawnError(shared_ptr<Player> player, int result);

	//@Override
	void registerIcons(IconRegister *iconRegister);
};