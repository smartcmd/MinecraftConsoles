#pragma once

class Minecraft;
class Level;
class Player;
class ItemInstance;
class Entity;

class Tutorial;

class GameMode
{
protected:
	Minecraft *minecraft;
public:
	bool instaBuild;

    GameMode(Minecraft *minecraft);
	virtual ~GameMode() {}

    virtual void initLevel(Level *level) ;
    virtual void startDestroyBlock(int x, int y, int z, int face) = 0;
    virtual bool destroyBlock(int x, int y, int z, int face);
    virtual void continueDestroyBlock(int x, int y, int z, int face) = 0;
    virtual void stopDestroyBlock() = 0;
    virtual void render(float a);
    virtual float getPickRange() = 0;
    virtual void initPlayer(shared_ptr<Player> player);
    virtual void tick();
    virtual bool canHurtPlayer() = 0;
    virtual void adjustPlayer(shared_ptr<Player> player);
    virtual bool useItem(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, bool bTestUseOnly=false);
    virtual bool useItemOn(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, int x, int y, int z, int face, bool bTestUseOnOnly=false, bool *pbUsedItem = NULL) = 0;

    virtual shared_ptr<Player> createPlayer(Level *level);
    virtual bool interact(shared_ptr<Player> player, shared_ptr<Entity> entity);
    virtual void attack(shared_ptr<Player> player, shared_ptr<Entity> entity);
    virtual shared_ptr<ItemInstance> handleInventoryMouseClick(int containerId, int slotNum, int buttonNum, bool quickKeyHeld, shared_ptr<Player> player);
    virtual void handleCloseInventory(int containerId, shared_ptr<Player> player);
	virtual void handleInventoryButtonClick(int containerId, int buttonId);

	virtual bool isCutScene();
	virtual void releaseUsingItem(shared_ptr<Player> player);
	virtual bool hasExperience();
	virtual bool hasMissTime();
	virtual bool hasInfiniteItems();
	virtual bool hasFarPickRange();
	virtual void handleCreativeModeItemAdd(shared_ptr<ItemInstance> clicked, int i);
	virtual void handleCreativeModeItemDrop(shared_ptr<ItemInstance> clicked);

	// 4J Stu - Added so we can send packets for this in the network game
	virtual bool handleCraftItem(int recipe, shared_ptr<Player> player);
	virtual void handleDebugOptions(unsigned int uiVal, shared_ptr<Player> player);

	// 4J Stu - Added for tutorial checks
	virtual bool isInputAllowed(int mapping) { return true; }
	virtual bool isTutorial() { return false; }
	virtual Tutorial *getTutorial() { return NULL; }
};
