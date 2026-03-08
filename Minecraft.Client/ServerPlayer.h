#pragma once
#include "../Minecraft.World/net.minecraft.world.entity.player.h"
#include "../Minecraft.World/net.minecraft.world.inventory.ContainerListener.h"
#include "../Minecraft.World/ChunkPos.h"
class PlayerConnection;
class MinecraftServer;
class ServerPlayerGameMode;
class Level;
class Stat;
class TileEntity;
class Entity;
class BrewingStandTileEntity;
class HopperTileEntity;
class MinecartHopper;
class BeaconTileEntity;
class EntityHorse;
class Merchant;
using namespace std;

class ServerPlayer : public Player, public net_minecraft_world_inventory::ContainerListener
{
public:
	eINSTANCEOF GetType() { return eTYPE_SERVERPLAYER; }
	shared_ptr<PlayerConnection> connection;
	MinecraftServer *server;
	ServerPlayerGameMode *gameMode;
	double lastMoveX, lastMoveZ;
	list<ChunkPos> chunksToSend;
	vector<int> entitiesToRemove;
	unordered_set<ChunkPos, ChunkPosKeyHash, ChunkPosKeyEq> seenChunks;
	int spewTimer;

	// 4J-Added, for 'Adventure Time' achievement.
	Biome *currentBiome;

private:
	float lastRecordedHealthAndAbsorption;
	float lastSentHealth;
	int lastSentFood;
	bool lastFoodSaturationZero;
	int lastSentExp;
	int invulnerableTime;
	int viewDistance;
	int64_t lastActionTime;
	int lastBrupSendTickCount; // 4J Added

public:
	ServerPlayer(MinecraftServer *server, Level *level, const wstring& name, ServerPlayerGameMode *gameMode);
	~ServerPlayer();
	void flagEntitiesToBeRemoved(unsigned int *flags, bool *removedFound);			// 4J added

	virtual void readAdditionalSaveData(CompoundTag *entityTag);
	virtual void addAdditonalSaveData(CompoundTag *entityTag);
	virtual void giveExperienceLevels(int amount);
	void initMenu();

protected:
	virtual void setDefaultHeadHeight();
public:
	virtual float getHeadHeight();
	virtual void tick();
	void flushEntitiesToRemove();
	virtual shared_ptr<ItemInstance> getCarried(int slot);
	virtual void die(DamageSource *source);
	virtual bool hurt(DamageSource *dmgSource, float dmg);
	virtual bool canHarmPlayer(shared_ptr<Player> target);
	bool canHarmPlayer(wstring targetName); // 4J: Added
	void doTick(bool sendChunks, bool dontDelayChunks = false, bool ignorePortal = false);
	void doTickA();
	void doChunkSendingTick(bool dontDelayChunks);
	void doTickB();
	virtual void changeDimension(int i);
private:
	void broadcast(shared_ptr<TileEntity> te, bool delay = false);
public:
	virtual void take(shared_ptr<Entity> e, int orgCount);
	virtual BedSleepingResult startSleepInBed(int x, int y, int z, bool bTestUse = false);

public:
	virtual void stopSleepInBed(bool forcefulWakeUp, bool updateLevelList, bool saveRespawnPoint);
	virtual void ride(shared_ptr<Entity> e);
protected:
	virtual void checkFallDamage(double ya, bool onGround);
public:
	void doCheckFallDamage(double ya, bool onGround);
private:
	int containerCounter;
public:
	bool ignoreSlotUpdateHack;
	int latency;
	bool wonGame;
	bool m_enteredEndExitPortal; // 4J Added

private:
	void nextContainerCounter();

public:
	virtual void openTextEdit(shared_ptr<TileEntity> sign);
	virtual bool startCrafting(int x, int y, int z);					// 4J added bool return
	virtual bool openFireworks(int x, int y, int z);					// 4J added
	virtual bool startEnchanting(int x, int y, int z, const wstring &name);					// 4J added bool return
	virtual bool startRepairing(int x, int y, int z);					// 4J added bool return
	virtual bool openContainer(shared_ptr<Container> container);		// 4J added bool return
	virtual bool openHopper(shared_ptr<HopperTileEntity> container);
	virtual bool openHopper(shared_ptr<MinecartHopper> container);
	virtual bool openFurnace(shared_ptr<FurnaceTileEntity> furnace);	// 4J added bool return
	virtual bool openTrap(shared_ptr<DispenserTileEntity> trap);		// 4J added bool return
	virtual bool openBrewingStand(shared_ptr<BrewingStandTileEntity> brewingStand); // 4J added bool return
	virtual bool openBeacon(shared_ptr<BeaconTileEntity> beacon);
	virtual bool openTrading(shared_ptr<Merchant> traderTarget, const wstring &name); // 4J added bool return
	virtual bool openHorseInventory(shared_ptr<EntityHorse> horse, shared_ptr<Container> container);
	virtual void slotChanged(AbstractContainerMenu *container, int slotIndex, shared_ptr<ItemInstance> item);
	void refreshContainer(AbstractContainerMenu *menu);
	virtual void refreshContainer(AbstractContainerMenu *container, vector<shared_ptr<ItemInstance> > *items);
	virtual void setContainerData(AbstractContainerMenu *container, int id, int value);
	virtual void closeContainer();
	void broadcastCarriedItem();
	void doCloseContainer();
	void setPlayerInput(float xa, float ya, bool jumping, bool sneaking);

	virtual void awardStat(Stat *stat, byteArray param);

	void disconnect();
	void resetSentInfo();
	virtual void displayClientMessage(int messageId);

protected:
	virtual void completeUsingItem();

public:
	virtual void startUsingItem(shared_ptr<ItemInstance> instance, int duration);
	virtual void restoreFrom(shared_ptr<Player> oldPlayer, bool restoreAll);

protected:
	virtual void onEffectAdded(MobEffectInstance *effect);
	virtual void onEffectUpdated(MobEffectInstance *effect, bool doRefreshAttributes);
	virtual void onEffectRemoved(MobEffectInstance *effect);

public:
	virtual void teleportTo(double x, double y, double z);
	virtual void crit(shared_ptr<Entity> entity);
	virtual void magicCrit(shared_ptr<Entity> entity);

	void onUpdateAbilities();
	ServerLevel *getLevel();
	void setGameMode(GameType *mode);
	void sendMessage(const wstring& message, ChatPacket::EChatPacketMessage type = ChatPacket::e_ChatCustom, int customData = -1, const wstring& additionalMessage = L"");
	bool hasPermission(EGameCommand command);
	// bool hasPermission(int permissionLevel, EGameCommand command);
	//void updateOptions(shared_ptr<ClientInformationPacket> packet); // 4J: Don't use
	int getViewDistance();
	//bool canChatInColor();
	//int getChatVisibility();
	Pos *getCommandSenderWorldPosition();
	void resetLastActionTime();

public:

	static int getFlagIndexForChunk(const ChunkPos& pos, int dimension);		// 4J - added
	int getPlayerViewDistanceModifier(); // 4J Added, returns a number which is subtracted from the default view distance

public:
	// 4J Stu - Added hooks for the game rules
	virtual void handleCollectItem(shared_ptr<ItemInstance> item);

#ifndef _CONTENT_PACKAGE
	void debug_setPosition(double,double,double,double,double);
#endif

protected:
	// 4J Added to record telemetry of player deaths, this should store the last source of damage
	ETelemetryChallenges m_lastDamageSource;
};
