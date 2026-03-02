#pragma once
using namespace std;

#include "Mob.h"
#include "Definitions.h"
#include "Abilities.h"
#include "FoodData.h"
#include "PlayerEnderChestContainer.h"
#include "CommandSender.h"

class AbstractContainerMenu;
class Stats;
class FishingHook;

class ItemEntity;
class Slot;
class Pos;

class FurnaceTileEntity;
class DispenserTileEntity;
class SignTileEntity;
class BrewingStandTileEntity;
class Inventory;
class Container;
class FoodData;
class DamageSource;
class Merchant;
class PlayerEnderChestContainer;
class GameType;

class Player : public Mob, public CommandSender
{
public:
	static const int MAX_NAME_LENGTH = 16 + 4;
	static const int MAX_HEALTH = 20;
	static const int SWING_DURATION = 6;
	static const int SLEEP_DURATION = 100;
	static const int WAKE_UP_DURATION = 10;

	static const int CHAT_VISIBILITY_FULL = 0;
	static const int CHAT_VISIBILITY_SYSTEM = 1;
	static const int CHAT_VISIBILITY_HIDDEN = 2;

	// 4J-PB - added for a red death fade in the gui
	static const int DEATHFADE_DURATION = 21;
private:
	static const int FLY_ACHIEVEMENT_SPEED = 25;

	static const int DATA_PLAYER_FLAGS_ID = 16;
	static const int DATA_PLAYER_RUNNING_ID = 17;

public:
	shared_ptr<Inventory> inventory;

private:
	shared_ptr<PlayerEnderChestContainer> enderChestInventory;

public:
	AbstractContainerMenu *inventoryMenu;
	AbstractContainerMenu *containerMenu;

protected:
	FoodData foodData;
	int jumpTriggerTime;

public:
	BYTE userType;
	int score;
	float oBob, bob;
	bool swinging;
	int swingTime;

	wstring name;
	int dimension;
	int takeXpDelay;

	// 4J-PB - track custom skin
	unsigned int m_uiPlayerCurrentSkin;
	void ChangePlayerSkin();

	// 4J-PB - not needed, since cutomtextureurl2 is the same thing wstring cloakTexture;

	double xCloakO, yCloakO, zCloakO;
	double xCloak, yCloak, zCloak;

	// 4J-HEG - store display name, added for Xbox One
	wstring displayName;

protected:
	// player sleeping in bed?
	bool m_isSleeping;

public:
	Pos *bedPosition;

private:
	int sleepCounter; // animation timer
	int deathFadeCounter; // animation timer

public:
	float bedOffsetX, bedOffsetY, bedOffsetZ;
	Stats *stats;

private:
	Pos *respawnPosition;
	Pos *minecartAchievementPos;

	//4J Gordon: These are in cms, every time they go > 1m they are entered into the stats
	int distanceWalk, distanceSwim, distanceFall, distanceClimb, distanceMinecart, distanceBoat, distancePig;

public:
	int changingDimensionDelay;

protected:
	bool isInsidePortal;

public:
	float portalTime, oPortalTime;

	Abilities abilities;

	int experienceLevel, totalExperience;
	float experienceProgress;

	// 4J Stu - Made protected so that we can access it from MultiPlayerLocalPlayer
protected:
	shared_ptr<ItemInstance> useItem;
	int useItemDuration;

protected:
	float defaultWalkSpeed;
	float defaultFlySpeed;

public:

	eINSTANCEOF GetType() { return eTYPE_PLAYER; }

	// 4J Added to default init
	void _init();

	Player(Level *level);
	virtual ~Player();

	virtual int getMaxHealth();
protected:
	virtual void defineSynchedData();

public:
	shared_ptr<ItemInstance> getUseItem();
	int getUseItemDuration();
	bool isUsingItem();	int getTicksUsingItem();
	void releaseUsingItem();
	void stopUsingItem();
	virtual bool isBlocking();

	virtual void tick();

protected:
	void spawnEatParticles(shared_ptr<ItemInstance> useItem, int count);
	virtual void completeUsingItem();

public:
	virtual void handleEntityEvent(byte id);

protected:
	bool isImmobile();
	virtual void closeContainer();

public:
	virtual void ride(shared_ptr<Entity> e);
	void prepareCustomTextures();
	virtual void rideTick();
	virtual void resetPos();

private:
	int getCurrentSwingDuration();

protected:
	virtual void serverAiStep();

public:
	virtual void aiStep();

private:
	virtual void touch(shared_ptr<Entity> entity);

public:
	//bool addResource(int resource); // 4J - Removed 1.0.1
	int getScore();
	virtual void die(DamageSource *source);
	void awardKillScore(shared_ptr<Entity> victim, int score);

protected:
	virtual int decreaseAirSupply(int currentSupply);

public:
	virtual bool isShootable();
	bool isCreativeModeAllowed();
	virtual shared_ptr<ItemEntity> drop();
	shared_ptr<ItemEntity> drop(shared_ptr<ItemInstance> item);
	shared_ptr<ItemEntity> drop(shared_ptr<ItemInstance> item, bool randomly);

protected:
	virtual void reallyDrop(shared_ptr<ItemEntity> thrownItem);

public:
	float getDestroySpeed(Tile *tile);
	bool canDestroy(Tile *tile);
	virtual void readAdditionalSaveData(CompoundTag *entityTag);
	virtual void addAdditonalSaveData(CompoundTag *entityTag);
	static Pos *getRespawnPosition(Level *level, CompoundTag *entityTag);
	virtual bool openContainer(shared_ptr<Container> container);	// 4J - added bool return
	virtual bool startEnchanting(int x, int y, int z);				// 4J - added bool return
	virtual bool startRepairing(int x, int y, int z);				// 4J - added bool return
	virtual bool startCrafting(int x, int y, int z);				// 4J - added boo return
	virtual void take(shared_ptr<Entity> e, int orgCount);
	virtual float getHeadHeight();

	// 4J-PB - added to keep the code happy with the change to make the third person view per player
	virtual int ThirdPersonView()	{return 0;}
	virtual void SetThirdPersonView(int val)	{}

protected:
	virtual void setDefaultHeadHeight();

public:
	shared_ptr<FishingHook> fishing;

	virtual bool hurt(DamageSource *source, int dmg);

protected:
	virtual int getDamageAfterMagicAbsorb(DamageSource *damageSource, int damage);
	virtual bool isPlayerVersusPlayer();
	void directAllTameWolvesOnTarget(shared_ptr<Mob> target, bool skipSitting);
	virtual void hurtArmor(int damage);

public:
	virtual int getArmorValue();
	float getArmorCoverPercentage();

protected:
	virtual void actuallyHurt(DamageSource *source, int dmg);

public:
	using Entity::interact;

	virtual bool openFurnace(shared_ptr<FurnaceTileEntity> container);		// 4J - added bool return
	virtual bool openTrap(shared_ptr<DispenserTileEntity> container);		// 4J - added bool return
	virtual void openTextEdit(shared_ptr<SignTileEntity> sign);
	virtual bool openBrewingStand(shared_ptr<BrewingStandTileEntity> brewingStand); // 4J - added bool return
	virtual bool openTrading(shared_ptr<Merchant> traderTarget); // 4J - added bool return
	virtual void openItemInstanceGui(shared_ptr<ItemInstance> itemInstance);
	virtual bool interact(shared_ptr<Entity> entity);
	virtual shared_ptr<ItemInstance> getSelectedItem();
	void removeSelectedItem();
	virtual double getRidingHeight();
	virtual void swing();
	virtual void attack(shared_ptr<Entity> entity);
	virtual void crit(shared_ptr<Entity> entity);
	virtual void magicCrit(shared_ptr<Entity> entity);
	virtual void respawn();

protected:
	static void animateRespawn(shared_ptr<Player> player, Level *level);

public:
	Slot *getInventorySlot(int slotId);
	virtual void remove();
	virtual bool isInWall();
	virtual bool isLocalPlayer();

	enum BedSleepingResult
	{
		OK, NOT_POSSIBLE_HERE, NOT_POSSIBLE_NOW, TOO_FAR_AWAY, OTHER_PROBLEM, NOT_SAFE
	};

	virtual BedSleepingResult startSleepInBed(int x, int y, int z, bool bTestUse = false);

private:
	void setBedOffset(int bedDirection);

public:
	/**
	* 
	* @param forcefulWakeUp
	*            If the player has been forced to wake up. When this happens,
	*            the client will skip the wake-up animation. For example, when
	*            the player is hurt or the bed is destroyed.
	* @param updateLevelList
	*            If the level's sleeping player list needs to be updated. This
	*            is usually the case.
	* @param saveRespawnPoint
	*            TODO
	*/
	virtual void stopSleepInBed(bool forcefulWakeUp, bool updateLevelList, bool saveRespawnPoint);

private:
	bool checkBed();

public:
	static Pos *checkBedValidRespawnPosition(Level *level, Pos *pos);
	float getSleepRotation();
	bool isSleeping();
	bool isSleepingLongEnough();
	int getSleepTimer();
	int getDeathFadeTimer();

protected:
	bool getPlayerFlag(int flag);
	void setPlayerFlag(int flag, bool value);

public:
	/**
	* This method is currently only relevant to client-side players. It will
	* try to load the messageId from the language file and display it to the
	* client.
	*/
	virtual void displayClientMessage(int messageId);
	Pos *getRespawnPosition();
	void setRespawnPosition(Pos *respawnPosition);
	virtual void awardStat(Stat *stat, byteArray param);

protected:
	void jumpFromGround();

public:
	void travel(float xa, float ya);
	void checkMovementStatistiscs(double dx, double dy, double dz);

private:
	void checkRidingStatistiscs(double dx, double dy, double dz);

	bool m_bAwardedOnARail;

protected:
	virtual void causeFallDamage(float distance);

public:
	virtual void killed(shared_ptr<Mob> mob);
	virtual Icon *getItemInHandIcon(shared_ptr<ItemInstance> item, int layer);
	virtual shared_ptr<ItemInstance> getArmor(int pos);
	virtual void handleInsidePortal();

	void increaseXp(int i);
	virtual void withdrawExperienceLevels(int amount);
	int getXpNeededForNextLevel();

private:
	void levelUp();

public:
	void causeFoodExhaustion(float amount);
	FoodData *getFoodData();
	bool canEat(bool magicalItem);
	bool isHurt();
	virtual void startUsingItem(shared_ptr<ItemInstance> instance, int duration);
	bool mayBuild(int x, int y, int z);

protected:
	virtual int getExperienceReward(shared_ptr<Player> killedBy);
	virtual bool isAlwaysExperienceDropper();

public:
	virtual wstring getAName();

	virtual void changeDimension(int i);
	virtual void restoreFrom(shared_ptr<Player> oldPlayer, bool restoreAll);

protected:
	bool makeStepSound();

public:
	void onUpdateAbilities();
	void setGameMode(GameType *mode);
	wstring getName();
	wstring getDisplayName(); // 4J added

	//Language getLanguage() { return Language.getInstance(); }
	//String localize(String key, Object... args) { return getLanguage().getElement(key, args); }

	shared_ptr<PlayerEnderChestContainer> getEnderChestInventory();

public:
	virtual shared_ptr<ItemInstance> getCarriedItem();

	virtual bool isInvisibleTo(shared_ptr<Player> player);

	static int hash_fnct(const shared_ptr<Player> k);
	static bool eq_test(const shared_ptr<Player> x, const shared_ptr<Player> y);

	// 4J Stu - Added to allow callback to tutorial to stay within Minecraft.Client
	// Overidden in LocalPlayer
	virtual void onCrafted(shared_ptr<ItemInstance> item) {}

	// 4J Overriding this so that we can have some different default skins
	virtual int getTexture();		// 4J changed from wstring to int
	void setPlayerDefaultSkin(EDefaultSkins skin);
	EDefaultSkins getPlayerDefaultSkin()												{ return m_skinIndex; }
	virtual void setCustomSkin(DWORD skinId);
	DWORD getCustomSkin()																	{return m_dwSkinId; }
	virtual void setCustomCape(DWORD capeId);
	DWORD getCustomCape()																	{return m_dwCapeId; }

	static DWORD getCapeIdFromPath(const wstring &cape);
	static wstring getCapePathFromId(DWORD capeId);
	static unsigned int getSkinAnimOverrideBitmask(DWORD skinId);

	// 4J Added
	void setXuid(PlayerUID xuid);
	PlayerUID getXuid()																			{ return m_xuid; }
	void setOnlineXuid(PlayerUID xuid)															{ m_OnlineXuid = xuid; }
	PlayerUID getOnlineXuid()																	{ return m_OnlineXuid; }
	void setUUID(const wstring &UUID)															{ m_UUID = UUID; }
	wstring getUUID()																			{ return m_UUID; }

	void setPlayerIndex(DWORD dwIndex)														{ m_playerIndex = dwIndex; }
	DWORD getPlayerIndex()																	{ return m_playerIndex; }

	void setIsGuest(bool bVal)																{ m_bIsGuest = bVal; }
	bool isGuest()																			{ return m_bIsGuest; }

	void setShowOnMaps(bool bVal)															{ m_bShownOnMaps = bVal; }
	bool canShowOnMaps()																	{ return m_bShownOnMaps && !getPlayerGamePrivilege(ePlayerGamePrivilege_Invisible); }
	
	virtual void sendMessage(const wstring& message, ChatPacket::EChatPacketMessage type = ChatPacket::e_ChatCustom, int customData = -1, const wstring& additionalMessage = L"") { }
private:
	PlayerUID m_xuid;
	PlayerUID m_OnlineXuid;

protected:
	wstring m_UUID; // 4J Added

	bool m_bShownOnMaps;

	bool m_bIsGuest;

private:
	EDefaultSkins m_skinIndex;
	DWORD m_dwSkinId,m_dwCapeId;

	// 4J Added - Used to show which colour the player is on the map/behind their name
	DWORD m_playerIndex;

	// 4J-PB - to track debug options from the server player
	unsigned int m_uiDebugOptions;

public:
	void SetDebugOptions(unsigned int uiVal)	{ m_uiDebugOptions=uiVal;}
	unsigned int GetDebugOptions(void)			{ return m_uiDebugOptions;}

	void StopSleeping() {}

public:
	// If you add things here, you should also add a message to ClientConnection::displayPrivilegeChanges to alert players to changes
	enum EPlayerGamePrivileges
	{
		ePlayerGamePrivilege_CannotMine = 0, // Only checked if trust system is on
		ePlayerGamePrivilege_CannotBuild, // Only checked if trust system is on
		ePlayerGamePrivilege_CannotAttackMobs, // Only checked if trust system is on
		ePlayerGamePrivilege_CannotAttackPlayers, //Only checked if trust system is on
		ePlayerGamePrivilege_Op,
		ePlayerGamePrivilege_CanFly,
		ePlayerGamePrivilege_ClassicHunger,
		ePlayerGamePrivilege_Invisible,
		ePlayerGamePrivilege_Invulnerable,

		ePlayerGamePrivilege_CreativeMode, // Used only to transfer across network, should never be used to determine if a player is in creative mode

		ePlayerGamePrivilege_CannotAttackAnimals, // Only checked if trust system is on
		ePlayerGamePrivilege_CanUseDoorsAndSwitches, // Only checked if trust system is on
		ePlayerGamePrivilege_CanUseContainers, // Only checked if trust system is on

		ePlayerGamePrivilege_CanToggleInvisible,
		ePlayerGamePrivilege_CanToggleFly,
		ePlayerGamePrivilege_CanToggleClassicHunger,
		ePlayerGamePrivilege_CanTeleport,

		// Currently enum is used to bitshift into an unsigned int
		ePlayerGamePrivilege_MAX = 32,
		ePlayerGamePrivilege_All = 33,
		ePlayerGamePrivilege_HOST,
	};
private:
	// 4J Added - Used to track what actions players have been allowed to perform by the host
	unsigned int m_uiGamePrivileges;

	unsigned int getPlayerGamePrivilege(EPlayerGamePrivileges privilege);
public:
	unsigned int getAllPlayerGamePrivileges() { return getPlayerGamePrivilege(ePlayerGamePrivilege_All); }

	static unsigned int getPlayerGamePrivilege(unsigned int uiGamePrivileges, EPlayerGamePrivileges privilege);
	void setPlayerGamePrivilege(EPlayerGamePrivileges privilege, unsigned int value);
	static void setPlayerGamePrivilege(unsigned int &uiGamePrivileges, EPlayerGamePrivileges privilege, unsigned int value);

	bool isAllowedToUse(Tile *tile);
	bool isAllowedToUse(shared_ptr<ItemInstance> item);
	bool isAllowedToInteract(shared_ptr<Entity> target);
	bool isAllowedToMine();
	bool isAllowedToAttackPlayers();
	bool isAllowedToAttackAnimals();
	bool isAllowedToHurtEntity(shared_ptr<Entity> target);
	bool isAllowedToFly();
	bool isAllowedToIgnoreExhaustion();
	bool isAllowedToTeleport();
	bool hasInvisiblePrivilege();
	bool hasInvulnerablePrivilege();
	bool isModerator();

	static void enableAllPlayerPrivileges(unsigned int &uigamePrivileges, bool enable);
	void enableAllPlayerPrivileges(bool enable);

	virtual bool canCreateParticles();

public:
	// 4J Stu - Added hooks for the game rules
	virtual void handleCollectItem(shared_ptr<ItemInstance> item) {}

	vector<ModelPart *> *GetAdditionalModelParts();
	void SetAdditionalModelParts(vector<ModelPart *> *ppAdditionalModelParts);

#if defined(__PS3__) || defined(__ORBIS__)
	enum ePlayerNameValidState
	{
		ePlayerNameValid_NotSet=0,
		ePlayerNameValid_True,
		ePlayerNameValid_False
	};

	ePlayerNameValidState GetPlayerNameValidState();
	void SetPlayerNameValidState(bool bState);
#endif
private:
	vector<ModelPart *> *m_ppAdditionalModelParts;
	bool m_bCheckedForModelParts;
	bool m_bCheckedDLCForModelParts;

#if defined(__PS3__) || defined(__ORBIS__)
	ePlayerNameValidState m_ePlayerNameValidState; // 4J-PB - to ensure we have the characters for this name in our font, or display a player number instead
#endif
};

struct PlayerKeyHash
{
	inline int operator() (const shared_ptr<Player> k) const 
	{ return Player::hash_fnct (k); }
};

struct PlayerKeyEq
{
	inline bool operator() (const shared_ptr<Player> x, const shared_ptr<Player> y) const 
	{ return Player::eq_test (x, y); }
};

