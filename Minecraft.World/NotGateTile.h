#pragma once
#include "TorchTile.h"
using namespace std;

class Random;

class NotGateTile : public TorchTile
{
	friend class Tile;

private:
	static const int RECENT_TOGGLE_TIMER = 20 * 3;
    static const int MAX_RECENT_TOGGLES = 8;

    bool on;

public:
    class Toggle
	{
	public:
        int x, y, z;
        __int64 when;

        Toggle(int x, int y, int z, __int64 when)
		{
            this->x = x;
            this->y = y;
            this->z = z;
            this->when = when;
        }
    };

private:
	static unordered_map<Level *, deque<Toggle> *> recentToggles;		// 4J - brought forward change from 1.3.2
public:
	static void removeLevelReferences(Level *level);				// 4J added
private:
    bool isToggledTooFrequently(Level *level, int x, int y, int z, bool add);

protected:
	NotGateTile(int id, bool on);

public:
	int getTickDelay();
	void onPlace(Level *level, int x, int y, int z);
	void onRemove(Level *level, int x, int y, int z, int id, int data);
	bool getSignal(LevelSource *level, int x, int y, int z, int face);

private:
	bool hasNeighborSignal(Level *level, int x, int y, int z);

public:
    void tick(Level *level, int x, int y, int z, Random *random);
    void neighborChanged(Level *level, int x, int y, int z, int type);

    bool getDirectSignal(Level *level, int x, int y, int z, int face);

    int getResource(int data, Random *random, int playerBonusLevel);
    bool isSignalSource();

public:
	void animateTick(Level *level, int xt, int yt, int zt, Random *random);
	int cloneTileId(Level *level, int x, int y, int z);
    void levelTimeChanged(Level *level, __int64 delta, __int64 newTime);

	void registerIcons(IconRegister *iconRegister);
};