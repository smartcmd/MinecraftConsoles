#include "stdafx.h"
#include "net.minecraft.locale.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.item.crafting.h"
#include "Achievements.h"
#include "ItemStat.h"
#include "GeneralStat.h"
#include "Stats.h"
#include "..\Minecraft.Client\StatsCounter.h"

const int Stats::BLOCKS_MINED_OFFSET = 0x1000000;
const int Stats::ITEMS_COLLECTED_OFFSET = 0x1010000;
const int Stats::ITEMS_CRAFTED_OFFSET = 0x1020000;
const int Stats::ADDITIONAL_STATS_OFFSET = 0x5010000; // Needs to be higher than Achievements::ACHIEVEMENT_OFFSET = 0x500000;

unordered_map<int, Stat*>* Stats::statsById = new unordered_map<int, Stat*>;

vector<Stat *> *Stats::all = new vector<Stat *>;
vector<Stat *> *Stats::generalStats = new vector<Stat *>;
vector<ItemStat *> *Stats::blocksMinedStats = new vector<ItemStat *>;
vector<ItemStat *> *Stats::itemsCollectedStats = new vector<ItemStat *>;
vector<ItemStat *> *Stats::itemsCraftedStats = new vector<ItemStat *>;

#if (defined _EXTENDED_ACHIEVEMENTS) && (!defined _DURANGO)
vector<ItemStat *> *Stats::blocksPlacedStats = new vector<ItemStat *>;
#endif

Stat *Stats::walkOneM = nullptr;
Stat *Stats::swimOneM = nullptr;
Stat *Stats::fallOneM = nullptr;
Stat *Stats::climbOneM = nullptr;
Stat *Stats::minecartOneM = nullptr;
Stat *Stats::boatOneM = nullptr;
Stat *Stats::pigOneM = nullptr;
Stat *Stats::portalsCreated = nullptr;
Stat *Stats::cowsMilked = nullptr;
Stat *Stats::netherLavaCollected = nullptr;
Stat *Stats::killsZombie = nullptr;
Stat *Stats::killsSkeleton = nullptr;
Stat *Stats::killsCreeper = nullptr;
Stat *Stats::killsSpider = nullptr;
Stat *Stats::killsSpiderJockey = nullptr;
Stat *Stats::killsZombiePigman = nullptr;
Stat *Stats::killsSlime = nullptr;
Stat *Stats::killsGhast = nullptr;
Stat *Stats::killsNetherZombiePigman = nullptr;

// 4J : WESTY : Added for new achievements.
Stat *Stats::befriendsWolf = nullptr;
Stat *Stats::totalBlocksMined = nullptr;
Stat *Stats::timePlayed = nullptr;

StatArray Stats::blocksMined;
StatArray Stats::itemsCollected;
StatArray Stats::itemsCrafted;

#if (defined _EXTENDED_ACHIEVEMENTS) && (!defined _DURANGO)
StatArray Stats::blocksPlaced;
StatArray Stats::rainbowCollection;
StatArray Stats::biomesVisisted;
#endif

Stat *Stats::killsEnderdragon = nullptr; // The number of times this player has dealt the killing blow to the Enderdragon
Stat *Stats::completeTheEnd = nullptr; // The number of times this player has been present when the Enderdragon has died

void Stats::staticCtor()
{
	Stats::walkOneM			= (new GeneralStat(2000, L"stat.walkOneM", static_cast<StatFormatter *>(Stat::distanceFormatter)))->setAwardLocallyOnly()->postConstruct();
	Stats::swimOneM			= (new GeneralStat(2001, L"stat.swimOneM", static_cast<StatFormatter *>(Stat::distanceFormatter)))->setAwardLocallyOnly()->postConstruct();
	Stats::fallOneM			= (new GeneralStat(2002, L"stat.fallOneM", static_cast<StatFormatter *>(Stat::distanceFormatter)))->setAwardLocallyOnly()->postConstruct();
	Stats::climbOneM		= (new GeneralStat(2003, L"stat.climbOneM", static_cast<StatFormatter *>(Stat::distanceFormatter)))->setAwardLocallyOnly()->postConstruct();
	Stats::minecartOneM		= (new GeneralStat(2004, L"stat.minecartOneM", static_cast<StatFormatter *>(Stat::distanceFormatter)))->setAwardLocallyOnly()->postConstruct();
	Stats::boatOneM			= (new GeneralStat(2005, L"stat.boatOneM", static_cast<StatFormatter *>(Stat::distanceFormatter)))->setAwardLocallyOnly()->postConstruct();
	Stats::pigOneM			= (new GeneralStat(2006, L"stat.pigOneM", static_cast<StatFormatter *>(Stat::distanceFormatter)))->setAwardLocallyOnly()->postConstruct();
	Stats::portalsCreated	= (new GeneralStat(2007, L"stat.portalsUsed"))->postConstruct();
	Stats::cowsMilked		= (new GeneralStat(2008, L"stat.cowsMilked"))->postConstruct();
	Stats::netherLavaCollected = (new GeneralStat(2009, L"stat.netherLavaCollected"))->postConstruct();
	Stats::killsZombie		= (new GeneralStat(2010, L"stat.killsZombie"))->postConstruct();
	Stats::killsSkeleton	= (new GeneralStat(2011, L"stat.killsSkeleton"))->postConstruct();
	Stats::killsCreeper		= (new GeneralStat(2012, L"stat.killsCreeper"))->postConstruct();
	Stats::killsSpider		= (new GeneralStat(2013, L"stat.killsSpider"))->postConstruct();
	Stats::killsSpiderJockey = (new GeneralStat(2014, L"stat.killsSpiderJockey"))->postConstruct();
	Stats::killsZombiePigman = (new GeneralStat(2015, L"stat.killsZombiePigman"))->postConstruct();
	Stats::killsSlime		= (new GeneralStat(2016, L"stat.killsSlime"))->postConstruct();
	Stats::killsGhast		= (new GeneralStat(2017, L"stat.killsGhast"))->postConstruct();
	Stats::killsNetherZombiePigman = (new GeneralStat(2018, L"stat.killsNetherZombiePigman"))->postConstruct();

	// 4J : WESTY : Added for new achievements.
	Stats::befriendsWolf = (new GeneralStat(2019, L"stat.befriendsWolf"))->postConstruct();
	Stats::totalBlocksMined = (new GeneralStat(2020, L"stat.totalBlocksMined"))->postConstruct();

	// 4J-PB - don't want the time played going to the server
	Stats::timePlayed = (new GeneralStat(2021, L"stat.timePlayed"))->setAwardLocallyOnly()->postConstruct();

// WARNING: NO NEW STATS CAN BE ADDED HERE
// These stats are directly followed by the achievemnts in the profile data, so cannot be changed without migrating the profile data

	buildBlockStats();

	Achievements::init();
	Achievements::staticCtor();

	// 4J Stu - Added this function to allow us to add news stats from TU9 onwards
	buildAdditionalStats();
}

void Stats::init()
{
}

bool Stats::blockStatsLoaded = false;

// WARNING: NO NEW STATS CAN BE ADDED HERE
// These stats are directly followed by the achievemnts in the profile data, so cannot be changed without migrating the profile data
void Stats::buildBlockStats()
{
	blocksMined = StatArray(32000);

	ItemStat* newStat = new ItemStat(BLOCKS_MINED_OFFSET + 0, L"mineBlock.dirt", Tile::dirt->id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::dirt->id] = newStat;
	blocksMined[Tile::grass->id] = newStat;
	blocksMined[Tile::farmland->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 1, L"mineBlock.stone", Tile::cobblestone->id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::cobblestone->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 2, L"mineBlock.sand", Tile::sand->id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::sand->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 3, L"mineBlock.cobblestone", Tile::stone->id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::stone->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 4, L"mineBlock.gravel", Tile::gravel->id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::gravel->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 5, L"mineBlock.clay", Tile::clay->id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::clay->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 6, L"mineBlock.obsidian", Tile::obsidian->id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::obsidian->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 7, L"mineBlock.coal", Tile::coalOre->id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::coalOre->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 8, L"mineBlock.iron", Tile::ironOre->id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::ironOre->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 9, L"mineBlock.gold", Tile::goldOre->id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::goldOre->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 10, L"mineBlock.diamond", Tile::diamondOre->id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::diamondOre->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 11, L"mineBlock.redstone", Tile::redStoneOre->id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::redStoneOre->id] = newStat;
	blocksMined[Tile::redStoneOre_lit->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 12, L"mineBlock.lapisLazuli", Tile::lapisOre->id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::lapisOre->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 13, L"mineBlock.netherrack", Tile::netherRack->id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::netherRack->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 14, L"mineBlock.soulSand", Tile::soulsand->id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::soulsand->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 15, L"mineBlock.glowstone", Tile::glowstone->id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::glowstone->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 16, L"mineBlock.wood", Tile::treeTrunk->id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::treeTrunk->id] = newStat;
	newStat->postConstruct();

// WARNING: NO NEW STATS CAN BE ADDED HERE
// These stats are directly followed by the achievemnts in the profile data, so cannot be changed without migrating the profile data


	blockStatsLoaded = true;
	buildCraftableStats();
}

bool Stats::itemStatsLoaded = false;

void Stats::buildItemStats()
{
	itemStatsLoaded = true;
	buildCraftableStats();
}

bool Stats::craftableStatsLoaded = false;


// WARNING: NO NEW STATS CAN BE ADDED HERE
// These stats are directly followed by the achievemnts in the profile data, so cannot be changed without migrating the profile data
void Stats::buildCraftableStats()
{
	if (!blockStatsLoaded || !itemStatsLoaded || craftableStatsLoaded)
	{
		// still waiting for the JVM to load stuff
		//Or stats already loaded
		return;
	}

	craftableStatsLoaded = true;

	//Collected stats

	itemsCollected = StatArray(32000);

	ItemStat* newStat = new ItemStat(ITEMS_COLLECTED_OFFSET + 0, L"collectItem.egg", Item::egg->id);
	itemsCollectedStats->push_back(newStat);
	itemsCollected[Item::egg->id] = newStat;
	newStat->postConstruct();

	// 4J Stu - The following stats were added as it was too easy to cheat the leaderboards by dropping and picking up these items
	// They are now changed to mining the block which involves a tiny bit more effort
	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 18, L"mineBlock.wheat", Tile::wheat_Id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::wheat_Id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 19, L"mineBlock.mushroom1", Tile::mushroom_brown_Id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::mushroom_brown_Id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(BLOCKS_MINED_OFFSET + 17, L"mineBlock.sugar", Tile::reeds_Id);
	blocksMinedStats->push_back(newStat);
	blocksMined[Tile::reeds_Id] = newStat;
	newStat->postConstruct();
#if 0
	newStat = new ItemStat(ITEMS_COLLECTED_OFFSET + 1, L"collectItem.wheat", Item::wheat->id);
	itemsCollectedStats->push_back(newStat);
	itemsCollected[Item::wheat->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_COLLECTED_OFFSET + 2, L"collectItem.mushroom", Tile::mushroom1->id);
	itemsCollectedStats->push_back(newStat);
	itemsCollected[Tile::mushroom1->id] = newStat;
	itemsCollected[Tile::mushroom2->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_COLLECTED_OFFSET + 3, L"collectItem.sugarCane", Item::reeds->id);
	itemsCollectedStats->push_back(newStat);
	itemsCollected[Item::reeds->id] = newStat;
	newStat->postConstruct();
#endif

	newStat = new ItemStat(ITEMS_COLLECTED_OFFSET + 4, L"collectItem.pumpkin", Tile::pumpkin->id);
	itemsCollectedStats->push_back(newStat);
	itemsCollected[Tile::pumpkin->id] = newStat;
	itemsCollected[Tile::litPumpkin->id] = newStat;
	newStat->postConstruct();

	//Crafted stats

	itemsCrafted = StatArray(32000);

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 0, L"craftItem.plank", Tile::wood->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Tile::wood->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 1, L"craftItem.workbench", Tile::workBench->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Tile::workBench->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 2, L"craftItem.stick", Item::stick->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::stick->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 3, L"craftItem.woodenShovel", Item::shovel_wood->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::shovel_wood->id] = newStat;
	newStat->postConstruct();

	// 4J : WESTY : Added for new achievements.
	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 4, L"craftItem.woodenPickAxe", Item::pickAxe_wood->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::pickAxe_wood->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 5, L"craftItem.stonePickAxe", Item::pickAxe_stone->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::pickAxe_stone->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 6, L"craftItem.ironPickAxe", Item::pickAxe_iron->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::pickAxe_iron->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 7, L"craftItem.diamondPickAxe", Item::pickAxe_diamond->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::pickAxe_diamond->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 8, L"craftItem.goldPickAxe", Item::pickAxe_gold->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::pickAxe_gold->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 9, L"craftItem.stoneShovel", Item::shovel_stone->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::shovel_stone->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 10, L"craftItem.ironShovel", Item::shovel_iron->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::shovel_iron->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 11, L"craftItem.diamondShovel", Item::shovel_diamond->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::shovel_diamond->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 12, L"craftItem.goldShovel", Item::shovel_gold->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::shovel_gold->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 13, L"craftItem.woodenAxe", Item::hatchet_wood->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::hatchet_wood->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 14, L"craftItem.stoneAxe", Item::hatchet_stone->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::hatchet_stone->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 15, L"craftItem.ironAxe", Item::hatchet_iron->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::hatchet_iron->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 16, L"craftItem.diamondAxe", Item::hatchet_diamond->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::hatchet_diamond->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 17, L"craftItem.goldAxe", Item::hatchet_gold->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::hatchet_gold->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 18, L"craftItem.woodenHoe", Item::hoe_wood->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::hoe_wood->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 19, L"craftItem.stoneHoe", Item::hoe_stone->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::hoe_stone->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 20, L"craftItem.ironHoe", Item::hoe_iron->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::hoe_iron->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 21, L"craftItem.diamondHoe", Item::hoe_diamond->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::hoe_diamond->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 22, L"craftItem.goldHoe", Item::hoe_gold->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::hoe_gold->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 23, L"craftItem.glowstone", Tile::glowstone_Id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Tile::glowstone_Id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 24, L"craftItem.tnt", Tile::tnt_Id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Tile::tnt_Id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 25, L"craftItem.bowl", Item::bowl->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::bowl->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 26, L"craftItem.bucket", Item::bucket_empty->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::bucket_empty->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 27, L"craftItem.flintAndSteel", Item::flintAndSteel->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::flintAndSteel->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 28, L"craftItem.fishingRod", Item::fishingRod->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::fishingRod->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 29, L"craftItem.clock", Item::clock->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::clock->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 30, L"craftItem.compass", Item::compass->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::compass->id] = newStat;
	newStat->postConstruct();

	newStat = new ItemStat(ITEMS_CRAFTED_OFFSET + 31, L"craftItem.map", Item::map->id);
	itemsCraftedStats->push_back(newStat);
	itemsCrafted[Item::map->id] = newStat;
	newStat->postConstruct();

// WARNING: NO NEW STATS CAN BE ADDED HERE
// These stats are directly followed by the achievemnts in the profile data, so cannot be changed without migrating the profile data

	//This sets up a static list of stat/leaderboard pairings, used to tell which leaderboards need an update
	StatsCounter::setupStatBoards();
}

// 4J Stu - Added this function to allow us to add news stats from TU9 onwards
void Stats::buildAdditionalStats()
{
	int offset = ADDITIONAL_STATS_OFFSET;

	// The order of these stats should not be changed, as the map directly to bits in the profile data

	// The number of times this player has dealt the killing blow to the Enderdragon
	Stats::killsEnderdragon = (new GeneralStat(offset++, L"stat.killsEnderdragon"))->postConstruct();

	// The number of times this player has been present when the Enderdragon has died
	Stats::completeTheEnd = (new GeneralStat(offset++, L"stat.completeTheEnd"))->postConstruct();


#if (defined _EXTENDED_ACHIEVEMENTS) && (!defined _DURANGO)
	{
		ItemStat *itemStat = new ItemStat(offset++, L"craftItem.flowerPot", Item::flowerPot_Id);
		itemsCraftedStats->push_back(itemStat);
		itemsCrafted[itemStat->getItemId()] = itemStat;
		itemStat->postConstruct();

		itemStat = new ItemStat(offset++, L"craftItem.sign", Item::sign_Id);
		itemsCraftedStats->push_back(itemStat);
		itemsCrafted[itemStat->getItemId()] = itemStat;
		itemStat->postConstruct();

		itemStat = new ItemStat(offset++, L"mineBlock.emerald", Tile::emeraldOre_Id);
		blocksMinedStats->push_back(itemStat);
		blocksMined[itemStat->getItemId()] = itemStat;
		itemStat->postConstruct();

		// 4J-JEV: We don't need itemsCollected(emerald) so I'm using it to
		// stor itemsBought(emerald) so I don't have to make yet another massive
		// StatArray for Items Bought.
		itemStat = new ItemStat(offset++, L"itemsBought.emerald", Item::emerald_Id);
		itemsCollectedStats->push_back(itemStat);
		itemsCollected[itemStat->getItemId()] = itemStat;
		itemStat->postConstruct();

		// 4J-JEV:	WHY ON EARTH DO THESE ARRAYS HAVE TO BE SO PAINFULLY LARGE WHEN THEY ARE GOING TO BE MOSTLY EMPTY!!!
		//			Either way, I'm making this one smaller because we don't need those record items (and we only need 2).
		blocksPlaced = StatArray(1000);

		itemStat = new ItemStat(offset++, L"blockPlaced.flowerPot", Tile::flowerPot_Id);
		blocksPlacedStats->push_back(itemStat);
		blocksPlaced[itemStat->getItemId()] = itemStat;
		itemStat->postConstruct();

		itemStat = new ItemStat(offset++, L"blockPlaced.sign", Tile::sign_Id);
		blocksPlacedStats->push_back(itemStat);
		blocksPlaced[itemStat->getItemId()] = itemStat;
		itemStat->postConstruct();

		itemStat = new ItemStat(offset++, L"blockPlaced.wallsign", Tile::wallSign_Id);
		blocksPlacedStats->push_back(itemStat);
		blocksPlaced[itemStat->getItemId()] = itemStat;
		itemStat->postConstruct();

		GeneralStat *generalStat = nullptr;

		rainbowCollection = StatArray(16);
		for (unsigned int i = 0; i < 16; i++)
		{
			generalStat = new GeneralStat(offset++, L"rainbowCollection." + std::to_wstring(i));
			generalStats->push_back(generalStat);
			rainbowCollection[i] = generalStat;
			generalStat->postConstruct();
		}

		biomesVisisted = StatArray(23);
		for (unsigned int i = 0; i < 23; i++)
		{
			generalStat = new GeneralStat(offset++, L"biomesVisited." + std::to_wstring(i));
			generalStats->push_back(generalStat);
			biomesVisisted[i] = generalStat;
			generalStat->postConstruct();
		}

		itemStat = new ItemStat(offset++, L"itemCrafted.porkchop", Item::porkChop_cooked_Id);
		itemsCraftedStats->push_back(itemStat);
		itemsCrafted[itemStat->getItemId()] = itemStat;
		itemStat->postConstruct();

		itemStat = new ItemStat(offset++, L"itemEaten.porkchop", Item::porkChop_cooked_Id);
		blocksPlacedStats->push_back(itemStat);
		blocksPlaced[itemStat->getItemId()] = itemStat;
		itemStat->postConstruct();
	}
#endif

}

Stat *Stats::get(int key)
{
	return statsById->at(key);
}
