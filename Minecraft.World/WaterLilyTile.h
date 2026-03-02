#pragma once
#include "Bush.h"

class WaterlilyTile : public Bush
{
private:
	// static const int col = 0x208030;

public:
	WaterlilyTile(int id);
    virtual void updateDefaultShape(); // 4J Added override

	virtual int getRenderShape();
	virtual void addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source);
    virtual AABB *getAABB(Level *level, int x, int y, int z);
    virtual int getColor() const;
    virtual int getColor(int auxData);
    virtual int getColor(LevelSource *level, int x, int y, int z);
	virtual int getColor(LevelSource *level, int x, int y, int z, int data); // 4J added
protected:
	virtual bool mayPlaceOn(int tile);
    virtual bool canSurvive(Level *level, int x, int y, int z);
    bool growTree(Level *level, int x, int y, int z, Random *random);
};
