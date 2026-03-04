#include "stdafx.h"

#include "..\..\..\Minecraft.World\StringHelpers.h"

#include "LeaderboardManager.h"

const wstring LeaderboardManager::filterNames[eNumFilterModes] =
	{
		L"Friends", L"MyScore", L"TopRank"
	};

void LeaderboardManager::DeleteInstance()
{
	delete m_instance;
	m_instance = NULL;
}

LeaderboardManager::LeaderboardManager()
{
	zeroReadParameters();

	m_myXUID = INVALID_XUID;
}

void LeaderboardManager::zeroReadParameters()
{
	m_difficulty = -1;
	m_statsType = eStatsType_UNDEFINED;
	m_readListener = NULL;
	m_startIndex = 0;
	m_readCount = 0;
	m_eFilterMode = eFM_UNDEFINED;
}

bool LeaderboardManager::ReadStats_Friends(LeaderboardReadListener *listener, int difficulty, EStatsType type, PlayerUID myUID, unsigned int startIndex, unsigned int readCount)
{
	zeroReadParameters();

	m_readListener = listener;
	m_difficulty = difficulty;
	m_statsType = type;
	
	m_eFilterMode = eFM_Friends;
	return true;
}

bool LeaderboardManager::ReadStats_MyScore(LeaderboardReadListener *listener, int difficulty, EStatsType type, PlayerUID myUID, unsigned int readCount)
{
	zeroReadParameters();

	m_readListener = listener;
	m_difficulty = difficulty;
	m_statsType = type;
	
	m_readCount = readCount;
	
	m_eFilterMode = eFM_MyScore;
	return true;
}

bool LeaderboardManager::ReadStats_TopRank(LeaderboardReadListener *listener, int difficulty, EStatsType type, unsigned int startIndex, unsigned int readCount)
{
	zeroReadParameters();

	m_readListener = listener;
	m_difficulty = difficulty;
	m_statsType = type;

	m_startIndex = startIndex;
	m_readCount = readCount;

	m_eFilterMode = eFM_TopRank;
	return true;
}

#ifndef _XBOX
void LeaderboardManager::printStats(ReadView &view)
{
	app.DebugPrintf("[LeaderboardManager] Printing stats:\n"
		"\tnumQueries=%i\n", view.m_numQueries);

	for (int i=0; i<view.m_numQueries; i++)
	{
		ReadScore score = view.m_queries[i];

		app.DebugPrintf( "\tname='%s'\n", wstringtofilename(wstring(score.m_name)) );
		app.DebugPrintf( "\trank='%i'\n", score.m_rank );
		
		app.DebugPrintf( "\tstatsData=[" );
		for (int j=0; j<score.m_statsSize; j++)
			app.DebugPrintf( " %i", score.m_statsData[j] );
		app.DebugPrintf( "]\n" );
	}
}


bool DebugReadListener::OnStatsReadComplete(LeaderboardManager::eStatsReturn success, int numResults, LeaderboardManager::ViewOut results)
{
	app.DebugPrintf("[DebugReadListener] OnStatsReadComplete, %s:\n", (success ? "success" : "FAILED") );
	LeaderboardManager::printStats(results);

	return true;
}

DebugReadListener *DebugReadListener::m_instance = new DebugReadListener();
#endif