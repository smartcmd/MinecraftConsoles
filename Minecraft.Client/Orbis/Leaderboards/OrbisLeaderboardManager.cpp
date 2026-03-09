#include "stdafx.h"

#include "OrbisLeaderboardManager.h"

#include "base64.h"

#include "..\Orbis_App.h"
#include "..\..\Common\Consoles_App.h"

#include "Common\Network\Sony\SQRNetworkManager.h"

#include "..\..\..\Minecraft.World\StringHelpers.h"

#include <cstdlib>

#include <np.h>
//#include <sys/ppu_thread.h>

#include "Orbis\OrbisExtras\ShutdownManager.h"


LeaderboardManager *LeaderboardManager::m_instance = new OrbisLeaderboardManager(); //Singleton instance of the LeaderboardManager

OrbisLeaderboardManager::OrbisLeaderboardManager()
{
	m_eStatsState = eStatsState_Idle;

	m_titleContext = -1;

	m_myXUID = INVALID_XUID;

	m_scores = nullptr; //m_stats = nullptr;

	m_statsType = eStatsType_Kills;
	m_difficulty = 0;

	m_requestId = 0;

	m_openSessions = 0;

	InitializeCriticalSection(&m_csViewsLock);

	m_running = false;
	m_threadScoreboard = nullptr;
}

OrbisLeaderboardManager::~OrbisLeaderboardManager()
{
	m_running = false;

	// 4J-JEV: Wait for thread to stop and hope it doesn't take too long.
	long long startShutdown = System::currentTimeMillis();
	while (m_threadScoreboard->isRunning())
	{
		Sleep(1);
		assert( (System::currentTimeMillis() - startShutdown) < 16 );
	}

	delete m_threadScoreboard;

	DeleteCriticalSection(&m_csViewsLock);
}

int OrbisLeaderboardManager::scoreboardThreadEntry(LPVOID lpParam)
{
	ShutdownManager::HasStarted(ShutdownManager::eLeaderboardThread);
	OrbisLeaderboardManager *self = reinterpret_cast<OrbisLeaderboardManager *>(lpParam);

	self->m_running = true;
	app.DebugPrintf("[LeaderboardManager] Thread started.\n");

	bool needsWriting = false;
	do
	{
		if (self->m_openSessions > 0 || needsWriting)
		{
			self->scoreboardThreadInternal();
		}

		EnterCriticalSection(&self->m_csViewsLock);
		needsWriting = self->m_views.size() > 0;
		LeaveCriticalSection(&self->m_csViewsLock);

		// 4J Stu - We can't write while we aren't signed in to live
		if (!ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()))
		{
			needsWriting = false;
		}

		if ( (!needsWriting) && (self->m_eStatsState != eStatsState_Getting) )
		{
			Sleep(50); // 4J-JEV: When we're not reading or writing.
		}

	} while (	(self->m_running || self->m_eStatsState == eStatsState_Getting || needsWriting)
				&& ShutdownManager::ShouldRun(ShutdownManager::eLeaderboardThread) 
			);

	// 4J-JEV, moved this here so setScore can finish up.
	sceNpScoreDestroyTitleCtx(self->m_titleContext);
	// TODO sceNpScoreTerm();
	app.DebugPrintf("[LeaderboardManager] Thread closed.\n");
	ShutdownManager::HasFinished(ShutdownManager::eLeaderboardThread);
	return 0;
}

void OrbisLeaderboardManager::scoreboardThreadInternal()
{
	// 4J-JEV: Just initialise the context the once now.
	if (m_titleContext == -1)
	{
		int primaryPad = ProfileManager.GetPrimaryPad();

		if (!ProfileManager.IsSignedInLive(primaryPad)) return;

		SceNpId npId;
		ProfileManager.GetSceNpId(primaryPad,&npId);

		int ret = sceNpScoreCreateNpTitleCtx(primaryPad, &npId);
		
		if (ret < 0)	return;
		else			m_titleContext = ret;
	}
	else assert( m_titleContext > 0 ); //Paranoia


	switch (m_eStatsState)
	{
	case eStatsState_Getting:
		// Player starts using async multiplayer feature
		// 4J-PB - Fix for SCEA FQA #4 - TRC R4064 - Incorrect usage of AsyncMultiplay 
		// Note 1: 
		// The following NP call should be reserved for asynchronous multiplayer modes that require PS Plus to be accessed. 
		//
		//	Note 2: 
		// The message is not displayed with a user without PlayStation®Plus subscription and they are able to access the Leaderboards.

		// NotifyAsyncPlusFeature();

		switch(m_eFilterMode)
		{
		case eFM_MyScore:
		case eFM_Friends:
			getScoreByIds();
			break;
		case eFM_TopRank:
			getScoreByRange();
			break;
		}
		break;

	case eStatsState_Canceled:
	case eStatsState_Failed:
	case eStatsState_Ready:
	case eStatsState_Idle:

		// 4J-JEV: Moved this here, I don't want reading and
		// writing going on at the same time.
		// --
		// 4J-JEV: Writing no longer changes the manager state,
		// we'll manage the write queue seperately.

		EnterCriticalSection(&m_csViewsLock);
		bool hasWork = !m_views.empty();
		LeaveCriticalSection(&m_csViewsLock);
		
		if (hasWork)
		{
			setScore();		
		}
		
		break;
	}
}

bool OrbisLeaderboardManager::getScoreByIds()
{
	if (m_eStatsState == eStatsState_Canceled) return false;

	// ----------------------------
	SceRtcTick last_sort_date;
	SceNpScoreRankNumber mTotalRecord;

	SceNpId *npIds = nullptr;
	

	int ret;
	uint32_t num = 0;

	SceNpScorePlayerRankData *ptr;
	SceNpScoreComment *comments;
	// ----------------------------

	// Check for invalid LManager state.
	assert( m_eFilterMode == eFM_Friends
		|| m_eFilterMode == eFM_MyScore);

	SceNpId myNpId;
	// 4J-PB - should it be user 0?
	if(!ProfileManager.IsSignedInLive(0))
	{
		app.DebugPrintf("[LeaderboardManager] OpenSession() fail: User isn't signed in to PSN\n");
		return false;
	}
	ProfileManager.GetSceNpId(0,&myNpId);

	// Get queried users.
	if (m_eFilterMode == eFM_Friends)
	{

		sce::Toolkit::NP::Utilities::Future<sce::Toolkit::NP::FriendsList> s_friendList;
		sce::Toolkit::NP::FriendInfoRequest request;

		memset(&request, 0, sizeof(request));
		request.userInfo.userId = ProfileManager.getUserID(ProfileManager.GetPrimaryPad());
		request.flag = SCE_TOOLKIT_NP_FRIENDS_LIST_ALL;
		ret = sce::Toolkit::NP::Friends::Interface::getFriendslist(&s_friendList, &request, false);

		if(ret != SCE_TOOLKIT_NP_SUCCESS)
		{
			// Error handling
			if (m_eStatsState != eStatsState_Canceled) m_eStatsState = eStatsState_Failed;
			app.DebugPrintf("[LeaderboardManager] getFriendslist fail\n");
			return false;
		}
		else if (s_friendList.hasResult())
		{
			// 4J-JEV: Friends list doesn't include player, leave space for them.
			num = s_friendList.get()->size() + 1;

			npIds = new SceNpId[num];

			int i = 0;

			sce::Toolkit::NP::FriendsList::const_iterator itr; 
			for (itr = s_friendList.get()->begin(); itr != s_friendList.get()->end(); itr++)
			{
				npIds[i] = itr->npid;
				i++;
			}

			npIds[num-1] = myNpId; // 4J-JEV: Append player to end of query.
		}
		else
		{
			// 4J-JEV: Something terrible must have happend,
			// 'getFriendslist' was supposed to be a synchronous operation.
			__debugbreak();

			// 4J-JEV: We can at least fall-back to just the players score.
			num = 1;
			npIds = new SceNpId[1];
			npIds[0] = myNpId;
		}
	}
	else if (m_eFilterMode == eFM_MyScore)
	{
		num = 1;
		npIds = new SceNpId[1];
		npIds[0] = myNpId;
	}

	ptr = new SceNpScorePlayerRankData[num];
	comments = new SceNpScoreComment[num];

	ZeroMemory(ptr, sizeof(SceNpScorePlayerRankData) * num);
	ZeroMemory(comments, sizeof(SceNpScoreComment) * num);

	/* app.DebugPrintf("sceNpScoreGetRankingByNpId(\n\t transaction=%i,\n\t boardID=0,\n\t npId=%i,\n\t friendCount*sizeof(SceNpId)=%i*%i=%i,\
	rankData=%i,\n\t friendCount*sizeof(SceNpScorePlayerRankData)=%i,\n\t nullptr, 0, nullptr, 0,\n\t friendCount=%i,\n...\n",
	transaction, npId, friendCount, sizeof(SceNpId), friendCount*sizeof(SceNpId),
	rankData, friendCount*sizeof(SceNpScorePlayerRankData), friendCount
	); */

	int boardId = getBoardId(m_difficulty, m_statsType);

	for (int batch=0; batch<num; batch+=100)
	{
		ret = sceNpScoreCreateTransactionCtx(m_titleContext);
		if (m_eStatsState == eStatsState_Canceled)
		{
			// Cancel operation has been called, abort.
			app.DebugPrintf("[LeaderboardManager]\tgetScoreByIds() - m_eStatsState == eStatsState_Canceled.\n");

			sceNpScoreDestroyTransactionCtx(ret);

			if (npIds		!= nullptr)	delete [] npIds;
			if (ptr			!= nullptr)	delete [] ptr;
			if (comments	!= nullptr)	delete [] comments;

			return false;
		}
		else if (ret < 0)
		{
			// Error occurred creating a transacion, abort.
			app.DebugPrintf("[LeaderboardManager]\tgetScoreByIds() - createTransaction failed, ret=0x%X\n", ret);

			m_eStatsState = eStatsState_Failed;

			if (npIds		!= nullptr)	delete [] npIds;
			if (ptr			!= nullptr)	delete [] ptr;
			if (comments	!= nullptr)	delete [] comments;

			return false;
		}
		else
		{
			// Transaction created successfully, continue.
			m_requestId = ret;
		}

		int tmpNum = min( num - batch, (unsigned int) 100 );

		app.DebugPrintf("[LeaderboardManager]\t Requesting ids %i-%i of %i.\n", batch, batch+tmpNum, num);

		ret = sceNpScoreGetRankingByNpId(
			m_requestId,
			boardId, // BoardId

			batch + npIds,		sizeof(SceNpId) * tmpNum,					//IN: Player IDs
			batch + ptr,		sizeof(SceNpScorePlayerRankData) * tmpNum,	//OUT: Rank Data

			batch + comments,	sizeof(SceNpScoreComment) * tmpNum,			//OUT: Comments

			nullptr, 0, // GameData. (unused)

			tmpNum,

			&last_sort_date,
			&mTotalRecord,

			nullptr // Reserved, specify null.
			);

		if (ret == SCE_NP_COMMUNITY_ERROR_ABORTED)
		{
			ret = sceNpScoreDestroyTransactionCtx(m_requestId);
			app.DebugPrintf("[LeaderboardManager] getScoreByIds(): 'sceNpScoreGetRankingByRange' aborted (0x%X).\n", ret);

			delete [] ptr;
			delete [] comments;
			delete [] npIds;

			return false;
		}
		else if (ret == SCE_NP_COMMUNITY_SERVER_ERROR_GAME_RANKING_NOT_FOUND)
		{
			// 4J-JEV: Keep going, other batches might have scores.
		}
		else if (ret<0) goto error3;

		// Return.
		sceNpScoreDestroyTransactionCtx(m_requestId);
		m_requestId = 0;
	}

	m_readCount = num;

	// Filter scorers and construct output structure.
	if (m_scores != nullptr) delete [] m_scores;
	m_scores = new ReadScore[m_readCount];
	convertToOutput(m_readCount, m_scores, ptr, comments);
	m_maxRank = m_readCount;

	app.DebugPrintf(
		"[LeaderboardManager] getScoreByIds(), Success!\n"
		"\t Board %i\n"
		"\t %i of %i results have an entry\n"
		"1stScore=%i\n",
		boardId,
		m_readCount, num,
		ptr->rankData.scoreValue
		);

	// Sort scores
	std::sort(m_scores, m_scores + m_readCount, SortByRank);

	delete [] ptr;
	delete [] comments;
	delete [] npIds;

	m_eStatsState = eStatsState_Ready;
	return true;

	// Error.
error3:
	if (ret!=SCE_NP_COMMUNITY_ERROR_ABORTED) //0x8002a109
		sceNpScoreDestroyTransactionCtx(m_requestId);
	m_requestId = 0;
	delete [] ptr;
	delete [] comments;
error2:
	if (npIds != nullptr) delete [] npIds;
error1:
	if (m_eStatsState != eStatsState_Canceled) m_eStatsState = eStatsState_Failed;
	app.DebugPrintf("[LeaderboardManger] getScoreByIds() FAILED, ret=0x%X\n", ret);
	return false;
}

bool OrbisLeaderboardManager::getScoreByRange()
{
	SceRtcTick last_sort_date;
	SceNpScoreRankNumber mTotalRecord;

	unsigned int num = m_readCount;
	SceNpScoreRankData *ptr;
	SceNpScoreComment *comments;

	assert(m_eFilterMode == eFM_TopRank);

	int ret = sceNpScoreCreateTransactionCtx(m_titleContext);
	if (m_eStatsState == eStatsState_Canceled)
	{
		// Cancel operation has been called, abort.
		app.DebugPrintf("[LeaderboardManager]\tgetScoreByRange() - m_eStatsState == eStatsState_Canceled.\n");
		sceNpScoreDestroyTransactionCtx(ret);
		return false;
	}
	else if (ret < 0)
	{
		// Error occurred creating a transacion, abort.
		m_eStatsState = eStatsState_Failed;
		app.DebugPrintf("[LeaderboardManager]\tgetScoreByRange() - createTransaction failed, ret=0x%X\n", ret);
		return false;
	}
	else
	{
		// Transaction created successfully, continue.
		m_requestId = ret;
	}

	ptr = new SceNpScoreRankData[num];
	comments = new SceNpScoreComment[num];

	int boardId = getBoardId(m_difficulty, m_statsType);
	ret = sceNpScoreGetRankingByRange(
		m_requestId,
		boardId, // BoardId

		m_startIndex,

		ptr, sizeof(SceNpScoreRankData) * num,	//OUT: Rank Data

		comments, sizeof(SceNpScoreComment) * num, //OUT: Comment Data

		nullptr, 0, // GameData.

		num,

		&last_sort_date,
		&m_maxRank,		// 'Total number of players registered in the target scoreboard.'

		nullptr // Reserved, specify null.
		);

	if (ret == SCE_NP_COMMUNITY_ERROR_ABORTED)
	{
		ret = sceNpScoreDestroyTransactionCtx(m_requestId);
		app.DebugPrintf("[LeaderboardManager] getScoreByRange(): 'sceNpScoreGetRankingByRange' aborted (0x%X).\n", ret);

		delete [] ptr;
		delete [] comments;

		return false;
	}
	else if (ret == SCE_NP_COMMUNITY_SERVER_ERROR_GAME_RANKING_NOT_FOUND)
	{
		ret = sceNpScoreDestroyTransactionCtx(m_requestId);
		app.DebugPrintf("[LeaderboardManager] getScoreByRange(): Game ranking not found.");

		delete [] ptr;
		delete [] comments;

		m_scores = nullptr;
		m_readCount = 0;

		m_eStatsState = eStatsState_Ready;
		return false;
	}
	else if (ret<0) goto error2;
	else 
	{
		app.DebugPrintf("[LeaderboardManager] getScoreByRange(), success, 1stScore=%i.\n", ptr->scoreValue);
	}

	// Return.
	sceNpScoreDestroyTransactionCtx(m_requestId);
	m_requestId = 0;

	//m_stats = ptr; //Maybe: addPadding(num,ptr);

	if (m_scores != nullptr) delete [] m_scores;
	m_readCount = ret;
	m_scores = new ReadScore[m_readCount];
	for (int i=0; i<m_readCount; i++)
	{
		//memcpy(m_scores+i, ptr+i, sizeof(SceNpScoreRankData));
		initReadScoreStruct(m_scores[i], ptr[i]);
		//fromBase32(m_scores+i, comments+i);
		fillReadScoreStruct(m_scores[i], comments[i]);
	}

	m_eStatsState = eStatsState_Ready;
	return true;

	// Error.
error2:
	if (ret!=SCE_NP_COMMUNITY_ERROR_ABORTED) //0x8002a109
		sceNpScoreDestroyTransactionCtx(m_requestId);
	m_requestId = 0;

	delete [] ptr;
	delete [] comments;
error1:
	if (m_eStatsState != eStatsState_Canceled) m_eStatsState = eStatsState_Failed;
	app.DebugPrintf("[LeaderboardManager]\tgetScoreByRange() failed, ret=0x%X\n", ret);
	return false;
}

bool OrbisLeaderboardManager::setScore()
{
	int ret;
	SceNpId npId;
	int32_t writeTitleContext = 0;
	SceNpScoreRankNumber tmp = 0;
	SceNpScoreComment comment;

	// Get next job.

	EnterCriticalSection(&m_csViewsLock);
	RegisterScore rscore = m_views.front();
	m_views.pop();
	LeaveCriticalSection(&m_csViewsLock);

	if ( ProfileManager.IsGuest(rscore.m_iPad) )
	{
		app.DebugPrintf("[LeaderboardManager] setScore(): m_iPad[%i] is guest.\n", rscore.m_iPad);
		return true;
	}

	ProfileManager.GetSceNpId(rscore.m_iPad, &npId);
	writeTitleContext = sceNpScoreCreateNpTitleCtx( 0, &npId);
	if (writeTitleContext < 0)
	{
		app.DebugPrintf("[LeaderboardManager] setScore(): sceNpScoreCreateTitleCtx FAILED, ret == %X.\n", ret);
		return false;
	}

	ret = sceNpScoreCreateTransactionCtx(writeTitleContext);

	// Start emptying queue if leaderboards has been closed.
	if (ret == SCE_NP_COMMUNITY_ERROR_NOT_INITIALIZED)
	{
		EnterCriticalSection(&m_csViewsLock);
		m_views.pop();
		LeaveCriticalSection(&m_csViewsLock);
	}

	// Error handling.
	if (ret<0)
	{
		app.DebugPrintf("[LeaderboardManager] setScore() FAILED, ret=0x%X\n", ret);
		sceNpScoreDestroyTitleCtx(writeTitleContext);
		return false;
	}
	m_requestId = ret;

	toBase32(&comment, (void *) &rscore.m_commentData);

	int boardId = getBoardId(rscore.m_difficulty, rscore.m_commentData.m_statsType);
	ret = sceNpScoreRecordScore(
		m_requestId,	// transId,
		boardId,	// boardId,
		rscore.m_score,		//IN: new score,

		&comment,	// Comments
		nullptr,		// GameInfo

		&tmp,		//OUT: current rank,
		nullptr,		//compareDate 

		nullptr // Reserved, specify null.
		);

	if (ret==SCE_NP_COMMUNITY_SERVER_ERROR_NOT_BEST_SCORE) //0x8002A415
		app.DebugPrintf("[LeaderboardManager] setScore(), doesn't beat current score, %i.\n", tmp);
	else if (ret==SCE_NP_COMMUNITY_ERROR_ABORTED) goto error1;//0x8002a109
	else if (ret<0) goto error2;
	else 
	{
		app.DebugPrintf("[LeaderboardManager] setScore(), success. boardId=%i, score=%i\n", boardId, rscore.m_score);
	}

	// Return.
	sceNpScoreDestroyTransactionCtx(m_requestId);
	m_requestId = 0;
	//m_eStatsState = eStatsState_Idle;
	return true;

	// Error.
error2:
	sceNpScoreDestroyTransactionCtx(m_requestId);
	m_requestId = 0;
error1:
	app.DebugPrintf("[LeaderboardManager] setScore() FAILED, ret=0x%X\n", ret);
	sceNpScoreDestroyTitleCtx(writeTitleContext);
	return false;
}

void OrbisLeaderboardManager::Tick()
{
	ReadView view;

	switch( m_eStatsState )
	{
	case eStatsState_Ready:
		{		
			assert(m_scores != nullptr || m_readCount == 0);

			view.m_numQueries = m_readCount;
			view.m_queries = m_scores;

			// 4J-JEV: Debugging.
			//LeaderboardManager::printStats(view);

			eStatsReturn ret = eStatsReturn_NoResults;
			if (view.m_numQueries > 0)
				ret = eStatsReturn_Success;

			if (m_readListener != nullptr)
			{
				app.DebugPrintf("[LeaderboardManager] OnStatsReadComplete(%i, %i, _), m_readCount=%i.\n", ret, m_maxRank, m_readCount);
				m_readListener->OnStatsReadComplete(ret, m_maxRank, view);
			}

			m_eStatsState = eStatsState_Idle;

			delete [] m_scores;
			m_scores = nullptr;
		}
		break;

	case eStatsState_Failed:
		{
			view.m_numQueries = 0;
			view.m_queries = nullptr;

			if ( m_readListener != nullptr )
				m_readListener->OnStatsReadComplete(eStatsReturn_NetworkError, 0, view);

			m_eStatsState = eStatsState_Idle;
		}
		break;

	case eStatsState_Canceled:
		{
			m_eStatsState = eStatsState_Idle;
		}
		break;

	default: // Getting or Idle.
		break;
	}
}

bool OrbisLeaderboardManager::OpenSession()
{
	if (m_openSessions == 0)
	{
		if (m_threadScoreboard == nullptr)
		{
			m_threadScoreboard = new C4JThread(&scoreboardThreadEntry, this, "4JScoreboard");
			m_threadScoreboard->SetProcessor(CPU_CORE_LEADERBOARDS);
			m_threadScoreboard->SetPriority(THREAD_PRIORITY_BELOW_NORMAL);
			m_threadScoreboard->Run();
		}

		app.DebugPrintf("[LeaderboardManager] OpenSession(): Starting sceNpScore utility.\n");
	}
	else
	{
		app.DebugPrintf("[LeaderboardManager] OpenSession(): Another session opened, total=%i\n", m_openSessions+1);
	}

	m_openSessions++;
	return true;
}

void OrbisLeaderboardManager::CloseSession()
{
	m_openSessions--;

	if (m_openSessions == 0)	app.DebugPrintf("[LeaderboardManager] CloseSession(): Quitting sceNpScore utility.\n");
	else						app.DebugPrintf("[LeaderboardManager] CloseSession(): %i sessions still open.\n", m_openSessions);
}

void OrbisLeaderboardManager::DeleteSession() {}

bool OrbisLeaderboardManager::WriteStats(unsigned int viewCount, ViewIn views)
{
	// Need to cancel read/write operation first.
	//if (m_eStatsState != eStatsState_Idle) return false;

	// Write relevant parameters.
	//RegisterScore *regScore = reinterpret_cast<RegisterScore *>(views);

	EnterCriticalSection(&m_csViewsLock);
	for (int i=0; i<viewCount; i++)
	{
		app.DebugPrintf("[LeaderboardManager] WriteStats(), starting. difficulty=%i, statsType=%i, score=%i\n",
			views[i].m_difficulty, views[i].m_commentData.m_statsType, views[i].m_score);

		m_views.push(views[i]);
	}
	LeaveCriticalSection(&m_csViewsLock);

	delete [] views; //*regScore;

	//m_eStatsState = eStatsState_Writing;
	return true;
}

// myUID ignored on PS3.
bool OrbisLeaderboardManager::ReadStats_Friends(LeaderboardReadListener *listener, int difficulty, EStatsType type, PlayerUID myUID, unsigned int startIndex, unsigned int readCount)
{
	// Need to cancel read/write operation first.
	if (m_eStatsState != eStatsState_Idle) return false;
	if (!LeaderboardManager::ReadStats_Friends(listener, difficulty, type, myUID, startIndex, readCount)) return false;

	//int ret = sceNpManagerGetNpId(&m_myNpId);
	//if (ret<0) return false;

	m_eStatsState = eStatsState_Getting;
	return true;
}

// myUID ignored on PS3.
bool OrbisLeaderboardManager::ReadStats_MyScore(LeaderboardReadListener *listener, int difficulty, EStatsType type, PlayerUID myUID, unsigned int readCount)
{
	// Need to cancel read/write operation first.
	if (m_eStatsState != eStatsState_Idle) return false;
	if (!LeaderboardManager::ReadStats_MyScore(listener, difficulty, type, myUID, readCount)) return false;

	//int ret = sceNpManagerGetNpId(&m_myNpId);
	//if (ret<0) return false;

	m_eStatsState = eStatsState_Getting;
	return true;
}

// myUID ignored on PS3.
bool OrbisLeaderboardManager::ReadStats_TopRank(LeaderboardReadListener *listener, int difficulty, EStatsType type, unsigned int startIndex, unsigned int readCount)
{
	// Need to cancel read/write operation first.
	if (m_eStatsState != eStatsState_Idle) return false;
	if (!LeaderboardManager::ReadStats_TopRank(listener, difficulty, type, startIndex, readCount)) return false;
	m_eStatsState = eStatsState_Getting;
	return true;
}

void OrbisLeaderboardManager::FlushStats() {}

void OrbisLeaderboardManager::CancelOperation()
{
	m_readListener = nullptr;
	m_eStatsState = eStatsState_Canceled;

	if (m_requestId != 0)
	{
		int ret = sceNpScoreAbortTransaction(m_requestId);

		if (ret < 0)
		{
			app.DebugPrintf("[LeaderboardManager] CancelOperation(): Problem encountered aborting current operation, 0x%X.\n", ret);
		}
		else
		{
			app.DebugPrintf("[LeaderboardManager] CancelOperation(): Operation aborted successfully.\n");
		}
	}
	else 
	{
		app.DebugPrintf("[LeaderboardManager] CancelOperation(): No current operation.\n");
	}
}

bool OrbisLeaderboardManager::isIdle()
{
	return m_eStatsState == eStatsState_Idle;
}

int OrbisLeaderboardManager::getBoardId(int difficulty, EStatsType statsType)
{
	switch (statsType)
	{
	case eStatsType_Travelling:
		if (0 <= difficulty && difficulty < 4) return 1 + difficulty; // [1,2,3,4]
		else return -1;

	case eStatsType_Mining:
		if (0 <= difficulty && difficulty < 4) return 5 + difficulty; // [5,6,7,8]
		else return -1;

	case eStatsType_Farming:
		if (0 <= difficulty && difficulty < 4) return 9 + difficulty; // [9,10,11,12]
		else return -1;

	case eStatsType_Kills:
		if (1 <= difficulty && difficulty < 4) return 13 + difficulty - 1; // [13,14,15,16]
		else return -1;

	default: return -1;
	}
}

// 4J-JEV: Filter out all friends who don't have scores.
/*
SceNpScoreRankData *OrbisLeaderboardManager::filterJustScorers(unsigned int &num, SceNpScorePlayerRankData *friendsData)
{
int num2 = 0;
for (int i=0; i<num; i++) if (friendsData[i].hasData) num2++;
num = num2; num2 = 0;
SceNpScoreRankData *out = new SceNpScoreRankData[num];
for (int i=0; i<num; i++) if (friendsData[i].hasData) out[num2++] = friendsData[i].rankData;
return out;
} */

// 4J-JEV: Unused, here if we want to switch LeaderboardManager::ViewOut to 'SceNpScorePlayerRankData'.
SceNpScorePlayerRankData *OrbisLeaderboardManager::addPadding(unsigned int num, SceNpScoreRankData *rankData)
{
	SceNpScorePlayerRankData *out = new SceNpScorePlayerRankData[num];
	for (int i=0; i<num; i++)
	{
		out[i].hasData = true;
		out[i].rankData = rankData[i];
	}
	delete [] rankData;
	return out;
}

// 4J-JEV: Filter and create output object.
void OrbisLeaderboardManager::convertToOutput(unsigned int &num, ReadScore *out, SceNpScorePlayerRankData *rankData, SceNpScoreComment *comm)
{
	int j=0;
	for (int i=0; i<num; i++)
	{
		if (rankData[i].hasData)
		{
			initReadScoreStruct( out[j], rankData[i].rankData );
			fillReadScoreStruct( out[j], comm[i] );

			j++;
		}
	}

	num = j;
}

void OrbisLeaderboardManager::toBinary(void *out, SceNpScoreComment *in)
{
	string decoded = base64_decode( string((char*)in) );
	memcpy(out, decoded.c_str(), RECORD_SIZE);
}

void OrbisLeaderboardManager::fromBinary(SceNpScoreComment **out, void *in)
{
	*out = (SceNpScoreComment *) new unsigned char[SCE_NP_SCORE_COMMENT_MAXLEN + 1];
	string encoded = base64_encode((unsigned char const *) in, RECORD_SIZE);
	memcpy(out, encoded.c_str(), SCE_NP_SCORE_COMMENT_MAXLEN);
}

void OrbisLeaderboardManager::toBase32(SceNpScoreComment *out, void *in)
{
	ZeroMemory(out,sizeof(SceNpScoreComment));
	PBYTE bytes = (PBYTE) in;
	char *chars = out->utf8Comment;

	for (int i = 0; i < SCE_NP_SCORE_COMMENT_MAXLEN; i++)
	{
		int sByte = (i*5) / 8;
		int eByte = (5+(i*5)) / 8;
		int dIndex = (i*5) % 8;

		unsigned char fivebits = 0;

		fivebits = *(bytes+sByte) << dIndex;

		if (eByte != sByte)
			fivebits = fivebits | *(bytes+eByte) >> (8-dIndex);

		fivebits = (fivebits>>3) & 0x1F;

		if (fivebits < 10)				// 0 - 9
			chars[i] = '0' + fivebits;
		else if (fivebits < 32)			// A - V
			chars[i] = 'A' + (fivebits-10);
		else
			assert(false);
	}

	toSymbols(out->utf8Comment);
}

void OrbisLeaderboardManager::fromBase32(void *out, SceNpScoreComment *in)
{
	PBYTE bytes = (PBYTE) out;
	ZeroMemory(bytes, RECORD_SIZE);

	fromSymbols(in->utf8Comment);

	char ch[2] = { 0, 0 };
	for (int i = 0; i < SCE_NP_SCORE_COMMENT_MAXLEN; i++)
	{
		ch[0] = in->utf8Comment[i];
		unsigned char fivebits = strtol(ch, nullptr, 32) << 3;

		int sByte = (i*5) / 8;
		int eByte = (5+(i*5)) / 8;
		int dIndex = (i*5) % 8;

		*(bytes + sByte) = *(bytes+sByte) | (fivebits >> dIndex);

		if (eByte != sByte)
			*(bytes + eByte) = fivebits << (8-dIndex);
	}
}

char symbBase32[32] = {
	' ', '!','\"', '#', '$', '%', '&','\'', '(', ')',
	'*', '+', '`', '-', '.', '/', ':', ';', '<', '=',
	'>', '?', '[','\\', ']', '^', '_', '{', '|', '}',
	'~', '@'
};

char charBase32[32] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V'
};

void OrbisLeaderboardManager::toSymbols(char *str)
{
	for (int i = 0; i < 63; i++)
	{
		for (int j=0; j < 32; j++)
		{
			if (str[i]==charBase32[j])
				str[i] =symbBase32[j];
		}
	}
}

void OrbisLeaderboardManager::fromSymbols(char *str)
{
	for (int i = 0; i < 63; i++)
	{
		for (int j=0; j < 32; j++)
		{
			if (str[i]==symbBase32[j])
				str[i] =charBase32[j];
		}
	}
}

bool OrbisLeaderboardManager::test_string(string testing)
{
#ifndef _CONTENT_PACKAGE
	static SceNpScoreComment comment;
	ZeroMemory(&comment, sizeof(SceNpScoreComment));
	memcpy(&comment, testing.c_str(), SCE_NP_SCORE_COMMENT_MAXLEN);

	int ctx = sceNpScoreCreateTransactionCtx(m_titleContext);
	if (ctx<0) return false;

	int ret = sceNpScoreCensorComment(ctx, (const char *) &comment, nullptr);

	if (ret == SCE_NP_COMMUNITY_SERVER_ERROR_CENSORED)
	{
		app.DebugPrintf("\n[TEST_STRING]: REJECTED ");
	}
	else if (ret < 0)
	{
		sceNpScoreDestroyTransactionCtx(ctx);
		return false;
	}
	else 
	{
		app.DebugPrintf("\n[TEST_STRING]: permitted ");
	}

	app.DebugPrintf("'%s'\n", comment.utf8Comment);
	sceNpScoreDestroyTransactionCtx(ctx);
	return true;
#else
	return true;
#endif
}

void OrbisLeaderboardManager::initReadScoreStruct(ReadScore &out, SceNpScoreRankData &rankData)
{
	ZeroMemory(&out, sizeof(ReadScore));

	// Init rank and onlineID
	out.m_uid.setOnlineID( rankData.npId.handle, true );
	out.m_rank = rankData.rank;

	// Convert to wstring and copy name.
	wstring wstrName = convStringToWstring( string(rankData.npId.handle.data) ).c_str();
	//memcpy(&out.m_name, wstrName.c_str(), XUSER_NAME_SIZE);
	out.m_name=wstrName;
}

void OrbisLeaderboardManager::fillReadScoreStruct(ReadScore &out, SceNpScoreComment &comment)
{
	StatsData statsData;
	fromBase32( (void *) &statsData, &comment );

	switch (statsData.m_statsType)
	{
	case eStatsType_Farming:
		out.m_statsSize = 6;
		out.m_statsData[0] = statsData.m_farming.m_eggs;
		out.m_statsData[1] = statsData.m_farming.m_wheat;
		out.m_statsData[2] = statsData.m_farming.m_mushroom;
		out.m_statsData[3] = statsData.m_farming.m_sugarcane;
		out.m_statsData[4] = statsData.m_farming.m_milk;
		out.m_statsData[5] = statsData.m_farming.m_pumpkin;
		break;
	case eStatsType_Mining:
		out.m_statsSize = 7;
		out.m_statsData[0] = statsData.m_mining.m_dirt;
		out.m_statsData[1] = statsData.m_mining.m_cobblestone;
		out.m_statsData[2] = statsData.m_mining.m_sand;
		out.m_statsData[3] = statsData.m_mining.m_stone;
		out.m_statsData[4] = statsData.m_mining.m_gravel;
		out.m_statsData[5] = statsData.m_mining.m_clay;
		out.m_statsData[6] = statsData.m_mining.m_obsidian;
		break;
	case eStatsType_Kills:
		out.m_statsSize = 7;
		out.m_statsData[0] = statsData.m_kills.m_zombie;
		out.m_statsData[1] = statsData.m_kills.m_skeleton;
		out.m_statsData[2] = statsData.m_kills.m_creeper;
		out.m_statsData[3] = statsData.m_kills.m_spider;
		out.m_statsData[4] = statsData.m_kills.m_spiderJockey;
		out.m_statsData[5] = statsData.m_kills.m_zombiePigman;
		out.m_statsData[6] = statsData.m_kills.m_slime;
		break;
	case eStatsType_Travelling:
		out.m_statsSize = 4;
		out.m_statsData[0] = statsData.m_travelling.m_walked;
		out.m_statsData[1] = statsData.m_travelling.m_fallen;
		out.m_statsData[2] = statsData.m_travelling.m_minecart;
		out.m_statsData[3] = statsData.m_travelling.m_boat;
		break;
	}
}

bool OrbisLeaderboardManager::SortByRank(const ReadScore &lhs, const ReadScore &rhs)
{
	return lhs.m_rank < rhs.m_rank;
}

// Notify plus feature use
/*void OrbisLeaderboardManager::NotifyAsyncPlusFeature()
{
	SceNpNotifyPlusFeatureParameter param = SceNpNotifyPlusFeatureParameter();
	param.userId = ProfileManager.getUserID(ProfileManager.GetPrimaryPad());
	param.size = sizeof(SceNpNotifyPlusFeatureParameter);
	param.features = SCE_NP_PLUS_FEATURE_ASYNC_MULTIPLAY;
	ZeroMemory(param.padding, sizeof(char) * 4);
	ZeroMemory(param.reserved, sizeof(uint8_t) * 32);

	int err = sceNpNotifyPlusFeature(&param);
	if (err != SCE_OK)
	{
		app.DebugPrintf("OrbisLeaderboardManager::NotifyAsyncPlusFeature: sceNpNotifyPlusFeature failed (0x%x)\n", err);
		assert(0);
	}
}*/