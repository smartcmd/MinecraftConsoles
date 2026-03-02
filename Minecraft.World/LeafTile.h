#pragma once
#include "TransparentTile.h"

class Random;
class ChunkRebuildData;

class LeafTile : public TransparentTile
{
	friend class Tile;
	friend class ChunkRebuildData;
public:
	static const wstring TEXTURES[2][4];
	static const int REQUIRED_WOOD_RANGE = 4;

    static const int UPDATE_LEAF_BIT = 8;
	static const int PERSISTENT_LEAF_BIT = 4;	// player-placed
    static const int NORMAL_LEAF = 0;
    static const int EVERGREEN_LEAF = 1;
    static const int BIRCH_LEAF = 2;
	static const int JUNGLE_LEAF = 3;

	static const int LEAF_NAMES_LENGTH = 4;

	static const unsigned int LEAF_NAMES[LEAF_NAMES_LENGTH];
private:
	static const int LEAF_TYPE_MASK = 3;

    // pppppppppp ppppppppppp pppppppppp ppppppp
    // ssssssssss sssssssssss s

    int fancyTextureSet;
	Icon *icons[2][4];

protected:
	LeafTile(int id);
	virtual ~LeafTile();
public:
	virtual int getColor() const;
	virtual int getColor(int data);

    virtual int getColor(LevelSource *level, int x, int y, int z);
	virtual int getColor(LevelSource *level, int x, int y, int z, int data);	// 4J added
    virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
    int *checkBuffer;

    virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual void animateTick(Level *level, int x, int y, int z, Random *random);
private:
	void die(Level *level, int x, int y, int z);
public:
	virtual int getResourceCount(Random *random);
    virtual int getResource(int data, Random *random, int playerBonusLevel);

	// 4J DCR: Brought forward from 1.2
	virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel);

	virtual void playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data);
protected:
	virtual int getSpawnResourcesAuxValue(int data);
public:
	virtual bool isSolidRender(bool isServerLevel = false);
    virtual Icon *getTexture(int face, int data);
    void setFancy(bool fancyGraphics);

protected:
	virtual shared_ptr<ItemInstance> getSilkTouchItemInstance(int data);

public:
    virtual void stepOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity);
	
	// 4J Added so we can check before we try to add a tile to the tick list if it's actually going to do seomthing
	virtual bool shouldTileTick(Level *level, int x,int y,int z);

	virtual unsigned int getDescriptionId(int iData = -1);
	void registerIcons(IconRegister *iconRegister);
};