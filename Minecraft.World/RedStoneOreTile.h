#pragma once
#include "Tile.h"

class Player;
class Random;

class RedStoneOreTile : public Tile
{
private:
	bool lit;
public:
	RedStoneOreTile(int id, bool lit);
    virtual int getTickDelay();
    virtual void attack(Level *level, int x, int y, int z, shared_ptr<Player> player);
    virtual void stepOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity);
	virtual bool TestUse();
    virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
private:
	virtual void interact(Level *level, int x, int y, int z);
public:
	virtual void tick(Level *level, int x, int y, int z, Random* random);
    virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual int getResourceCountForLootBonus(int bonusLevel, Random *random);
    virtual int getResourceCount(Random *random);
	virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel);
    virtual void animateTick(Level *level, int x, int y, int z, Random *random);
	
	// 4J Added so we can check before we try to add a tile to the tick list if it's actually going to do seomthing
	virtual bool shouldTileTick(Level *level, int x,int y,int z);
private:
	void poofParticles(Level *level, int x, int y, int z);
protected:
	virtual shared_ptr<ItemInstance> getSilkTouchItemInstance(int data);
};
