#include "stdafx.h"

#include "Monster.h"

#include "GenericStats.h"

GenericStats *GenericStats::instance = nullptr;

Stat* GenericStats::get_walkOneM()
{
	return nullptr;
}

Stat* GenericStats::get_swimOneM()
{
	return nullptr;
}

Stat* GenericStats::get_fallOneM()
{
	return nullptr;
}

Stat* GenericStats::get_climbOneM()
{
	return nullptr;
}

Stat* GenericStats::get_minecartOneM()
{
	return nullptr;
}

Stat* GenericStats::get_boatOneM()
{
	return nullptr;
}

Stat* GenericStats::get_pigOneM()
{
	return nullptr;
}

Stat* GenericStats::get_portalsCreated()
{
	return nullptr;
}

Stat* GenericStats::get_cowsMilked()
{
	return nullptr;
}

Stat* GenericStats::get_netherLavaCollected()
{
	return nullptr;
}

Stat* GenericStats::get_killMob()
{
	return nullptr; 
}

Stat* GenericStats::get_killsZombie()
{ 
	return nullptr; 
}

Stat* GenericStats::get_killsSkeleton() 
{ 
	return nullptr; 
}

Stat* GenericStats::get_killsCreeper() 
{ 
	return nullptr;
}

Stat* GenericStats::get_killsSpider() 
{ 
	return nullptr; 
}

Stat* GenericStats::get_killsSpiderJockey() 
{ 
	return nullptr;
}

Stat* GenericStats::get_killsZombiePigman() 
{ 
	return nullptr; 
}

Stat* GenericStats::get_killsSlime() 
{ 
	return nullptr;
}

Stat* GenericStats::get_killsGhast() 
{ 
	return nullptr;
}

Stat* GenericStats::get_killsNetherZombiePigman() 
{ 
	return nullptr;
}

Stat* GenericStats::get_breedEntity(eINSTANCEOF entityId)
{
	return nullptr;
}

Stat* GenericStats::get_tamedEntity(eINSTANCEOF entityId)
{
	return nullptr;
}

Stat* GenericStats::get_curedEntity(eINSTANCEOF entityId)
{
	return nullptr;
}

Stat* GenericStats::get_craftedEntity(eINSTANCEOF entityId)
{
	return nullptr;
}

Stat* GenericStats::get_shearedEntity(eINSTANCEOF entityId)
{
	return nullptr;
}

Stat* GenericStats::get_totalBlocksMined()
{
	return nullptr;
}

Stat* GenericStats::get_timePlayed()
{
	return nullptr;
}

Stat* GenericStats::get_blocksPlaced(int blockId)
{
	return nullptr;
}

Stat* GenericStats::get_blocksMined(int blockId)
{
	return nullptr;
}

Stat* GenericStats::get_itemsCollected(int itemId, int itemAux)
{
	return nullptr;
}

Stat* GenericStats::get_itemsCrafted(int itemId)
{
	return nullptr;
}

Stat* GenericStats::get_itemsSmelted(int itemId)
{
	return this->get_itemsCrafted(itemId);
}

Stat* GenericStats::get_itemsUsed(int itemId)
{
	return nullptr;
}

Stat *GenericStats::get_itemsBought(int itemId)
{
	return nullptr;	
}

Stat* GenericStats::get_killsEnderdragon()
{
	return nullptr;
}

Stat* GenericStats::get_completeTheEnd()
{
	return nullptr;
}

Stat* GenericStats::get_changedDimension(int from, int to)
{
	return nullptr;
}

Stat* GenericStats::get_enteredBiome(int biomeId)
{
	return nullptr;
}

Stat* GenericStats::get_achievement(eAward achievementId)
{
	return nullptr;
}

Stat* GenericStats::openInventory() 
{
	return instance->get_achievement( eAward_TakingInventory ); 
}

Stat* GenericStats::mineWood() 
{
	return instance->get_achievement( eAward_GettingWood ); 
}

Stat* GenericStats::buildWorkbench() 
{
	return instance->get_achievement( eAward_Benchmarking ); 
}

Stat* GenericStats::buildPickaxe() 
{
	return instance->get_achievement( eAward_TimeToMine); 
}

Stat* GenericStats::buildFurnace() 
{
	return instance->get_achievement( eAward_HotTopic ); 
}

Stat* GenericStats::acquireIron() 
{
	return instance->get_achievement( eAward_AquireHardware ); 
}

Stat* GenericStats::buildHoe() 
{
	return instance->get_achievement( eAward_TimeToFarm ); 
}

Stat* GenericStats::makeBread() 
{
	return instance->get_achievement( eAward_BakeBread ); 
}

Stat* GenericStats::bakeCake() 
{
	return instance->get_achievement( eAward_TheLie ); 
}

Stat* GenericStats::buildBetterPickaxe() 
{
	return instance->get_achievement( eAward_GettingAnUpgrade ); 
}

Stat* GenericStats::cookFish() 
{
	return instance->get_achievement( eAward_DeliciousFish );
}

Stat* GenericStats::onARail() 
{
	return instance->get_achievement( eAward_OnARail );
}

Stat* GenericStats::buildSword() 
{
	return instance->get_achievement( eAward_TimeToStrike );
}

Stat* GenericStats::killEnemy() 
{
	return instance->get_achievement( eAward_MonsterHunter );
}

Stat* GenericStats::killCow() 
{
	return instance->get_achievement( eAward_CowTipper );
}

Stat* GenericStats::flyPig() 
{
	return instance->get_achievement( eAward_WhenPigsFly );
}

Stat* GenericStats::snipeSkeleton() 
{
#ifndef _XBOX
	return instance->get_achievement( eAward_snipeSkeleton );
#else
	return nullptr;
#endif
}

Stat* GenericStats::diamonds() 
{
#ifndef _XBOX
	return instance->get_achievement( eAward_diamonds );
#else
	return nullptr;
#endif
}

Stat* GenericStats::ghast() 
{
#ifndef _XBOX
	return instance->get_achievement( eAward_ghast );
#else
	return nullptr;
#endif
}

Stat* GenericStats::blazeRod() 
{
#ifndef _XBOX
	return instance->get_achievement( eAward_blazeRod );
#else
	return nullptr;
#endif
}

Stat* GenericStats::potion() 
{
#ifndef _XBOX
	return instance->get_achievement( eAward_potion );
#else
	return nullptr;
#endif
}

Stat* GenericStats::theEnd() 
{
#ifndef _XBOX
	return instance->get_achievement( eAward_theEnd );
#else
	return nullptr;
#endif
}

Stat* GenericStats::winGame() 
{
#ifndef _XBOX
	return instance->get_achievement( eAward_winGame );
#else
	return nullptr;
#endif
}

Stat* GenericStats::enchantments() 
{
#ifndef _XBOX
	return instance->get_achievement( eAward_enchantments );
#else
	return nullptr;
#endif
}

Stat* GenericStats::overkill() 
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement( eAward_overkill );
#else
	return nullptr;
#endif
}

Stat* GenericStats::bookcase()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement( eAward_bookcase );
#else
	return nullptr;
#endif
}

Stat* GenericStats::leaderOfThePack() 
{
	return instance->get_achievement( eAward_LeaderOfThePack );
}

Stat* GenericStats::MOARTools() 
{
	return instance->get_achievement( eAward_MOARTools );
}

Stat* GenericStats::dispenseWithThis() 
{
	return instance->get_achievement( eAward_DispenseWithThis );
}

Stat* GenericStats::InToTheNether() 
{
	return instance->get_achievement( eAward_InToTheNether );
}

Stat* GenericStats::socialPost() 
{
	return instance->get_achievement( eAward_socialPost );
}

Stat* GenericStats::eatPorkChop() 
{
	return instance->get_achievement( eAward_eatPorkChop );
}

Stat* GenericStats::play100Days() 
{
	return instance->get_achievement( eAward_play100Days );
}

Stat* GenericStats::arrowKillCreeper() 
{
	return instance->get_achievement( eAward_arrowKillCreeper );
}

Stat* GenericStats::mine100Blocks() 
{
	return instance->get_achievement( eAward_mine100Blocks );
}

Stat* GenericStats::kill10Creepers() 
{
	return instance->get_achievement( eAward_kill10Creepers );
}

Stat* GenericStats::adventuringTime()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_adventuringTime);
#else
	return nullptr;
#endif
}

Stat* GenericStats::repopulation()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_repopulation);
#else
	return nullptr;
#endif
}

Stat* GenericStats::porkChop()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_eatPorkChop);
#else
	return nullptr;
#endif
}

Stat* GenericStats::diamondsToYou()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_diamondsToYou);
#else
	return nullptr;
#endif
}

Stat* GenericStats::passingTheTime()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_play100Days);
#else
	return nullptr;
#endif
}

Stat* GenericStats::archer()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_arrowKillCreeper);
#else
	return nullptr;
#endif
}

Stat* GenericStats::theHaggler()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_theHaggler);
#else
	return nullptr;
#endif
}

Stat* GenericStats::potPlanter()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_potPlanter);
#else
	return nullptr;
#endif
}

Stat* GenericStats::itsASign()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_itsASign);
#else
	return nullptr;
#endif
}

Stat* GenericStats::ironBelly()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_ironBelly);
#else
	return nullptr;
#endif
}

Stat* GenericStats::haveAShearfulDay()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_haveAShearfulDay);
#else
	return nullptr;
#endif
}

Stat* GenericStats::rainbowCollection()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_rainbowCollection);
#else
	return nullptr;
#endif
}

Stat* GenericStats::stayinFrosty()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_stayinFrosty);
#else
	return nullptr;
#endif
}

Stat* GenericStats::chestfulOfCobblestone()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_chestfulOfCobblestone);
#else
	return nullptr;
#endif
}

Stat* GenericStats::renewableEnergy()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_renewableEnergy);
#else
	return nullptr;
#endif
}

Stat* GenericStats::musicToMyEars()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_musicToMyEars);
#else
	return nullptr;
#endif
}

Stat* GenericStats::bodyGuard()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_bodyGuard);
#else
	return nullptr;
#endif
}

Stat* GenericStats::ironMan()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_ironMan);
#else
	return nullptr;
#endif
}

Stat* GenericStats::zombieDoctor()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_zombieDoctor);
#else
	return nullptr;
#endif
}

Stat* GenericStats::lionTamer()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->get_achievement(eAward_lionTamer);
#else
	return nullptr;
#endif
}

byteArray GenericStats::getParam_walkOneM(int distance)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_swimOneM(int distance)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_fallOneM(int distance)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_climbOneM(int distance)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_minecartOneM(int distance)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_boatOneM(int distance)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_pigOneM(int distance)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_cowsMilked()
{
	return getParam_noArgs();
}

byteArray GenericStats::getParam_blocksPlaced(int id, int data, int count)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_blocksMined(int id, int data, int count)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_itemsCollected(int id, int aux, int count)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_itemsCrafted(int id, int aux, int count)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_itemsSmelted(int id, int aux, int count)
{
	return this->getParam_itemsCrafted(id,aux,count);
}

byteArray GenericStats::getParam_itemsUsed(shared_ptr<Player> plr, shared_ptr<ItemInstance> itm)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_itemsBought(int id, int aux, int count)
{
	return getParam_noArgs();
}

byteArray GenericStats::getParam_mobKill(shared_ptr<Player> plr, shared_ptr<Mob> mob, DamageSource *dmgSrc)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_breedEntity(eINSTANCEOF entityId)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_tamedEntity(eINSTANCEOF entityId)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_curedEntity(eINSTANCEOF entityId)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_craftedEntity(eINSTANCEOF entityId)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_shearedEntity(eINSTANCEOF entityId)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_time(int timediff)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_changedDimension(int from, int to)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_enteredBiome(int biomeId)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_achievement(eAward id)
{
	return getParam_noArgs(); // Really just a count on most platforms.
}

byteArray GenericStats::getParam_onARail(int distance)
{
	return getParam_achievement(eAward_OnARail);
}

byteArray GenericStats::getParam_overkill(int damage)
{
#ifdef _XBOX
	return getParam_noArgs();
#else
	return getParam_achievement(eAward_overkill);
#endif
}

byteArray GenericStats::getParam_openInventory(int menuId)
{
	return getParam_achievement(eAward_TakingInventory);
}

byteArray GenericStats::getParam_chestfulOfCobblestone(int cobbleStone)
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return getParam_achievement(eAward_chestfulOfCobblestone);
#else
	return getParam_noArgs();
#endif
}

byteArray GenericStats::getParam_musicToMyEars(int recordId)
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return getParam_achievement(eAward_musicToMyEars);
#else
	return getParam_noArgs();
#endif
}

byteArray GenericStats::getParam_noArgs()
{
	return byteArray(); // TODO
}

byteArray GenericStats::param_walk(int distance)
{
	return instance->getParam_walkOneM(distance);
}

byteArray GenericStats::param_swim(int distance)
{
	return instance->getParam_swimOneM(distance);
}

byteArray GenericStats::param_fall(int distance)
{
	return instance->getParam_fallOneM(distance);
}

byteArray GenericStats::param_climb(int distance)
{
	return instance->getParam_climbOneM(distance);
}

byteArray GenericStats::param_minecart(int distance)
{
	return instance->getParam_minecartOneM(distance);
}

byteArray GenericStats::param_boat(int distance)
{
	return instance->getParam_boatOneM(distance);
}

byteArray GenericStats::param_pig(int distance)
{
	return instance->getParam_pigOneM(distance);
}

byteArray GenericStats::param_cowsMilked()
{
	return instance->getParam_cowsMilked();
}

byteArray GenericStats::param_blocksPlaced(int id, int data, int count)
{
	return instance->getParam_blocksPlaced(id,data,count);
}

byteArray GenericStats::param_blocksMined(int id, int data, int count)
{
	return instance->getParam_blocksMined(id,data,count);
}

byteArray GenericStats::param_itemsCollected(int id, int aux, int count)
{ 
	return instance->getParam_itemsCollected(id,aux,count); 
}

byteArray GenericStats::param_itemsCrafted(int id, int aux, int count)
{ 
	return instance->getParam_itemsCrafted(id,aux,count); 
}

byteArray GenericStats::param_itemsSmelted(int id, int aux, int count)
{
	return instance->getParam_itemsSmelted(id,aux,count);
}

byteArray GenericStats::param_itemsUsed(shared_ptr<Player> plr, shared_ptr<ItemInstance> itm)
{
	if ( (plr != nullptr) && (itm != nullptr) )	return instance->getParam_itemsUsed(plr, itm);
	else									return instance->getParam_noArgs();
}

byteArray GenericStats::param_itemsBought(int id, int aux, int count)
{
	return instance->getParam_itemsBought(id,aux,count);
}

byteArray GenericStats::param_mobKill(shared_ptr<Player> plr, shared_ptr<Mob> mob, DamageSource *dmgSrc)
{
	if ( (plr != nullptr) && (mob != nullptr) )	return instance->getParam_mobKill(plr, mob, dmgSrc);	
	else									return instance->getParam_noArgs();
}

byteArray GenericStats::param_breedEntity(eINSTANCEOF entityId)
{
	return instance->getParam_breedEntity(entityId);
}

byteArray GenericStats::param_tamedEntity(eINSTANCEOF entityId)
{
	return instance->getParam_tamedEntity(entityId);
}

byteArray GenericStats::param_curedEntity(eINSTANCEOF entityId)
{
	return instance->getParam_curedEntity(entityId);
}

byteArray GenericStats::param_craftedEntity(eINSTANCEOF entityId)
{
	return instance->getParam_craftedEntity(entityId);
}

byteArray GenericStats::param_shearedEntity(eINSTANCEOF entityId)
{
	return instance->getParam_shearedEntity(entityId);
}

byteArray GenericStats::param_time(int timediff)
{
	return instance->getParam_time(timediff);
}

byteArray GenericStats::param_changedDimension(int from, int to)
{
	return instance->getParam_changedDimension(from,to);
}

byteArray GenericStats::param_enteredBiome(int biomeId)
{
	return instance->getParam_enteredBiome(biomeId);
}

byteArray GenericStats::param_noArgs()
{ 
	return instance->getParam_noArgs(); 
}

byteArray GenericStats::param_openInventory()
{
	return instance->getParam_achievement(eAward_TakingInventory);
}

byteArray GenericStats::param_mineWood()
{
	return instance->getParam_achievement(eAward_GettingWood);
}

byteArray GenericStats::param_buildWorkbench()
{
	return instance->getParam_achievement(eAward_Benchmarking);
}

byteArray GenericStats::param_buildPickaxe()
{
	return instance->getParam_achievement(eAward_TimeToMine);
}

byteArray GenericStats::param_buildFurnace()
{
	return instance->getParam_achievement(eAward_HotTopic);
}

byteArray GenericStats::param_acquireIron()
{
	return instance->getParam_achievement(eAward_AquireHardware);
}

byteArray GenericStats::param_buildHoe()
{
	return instance->getParam_achievement(eAward_TimeToFarm);
}

byteArray GenericStats::param_makeBread()
{
	return instance->getParam_achievement(eAward_BakeBread);
}

byteArray GenericStats::param_bakeCake()
{
	return instance->getParam_achievement(eAward_TheLie);
}

byteArray GenericStats::param_buildBetterPickaxe()
{
	return instance->getParam_achievement(eAward_GettingAnUpgrade);
}

byteArray GenericStats::param_cookFish()
{
	return instance->getParam_achievement(eAward_DeliciousFish);
}

byteArray GenericStats::param_onARail(int distance)
{
	return instance->getParam_onARail(distance);
}

byteArray GenericStats::param_buildSword()
{
	return instance->getParam_achievement(eAward_TimeToStrike);
}

byteArray GenericStats::param_killEnemy()
{
	return instance->getParam_achievement(eAward_MonsterHunter);
}

byteArray GenericStats::param_killCow()
{
	return instance->getParam_achievement(eAward_CowTipper);
}

byteArray GenericStats::param_flyPig()
{
	return instance->getParam_achievement(eAward_WhenPigsFly);
}

byteArray GenericStats::param_snipeSkeleton()
{
#ifdef _XBOX
	return instance->getParam_noArgs();
#else
	return instance->getParam_achievement(eAward_snipeSkeleton);
#endif
}

byteArray GenericStats::param_diamonds()
{
#ifdef _XBOX
	return instance->getParam_noArgs();
#else
	return instance->getParam_achievement(eAward_diamonds);
#endif
}

byteArray GenericStats::param_ghast()
{
#ifdef _XBOX
	return instance->getParam_noArgs();
#else
	return instance->getParam_achievement(eAward_ghast);
#endif
}

byteArray GenericStats::param_blazeRod()
{
#ifdef _XBOX
	return instance->getParam_noArgs();
#else
	return instance->getParam_achievement(eAward_blazeRod);
#endif
}

byteArray GenericStats::param_potion()
{
#ifdef _XBOX
	return instance->getParam_noArgs();
#else
	return instance->getParam_achievement(eAward_potion);
#endif
}

byteArray GenericStats::param_theEnd()
{
#ifdef _XBOX
	return instance->getParam_noArgs();
#else
	return instance->getParam_achievement(eAward_theEnd);
#endif
}

byteArray GenericStats::param_winGame()
{
#ifdef _XBOX
	return instance->getParam_noArgs();
#else
	return instance->getParam_achievement(eAward_winGame);
#endif
}

byteArray GenericStats::param_enchantments()
{
#ifdef _XBOX
	return instance->getParam_noArgs();
#else
	return instance->getParam_achievement(eAward_enchantments);
#endif
}

byteArray GenericStats::param_overkill(int dmg)
{
	return instance->getParam_overkill(dmg);
}

byteArray GenericStats::param_bookcase()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_bookcase);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_leaderOfThePack()
{
	return instance->getParam_achievement(eAward_LeaderOfThePack);
}

byteArray GenericStats::param_MOARTools()
{
	return instance->getParam_achievement(eAward_MOARTools);
}

byteArray GenericStats::param_dispenseWithThis()
{
	return instance->getParam_achievement(eAward_DispenseWithThis);
}

byteArray GenericStats::param_InToTheNether()
{
	return instance->getParam_achievement(eAward_InToTheNether);
}

byteArray GenericStats::param_socialPost()
{
	return instance->getParam_achievement(eAward_socialPost);
}

byteArray GenericStats::param_eatPorkChop()
{
	return instance->getParam_achievement(eAward_eatPorkChop);
}

byteArray GenericStats::param_play100Days()
{
	return instance->getParam_achievement(eAward_play100Days);
}

byteArray GenericStats::param_arrowKillCreeper()
{
	return instance->getParam_achievement(eAward_arrowKillCreeper);
}

byteArray GenericStats::param_mine100Blocks()
{
	return instance->getParam_achievement(eAward_mine100Blocks);
}

byteArray GenericStats::param_kill10Creepers()
{
	return instance->getParam_achievement(eAward_kill10Creepers);
}

byteArray GenericStats::param_adventuringTime()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_adventuringTime);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_repopulation()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_repopulation);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_porkChop()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_eatPorkChop);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_diamondsToYou()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_diamondsToYou);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_passingTheTime()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_play100Days);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_archer()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_arrowKillCreeper);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_theHaggler()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_theHaggler);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_potPlanter()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_potPlanter);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_itsASign()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_itsASign);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_ironBelly()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_ironBelly);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_haveAShearfulDay()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_haveAShearfulDay);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_rainbowCollection()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_rainbowCollection);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_stayinFrosty()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_stayinFrosty);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_chestfulOfCobblestone(int cobbleStone)
{
	return instance->getParam_chestfulOfCobblestone(cobbleStone);
}

byteArray GenericStats::param_renewableEnergy()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_renewableEnergy);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_musicToMyEars(int recordId)
{
	return instance->getParam_musicToMyEars(recordId);
}

byteArray GenericStats::param_bodyGuard()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_bodyGuard);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_ironMan()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_ironMan);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_zombieDoctor()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_zombieDoctor);
#else
	return instance->getParam_noArgs();
#endif
}

byteArray GenericStats::param_lionTamer()
{
#ifdef _EXTENDED_ACHIEVEMENTS
	return instance->getParam_achievement(eAward_lionTamer);
#else
	return instance->getParam_noArgs();
#endif
}
