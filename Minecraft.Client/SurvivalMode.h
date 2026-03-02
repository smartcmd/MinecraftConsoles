#pragma once
#include "GameMode.h"

class SurvivalMode : public GameMode
{
private:
	int xDestroyBlock;
    int yDestroyBlock;
    int zDestroyBlock;
    float destroyProgress;
    float oDestroyProgress;
    int destroyTicks;		// 4J was float but doesn't seem to need to be
    int destroyDelay;

public:
	SurvivalMode(Minecraft *minecraft);
	SurvivalMode(SurvivalMode *copy);
    virtual void initPlayer(shared_ptr<Player> player);
    virtual void init();
	virtual bool canHurtPlayer();
    virtual bool destroyBlock(int x, int y, int z, int face);
    virtual void startDestroyBlock(int x, int y, int z, int face);
    virtual void stopDestroyBlock();
    virtual void continueDestroyBlock(int x, int y, int z, int face);
    virtual void render(float a);
    virtual float getPickRange();
    virtual void initLevel(Level *level);
	virtual shared_ptr<Player> createPlayer(Level *level);
    virtual void tick();
	virtual bool useItemOn(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, int x, int y, int z, int face, bool bTestUseOnOnly=false, bool *pbUsedItem=NULL);
	virtual bool hasExperience();
};