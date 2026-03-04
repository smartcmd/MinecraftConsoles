#include "stdafx.h"

#include "XboxLeaderboardManager.h"

#include "..\Network\NetworkPlayerXbox.h"

#ifdef _XBOX
LeaderboardManager *LeaderboardManager::m_instance = new XboxLeaderboardManager(); //Singleton instance of the LeaderboardManager
#endif

const XboxLeaderboardManager::LeaderboardDescriptor XboxLeaderboardManager::LEADERBOARD_DESCRIPTORS[XboxLeaderboardManager::NUM_LEADERBOARDS][4] = {
	{
		XboxLeaderboardManager::LeaderboardDescriptor( STATS_VIEW_TRAVELLING_PEACEFUL,	4,	STATS_COLUMN_TRAVELLING_PEACEFUL_WALKED,	STATS_COLUMN_TRAVELLING_PEACEFUL_FALLEN,	STATS_COLUMN_TRAVELLING_PEACEFUL_MINECART,	STATS_COLUMN_TRAVELLING_PEACEFUL_BOAT,	NULL, NULL, NULL,NULL),
		XboxLeaderboardManager::LeaderboardDescriptor( STATS_VIEW_TRAVELLING_EASY,		4,	STATS_COLUMN_TRAVELLING_EASY_WALKED,		STATS_COLUMN_TRAVELLING_EASY_FALLEN,		STATS_COLUMN_TRAVELLING_EASY_MINECART,		STATS_COLUMN_TRAVELLING_EASY_BOAT,		NULL, NULL, NULL,NULL),
		XboxLeaderboardManager::LeaderboardDescriptor( STATS_VIEW_TRAVELLING_NORMAL,	4,	STATS_COLUMN_TRAVELLING_NORMAL_WALKED,		STATS_COLUMN_TRAVELLING_NORMAL_FALLEN,		STATS_COLUMN_TRAVELLING_NORMAL_MINECART,	STATS_COLUMN_TRAVELLING_NORMAL_BOAT,	NULL, NULL, NULL,NULL),
		XboxLeaderboardManager::LeaderboardDescriptor( STATS_VIEW_TRAVELLING_HARD,		4,	STATS_COLUMN_TRAVELLING_HARD_WALKED,		STATS_COLUMN_TRAVELLING_HARD_FALLEN,		STATS_COLUMN_TRAVELLING_HARD_MINECART,		STATS_COLUMN_TRAVELLING_HARD_BOAT,		NULL, NULL, NULL,NULL),
	}, {
		XboxLeaderboardManager::LeaderboardDescriptor( STATS_VIEW_MINING_BLOCKS_PEACEFUL,	7,	STATS_COLUMN_MINING_BLOCKS_PEACEFUL_DIRT,	STATS_COLUMN_MINING_BLOCKS_PEACEFUL_STONE,	STATS_COLUMN_MINING_BLOCKS_PEACEFUL_SAND,	STATS_COLUMN_MINING_BLOCKS_PEACEFUL_COBBLESTONE,	STATS_COLUMN_MINING_BLOCKS_PEACEFUL_GRAVEL,	STATS_COLUMN_MINING_BLOCKS_PEACEFUL_CLAY,	STATS_COLUMN_MINING_BLOCKS_PEACEFUL_OBSIDIAN,	NULL ),
		XboxLeaderboardManager::LeaderboardDescriptor( STATS_VIEW_MINING_BLOCKS_EASY,		7,	STATS_COLUMN_MINING_BLOCKS_EASY_DIRT,		STATS_COLUMN_MINING_BLOCKS_EASY_STONE,		STATS_COLUMN_MINING_BLOCKS_EASY_SAND,		STATS_COLUMN_MINING_BLOCKS_EASY_COBBLESTONE,		STATS_COLUMN_MINING_BLOCKS_EASY_GRAVEL,		STATS_COLUMN_MINING_BLOCKS_EASY_CLAY,		STATS_COLUMN_MINING_BLOCKS_EASY_OBSIDIAN,		NULL ),
		XboxLeaderboardManager::LeaderboardDescriptor( STATS_VIEW_MINING_BLOCKS_NORMAL,		7,	STATS_COLUMN_MINING_BLOCKS_NORMAL_DIRT,		STATS_COLUMN_MINING_BLOCKS_NORMAL_STONE,	STATS_COLUMN_MINING_BLOCKS_NORMAL_SAND,		STATS_COLUMN_MINING_BLOCKS_NORMAL_COBBLESTONE,		STATS_COLUMN_MINING_BLOCKS_NORMAL_GRAVEL,	STATS_COLUMN_MINING_BLOCKS_NORMAL_CLAY,		STATS_COLUMN_MINING_BLOCKS_NORMAL_OBSIDIAN,		NULL ),
		XboxLeaderboardManager::LeaderboardDescriptor( STATS_VIEW_MINING_BLOCKS_HARD,		7,	STATS_COLUMN_MINING_BLOCKS_HARD_DIRT,		STATS_COLUMN_MINING_BLOCKS_HARD_STONE,		STATS_COLUMN_MINING_BLOCKS_HARD_SAND,		STATS_COLUMN_MINING_BLOCKS_HARD_COBBLESTONE,		STATS_COLUMN_MINING_BLOCKS_HARD_GRAVEL,		STATS_COLUMN_MINING_BLOCKS_HARD_CLAY,		STATS_COLUMN_MINING_BLOCKS_HARD_OBSIDIAN,		NULL ),
	}, {
		XboxLeaderboardManager::LeaderboardDescriptor( STATS_VIEW_FARMING_PEACEFUL,	6, STATS_COLUMN_FARMING_PEACEFUL_EGGS,	STATS_COLUMN_FARMING_PEACEFUL_WHEAT,	STATS_COLUMN_FARMING_PEACEFUL_MUSHROOMS,STATS_COLUMN_FARMING_PEACEFUL_SUGARCANE,STATS_COLUMN_FARMING_PEACEFUL_MILK,	STATS_COLUMN_FARMING_PEACEFUL_PUMPKINS,	NULL, NULL ),
		XboxLeaderboardManager::LeaderboardDescriptor( STATS_VIEW_FARMING_EASY,		6, STATS_COLUMN_FARMING_EASY_EGGS,		STATS_COLUMN_FARMING_PEACEFUL_WHEAT,	STATS_COLUMN_FARMING_EASY_MUSHROOMS,	STATS_COLUMN_FARMING_EASY_SUGARCANE,	STATS_COLUMN_FARMING_EASY_MILK,		STATS_COLUMN_FARMING_EASY_PUMPKINS,		NULL, NULL ),
		XboxLeaderboardManager::LeaderboardDescriptor( STATS_VIEW_FARMING_NORMAL,	6, STATS_COLUMN_FARMING_NORMAL_EGGS,	STATS_COLUMN_FARMING_NORMAL_WHEAT,		STATS_COLUMN_FARMING_NORMAL_MUSHROOMS,	STATS_COLUMN_FARMING_NORMAL_SUGARCANE,	STATS_COLUMN_FARMING_NORMAL_MILK,	STATS_COLUMN_FARMING_NORMAL_PUMPKINS,	NULL, NULL ),
		XboxLeaderboardManager::LeaderboardDescriptor( STATS_VIEW_FARMING_HARD,		6, STATS_COLUMN_FARMING_HARD_EGGS,		STATS_COLUMN_FARMING_HARD_WHEAT,		STATS_COLUMN_FARMING_HARD_MUSHROOMS,	STATS_COLUMN_FARMING_HARD_SUGARCANE,	STATS_COLUMN_FARMING_HARD_MILK,		STATS_COLUMN_FARMING_HARD_PUMPKINS,		NULL, NULL ),
	}, {
		XboxLeaderboardManager::LeaderboardDescriptor( NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL ),
		XboxLeaderboardManager::LeaderboardDescriptor( STATS_VIEW_KILLS_EASY,	7, STATS_COLUMN_KILLS_EASY_ZOMBIES,		STATS_COLUMN_KILLS_EASY_SKELETONS,		STATS_COLUMN_KILLS_EASY_CREEPERS,	STATS_COLUMN_KILLS_EASY_SPIDERS,	STATS_COLUMN_KILLS_EASY_SPIDERJOCKEYS,		STATS_COLUMN_KILLS_EASY_ZOMBIEPIGMEN,	STATS_COLUMN_KILLS_EASY_SLIME,		NULL ),
		XboxLeaderboardManager::LeaderboardDescriptor( STATS_VIEW_KILLS_NORMAL,	7, STATS_COLUMN_KILLS_NORMAL_ZOMBIES,	STATS_COLUMN_KILLS_NORMAL_SKELETONS,	STATS_COLUMN_KILLS_NORMAL_CREEPERS,	STATS_COLUMN_KILLS_NORMAL_SPIDERS,	STATS_COLUMN_KILLS_NORMAL_SPIDERJOCKEYS,	STATS_COLUMN_KILLS_NORMAL_ZOMBIEPIGMEN,	STATS_COLUMN_KILLS_NORMAL_SLIME,	NULL ),
		XboxLeaderboardManager::LeaderboardDescriptor( STATS_VIEW_KILLS_HARD,	7, STATS_COLUMN_KILLS_HARD_ZOMBIES,		STATS_COLUMN_KILLS_HARD_SKELETONS,		STATS_COLUMN_KILLS_HARD_CREEPERS,	STATS_COLUMN_KILLS_HARD_SPIDERS,	STATS_COLUMN_KILLS_HARD_SPIDERJOCKEYS,		STATS_COLUMN_KILLS_HARD_ZOMBIEPIGMEN,	STATS_COLUMN_KILLS_HARD_SLIME,		NULL ),
	},
};

XboxLeaderboardManager::XboxLeaderboardManager()
{
	m_eStatsState = eStatsState_Idle;
	m_statsRead = false;

	m_hSession = NULL;
	m_spec = NULL;
	m_stats = NULL;

	m_isQNetSession = false;
	m_endingSession = false;
}

XboxLeaderboardManager::~XboxLeaderboardManager() {}

void XboxLeaderboardManager::Tick()
{
	if( m_eStatsState == eStatsState_Getting )
	{

		/*if( IsStatsReadComplete() )
			if( m_readCompleteCallback != NULL )
				m_readCompleteCallback(m_readCompleteUserdata);*/

		if ( IsStatsReadComplete() )
			if (m_readListener != NULL)
			{
				// 4J Stu - If the state is other than ready, then we don't have any stats to sort
				if(m_eFilterMode == LeaderboardManager::eFM_Friends && m_eStatsState == eStatsState_Ready) SortFriendStats();

				bool ret = m_readListener->OnStatsReadComplete(m_eStatsState==eStatsState_Ready, 0, m_stats); // TODO
				if (ret) m_eStatsState = eStatsState_Idle;
			}
	}

	if( m_endingSession )
	{
		if( XHasOverlappedIoCompleted( &m_endSessionOverlapped ))
		{
			DeleteSession();
			m_endingSession = false;
		}
	}
}

bool XboxLeaderboardManager::OpenSession()
{
	//Can't create a new session when deleting a previous one
	if (m_endingSession) return false;

	//We've already got an open session
	if (m_hSession != NULL) return true;

	int lockedProfile = ProfileManager.GetLockedProfile();
	if( lockedProfile == -1 )
	{
		m_hSession = NULL;
		return false;
	}

	XUserGetXUID(lockedProfile, &m_myXUID);
	XUserSetContext(lockedProfile, X_CONTEXT_GAME_TYPE, X_CONTEXT_GAME_TYPE_STANDARD);

	//We already have a QNet session (ie we are joining/in/leaving a multiplayer game)
	if(g_NetworkManager.IsInSession())
	{
		if(g_NetworkManager.IsInStatsEnabledSession())
		{
			m_isQNetSession = true;
			return true;
		}
		//else
		//{
		//	return false;
		//}
	}

	XSESSION_INFO	sessionInfo;
    ULONGLONG		sessionNonce;

	DWORD ret = XSessionCreate(XSESSION_CREATE_USES_STATS | XSESSION_CREATE_HOST, lockedProfile, 8, 8, &sessionNonce, &sessionInfo, NULL, &m_hSession);
	if( ret != ERROR_SUCCESS )
	{
		m_hSession = NULL;
		return false;
	}

	DWORD userIndices[1] = { lockedProfile };
	BOOL privateSlots[1] = { FALSE };
	ret = XSessionJoinLocal(m_hSession, 1, userIndices, privateSlots, NULL);
	if( ret != ERROR_SUCCESS )
	{
		m_hSession = NULL;
		return false;
	}
	
	ret = XSessionStart(m_hSession, 0, NULL);
	if( ret != ERROR_SUCCESS )
	{
		m_hSession = NULL;
		return false;
	}

	return true;
}

void XboxLeaderboardManager::CloseSession()
{
	if( !m_endingSession )
	{
		if (!XHasOverlappedIoCompleted(&m_overlapped))
			XCancelOverlapped(&m_overlapped);

		if (m_isQNetSession == true)
		{
			m_isQNetSession = false;
			return;
		}

		if (m_hSession == NULL) return;

		memset(&m_endSessionOverlapped, 0, sizeof(m_endSessionOverlapped));

		DWORD ret = XSessionEnd( m_hSession, &m_endSessionOverlapped );
		if (ret == ERROR_SUCCESS || ret == ERROR_IO_PENDING)
		{
			m_endingSession = true;
		}
		else
		{
			//Failed so just delete the session
			if (ret != ERROR_SUCCESS) DeleteSession();
		}

		m_readListener = NULL;
	}
}

void XboxLeaderboardManager::DeleteSession()
{
	XSessionDelete(m_hSession, NULL);
	CloseHandle(m_hSession);
	m_hSession = NULL;
}

bool XboxLeaderboardManager::WriteStats(unsigned int viewCount, ViewIn views)
{
	DWORD ret = S_OK;

	// some debug code to catch the leaderboard write with 7 views
#ifndef _CONTENT_PACKAGE
	if(viewCount>5)	__debugbreak();
#endif

	// 4J Stu - If we are online we already have a session, so use that
	if(m_isQNetSession == true)
	{
		INetworkPlayer *player = g_NetworkManager.GetPlayerByXuid(m_myXUID);
		if(player != NULL)
		{
			ret = ((NetworkPlayerXbox *)player)->GetQNetPlayer()->WriteStats(viewCount,views);
			//printf("Wrote stats to QNet player\n");
		}
		else
		{
			ret = ERROR_NO_SUCH_USER;
			assert(false && "Failed to write stats to a QNET session as no player exists with that XUID");
			app.DebugPrintf("Failed to write stats to a QNET session as no player exists with that XUID\n");
		}
	}
	else
	{
		ret = XSessionWriteStats(m_hSession, m_myXUID, viewCount, views, NULL);
	}
	if (ret != ERROR_SUCCESS) return false;
	return true;
}

void XboxLeaderboardManager::CancelOperation()
{
	//Need to have a session open
	if( m_hSession == NULL )
		if( !OpenSession() )
			return;

	//Abort any current read operation
	if( m_eStatsState == eStatsState_Getting )
		if( !XHasOverlappedIoCompleted( &m_overlapped ))
			XCancelOverlapped( &m_overlapped );

	m_eStatsState = eStatsState_Idle;
}

bool XboxLeaderboardManager::ReadStats_MyScore(LeaderboardReadListener *callback, int difficulty, EStatsType type, PlayerUID myUID, unsigned int readCount)
{
	if (!readStats(difficulty,type)) return false;
	if (!LeaderboardManager::ReadStats_MyScore(callback, difficulty, type, myUID, readCount)) return false;

	HANDLE hEnumerator;
	DWORD ret;
	
	//DWORD readCount = 0;

	m_numStats = 0;
	ret = XUserCreateStatsEnumeratorByXuid(
				0,
				m_myXUID,
				readCount,
				1, //specCount,
				m_spec,
				&m_numStats,
				&hEnumerator);

	if( ret != ERROR_SUCCESS ) return false;

	//Allocate a buffer for the stats
	m_stats = (PXUSER_STATS_READ_RESULTS) new BYTE[m_numStats];
	if (m_stats == NULL) return false;

	memset(m_stats, 0, m_numStats);
	memset(&m_overlapped, 0, sizeof(m_overlapped));

	ret = XEnumerate(
			hEnumerator,		// Enumeration handle
			m_stats,			// Buffer
			m_numStats,			// Size of buffer
			NULL,				// Number of rows returned; not used for async
			&m_overlapped );	// Overlapped structure; not used for sync

	if ( (ret!=ERROR_SUCCESS) && (ret!=ERROR_IO_PENDING) ) return false;

	m_eStatsState = eStatsState_Getting;
	return true;
}

bool XboxLeaderboardManager::ReadStats_Friends(LeaderboardReadListener *callback, int difficulty, EStatsType type, PlayerUID myUID, unsigned int startIndex, unsigned int readCount)
{
	if (!readStats(difficulty,type)) return false;
	if (!LeaderboardManager::ReadStats_Friends(callback, difficulty, type, myUID, startIndex, readCount)) return false;

	HANDLE hEnumerator;
	DWORD ret;

	unsigned int friendCount;
	XUID *friends;
	
	getFriends(friendCount, &friends);

	if(friendCount == 0 || friends == NULL)
	{
		app.DebugPrintf("XboxLeaderboardManager::ReadStats_Friends - No friends found. Possibly you are offline?\n");
		return false;
	}
	assert(friendCount > 0 && friends != NULL);

	m_numStats = 0;
	ret = XUserReadStats(
							0,
							friendCount,
							friends,
							1, //specCount,
							m_spec,
							&m_numStats,
							NULL,
							NULL
						);

	//Annoyingly, this returns ERROR_INSUFFICIENT_BUFFER when it is being asked to calculate the size of the buffer by passing zero resultsSize
	if ( (ret!=ERROR_SUCCESS) && (ret!=ERROR_INSUFFICIENT_BUFFER) ) return false;

	//Allocate a buffer for the stats
	m_stats = (PXUSER_STATS_READ_RESULTS) new BYTE[m_numStats];
	if (m_stats == NULL) return false;
	
	memset(m_stats, 0, m_numStats);
	memset(&m_overlapped, 0, sizeof(m_overlapped));

	ret = XUserReadStats(
			0,
			friendCount,
			friends,
			1,
			m_spec,
			&m_numStats,
			m_stats,
			&m_overlapped);

	if( (ret!=ERROR_SUCCESS) && (ret!=ERROR_IO_PENDING) ) return false;

	m_eStatsState = eStatsState_Getting;
	return true;
}

bool XboxLeaderboardManager::ReadStats_TopRank(LeaderboardReadListener *callback, int difficulty, EStatsType type , unsigned int startIndex, unsigned int readCount)
{
	if (!readStats(difficulty,type)) return false;
	if (!LeaderboardManager::ReadStats_TopRank(callback, difficulty, type, startIndex, readCount)) return false;

	HANDLE hEnumerator;

	m_numStats = 0;
	DWORD ret = XUserCreateStatsEnumeratorByRank(
				0,					// Current title ID
				startIndex,			// Index to start enumerating from
				readCount,			// Number of rows to retrieve
				1,					// Number of stats specs
				m_spec,				// Stats spec,
				&m_numStats,		// Size of buffer
				&hEnumerator );		// Enumeration handle

	if( ret != ERROR_SUCCESS ) return false;

	//Allocate a buffer for the stats
	m_stats = (PXUSER_STATS_READ_RESULTS) new BYTE[m_numStats];
	if (m_stats == NULL) return false;

	memset(m_stats, 0, m_numStats);
	memset(&m_overlapped, 0, sizeof(m_overlapped));

	ret = XEnumerate(
			hEnumerator,		// Enumeration handle
			m_stats,			// Buffer
			m_numStats,			// Size of buffer
			NULL,				// Number of rows returned; not used for async
			&m_overlapped );	// Overlapped structure; not used for sync

	if( (ret!=ERROR_SUCCESS) && (ret!=ERROR_IO_PENDING) ) return false;

	m_eStatsState = eStatsState_Getting;
	return true;
}

bool XboxLeaderboardManager::readStats(int difficulty, EStatsType type)
{
	//Need to have a session open
	if (m_hSession==NULL) if(!OpenSession()) return false;

	m_eStatsState = eStatsState_Failed;
	m_statsRead = false;

	if (m_stats) delete [] m_stats;

	//Setup the spec structure for the read request
	m_spec = new XUSER_STATS_SPEC[1];
	m_spec[0].dwViewId = LEADERBOARD_DESCRIPTORS[(int)type][difficulty].m_viewId;
	m_spec[0].dwNumColumnIds = LEADERBOARD_DESCRIPTORS[(int)type][difficulty].m_columnCount;
	for (unsigned int i=0; i<m_spec[0].dwNumColumnIds; ++i)
		m_spec[0].rgwColumnIds[i] = LEADERBOARD_DESCRIPTORS[(int)type][difficulty].m_columnIds[i];

	return true;
}

void XboxLeaderboardManager::FlushStats()
{
	if( m_hSession == NULL || m_isQNetSession ) return;
	memset(&m_flushStatsOverlapped, 0, sizeof(m_flushStatsOverlapped));
	XSessionFlushStats(m_hSession, &m_flushStatsOverlapped);
}

bool XboxLeaderboardManager::IsStatsReadComplete()
{
	//If the read has completed
	if( XHasOverlappedIoCompleted( &m_overlapped ) )
	{
		//If we've had an error
		if( XGetOverlappedExtendedError( &m_overlapped ) != ERROR_SUCCESS )
		{
			m_eStatsState = eStatsState_Failed;
			if( m_stats )
			{
				delete [] m_stats;
				m_stats = NULL;
			}
		}
		else
		{
			//If we've not got any results
			if( !m_stats || m_stats->dwNumViews == 0 || m_stats->pViews[0].dwNumRows == 0 )
			{
				m_eStatsState = eStatsState_NoResults;
				if( m_stats )
				{
					delete [] m_stats;
					m_stats = NULL;
				}
			}
			else
			{
				m_eStatsState = eStatsState_Ready;
			}
		}
		return true;
	}
	
	return false;
}

int XboxLeaderboardManager::FriendSortFunction(const void* a, const void* b)
{
	return ((int)((XUSER_STATS_ROW*)a)->dwRank) - ((int)((XUSER_STATS_ROW*)b)->dwRank);
}

void XboxLeaderboardManager::SortFriendStats()
{
	for( unsigned int leaderboardIndex=0 ; leaderboardIndex<m_stats->dwNumViews ; ++leaderboardIndex )
	{
		//First filter out any friends who aren't in the leaderboard
		XUSER_STATS_ROW* pRow = m_stats->pViews[leaderboardIndex].pRows;
		//View rows is returned not including XUIDs that aren't in this leaderboard, but we want to recalculate that sort of thing
		m_stats->pViews[leaderboardIndex].dwTotalViewRows = m_stats->pViews[leaderboardIndex].dwNumRows;
		for( unsigned int rowIndex=0 ; rowIndex<m_stats->pViews[leaderboardIndex].dwTotalViewRows ; )
		{
			if( pRow->dwRank == 0 )
			{
				memmove( pRow, pRow + 1, ( ( m_stats->pViews[leaderboardIndex].dwTotalViewRows - rowIndex ) - 1 ) * sizeof(XUSER_STATS_ROW) );
				m_stats->pViews[leaderboardIndex].dwTotalViewRows--;
			}
			else
			{
				rowIndex++;
				pRow++;
			}
		}
		
		//Then sort by rank
		qsort( m_stats->pViews[leaderboardIndex].pRows, m_stats->pViews[leaderboardIndex].dwTotalViewRows, sizeof(XUSER_STATS_ROW), FriendSortFunction );
		m_stats->pViews[leaderboardIndex].dwNumRows = m_stats->pViews[leaderboardIndex].dwTotalViewRows;
	}
}

#if 0
void XboxLeaderboardManager::SetStatsRetrieved(bool success)
{
	if( m_stats != NULL )
	{
		delete [] m_stats;
		m_stats = NULL;
	}

	m_statsRead = success;
	m_eStatsState = eStatsState_Idle;
}
#endif


// 4J-JEV: Adapted/stolen from 'XUI_Leaderboards'.
bool XboxLeaderboardManager::getFriends(unsigned int &friendsCount, PlayerUID** friends)
{
	DWORD resultsSize;
	HANDLE hEnumerator;
	DWORD ret;
	DWORD numFriends;

	//First, get a list of (up to 100) friends (this is the maximum that the enumerator currently supports)
	ret = XFriendsCreateEnumerator( ProfileManager.GetLockedProfile(), 0, 100, &resultsSize, &hEnumerator);
	if(ret!=ERROR_SUCCESS) return false;

	XONLINE_FRIEND *xonlineFriends = (XONLINE_FRIEND*) new BYTE[resultsSize];
	
	ret = XEnumerate(
						hEnumerator,
						xonlineFriends,
						resultsSize,
						&numFriends,
						NULL
					);

	if (ret!=ERROR_SUCCESS) friendsCount = 0;

	PlayerUID *filteredFriends = new PlayerUID[numFriends+1];

	friendsCount = 0;
	for (unsigned int friendIndex=0; friendIndex<numFriends; ++friendIndex)
	{
		if( ( xonlineFriends[friendIndex].dwFriendState & (XONLINE_FRIENDSTATE_FLAG_SENTREQUEST | XONLINE_FRIENDSTATE_FLAG_RECEIVEDREQUEST) ) == 0 )
		{
			filteredFriends[friendsCount++] = xonlineFriends[friendIndex].xuid;
		}
	}

	filteredFriends[friendsCount++] = m_myXUID; //m_filteredFriends[m_numFilteredFriends++] = LeaderboardManager::Instance()->GetMyXUID();

	// Return.
	*friends = filteredFriends;
	return true;
}