#pragma once

#include "AgableMob.h"
#include "Npc.h"
#include "Merchant.h"
#include "ParticleTypes.h"

class Level;
class Village;
class MerchantRecipeList;
class MerchantRecipe;

class Villager : public AgableMob, public Npc, public Merchant
{
public:
	eINSTANCEOF GetType() { return eTYPE_VILLAGER; }
	static Entity *create(Level *level) { return new Villager(level); }

	//public static final String comment = "No, I won't 'fix' these! They're fine!! - Notch";

public:
	static const int PROFESSION_FARMER = 0;
	static const int PROFESSION_LIBRARIAN = 1;
	static const int PROFESSION_PRIEST = 2;
	static const int PROFESSION_SMITH = 3;
	static const int PROFESSION_BUTCHER = 4;
	static const int PROFESSION_MAX = 5;

private:
	static const int DATA_PROFESSION_ID = 16;
	int villageUpdateInterval;

	bool inLove;
	bool chasing;
	weak_ptr<Village> village;

	weak_ptr<Player> tradingPlayer;
	MerchantRecipeList *offers;
	int updateMerchantTimer;
	bool addRecipeOnUpdate;
	int riches;
    wstring lastPlayerTradeName;

    bool rewardPlayersOnFirstVillage;

private:

	void _init(int profession);

public:
	Villager(Level *level);
	Villager(Level *level, int profession);
	~Villager();

	virtual bool useNewAi();

protected:
	virtual void serverAiMobStep();

public:
	virtual bool interact(shared_ptr<Player> player);

protected:
	virtual void defineSynchedData();

public:
	virtual int getMaxHealth();
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

	virtual int getTexture();

protected:
	virtual bool removeWhenFarAway();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();

public:
	void setProfession(int profession);
	int getProfession();
	bool isInLove();
	void setInLove(bool inLove);
	void setChasing(bool chasing);
	bool isChasing();
	void setLastHurtByMob(shared_ptr<Mob> mob);
	void die(DamageSource *source);

	void handleEntityEvent(byte id);

private:
	void addParticlesAroundSelf(ePARTICLE_TYPE particle);

public:
	void setTradingPlayer(shared_ptr<Player> player);
	shared_ptr<Player> getTradingPlayer();
	bool isTrading();
	void notifyTrade(MerchantRecipe *activeRecipe);
	void notifyTradeUpdated(shared_ptr<ItemInstance> item);
	MerchantRecipeList *getOffers(shared_ptr<Player> forPlayer);

private:
	float baseRecipeChanceMod;

	float getRecipeChance(float baseChance);
	void addOffers(int addCount);

public:
	void overrideOffers(MerchantRecipeList *recipeList);

private:
	static unordered_map<int, pair<int,int> > MIN_MAX_VALUES;
	static unordered_map<int, pair<int,int> > MIN_MAX_PRICES;

public:
	static void staticCtor();

private:
	/**
	* Adds a merchant recipe that trades items for a single ruby.
	*
	* @param list
	* @param itemId
	* @param random
	* @param likelyHood
	*/
	static void addItemForTradeIn(MerchantRecipeList *list, int itemId, Random *random, float likelyHood);
	static shared_ptr<ItemInstance> getItemTradeInValue(int itemId, Random *random);
	static int getTradeInValue(int itemId, Random *random);

	/**
	* Adds a merchant recipe that trades rubies for an item. If the cost is
	* negative, one ruby will give several of that item.
	*
	* @param list
	* @param itemId
	* @param random
	* @param likelyHood
	*/
	static void addItemForPurchase(MerchantRecipeList *list, int itemId, Random *random, float likelyHood);
	static int getPurchaseCost(int itemId, Random *random);

public:
	void finalizeMobSpawn();
	void setRewardPlayersInVillage();
	shared_ptr<AgableMob> getBreedOffspring(shared_ptr<AgableMob> target);

	virtual int getDisplayName();
};