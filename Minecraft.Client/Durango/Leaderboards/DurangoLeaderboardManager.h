#pragma once

#include "Common\Leaderboards\LeaderboardManager.h"

namespace P = Platform;
namespace PC = Platform::Collections;
namespace WF = Windows::Foundation;
namespace WFC = Windows::Foundation::Collections;
namespace MXSL = Microsoft::Xbox::Services::Leaderboard;

class DurangoLeaderboardManager : public LeaderboardManager
{
protected:
	enum EStatsState
	{
		eStatsState_Idle,
		eStatsState_GettingLeaderboardInfo,
		eStatsState_ReceivedLeaderboardInfo,
		eStatsState_GettingStatsInfo,
		eStatsState_ReceivedStatsInfo,
		eStatsState_Failed,
		eStatsState_Ready,
		eStatsState_Canceled,
		//eStatsState_Writing,
		eStatsState_Max
	};

private:
	unsigned short m_openSessions;
	
	CRITICAL_SECTION m_csStatsState;
	EStatsState m_eStatsState;			//State of the stats read

	ReadScore *m_scores;
	unsigned int m_maxRank;

	MXS::XboxLiveContext^ m_xboxLiveContext;
	wstring m_leaderboardNames[4][eStatsType_MAX];
	wstring m_socialLeaderboardNames[4][eStatsType_MAX];
	std::vector<wstring> m_leaderboardStatNames[4][eStatsType_MAX];

	// Display names for the current scores
	std::vector<wstring> m_displayNames;
	bool m_waitingForProfiles;

	int m_difficulty;
	EStatsType m_type;
	PC::Vector<P::String^>^ m_statNames;	
	PC::Vector<P::String^>^ m_xboxUserIds;
	WF::IAsyncOperation<MXSL::LeaderboardResult^>^ m_leaderboardAsyncOp;
	WF::IAsyncOperation<WFC::IVectorView<Microsoft::Xbox::Services::UserStatistics::UserStatisticsResult^ >^ >^ m_statsAsyncOp;

public:
	DurangoLeaderboardManager();

	virtual void Tick();

	//Open a session
	virtual bool OpenSession();

	//Close a session
	virtual void CloseSession();

	//Delete a session
	virtual void DeleteSession();

	//Write the given stats
	//This is called synchronously and will not free any memory allocated for views when it is done

	virtual bool WriteStats(unsigned int viewCount, ViewIn views);

	virtual bool ReadStats_Friends(LeaderboardReadListener *listener, int difficulty, EStatsType type, PlayerUID myUID, unsigned int startIndex, unsigned int readCount);
	virtual bool ReadStats_MyScore(LeaderboardReadListener *listener, int difficulty, EStatsType type, PlayerUID myUID, unsigned int readCount);
	virtual bool ReadStats_TopRank(LeaderboardReadListener *listener, int difficulty, EStatsType type, unsigned int startIndex, unsigned int readCount);

	//Perform a flush of the stats
	virtual void FlushStats();

	//Cancel the current operation
	virtual void CancelOperation();

	//Is the leaderboard manager idle.
	virtual bool isIdle();

	static void GetProfilesCallback(LPVOID param, std::vector<Microsoft::Xbox::Services::Social::XboxUserProfile^> profiles);

private:
	void runLeaderboardRequest(WF::IAsyncOperation<MXSL::LeaderboardResult^>^ asyncOp, int difficulty, EStatsType type, unsigned int readCount, EFilterMode filter);
	void updateStatsInfo(int userIndex, int difficulty, EStatsType type, WFC::IVectorView<MXS::UserStatistics::Statistic^>^ statsResult);

	EStatsState getState();
	void setState(EStatsState newState);
	wstring stateToString(EStatsState eState);
};
