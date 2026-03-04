#pragma once

#include "Common\Leaderboards\LeaderboardManager.h"

class OrbisLeaderboardManager : public LeaderboardManager
{
protected:
	enum EStatsState
	{
		eStatsState_Idle,
		eStatsState_Getting,
		eStatsState_Failed,
		eStatsState_Ready,
		eStatsState_Canceled,
		//eStatsState_Writing,
		eStatsState_Max
	};

public:
	OrbisLeaderboardManager();
	virtual ~OrbisLeaderboardManager();

private:
	unsigned short m_openSessions;

	C4JThread *m_threadScoreboard;
	bool m_running;

	int		m_titleContext;
	int32_t m_requestId;

	//SceNpId m_myNpId;

	static int scoreboardThreadEntry(LPVOID lpParam);
	void scoreboardThreadInternal();

	bool getScoreByIds();
	bool getScoreByRange();

	bool setScore();
	queue<RegisterScore> m_views;

	CRITICAL_SECTION m_csViewsLock;

	EStatsState		m_eStatsState;			//State of the stats read
	//	EFilterMode		m_eFilterMode;

	ReadScore *m_scores;
	unsigned int m_maxRank;
	//SceNpScoreRankData *m_stats;

public:
	virtual void Tick();// {}

	//Open a session
	virtual bool OpenSession();// { return true; }

	//Close a session
	virtual void CloseSession();// {}

	//Delete a session
	virtual void DeleteSession();// {}

	//Write the given stats
	//This is called synchronously and will not free any memory allocated for views when it is done

	virtual bool WriteStats(unsigned int viewCount, ViewIn views);// { return false; }

	virtual bool ReadStats_Friends(LeaderboardReadListener *callback, int difficulty, EStatsType type, PlayerUID myUID, unsigned int startIndex, unsigned int readCount);// { return false; }
	virtual bool ReadStats_MyScore(LeaderboardReadListener *callback, int difficulty, EStatsType type, PlayerUID myUID, unsigned int readCount);// { return false; }
	virtual bool ReadStats_TopRank(LeaderboardReadListener *callback, int difficulty, EStatsType type, unsigned int startIndex, unsigned int readCount);// { return false; }

	//Perform a flush of the stats
	virtual void FlushStats();// {}

	//Cancel the current operation
	virtual void CancelOperation();// {}

	//Is the leaderboard manager idle.
	virtual bool isIdle();// { return true; }


private:
	int getBoardId(int difficulty, EStatsType);

	//LeaderboardManager::ReadScore *filterJustScorers(unsigned int &num, LeaderboardManager::ReadScore *friendsData);

	SceNpScorePlayerRankData *addPadding(unsigned int num, SceNpScoreRankData *rankData);

	void convertToOutput(unsigned int &num, ReadScore *out, SceNpScorePlayerRankData *rankData, SceNpScoreComment *comm);

	void toBinary(void *out, SceNpScoreComment *in);
	void fromBinary(SceNpScoreComment **out, void *in);

	void toBase32(SceNpScoreComment *out, void *in);
	void fromBase32(void *out, SceNpScoreComment *in);

	void toSymbols(char *);
	void fromSymbols(char *);

	bool test_string(string);

	void initReadScoreStruct(ReadScore &out, SceNpScoreRankData &);
	void fillReadScoreStruct(ReadScore &out, SceNpScoreComment &comment);

	static bool SortByRank(const ReadScore &lhs, const ReadScore &rhs);

	//void NotifyAsyncPlusFeature();
};
