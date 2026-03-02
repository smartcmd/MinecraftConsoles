#pragma once
#include "GameMode.h"

class CreativeMode : public GameMode
{
private:
	int destroyDelay;

public:
	CreativeMode(Minecraft *minecraft);
	virtual void init();
	static void enableCreativeForPlayer(shared_ptr<Player> player);
	static void disableCreativeForPlayer(shared_ptr<Player> player);
	virtual void adjustPlayer(shared_ptr<Player> player);
	static void creativeDestroyBlock(Minecraft *minecraft, GameMode *gameMode, int x, int y, int z, int face);
	virtual bool useItemOn(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, int x, int y, int z, int face, bool bTestUseOnOnly=false, bool *pbUsedItem = NULL);
	virtual void startDestroyBlock(int x, int y, int z, int face);
	virtual void continueDestroyBlock(int x, int y, int z, int face);
	virtual void stopDestroyBlock();
	virtual bool canHurtPlayer();
	virtual void initLevel(Level *level);
	virtual float getPickRange();
	virtual bool hasMissTime();
	virtual bool hasInfiniteItems();
	virtual bool hasFarPickRange();
};