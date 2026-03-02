#include "stdafx.h"
#include "Achievements.h"
#include "Item.h"
#include "Tile.h"
#include "CommonStats.h"

Stat *CommonStats::get_stat(int i)
{
	return Stats::get(i);
}

Stat* CommonStats::get_walkOneM()	{ return Stats::walkOneM; }

Stat* CommonStats::get_swimOneM()	{ return Stats::swimOneM; }

Stat* CommonStats::get_fallOneM()	{ return Stats::fallOneM; }

Stat* CommonStats::get_climbOneM()	{ return Stats::climbOneM; }

Stat* CommonStats::get_minecartOneM()	{ return Stats::minecartOneM; }

Stat* CommonStats::get_boatOneM()	{ return Stats::boatOneM; }

Stat* CommonStats::get_pigOneM()	{ return Stats::pigOneM; }

Stat* CommonStats::get_portalsCreated()	{ return Stats::portalsCreated; }

Stat* CommonStats::get_cowsMilked() { return Stats::cowsMilked; }

Stat* CommonStats::get_netherLavaCollected()	{ return Stats::netherLavaCollected; }

Stat* CommonStats::get_killsZombie()	{ return Stats::killsZombie; }

Stat* CommonStats::get_killsSkeleton()	{ return Stats::killsSkeleton; }

Stat* CommonStats::get_killsCreeper()	{ return Stats::killsCreeper; }

Stat* CommonStats::get_killsSpider() { return Stats::killsSpider; }

Stat* CommonStats::get_killsSpiderJockey() { return Stats::killsSpiderJockey; }

Stat* CommonStats::get_killsZombiePigman() { return Stats::killsZombiePigman; }

Stat* CommonStats::get_killsSlime() { return Stats::killsSlime; }

Stat* CommonStats::get_killsGhast() { return Stats::killsGhast; }

Stat* CommonStats::get_killsNetherZombiePigman() { return Stats::killsNetherZombiePigman; }

Stat *CommonStats::get_breedEntity(eINSTANCEOF mobType)
{
	if (mobType == eTYPE_COW)			return GenericStats::repopulation();
	else								return NULL;
}

Stat *CommonStats::get_tamedEntity(eINSTANCEOF mobType)
{
	if (mobType == eTYPE_OCELOT)		return GenericStats::lionTamer();
	else if (mobType == eTYPE_WOLF)		return Stats::befriendsWolf;
	else								return NULL;
}

Stat *CommonStats::get_craftedEntity(eINSTANCEOF mobType)
{
	if (mobType == eTYPE_VILLAGERGOLEM)	return GenericStats::bodyGuard();
	else								return NULL;
}

Stat *CommonStats::get_shearedEntity(eINSTANCEOF mobType)
{
	if (mobType == eTYPE_SHEEP)			return GenericStats::haveAShearfulDay();
	else								return NULL;
}

Stat *CommonStats::get_totalBlocksMined() { return Stats::totalBlocksMined; }

Stat* CommonStats::get_timePlayed() { return Stats::timePlayed; }

Stat* CommonStats::get_blocksPlaced(int blockId)
{ 
#if (defined _EXTENDED_ACHIEVEMENTS) && (!defined _XBOX_ONE)
	return Stats::blocksPlaced[blockId];
#else
	return NULL;
#endif
}

Stat *CommonStats::get_blocksMined(int blockId) { return Stats::blocksMined[blockId]; }

Stat *CommonStats::get_itemsCollected(int itemId, int itemAux)
{ 
	// 4J-JEV: We don't need itemsCollected(emerald) so I'm using it to
	// stor itemsBought(emerald) so I don't have to make yet another massive
	// StatArray for Items Bought.
#if (defined _EXTENDED_ACHIEVEMENTS) && (!defined _XBOX_ONE)
	if (itemId == Tile::wool_Id)			return Stats::rainbowCollection[itemAux];
#endif

	if (itemId != Item::emerald_Id)			return Stats::itemsCollected[itemId];
	else									return NULL;
}

Stat *CommonStats::get_itemsCrafted(int itemId) { return Stats::itemsCrafted[itemId]; }

Stat *CommonStats::get_itemsSmelted(int itemId) { return Stats::itemsCrafted[itemId]; }

Stat *CommonStats::get_itemsUsed(int itemId)
{
#if (defined _EXTENDED_ACHIEVEMENTS) && (!defined _XBOX_ONE)
	// 4J-JEV: I've done the same thing here, we can't place these items anyway.
	if (itemId == Item::porkChop_cooked_Id)	return Stats::blocksPlaced[itemId];
#endif

	return NULL;
}

Stat *CommonStats::get_itemsBought(int itemId)
{
	// 4J-JEV: We don't need itemsCollected(emerald) so I'm using it to
	// stor itemsBought(emerald) so I don't have to make yet another massive
	// StatArray for Items Bought.

	if (itemId == Item::emerald_Id)	return Stats::itemsCollected[itemId];
	else							return NULL;
}

Stat *CommonStats::get_killsEnderdragon() { return Stats::killsEnderdragon; }

Stat *CommonStats::get_completeTheEnd() { return Stats::completeTheEnd; }

Stat *CommonStats::get_enteredBiome(int biomeId)
{
#if (defined _EXTENDED_ACHIEVEMENTS) && (!defined _XBOX_ONE)
	return Stats::biomesVisisted[biomeId];
#else
	return NULL;
#endif
}

Stat *CommonStats::get_achievement(eAward achievementId)
{
	switch (achievementId)
	{
	case eAward_TakingInventory:	return (Stat *) Achievements::openInventory;	
	case eAward_GettingWood:		return (Stat *) Achievements::mineWood;
	case eAward_Benchmarking:		return (Stat *) Achievements::buildWorkbench;		
	case eAward_TimeToMine:			return (Stat *) Achievements::buildPickaxe;
	case eAward_HotTopic:			return (Stat *) Achievements::buildFurnace;
	case eAward_AquireHardware:		return (Stat *) Achievements::acquireIron;
	case eAward_TimeToFarm:			return (Stat *) Achievements::buildHoe;
	case eAward_BakeBread:			return (Stat *) Achievements::makeBread;
	case eAward_TheLie:				return (Stat *) Achievements::bakeCake;
	case eAward_GettingAnUpgrade:	return (Stat *) Achievements::buildBetterPickaxe;	
	case eAward_DeliciousFish:		return (Stat *) Achievements::cookFish;
	case eAward_OnARail:			return (Stat *) Achievements::onARail;			
	case eAward_TimeToStrike:		return (Stat *) Achievements::buildSword;		
	case eAward_MonsterHunter:		return (Stat *) Achievements::killEnemy;
	case eAward_CowTipper:			return (Stat *) Achievements::killCow;
	case eAward_WhenPigsFly:		return (Stat *) Achievements::flyPig;
	case eAward_LeaderOfThePack:	return (Stat *) Achievements::leaderOfThePack;
	case eAward_MOARTools:			return (Stat *) Achievements::MOARTools;
	case eAward_DispenseWithThis:	return (Stat *) Achievements::dispenseWithThis;
	case eAward_InToTheNether:		return (Stat *) Achievements::InToTheNether;
	case eAward_mine100Blocks:		return (Stat *) Achievements::mine100Blocks;
	case eAward_kill10Creepers:		return (Stat *) Achievements::kill10Creepers;	
	case eAward_eatPorkChop:		return (Stat *) Achievements::eatPorkChop;
	case eAward_play100Days:		return (Stat *) Achievements::play100Days;
	case eAward_arrowKillCreeper:	return (Stat *) Achievements::arrowKillCreeper;	
	case eAward_socialPost:			return (Stat *) Achievements::socialPost;

#ifndef _XBOX
	case eAward_snipeSkeleton:		return (Stat *) Achievements::snipeSkeleton;
	case eAward_diamonds:			return (Stat *) Achievements::diamonds;
	case eAward_portal:				return (Stat *) NULL;						// TODO
	case eAward_ghast:				return (Stat *) Achievements::ghast;
	case eAward_blazeRod:			return (Stat *) Achievements::blazeRod;
	case eAward_potion:				return (Stat *) Achievements::potion;
	case eAward_theEnd:				return (Stat *) Achievements::theEnd;
	case eAward_winGame:			return (Stat *) Achievements::winGame;
	case eAward_enchantments:		return (Stat *) Achievements::enchantments;
#endif

#ifdef _EXTENDED_ACHIEVEMENTS
	case eAward_overkill:				return (Stat *) Achievements::overkill;		// Restored old ach.
	case eAward_bookcase:				return (Stat *) Achievements::bookcase;		// Restored old ach.

	case eAward_adventuringTime:		return (Stat *) Achievements::adventuringTime;
	case eAward_repopulation:			return (Stat *) Achievements::repopulation;
	case eAward_diamondsToYou:			return (Stat *) Achievements::diamondsToYou;
	//case eAward_passingTheTime:			return (Stat *) Achievements::passingTheTime;
	//case eAward_archer:					return (Stat *) Achievements::archer;
	case eAward_theHaggler:				return (Stat *) Achievements::theHaggler;
	case eAward_potPlanter:				return (Stat *) Achievements::potPlanter;
	case eAward_itsASign:				return (Stat *) Achievements::itsASign;
	case eAward_ironBelly:				return (Stat *) Achievements::ironBelly;
	case eAward_haveAShearfulDay:		return (Stat *) Achievements::haveAShearfulDay;
	case eAward_rainbowCollection:		return (Stat *) Achievements::rainbowCollection;
	case eAward_stayinFrosty:			return (Stat *) Achievements::stayinFrosty;
	case eAward_chestfulOfCobblestone:	return (Stat *) Achievements::chestfulOfCobblestone;
	case eAward_renewableEnergy:		return (Stat *) Achievements::renewableEnergy;
	case eAward_musicToMyEars:			return (Stat *) Achievements::musicToMyEars;
	case eAward_bodyGuard:				return (Stat *) Achievements::bodyGuard;
	case eAward_ironMan:				return (Stat *) Achievements::ironMan;
	case eAward_zombieDoctor:			return (Stat *) Achievements::zombieDoctor;
	case eAward_lionTamer:				return (Stat *) Achievements::lionTamer;
#endif

	default:						return (Stat *) NULL;
	}
}

byteArray CommonStats::getParam_walkOneM(int distance)
{
	return makeParam(distance);
}

byteArray CommonStats::getParam_swimOneM(int distance)
{
	return makeParam(distance);
}

byteArray CommonStats::getParam_fallOneM(int distance)
{
	return makeParam(distance);
}

byteArray CommonStats::getParam_climbOneM(int distance)
{
	return makeParam(distance);
}

byteArray CommonStats::getParam_minecartOneM(int distance)
{
	return makeParam(distance);
}

byteArray CommonStats::getParam_boatOneM(int distance)
{
	return makeParam(distance);
}

byteArray CommonStats::getParam_pigOneM(int distance)
{
	return makeParam(distance);
}

byteArray CommonStats::getParam_blocksMined(int id, int data, int count)
{
	return makeParam(count);
}

byteArray CommonStats::getParam_itemsCollected(int id, int aux, int count)
{
	return makeParam(count);
}

byteArray CommonStats::getParam_itemsCrafted(int id, int aux, int count)
{
	return makeParam(count);
}

byteArray CommonStats::getParam_itemsSmelted(int id, int aux, int count)
{
	return makeParam(count);
}

byteArray CommonStats::getParam_itemsUsed(int id, int aux, int count)
{
	return makeParam(count);
}

byteArray CommonStats::getParam_itemsBought(int id, int aux, int count)
{
	return makeParam(count);
}

byteArray CommonStats::getParam_time(int timediff)
{
	return makeParam(timediff);
}

byteArray CommonStats::getParam_noArgs()
{
	return makeParam();
}

byteArray CommonStats::makeParam(int count)
{
	byteArray out( sizeof(int) );
	memcpy(out.data,&count,sizeof(int));
	return out;
}

int CommonStats::readParam(byteArray paramBlob)
{
	if (paramBlob.length == sizeof(int))	return *( (int*) paramBlob.data );
	else									return 1;
}