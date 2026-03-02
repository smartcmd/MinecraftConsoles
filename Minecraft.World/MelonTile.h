#pragma once
#include "Tile.h"

class ChunkRebuildData;
class MelonTile : public Tile
{
	friend class ChunkRebuildData;
private:
	Icon *iconTop;

	// 4J Stu - I don't know why this is protected in Java
	//protected:
public:
	MelonTile(int id);
public:
	virtual Icon *getTexture(int face, int data);
	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual int getResourceCount(Random *random);
	virtual int getResourceCountForLootBonus(int bonusLevel, Random *random);
	void registerIcons(IconRegister *iconRegister);
};