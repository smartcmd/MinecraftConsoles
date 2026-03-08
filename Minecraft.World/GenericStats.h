#pragma once

#include "Stat.h"
#include "Stats.h"

//#include "DamageSource.h"

//#include "../Minecraft.Client/Common/Console_Awards_enum.h"

/**
	4J-JEV:
		Java version exposed the static instance of each stat.

		This was inconvient for me as I needed to structure the stats/achievements differently
		on Durango.

		Using getters like this means we can use different Stats easilly on different
		platforms and still have a convenient identifier to use to award them.
*/
class GenericStats
{
private: // Static instance.
	static GenericStats *instance;

public:
	static void setInstance(GenericStats *newInstance) { instance = newInstance; }
	static GenericStats *getInstance() { return instance; }

	// For retrieving a stat from an id.
	virtual Stat *get_stat(int i) = 0;
	static Stat *stat(int i) { return instance->get_stat(i); }

	// STATS - STATIC //

	static Stat* walkOneM() { return instance->get_walkOneM(); }
	static Stat* swimOneM() { return instance->get_swimOneM(); }
	static Stat* fallOneM() { return instance->get_fallOneM(); }
	static Stat* climbOneM() { return instance->get_climbOneM(); }
	static Stat* minecartOneM() { return instance->get_minecartOneM(); }
	static Stat* boatOneM() { return instance->get_boatOneM(); }
	static Stat* pigOneM() { return instance->get_pigOneM(); }
	static Stat* portalsCreated() { return instance->get_portalsCreated(); }
	static Stat* cowsMilked() { return instance->get_cowsMilked(); }
	static Stat* netherLavaCollected() { return instance->get_netherLavaCollected(); }

	static Stat* killMob() { return instance->get_killMob(); }

	static Stat* killsZombie() { return instance->get_killsZombie(); }
	static Stat* killsSkeleton() { return instance->get_killsSkeleton(); }
	static Stat* killsCreeper() { return instance->get_killsCreeper(); }
	static Stat* killsSpider() { return instance->get_killsSpider(); }
	static Stat* killsSpiderJockey() { return instance->get_killsSpiderJockey(); }
	static Stat* killsZombiePigman() { return instance->get_killsZombiePigman(); }
	static Stat* killsSlime() { return instance->get_killsSlime(); }
	static Stat* killsGhast() { return instance->get_killsGhast(); }
	static Stat* killsNetherZombiePigman() { return instance->get_killsNetherZombiePigman(); }

	static Stat* breedEntity(eINSTANCEOF entityId) { return instance->get_breedEntity(entityId); }
	static Stat* tamedEntity(eINSTANCEOF entityId) { return instance->get_tamedEntity(entityId); }
	static Stat* curedEntity(eINSTANCEOF entityId) { return instance->get_curedEntity(entityId); }
	static Stat* craftedEntity(eINSTANCEOF entityId) { return instance->get_craftedEntity(entityId); }
	static Stat* shearedEntity(eINSTANCEOF entityId) { return instance->get_shearedEntity(entityId); }

	static Stat* totalBlocksMined() { return instance->get_totalBlocksMined(); }
	static Stat* timePlayed() { return instance->get_timePlayed(); }

	static Stat* blocksPlaced(int blockId) { return instance->get_blocksPlaced(blockId); }
	static Stat* blocksMined(int blockId) { return instance->get_blocksMined(blockId); }
	static Stat* itemsCollected(int itemId, int itemAux) { return instance->get_itemsCollected(itemId, itemAux); }
	static Stat* itemsCrafted(int itemId) { return instance->get_itemsCrafted(itemId); }
	static Stat* itemsSmelted(int itemId) { return instance->get_itemsSmelted(itemId); }	// 4J-JEV: Diffentiation needed, when only one type of event should be sent (eg iron smelting).
	static Stat* itemsUsed(int itemId) { return instance->get_itemsUsed(itemId); }
	static Stat* itemsBought(int itemId) { return instance->get_itemsBought(itemId); }

	static Stat* killsEnderdragon() { return instance->get_killsEnderdragon(); }
	static Stat* completeTheEnd() { return instance->get_completeTheEnd(); }

	static Stat* changedDimension(int from, int to) { return instance->get_changedDimension(from,to); }
	static Stat* enteredBiome(int biomeId) { return instance->get_enteredBiome(biomeId); }

	// ACHIEVEMENTS - STATIC //

	static Stat* achievement(eAward achievementId) { return instance->get_achievement(achievementId); }

	static Stat* openInventory();
	static Stat* mineWood();
	static Stat* buildWorkbench();
	static Stat* buildPickaxe();
	static Stat* buildFurnace();
	static Stat* acquireIron();
	static Stat* buildHoe();
	static Stat* makeBread();
	static Stat* bakeCake();
	static Stat* buildBetterPickaxe();
	static Stat* cookFish();
	static Stat* onARail();
	static Stat* buildSword();
	static Stat* killEnemy();
	static Stat* killCow();
	static Stat* flyPig();
	static Stat* snipeSkeleton();
	static Stat* diamonds();
	static Stat* ghast();
	static Stat* blazeRod();
	static Stat* potion();
	static Stat* theEnd();
	static Stat* winGame();
	static Stat* enchantments();
	static Stat* overkill();
	static Stat* bookcase();

	static Stat* leaderOfThePack();
	static Stat* MOARTools();
	static Stat* dispenseWithThis();
	static Stat* InToTheNether();

	static Stat* socialPost();
	static Stat* eatPorkChop();
	static Stat* play100Days();
	static Stat* arrowKillCreeper();
	static Stat* mine100Blocks();
	static Stat* kill10Creepers();

	static Stat* adventuringTime();			// Requires new Stat
	static Stat* repopulation();			
	static Stat* diamondsToYou();			// +Durango
	static Stat* porkChop();				// Req Stat?
	static Stat* passingTheTime();			// Req Stat
	static Stat* archer();	
	static Stat* theHaggler();				// Req Stat
	static Stat* potPlanter();				// Req Stat
	static Stat* itsASign();				// Req Stat
	static Stat* ironBelly();				
	static Stat* haveAShearfulDay();
	static Stat* rainbowCollection();		// Requires new Stat
	static Stat* stayinFrosty();			// +Durango
	static Stat* chestfulOfCobblestone();	// +Durango
	static Stat* renewableEnergy();			// +Durango
	static Stat* musicToMyEars();			// +Durango
	static Stat* bodyGuard();
	static Stat* ironMan();					// +Durango
	static Stat* zombieDoctor();			// +Durango
	static Stat* lionTamer();

	// STAT PARAMS - STATIC //

	static byteArray param_walk(int distance);
	static byteArray param_swim(int distance);
	static byteArray param_fall(int distance);
	static byteArray param_climb(int distance);
	static byteArray param_minecart(int distance);
	static byteArray param_boat(int distance);
	static byteArray param_pig(int distance);

	static byteArray param_cowsMilked();

	static byteArray param_blocksPlaced(int id, int data, int count);
	static byteArray param_blocksMined(int id, int data, int count);
	static byteArray param_itemsCollected(int id, int aux, int count);
	static byteArray param_itemsCrafted(int id, int aux, int count);
	static byteArray param_itemsSmelted(int id, int aux, int cound);
	static byteArray param_itemsUsed(shared_ptr<Player> plr, shared_ptr<ItemInstance> itm);
	static byteArray param_itemsBought(int id, int aux, int count);

	static byteArray param_mobKill(shared_ptr<Player> plr, shared_ptr<Mob> mob, DamageSource *dmgSrc);

	static byteArray param_breedEntity(eINSTANCEOF mobType);
	static byteArray param_tamedEntity(eINSTANCEOF mobType);
	static byteArray param_curedEntity(eINSTANCEOF mobType);
	static byteArray param_craftedEntity(eINSTANCEOF mobType);
	static byteArray param_shearedEntity(eINSTANCEOF mobType);

	static byteArray param_time(int timediff);

	static byteArray param_changedDimension(int from, int to);
	static byteArray param_enteredBiome(int biomeId);

	//static byteArray param_achievement(eAward id);

	//static byteArray param_ach_onARail();
	//static byteArray param_overkill(int damage); //TODO
	//static byteArray param_openInventory(int menuId);
	//static byteArray param_chestfulOfCobblestone();
	//static byteArray param_musicToMyEars(int recordId);

	static byteArray param_noArgs();

	// STATIC + VIRTUAL - ACHIEVEMENT - PARAMS //

	static byteArray param_openInventory();
	static byteArray param_mineWood();
	static byteArray param_buildWorkbench();
	static byteArray param_buildPickaxe();
	static byteArray param_buildFurnace();
	static byteArray param_acquireIron();
	static byteArray param_buildHoe();
	static byteArray param_makeBread();
	static byteArray param_bakeCake();
	static byteArray param_buildBetterPickaxe();
	static byteArray param_cookFish();
	static byteArray param_onARail(int distance);
	static byteArray param_buildSword();
	static byteArray param_killEnemy();
	static byteArray param_killCow();
	static byteArray param_flyPig();
	static byteArray param_snipeSkeleton();
	static byteArray param_diamonds();
	static byteArray param_ghast();
	static byteArray param_blazeRod();
	static byteArray param_potion();
	static byteArray param_theEnd();
	static byteArray param_winGame();
	static byteArray param_enchantments();
	static byteArray param_overkill(int dmg);
	static byteArray param_bookcase();

	static byteArray param_leaderOfThePack();
	static byteArray param_MOARTools();
	static byteArray param_dispenseWithThis();
	static byteArray param_InToTheNether();

	static byteArray param_socialPost();
	static byteArray param_eatPorkChop();
	static byteArray param_play100Days();
	static byteArray param_arrowKillCreeper();
	static byteArray param_mine100Blocks();
	static byteArray param_kill10Creepers();

	static byteArray param_adventuringTime();
	static byteArray param_repopulation();
	static byteArray param_porkChop();
	static byteArray param_diamondsToYou();
	static byteArray param_passingTheTime();
	static byteArray param_archer();
	static byteArray param_theHaggler();
	static byteArray param_potPlanter();
	static byteArray param_itsASign();
	static byteArray param_ironBelly();
	static byteArray param_haveAShearfulDay();
	static byteArray param_rainbowCollection();
	static byteArray param_stayinFrosty();
	static byteArray param_chestfulOfCobblestone(int cobbleStone);
	static byteArray param_renewableEnergy();
	static byteArray param_musicToMyEars(int recordId);
	static byteArray param_bodyGuard();
	static byteArray param_ironMan();
	static byteArray param_zombieDoctor();
	static byteArray param_lionTamer();

protected:
	// ACHIEVEMENTS - VIRTUAL //
	
	virtual Stat* get_achievement(eAward achievementId);


	// STATS - VIRTUAL //

	virtual Stat* get_walkOneM();
	virtual Stat* get_swimOneM();
	virtual Stat* get_fallOneM();
	virtual Stat* get_climbOneM();
	virtual Stat* get_minecartOneM();
	virtual Stat* get_boatOneM();
	virtual Stat* get_pigOneM();
	virtual Stat* get_portalsCreated();
	virtual Stat* get_cowsMilked();
	virtual Stat* get_netherLavaCollected();

	virtual Stat* get_killMob();

	virtual Stat* get_killsZombie();
	virtual Stat* get_killsSkeleton();
	virtual Stat* get_killsCreeper();
	virtual Stat* get_killsSpider();
	virtual Stat* get_killsSpiderJockey();
	virtual Stat* get_killsZombiePigman();
	virtual Stat* get_killsSlime();
	virtual Stat* get_killsGhast();
	virtual Stat* get_killsNetherZombiePigman();

	virtual Stat* get_breedEntity(eINSTANCEOF entityId);
	virtual Stat* get_tamedEntity(eINSTANCEOF entityId);
	virtual Stat* get_curedEntity(eINSTANCEOF entityId);
	virtual Stat* get_craftedEntity(eINSTANCEOF entityId);
	virtual Stat* get_shearedEntity(eINSTANCEOF entityId);

	virtual Stat* get_totalBlocksMined();
	virtual Stat* get_timePlayed();

	virtual Stat* get_blocksPlaced(int blockId);
	virtual Stat* get_blocksMined(int blockId);
	virtual Stat* get_itemsCollected(int itemId, int itemAux);
	virtual Stat* get_itemsCrafted(int itemId);
	virtual Stat* get_itemsSmelted(int itemId);
	virtual Stat* get_itemsUsed(int itemId);
	virtual Stat* get_itemsBought(int itemId);

	virtual Stat* get_killsEnderdragon();
	virtual Stat* get_completeTheEnd();

	virtual Stat* get_changedDimension(int from, int to);
	virtual Stat* get_enteredBiome(int biomeId);

	// STAT PARAMS - VIRTUAL //

	virtual byteArray getParam_walkOneM(int distance);
	virtual byteArray getParam_swimOneM(int distance);
	virtual byteArray getParam_fallOneM(int distance);
	virtual byteArray getParam_climbOneM(int distance);
	virtual byteArray getParam_minecartOneM(int distance);
	virtual byteArray getParam_boatOneM(int distance);
	virtual byteArray getParam_pigOneM(int distance);

	virtual byteArray getParam_cowsMilked();

	virtual byteArray getParam_blocksPlaced(int id, int data, int count);
	virtual byteArray getParam_blocksMined(int id, int data, int count);
	virtual byteArray getParam_itemsCollected(int id, int aux, int count);
	virtual byteArray getParam_itemsCrafted(int id, int aux, int count);
	virtual byteArray getParam_itemsSmelted(int id, int aux, int count);
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

	virtual byteArray getParam_onARail(int distance);
	virtual byteArray getParam_overkill(int damage);
	virtual byteArray getParam_openInventory(int menuId);
	virtual byteArray getParam_chestfulOfCobblestone(int cobbleStone);
	virtual byteArray getParam_musicToMyEars(int recordId);

	virtual byteArray getParam_noArgs();
};

// Req Stats