#pragma once

#include <xuiresource.h>
#include <xuiapp.h>

#include "..\..\Common\Leaderboards\LeaderboardManager.h"

class XboxLeaderboardManager : public LeaderboardManager
{
public:
	enum EStatsState
	{
		eStatsState_Idle=0,		//No current stats read operation
		eStatsState_Getting,	//Stats read in progress
		eStatsState_Failed,		//Stats read failed
		eStatsState_Ready,		//Stats read completed, ready for use
		eStatsState_NoResults,	//Stats read completed, no results found
		eNumStatStates,
	};

protected: // LEADERBOARD DESCRIPTIONS //
		// From 'XUI_Leaderboards.cpp'.

	// 4J Stu - Because the kills leaderboard doesn't a peaceful entry there are some special
	// handling to make it skip that. We have re-arranged the order of the leaderboards so
	// I am making this in case we do it again.
	// 4J Stu - Made it a member of the class, rather than a #define
	static const int LEADERBOARD_KILLS_POSITION = 3;

	static const int NUM_LEADERBOARDS	= 4;//6;	//Number of leaderboards
	static const int NUM_ENTRIES		= 101;		//Cache up to this many entries
	static const int READ_SIZE			= 15;		//Read this many entries at a time

	struct LeaderboardDescriptor {
		DWORD	m_viewId;
		DWORD	m_columnCount;
		WORD	m_columnIds[8];

		LeaderboardDescriptor(	DWORD viewId, DWORD columnCount,
								WORD columnId_0, WORD columnId_1, WORD columnId_2, WORD columnId_3,
								WORD columnId_4, WORD columnId_5, WORD columnId_6, WORD columnId_7)
		{
			m_viewId = viewId;
			m_columnCount = columnCount;
			m_columnIds[0] = columnId_0;
			m_columnIds[1] = columnId_1;
			m_columnIds[2] = columnId_2;
			m_columnIds[3] = columnId_3;
			m_columnIds[4] = columnId_4;
			m_columnIds[5] = columnId_5;
			m_columnIds[6] = columnId_6;
			m_columnIds[7] = columnId_7;
		}
	};

	static const LeaderboardDescriptor LEADERBOARD_DESCRIPTORS[NUM_LEADERBOARDS][4];


private:

	EStatsState		m_eStatsState;			//State of the stats read
	bool			m_statsRead;			//Whether or not the stats read operation has completed

	HANDLE			m_hSession;				//Current session
	XOVERLAPPED		m_overlapped;			//Overlapped structure used for async actions
	XUSER_STATS_SPEC*	m_spec;				//Spec structure used in reads

	DWORD	m_numStats; 
	PXUSER_STATS_READ_RESULTS m_stats;		//Structure that stats are read into

	bool			m_isQNetSession;		//Session is being managed via QNet
	bool			m_endingSession;		//Session is currently being ended
	XOVERLAPPED		m_endSessionOverlapped;	//Overlapped stucture for end session async action

	XOVERLAPPED		m_flushStatsOverlapped;

public:
	XboxLeaderboardManager();
	~XboxLeaderboardManager();

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
	
	virtual bool ReadStats_Friends(LeaderboardReadListener *callback, int difficulty, EStatsType type, PlayerUID myUID, unsigned int startIndex, unsigned int readCount);
	virtual bool ReadStats_MyScore(LeaderboardReadListener *callback, int difficulty, EStatsType type, PlayerUID myUID, unsigned int readCount);
	virtual bool ReadStats_TopRank(LeaderboardReadListener *callback, int difficulty, EStatsType type , unsigned int startIndex, unsigned int readCount);

	//Perform a flush of the stats
	virtual void FlushStats();

	//Cancel the current operation
	virtual void CancelOperation();

	virtual bool isIdle() { return m_eStatsState != eStatsState_Getting; }

private:
	bool readStats(int difficulty, EStatsType type);

	//Check if the stats read operation has completed
	bool IsStatsReadComplete();
	
	//Function used by qsort to sort friends
	static int FriendSortFunction(const void* a, const void* b);

	//Sort the list of friend stats
	void SortFriendStats();

	bool getFriends(unsigned int &friendsCount, XUID** friends);

#if 0
public:		
	
	//Should be called once the stats have been retrieved and used by the application
	void SetStatsRetrieved(bool success);

	EStatsState					GetStatsState()		{ return m_eStatsState; }
	bool						GetStatsRead()		{ return m_statsRead; }
	
	PXUSER_STATS_READ_RESULTS	GetStats()			{ return m_stats; }
#endif
public:
	PlayerUID GetMyXUID() { return m_myXUID; }

};
