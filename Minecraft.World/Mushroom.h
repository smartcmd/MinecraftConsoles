#pragma once
#include "Bush.h"

class Random;

class Mushroom : public Bush
{
	friend class Tile;
protected:
	Mushroom(int id);
public:
	virtual void updateDefaultShape(); // 4J Added override
	virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual bool mayPlace(Level *level, int x, int y, int z);
protected:
	virtual bool mayPlaceOn(int tile);
public:
	virtual bool canSurvive(Level *level, int x, int y, int z);
	bool growTree(Level *level, int x, int y, int z, Random *random);
};
