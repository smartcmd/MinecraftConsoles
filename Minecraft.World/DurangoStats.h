#pragma once

#include "GeneralStat.h"

#include "GenericStats.h"

enum ELeaderboardId
{
	eLeaderboardId_TRAVELLING = 0,
	eLeaderboardId_MINING = 1,
	eLeaderboardId_FARMING = 2,
	eLeaderboardId_KILLING = 3
};

class DsItemEvent : public Stat
{
public:
	static string nameMethods[];

	enum eAcquisitionMethod
	{
		eAcquisitionMethod_None = 0,
	
		eAcquisitionMethod_Pickedup,
		eAcquisitionMethod_Crafted,
		eAcquisitionMethod_TakenFromChest,
		eAcquisitionMethod_TakenFromEnderchest,
		eAcquisitionMethod_Bought,
		eAcquisitionMethod_Smithed,
		eAcquisitionMethod_Mined,

		eAcquisitionMethod_Placed,
	
		eAcquisitionMethod_MAX
	};

	typedef struct _Param
	{
		int methodId, itemId, itemAux, itemCount;
	} Param;

	DsItemEvent(int id, const wstring &name);

	bool onLeaderboard(ELeaderboardId leaderboard, eAcquisitionMethod methodId, Param *param);
	int mergeIds(int itemId);

	virtual void handleParamBlob(shared_ptr<LocalPlayer> plr, byteArray param);
	static byteArray createParamBlob(eAcquisitionMethod methodId, int itemId, int itemAux, int itemCount);
};

class DsMobKilled : public Stat
{
public:
	static const bool RANGED = true;
	static const bool MELEE = false;

	static const int SPIDER_JOCKEY_ID = 49;

	DsMobKilled(int id, const wstring &name);

	typedef struct { bool isRanged; int mobType, weaponId, distance, damage; } Param;
	virtual void handleParamBlob(shared_ptr<LocalPlayer> plr, byteArray param);
	static byteArray createParamBlob(shared_ptr<Player> plr, shared_ptr<Mob> mob, DamageSource *dmgSrc);
};

class DsMobInteract : public Stat
{
protected:
	static string nameInteract[];

public:
	enum eInteract
	{
		eInteract_None = 0,
		eInteract_Breed,
		eInteract_Tamed,
		eInteract_Cured,
		eInteract_Crafted,
		eInteract_Sheared
	};

	DsMobInteract(int id, const wstring &name);

	typedef struct { int interactionType, mobId; } Param;
	virtual void handleParamBlob(shared_ptr<LocalPlayer> plr, byteArray param);
	static byteArray createParamBlob(eInteract interactionId, int entityId);
};

class DsTravel : public Stat
{
public:
	static string nameMethods[];

	enum eMethod {
		eMethod_walk,
		eMethod_swim,
		eMethod_fall,
		eMethod_climb,
		eMethod_minecart,
		eMethod_boat,
		eMethod_pig,

		eMethod_time, // Time is a dimension too right... 

		eMethod_MAX
	};

	static unsigned int CACHE_SIZES[eMethod_MAX];

	DsTravel(int id, const wstring &name);

	typedef struct { eMethod method; int distance; } Param;
	virtual void handleParamBlob(shared_ptr<LocalPlayer> plr, byteArray paramBlob);
	static byteArray createParamBlob(eMethod method, int distance);

	void			flush(shared_ptr<LocalPlayer> plr);

protected:
	unsigned int	param_cache[MAX_LOCAL_PLAYERS][eMethod_MAX];
	int				cache(int iPad, Param &param);
	void			write(shared_ptr<LocalPlayer> plr, eMethod method, int distance);
};

class DsItemUsed : public Stat
{
public:
	DsItemUsed(int id, const wstring &name);
	typedef struct { int itemId, aux, count, health, hunger; } Param;
	virtual void handleParamBlob(shared_ptr<LocalPlayer> plr, byteArray paramBlob);
	static byteArray createParamBlob(int itemId, int aux, int count, int health, int hunger);
};

class DsAchievement : public Stat
{
public:
	DsAchievement(int id, const wstring &name);
	
	virtual void handleParamBlob(shared_ptr<LocalPlayer> plr, byteArray paramBlob);

	typedef struct { eAward award; } SmallParam;
	static byteArray createSmallParamBlob(eAward id);

	typedef struct { eAward award; int count; } LargeParam;
	static byteArray createLargeParamBlob(eAward id, int count);
};

class DsChangedDimension : public Stat
{
public:
	DsChangedDimension(int id, const wstring &name);
	typedef struct { int fromDimId, toDimId; } Param;
	virtual void handleParamBlob(shared_ptr<LocalPlayer> plr, byteArray paramBlob);
	static byteArray createParamBlob(int fromDimId, int toDimId);
};

class DsEnteredBiome : public Stat
{
public:
	DsEnteredBiome(int id, const wstring &name);
	typedef struct { int biomeId; } Param;
	virtual void handleParamBlob(shared_ptr<LocalPlayer> plr, byteArray paramBlob);
	static byteArray createParamBlob(int biomeId);
};

class DurangoStats : public GenericStats
{
public:
	static DurangoStats *getInstance() { return static_cast<DurangoStats *>(GenericStats::getInstance()); }

protected:
	enum {
		itemsAcquired_Id = 1,
		itemUsed_Id,
		travel_Id,
		mobKilled_Id,
		mobInteract_Id,
		binAchievement_Id,
		binAchievementLocal_Id,
		changedDimension_Id,
		enteredBiome_Id,
	};

	DsItemEvent			*itemsAcquired;
	DsTravel			*travel;

	DsMobKilled			*mobKilled;
	DsMobInteract		*mobInteract;

	DsAchievement		*achievement;
	DsAchievement		*achievementLocal;

	DsItemUsed			*itemUsed;

	DsChangedDimension	*changedDimension;
	DsEnteredBiome		*enteredBiome;

	GUID				playerSessionId;
	Platform::String^	multiplayerCorrelationId;

public:
	DurangoStats();
	~DurangoStats();

	virtual Stat *get_stat(int i);


protected:
	// Stats

	virtual Stat* get_walkOneM();
	virtual Stat* get_swimOneM();
	virtual Stat* get_fallOneM();
	virtual Stat* get_climbOneM();
	virtual Stat* get_minecartOneM();
	virtual Stat* get_boatOneM();
	virtual Stat* get_pigOneM();

	virtual Stat* get_cowsMilked();

	// Kills.
	virtual Stat* get_killMob();

	// Mob-Interactions.
	virtual Stat* get_breedEntity(eINSTANCEOF entityId);
	virtual Stat* get_tamedEntity(eINSTANCEOF entityId);
	virtual Stat* get_curedEntity(eINSTANCEOF entityId);
	virtual Stat* get_craftedEntity(eINSTANCEOF entityId);
	virtual Stat* get_shearedEntity(eINSTANCEOF entityId);

	virtual Stat* get_timePlayed();

	virtual Stat* get_blocksPlaced(int blockId);
	virtual Stat* get_blocksMined(int blockId);
	virtual Stat* get_itemsCollected(int itemId, int itemAux);
	virtual Stat* get_itemsCrafted(int itemId);
	virtual Stat* get_itemsSmelted(int itemId);
	virtual Stat* get_itemsUsed(int itemId);
	virtual Stat* get_itemsBought(int itemId);

	virtual Stat* get_changedDimension(int from, int to);
	virtual Stat* get_enteredBiome(int biomeId);

	// Achievements 

	virtual Stat* get_achievement(eAward achievementId);


	// Parameters

	virtual byteArray getParam_walkOneM(int distance);
	virtual byteArray getParam_swimOneM(int distance);
	virtual byteArray getParam_fallOneM(int distance);
	virtual byteArray getParam_climbOneM(int distance);
	virtual byteArray getParam_minecartOneM(int distance);
	virtual byteArray getParam_boatOneM(int distance);
	virtual byteArray getParam_pigOneM(int distance);

	virtual byteArray getParam_cowsMilked();

	virtual byteArray getParam_blocksPlaced(int blockId, int data, int count);
	virtual byteArray getParam_blocksMined(int blockId, int data, int count);
	virtual byteArray getParam_itemsCollected(int id, int aux, int count);
	virtual byteArray getParam_itemsCrafted(int id, int aux, int count);
	virtual byteArray getParam_itemsUsed(shared_ptr<Player> plr, shared_ptr<ItemInstance> itm);
	virtual byteArray getParam_itemsBought(int id, int aux, int count);

	virtual byteArray getParam_mobKill(shared_ptr<Player> plr, shared_ptr<Mob> mob, DamageSource *dmgSrc);

	virtual byteArray getParam_breedEntity(eINSTANCEOF entityId);
	virtual byteArray getParam_tamedEntity(eINSTANCEOF entityId);
	virtual byteArray getParam_curedEntity(eINSTANCEOF entityId);
	virtual byteArray getParam_craftedEntity(eINSTANCEOF entityId);
	virtual byteArray getParam_shearedEntity(eINSTANCEOF entityId);

	virtual byteArray getParam_time(int timediff);

	virtual byteArray getParam_changedDimension(int from, int to);
	virtual byteArray getParam_enteredBiome(int biomeId);

	virtual byteArray getParam_achievement(eAward id);

	virtual byteArray getParam_onARail(int dist);
	virtual byteArray getParam_chestfulOfCobblestone(int count);
	// virtual byteArray getParam_openInventory(
	virtual byteArray getParam_overkill(int dmg);
	virtual byteArray getParam_musicToMyEars(int recordId);

	// Helpers

public:
	// Achievements that have no parameters, you have earned them or not.
	static bool binaryAchievement(eAward achievementId);

	// Achievements that have parameters, but the event is specifically for this achievement.
	static bool enhancedAchievement(eAward achievementId);

	static void generatePlayerSession();
	static LPCGUID getPlayerSession();

	static void setMultiplayerCorrelationId(Platform::String^ mpcId);
	static LPCWSTR getMultiplayerCorrelationId();

	static LPCWSTR getUserId(shared_ptr<LocalPlayer> plr);
	static LPCWSTR getUserId(int iPad);

	static void playerSessionStart(PlayerUID,shared_ptr<Player>);
	static void playerSessionStart(int iPad);
	static void playerSessionPause(int iPad);
	static void playerSessionResume(int iPad);
	static void playerSessionEnd(int iPad);
};