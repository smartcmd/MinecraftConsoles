#include "stdafx.h"

#include "..\Minecraft.World\Tile.h"
#include "..\Minecraft.World\Item.h"

#include "..\Minecraft.World\DurangoStats.h"

#include "..\Minecraft.World\EntityIO.h"

#include "..\Minecraft.World\StringHelpers.h"

#include "Common\Console_Awards_enum.h"

#include "DurangoStatsDebugger.h"

namespace WFC = Windows::Foundation::Collections;
namespace CC = concurrency;

StatParam::StatParam(const wstring &base)
{
	m_base = base;
	//m_numArgs = numArgs;
	m_args = vector<int>();

    unsigned int count=0;
    wstring::size_type pos =base.find(L"*");
    while(pos!=string::npos)
    {
        count++;
        pos=base.find(L"*",pos+1);
    }

	m_numArgs = count;
}

void StatParam::addArgs(int v1, ...)
{
	va_list argptr;
	va_start(argptr, v1);
	m_args.push_back(v1);
	for (int i=0; i<(m_numArgs-1); i++)
	{
		int vi = va_arg(argptr, int);
		m_args.push_back(vi);
	}
	va_end(argptr);
}

vector<wstring> *StatParam::getStats()
{
	vector<wstring> *out = new vector<wstring>();
	
	static const int MAXSIZE = 256;
	static const wstring SUBSTR = L"*";

	wstring wstr_itr, wstr_num;

	if (m_args.size() <= 0 || m_numArgs <= 0) 
	{
		out->push_back(m_base);
	}
	else
	{
		for (int i = 0; i < m_args.size(); i += m_numArgs)
		{
			wstr_itr = m_base;

			if (m_numArgs > 0)
			{
				for (int j=0; j<m_numArgs; j++)
				{
					wstr_num = to_wstring(m_args.at(i+j));
					size_t sz = wstr_itr.find(SUBSTR);

					if (sz != wstring::npos)
						wstr_itr.replace(sz, SUBSTR.length(), wstr_num);
				}
			}

			out->push_back( wstr_itr );
		}
	}

	return out;
}

DurangoStatsDebugger *DurangoStatsDebugger::instance = NULL;

DurangoStatsDebugger::DurangoStatsDebugger()
{
	InitializeCriticalSection(&m_retrievedStatsLock);
}

vector<wstring> *DurangoStatsDebugger::getStats()
{
	vector<wstring> *out = new vector<wstring>();

	for (auto it = m_stats.begin(); it!=m_stats.end(); it++)
	{
		vector<wstring> *sublist = (*it)->getStats();
		out->insert(out->end(), sublist->begin(), sublist->end());
	}

	return out;
}

DurangoStatsDebugger *DurangoStatsDebugger::Initialize()
{
	DurangoStatsDebugger *out = new DurangoStatsDebugger(); 

	StatParam *sp = new StatParam(L"McItemAcquired.AcquisitionMethodId.*.ItemId.*.ItemAux.*");
	sp->addArgs(1, Tile::dirt_Id, 0); // works
	sp->addArgs(2, Tile::dirt_Id, 0); // works

	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Tile::cloth_Id,  0); // fixed (+ach 'Rainbow Collection')
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Tile::cloth_Id,  1);
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Tile::cloth_Id,  2);
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Tile::cloth_Id,  3);
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Tile::cloth_Id,  4);
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Tile::cloth_Id,  5);
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Tile::cloth_Id,  6);
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Tile::cloth_Id,  7);
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Tile::cloth_Id,  8);
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Tile::cloth_Id,  9);
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Tile::cloth_Id, 10);
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Tile::cloth_Id, 11);
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Tile::cloth_Id, 12);
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Tile::cloth_Id, 13);
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Tile::cloth_Id, 14);
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Tile::cloth_Id, 15);

	out->m_stats.push_back(sp);

	sp = new StatParam(L"McItemAcquired.AcquisitionMethodId.*.ItemId.*");
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Tile::dirt_Id);				// works
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Crafted,	Item::milk_Id);				// works.
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Crafted,	Tile::dirt_Id);				// works.
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Crafted,	Item::porkChop_cooked_Id);	// BROKEN! (ach 'Pork Chop' configured incorrectly)
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Crafted,	Item::cake_Id);				// works. (ach 'The Lie' configured incorrectly)
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Bought,		Item::emerald_Id);			// fixed (+ach)
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Crafted,	Item::ironIngot_Id);		// works. (+ach 'Acquired Hardware')
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Pickedup,	Item::fish_raw_Id);			// works. (+ach 'Delicious Fish')
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Crafted,	Item::fish_cooked_Id);		// works. (+ach 'Delicious Fish')
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Crafted,	Item::sign_Id);
	sp->addArgs(DsItemEvent::eAcquisitionMethod_Crafted,	Item::flowerPot_Id);		// FIXING!
	out->m_stats.push_back(sp);

	sp = new StatParam(L"McItemAcquired.DifficultyLevelId.*.AcquisitionMethodId.*.ItemId.*");
	sp->addArgs(1, 1, Tile::dirt_Id); // works
	sp->addArgs(2, 2, Tile::dirt_Id); // works
	out->m_stats.push_back(sp);

	sp = new StatParam(L"McItemUsed.ItemId.*.ItemAux.*");
	//sp->addArgs(Item::apple_Id, 0); 
	//sp->addArgs(Item::cake_Id, 0);
	sp->addArgs(Item::beef_raw_Id, 0);			// works
	sp->addArgs(Item::porkChop_cooked_Id, 0);	// works
	out->m_stats.push_back(sp);

	sp = new StatParam(L"MinHungerWhenEaten.ItemId.*");
	//sp->addArgs(Item::apple_Id); 
	//sp->addArgs(Item::cake_Id);
	sp->addArgs(Item::beef_raw_Id); // works
	sp->addArgs(Item::rotten_flesh_Id); // works (+ach IronBelly)
	out->m_stats.push_back(sp);

	sp = new StatParam(L"BlockBroken.BlockId.*");
	sp->addArgs( Tile::dirt_Id );
	sp->addArgs( Tile::rock_Id ); 
	sp->addArgs( Tile::emeraldOre_Id );
	out->m_stats.push_back(sp);

	sp = new StatParam(L"BlockBroken.BlockId.*.BlockAux.*");
	sp->addArgs( Tile::dirt_Id, 0 );
	sp->addArgs( Tile::rock_Id, 0 );
	out->m_stats.push_back(sp);

	sp = new StatParam(L"BlockBroken.DifficultyLevelId.*.BlockId.*");
	sp->addArgs( 1, Tile::dirt_Id );
	sp->addArgs( 2, Tile::dirt_Id );
	sp->addArgs( 1, Tile::rock_Id );
	sp->addArgs( 2, Tile::rock_Id );
	out->m_stats.push_back(sp);

	sp = new StatParam(L"BlockBroken");
	sp->addArgs( -1 );
	out->m_stats.push_back(sp);

	sp = new StatParam(L"BlockPlaced.BlockId.*");
	sp->addArgs( Tile::dirt_Id );
	sp->addArgs( Tile::stoneBrick_Id ); 
	sp->addArgs( Tile::sand_Id );		// works
	sp->addArgs( Tile::sign_Id );		// fixed
	sp->addArgs( Tile::wallSign_Id );	// fixed
	out->m_stats.push_back(sp);

	sp = new StatParam(L"MobKilled.KillTypeId.*.EnemyRoleId.*.PlayerWeaponId.*"); // BROKEN!
	sp->addArgs( /*MELEE*/ 0, ioid_Cow, 0 );
	sp->addArgs( /*MELEE*/ 0, ioid_Cow, Item::sword_stone_Id );
	sp->addArgs( /*MELEE*/ 0, ioid_Pig, Item::sword_stone_Id );
	out->m_stats.push_back(sp);

	sp = new StatParam(L"MaxKillDistance.KillTypeId.*.EnemyRoleId.*.PlayerWeaponId.*"); // BROKEN!
	sp->addArgs( /*MELEE*/ 0, ioid_Cow, Item::sword_stone_Id );
	sp->addArgs( /*MELEE*/ 0, ioid_Pig, Item::sword_stone_Id );
	sp->addArgs( /*RANGE*/ 1, ioid_Creeper, ioid_Arrow );			// FIXING!
	out->m_stats.push_back(sp);

	sp = new StatParam(L"MobKilled.KillTypeId.*.EnemyRoleId.*"); // BROKEN!
	sp->addArgs( /*MELEE*/ 0, ioid_Cow );
	sp->addArgs( /*MELEE*/ 0, ioid_Pig );
	out->m_stats.push_back(sp);

	sp = new StatParam(L"MobKilled.EnemyRoleId.*"); // BROKEN!
	sp->addArgs( ioid_Cow );
	sp->addArgs( ioid_Pig );
	sp->addArgs( ioid_Zombie );
	sp->addArgs( ioid_Spiderjocky );
	out->m_stats.push_back(sp);

	sp = new StatParam(L"MobKilledTotal.DifficultyLevelId.*.EnemyRoleId.*"); // BROKEN!
	sp->addArgs( 1, ioid_Cow );
	sp->addArgs( 2, ioid_Cow );
	out->m_stats.push_back(sp);

	sp = new StatParam(L"MobKilledTotal"); // BROKEN!
	sp->addArgs(-1);
	out->m_stats.push_back(sp);

	sp = new StatParam(L"MobInteract.MobId.*.Interaction.*");
	// sp->addArgs( ioid_Cow,	/*MILKED*/ 6 );	 no longer an interaction type.
	sp->addArgs( ioid_Cow,				/*BRED*/ 1 );		// fixed (+ach)
	sp->addArgs( ioid_Sheep,			/*SHEARED*/ 5 );	// works (+ach)
	sp->addArgs( ioid_VillagerGolem,	/*CRAFTED*/ 4 );	// works (+ach)
	sp->addArgs( ioid_Ozelot,			/*TAMED*/ 2 );		// works (+ach)
	sp->addArgs( ioid_Zombie,			/*CURED*/ 3 );		// fixed (+ach)
	out->m_stats.push_back(sp);

	sp = new StatParam(L"EnteredNewBiome.BiomeId.*");
	for (int i=0; i<20; i++) 
		sp->addArgs( i );
	out->m_stats.push_back(sp);

	sp = new StatParam(L"AchievementGet.AchievementId.*");
	sp->addArgs( eAward_TakingInventory );	// works (+ach)
	sp->addArgs( eAward_WhenPigsFly );		// works (+ach)
	sp->addArgs( eAward_InToTheNether );	// works (+ach)
	sp->addArgs( eAward_theEnd );			// works (+ach)
	sp->addArgs( eAward_winGame );			// fixed (+ach)
	sp->addArgs( eAward_diamondsToYou );
	sp->addArgs( eAward_stayinFrosty );		// works (achievement configured incorrectly)
	sp->addArgs( eAward_ironMan );			// fixed (+ach)
	sp->addArgs( eAward_renewableEnergy );
	out->m_stats.push_back(sp);

	sp = new StatParam(L"TimePlayed.DifficultyLevelId.*"); // BROKEN!
	for (int i=0; i<4; i++) sp->addArgs(i); // Difficulty Levels.
	out->m_stats.push_back(sp);

	sp = new StatParam(L"DistanceTravelled.DifficultyLevelId.*.TravelMethodId.*");
	for (int i = 0; i<4; i++)
	{
		sp->addArgs( i, DsTravel::eMethod_walk );
		sp->addArgs( i, DsTravel::eMethod_climb );
		sp->addArgs( i, DsTravel::eMethod_fall );
		sp->addArgs( i, DsTravel::eMethod_minecart );
		sp->addArgs( i, DsTravel::eMethod_swim );
		sp->addArgs( i, DsTravel::eMethod_pig );
	}
	out->m_stats.push_back(sp);

	sp = new StatParam(L"DistanceTravelled");
	sp->addArgs( /*NO ARGUMENTS*/ -1 );
	out->m_stats.push_back(sp);

	sp = new StatParam(L"PlayedMusicDisc.DiscId.*"); // works (+ach)
	for (int i = 2256; i < 2268; i++)
		sp->addArgs( i );
	out->m_stats.push_back(sp);

	sp = new StatParam(L"ChestfulOfCobblestone");	// works. (+ach)
	sp->addArgs( /*NO ARGUMENTS*/ -1 );	
	out->m_stats.push_back(sp);

	sp = new StatParam(L"Overkill"); // works. (+ach)
	sp->addArgs( /*NO ARGUMENTS*/ -1 );
	out->m_stats.push_back(sp);

	sp = new StatParam(L"OnARail"); // works. (+ach)
	sp->addArgs( /*NO ARGUMENTS*/ -1 );
	out->m_stats.push_back(sp);

	sp = new StatParam(L"Leaderboard.LeaderboardId.*.DifficultyLevelId.*");
	for (int i = 0; i < 4; i++)
	{
		sp->addArgs( eLeaderboardId_FARMING,	i );
		sp->addArgs( eLeaderboardId_MINING,		i );
		sp->addArgs( eLeaderboardId_TRAVELLING,	i );
		sp->addArgs( eLeaderboardId_KILLING,	i );
	}
	out->m_stats.push_back(sp);

	
	// Debugging 1 //

	sp = new StatParam(L"DistanceTravelled"); 
	sp->addArgs( /*NO ARGUMENTS*/ -1 );
	out->m_stats.push_back(sp);

	sp = new StatParam(L"BlockBroken"); 
	sp->addArgs( /*NO ARGUMENTS*/ -1 );
	out->m_stats.push_back(sp);

	sp = new StatParam(L"MobKilledTotal"); 
	sp->addArgs( /*NO ARGUMENTS*/ -1 );
	out->m_stats.push_back(sp);
	
	return out;
}

void DurangoStatsDebugger::PrintStats(int iPad)
{
	if (instance == NULL) instance = Initialize();

	vector<wstring> *tmp = instance->getStats();
	instance->m_printQueue.insert(instance->m_printQueue.end(), tmp->begin(), tmp->end());

	// app.DebugPrintf("[DEBUG] START\n");
	// for (wstring t : *tmp) app.DebugPrintf("[DEBUG] %s\n", wstringtofilename(t));
	// app.DebugPrintf("[DEBUG] END\n");

	instance->retrieveStats(iPad);

	app.DebugPrintf("[DurangoStatsDebugger] (%i) Results returned, starting printing.\n", instance->m_retrievedStats.size());
	for (StatResult result : instance->m_retrievedStats)
	{
		app.DebugPrintf("[DSB] '%s' == ", wstringtofilename(result.m_statName));
		app.DebugPrintf("%s\n", wstringtofilename(result.m_score));
	}

	// Empty list.
	EnterCriticalSection(&instance->m_retrievedStatsLock);
	instance->m_retrievedStats.erase(
		instance->m_retrievedStats.begin(),
		instance->m_retrievedStats.end()
		);
	LeaveCriticalSection(&instance->m_retrievedStatsLock);

}

void DurangoStatsDebugger::retrieveStats(int iPad)
{
	MXS::XboxLiveContext ^xboxLiveContext;
	try
    {
        WFC::IVectorView<Windows::Xbox::System::User^>^ userList = Windows::Xbox::System::User::Users;
        if( userList != nullptr ) 
        {
			for (Windows::Xbox::System::User^ user : userList)
			{
				if( user->IsSignedIn && !user->IsGuest )
				{
					xboxLiveContext = ref new MXS::XboxLiveContext(user);
					break;
				}
			}

			if (xboxLiveContext == nullptr) 
			{
				app.DebugPrintf("[DurangoStatsDebugger] Problem occured while creating 'XboxLiveContext'.\n"); return;
			}
		}
    }
    catch (Platform::Exception^ ex)
    {
		app.DebugPrintf("[DurangoStatsDebugger] Problem occured while creating 'XboxLiveContext'.\n"); return;
    }

	if ( xboxLiveContext )	{ app.DebugPrintf("[DurangoStatsDebugger] 'XboxLiveContext' created successfully.\n"); }
	else					{ app.DebugPrintf("[DurangoStatsDebugger] 'XboxLiveContext' not created.\n"); return;  }

	PlayerUID xuid;
	ProfileManager.GetXUID(iPad, &xuid, true);

	const int readCount = 1, difficulty = 1, type = 0;

	// ----------------------------------------- //
	
	byte runningThreads = 0;
	byte *r_runningThreads = &runningThreads;

	if (xuid.toString().compare(L"") == 0) 
	{
		app.DebugPrintf("[DurangoStatsDebugger] NO LOGGED IN USER!\n");
		return;
	}

	string plrname= wstringtofilename(xuid.toString());
	app.DebugPrintf(
		"[DurangoStatsDebugger] Retrieving (%i) stats for '%s'.\n",
		m_printQueue.size(),
		plrname.c_str()
		);

	static const unsigned short R_SIZE = 20;

	// Create Stat retrieval threads until there is no long any stats to start retrieving.
	while ( !instance->m_printQueue.empty() )
	{
		vector<wstring> *printing = new vector<wstring>();

		if (m_printQueue.size() > R_SIZE)
		{
			printing->insert( printing->end(), m_printQueue.begin(), m_printQueue.begin() + R_SIZE );
			m_printQueue.erase( m_printQueue.begin(), m_printQueue.begin() + R_SIZE );
		}
		else
		{
			printing->insert( printing->end(), m_printQueue.begin(), m_printQueue.end() );
			m_printQueue.erase( m_printQueue.begin(), m_printQueue.end() );
		}

		// ------------------------------------------ //

		app.DebugPrintf("[DurangoStatsDebugger] Starting retrieval operation (%i/%i stats).\n", printing->size(), R_SIZE);

		runningThreads++;

		// Fill statNames string.
		PC::Vector<P::String^>^ statNames = ref new PC::Vector<P::String^>();
		for ( auto it = printing->begin(); it != printing->end(); it++ )
			statNames->Append( ref new P::String(it->c_str()) );

		// Create vector of the XboxId we want.
		PC::Vector<P::String^>^ xboxUserIds = ref new PC::Vector<P::String^>();
		xboxUserIds->Append(ref new P::String(xuid.toString().c_str()));

		auto asyncOp = xboxLiveContext->UserStatisticsService->GetMultipleUserStatisticsAsync(
			xboxUserIds->GetView(), // the collection of Xbox user IDs whose stats we want to retrieve
			SERVICE_CONFIG_ID, // the service config that contains the stats we want
			statNames->GetView() // a list of stat names we want
			);

		CC::create_task(asyncOp)
			.then( [this,printing,iPad,r_runningThreads] (CC::task<WFC::IVectorView<MXS::UserStatistics::UserStatisticsResult^>^> resultListTask) 
		{
			try
			{
				WFC::IVectorView<MXS::UserStatistics::UserStatisticsResult^>^ resultList = resultListTask.get();
				int userIndex = 0;
				for( MXS::UserStatistics::UserStatisticsResult^ result : resultList )
				{
					for( UINT index = 0; index<result->ServiceConfigurationStatistics->Size; index++ )
					{
						MXS::UserStatistics::ServiceConfigurationStatistic^ configStat = result->ServiceConfigurationStatistics->GetAt(index);

						app.DebugPrintf("[DurangoStatsDebugger] Retrieve complete, %i results returned.\n", configStat->Statistics->Size);

						for (auto result : configStat->Statistics)
						{
							StatResult sr = { iPad, result->StatisticName->Data(), result->Value->Data() };
							this->addRetrievedStat(sr);
						}
					}
					++userIndex;
				}

				
			}
			catch (Platform::Exception ^ex)
			{
				app.DebugPrintf("[DurangoStatsDebugger] resultListTask.get() encountered an exception:\n\t'%s'\n", ex->ToString()->Data() );
			}

			(*r_runningThreads)--;
		});
	}

	while (runningThreads > 0) Sleep(5);
}

void DurangoStatsDebugger::addRetrievedStat(StatResult result)
{
	EnterCriticalSection(&m_retrievedStatsLock);
	m_retrievedStats.push_back(result);	
	LeaveCriticalSection(&m_retrievedStatsLock);
}