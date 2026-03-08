#pragma once

#include "Common/Leaderboards/LeaderboardManager.h"

class WindowsLeaderboardManager : public LeaderboardManager
{
public:
	virtual void Tick() {}

	//Open a session
	virtual bool OpenSession() { return true; }

	//Close a session
	virtual void CloseSession() {}

	//Delete a session
	virtual void DeleteSession() {}

	//Write the given stats
	//This is called synchronously and will not free any memory allocated for views when it is done

	virtual bool WriteStats(unsigned int viewCount, ViewIn views) { return false; }

	virtual bool ReadStats_Friends(LeaderboardReadListener *callback, int difficulty, EStatsType type, PlayerUID myUID) { return false; }
	virtual bool ReadStats_MyScore(LeaderboardReadListener *callback, int difficulty, EStatsType type, PlayerUID myUID, unsigned int readCount) { return false; }
	virtual bool ReadStats_TopRank(LeaderboardReadListener *callback, int difficulty, EStatsType type, unsigned int startIndex, unsigned int readCount) { return false; }

	//Perform a flush of the stats
	virtual void FlushStats() {}

	//Cancel the current operation
	virtual void CancelOperation() {}

	//Is the leaderboard manager idle.
	virtual bool isIdle() { return true; }
};
