#pragma once
#include "Tile.h"

class Random;

class StoneMonsterTile : public Tile
{
public:
	static const int HOST_ROCK = 0;
    static const int HOST_COBBLE = 1;
    static const int HOST_STONEBRICK = 2;

	static const int STONE_MONSTER_NAMES_LENGTH = 3;

	static const unsigned int STONE_MONSTER_NAMES[STONE_MONSTER_NAMES_LENGTH];

	// 4J Stu - I don't know why this is protected in Java
//protected:
public:
    StoneMonsterTile(int id);
public:
    virtual Icon *getTexture(int face, int data);
	void registerIcons(IconRegister *iconRegister);
    virtual void destroy(Level *level, int x, int y, int z, int data);
    virtual int getResourceCount(Random *random);

    static bool isCompatibleHostBlock(int block);
    static int getDataForHostBlock(int block);
	static Tile *getHostBlockForData(int data);

	virtual unsigned int getDescriptionId(int iData = -1);

protected:
	virtual shared_ptr<ItemInstance> getSilkTouchItemInstance(int data);

public:
	int cloneTileData(Level *level, int x, int y, int z);
};