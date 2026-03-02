#pragma once
#include "GameMode.h"
class ClientConnection;
class GameType;
class Vec3;

class MultiPlayerGameMode
{
private:
	int xDestroyBlock;
    int yDestroyBlock;
    int zDestroyBlock;
	shared_ptr<ItemInstance> destroyingItem;
    float destroyProgress;
    int destroyTicks;		// 4J was float but doesn't seem to need to be
    int destroyDelay;
    bool isDestroying;
	GameType *localPlayerMode;
    ClientConnection *connection;

protected:
	Minecraft *minecraft;

public:
	MultiPlayerGameMode(Minecraft *minecraft, ClientConnection *connection);
	static void creativeDestroyBlock(Minecraft *minecraft, MultiPlayerGameMode *gameMode, int x, int y, int z, int face);
	void adjustPlayer(shared_ptr<Player> player);
	bool isCutScene();
	void setLocalMode(GameType *mode);
    virtual void initPlayer(shared_ptr<Player> player);
	virtual bool canHurtPlayer();
    virtual bool destroyBlock(int x, int y, int z, int face);
    virtual void startDestroyBlock(int x, int y, int z, int face);
    virtual void stopDestroyBlock();
    virtual void continueDestroyBlock(int x, int y, int z, int face);
    virtual float getPickRange();
    virtual void tick();
private:
	int carriedItem;

private:
	bool sameDestroyTarget(int x, int y, int z);
	void ensureHasSentCarriedItem();
public:
    virtual bool useItemOn(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, int x, int y, int z, int face, Vec3 *hit, bool bTestUseOnly=false, bool *pbUsedItem=NULL);
    virtual bool useItem(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, bool bTestUseOnly=false);
    virtual shared_ptr<MultiplayerLocalPlayer> createPlayer(Level *level);
    virtual void attack(shared_ptr<Player> player, shared_ptr<Entity> entity);
    virtual bool interact(shared_ptr<Player> player, shared_ptr<Entity> entity);
    virtual shared_ptr<ItemInstance> handleInventoryMouseClick(int containerId, int slotNum, int buttonNum, bool quickKeyHeld, shared_ptr<Player> player);
	virtual void handleInventoryButtonClick(int containerId, int buttonId);
	virtual void handleCreativeModeItemAdd(shared_ptr<ItemInstance> clicked, int slot);
	virtual void handleCreativeModeItemDrop(shared_ptr<ItemInstance> clicked);
	virtual void releaseUsingItem(shared_ptr<Player> player);
	virtual bool hasExperience();
	virtual bool hasMissTime();
	virtual bool hasInfiniteItems();
	virtual bool hasFarPickRange();
	virtual bool isServerControlledInventory();
	
	// 4J Stu - Added so we can send packets for this in the network game
	virtual bool handleCraftItem(int recipe, shared_ptr<Player> player);
	virtual void handleDebugOptions(unsigned int uiVal, shared_ptr<Player> player);

	// 4J Stu - Added for tutorial checks
	virtual bool isInputAllowed(int mapping) { return true; }
	virtual bool isTutorial() { return false; }
	virtual Tutorial *getTutorial() { return NULL; }
};