#pragma once
#include "Tile.h"
#include "Definitions.h"

class Random;
class ChunkRebuildData;
class FireTile : public Tile
{
	friend class Tile;
	friend class ChunkRebuildData;
public:
	static const wstring TEXTURE_FIRST;
	static const wstring TEXTURE_SECOND;

	static const int FLAME_INSTANT = 60;
	static const int FLAME_EASY = 30;
	static const int FLAME_MEDIUM = 15;
	static const int FLAME_HARD = 5;

	static const int BURN_INSTANT = 100;
	static const int BURN_EASY = 60;
	static const int BURN_MEDIUM = 20;
	static const int BURN_HARD = 5;
	static const int BURN_NEVER = 0;

private:
	int *flameOdds;
	int *burnOdds;
	Icon **icons;
protected:
	FireTile(int id);
	virtual ~FireTile();
public:
	void init();
private:
	void setFlammable(int id, int flame, int burn);
public:
	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual bool blocksLight();
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool isCubeShaped();
	virtual int getRenderShape();
	virtual int getResourceCount(Random *random);
	virtual int getTickDelay(Level *level);
	virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual bool canInstantlyTick();

private:
	void checkBurnOut(Level *level, int x, int y, int z, int chance, Random *random, int age);
	bool isValidFireLocation(Level *level, int x, int y, int z);
	int getFireOdds(Level *level, int x, int y, int z);
public:
	virtual bool mayPick();
	bool canBurn(LevelSource *level, int x, int y, int z);
	int getFlammability(Level *level, int x, int y, int z, int odds);
	virtual bool mayPlace(Level *level, int x, int y, int z);
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual void onPlace(Level *level, int x, int y, int z);
	bool isFlammable(int tile);
	virtual void animateTick(Level *level, int x, int y, int z, Random *random);
public:
	void registerIcons(IconRegister *iconRegister);
	Icon *getTextureLayer(int layer);
	Icon *getTexture(int face, int data);
};