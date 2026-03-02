#pragma once

#include "LeaderboardManager.h"

// 4J-JEV: Simple interface for handling ReadStat failures.
class LeaderboardInterface
{
private:
	LeaderboardManager				*m_manager;
	bool							m_pending;

	// Arguments.
	LeaderboardManager::EFilterMode	m_filter;
	LeaderboardReadListener			*m_callback;
	int								m_difficulty;
	LeaderboardManager::EStatsType	m_type;
	PlayerUID						m_myUID;
	unsigned int					m_startIndex;
	unsigned int					m_readCount;

public:
	LeaderboardInterface(LeaderboardManager *man);
	~LeaderboardInterface();

	void ReadStats_Friends(LeaderboardReadListener *callback, int difficulty, LeaderboardManager::EStatsType type, PlayerUID myUID, unsigned int startIndex, unsigned int readCount);
	void ReadStats_MyScore(LeaderboardReadListener *callback, int difficulty, LeaderboardManager::EStatsType type, PlayerUID myUID, unsigned int readCount);
	void ReadStats_TopRank(LeaderboardReadListener *callback, int difficulty, LeaderboardManager::EStatsType type, unsigned int startIndex, unsigned int readCount);

	void CancelOperation();

	void tick();

private:
	bool callManager();
};