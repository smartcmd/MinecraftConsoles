#include "stdafx.h"
#include "DurangoLeaderboardManager.h"
#include "../../../Minecraft.World/StringHelpers.h"

namespace WFC = Windows::Foundation::Collections;
namespace CC = concurrency;

LeaderboardManager *LeaderboardManager::m_instance = new DurangoLeaderboardManager(); //Singleton instance of the LeaderboardManager

DurangoLeaderboardManager::DurangoLeaderboardManager()
{
	m_eStatsState = eStatsState_Idle;
	InitializeCriticalSection(&m_csStatsState);

	m_openSessions = 0;
	m_xboxLiveContext = nullptr;
	m_scores = NULL;
	m_readCount = 0;
	m_maxRank = 0;
	m_waitingForProfiles = false;

	m_difficulty = 0;
	m_type = eStatsType_UNDEFINED;
	m_statNames = ref new PC::Vector<P::String^>();
	m_xboxUserIds = ref new PC::Vector<P::String^>();
	m_leaderboardAsyncOp = nullptr;
	m_statsAsyncOp = nullptr;

	for(unsigned int difficulty = 0; difficulty < 4; ++difficulty)
	{
		m_leaderboardNames[difficulty][eStatsType_Travelling] = L"LeaderboardTravelling" + std::to_wstring(difficulty);
		m_leaderboardNames[difficulty][eStatsType_Mining] = L"LeaderboardMining" + std::to_wstring(difficulty);
		m_leaderboardNames[difficulty][eStatsType_Farming] = L"LeaderboardFarming" + std::to_wstring(difficulty);
		m_leaderboardNames[difficulty][eStatsType_Kills] = L"LeaderboardKills" + std::to_wstring(difficulty);

		m_socialLeaderboardNames[difficulty][eStatsType_Travelling] = L"Leaderboard.LeaderboardId.0.DifficultyLevelId." + std::to_wstring(difficulty);
		m_socialLeaderboardNames[difficulty][eStatsType_Mining] = L"Leaderboard.LeaderboardId.1.DifficultyLevelId." + std::to_wstring(difficulty);
		m_socialLeaderboardNames[difficulty][eStatsType_Farming] = L"Leaderboard.LeaderboardId.2.DifficultyLevelId." + std::to_wstring(difficulty);
		m_socialLeaderboardNames[difficulty][eStatsType_Kills] = L"Leaderboard.LeaderboardId.3.DifficultyLevelId." + std::to_wstring(difficulty);

		m_leaderboardStatNames[difficulty][eStatsType_Travelling].push_back( L"DistanceTravelled.DifficultyLevelId." + std::to_wstring(difficulty) + L".TravelMethodId.0"); // Walked
		m_leaderboardStatNames[difficulty][eStatsType_Travelling].push_back( L"DistanceTravelled.DifficultyLevelId." + std::to_wstring(difficulty) + L".TravelMethodId.2"); // Fallen
		m_leaderboardStatNames[difficulty][eStatsType_Travelling].push_back( L"DistanceTravelled.DifficultyLevelId." + std::to_wstring(difficulty) + L".TravelMethodId.4"); // Minecart
		m_leaderboardStatNames[difficulty][eStatsType_Travelling].push_back( L"DistanceTravelled.DifficultyLevelId." + std::to_wstring(difficulty) + L".TravelMethodId.5"); // Boat

		m_leaderboardStatNames[difficulty][eStatsType_Mining].push_back( L"BlockBroken.DifficultyLevelId." + std::to_wstring(difficulty) + L".BlockId.3"); // Dirt
		m_leaderboardStatNames[difficulty][eStatsType_Mining].push_back( L"BlockBroken.DifficultyLevelId." + std::to_wstring(difficulty) + L".BlockId.4"); // Cobblestone
		m_leaderboardStatNames[difficulty][eStatsType_Mining].push_back( L"BlockBroken.DifficultyLevelId." + std::to_wstring(difficulty) + L".BlockId.12"); // Sand
		m_leaderboardStatNames[difficulty][eStatsType_Mining].push_back( L"BlockBroken.DifficultyLevelId." + std::to_wstring(difficulty) + L".BlockId.1"); // Stone
		m_leaderboardStatNames[difficulty][eStatsType_Mining].push_back( L"BlockBroken.DifficultyLevelId." + std::to_wstring(difficulty) + L".BlockId.13"); // Gravel
		m_leaderboardStatNames[difficulty][eStatsType_Mining].push_back( L"BlockBroken.DifficultyLevelId." + std::to_wstring(difficulty) + L".BlockId.82"); // Clay
		m_leaderboardStatNames[difficulty][eStatsType_Mining].push_back( L"BlockBroken.DifficultyLevelId." + std::to_wstring(difficulty) + L".BlockId.49"); // Obsidian

		m_leaderboardStatNames[difficulty][eStatsType_Farming].push_back( L"McItemAcquired.DifficultyLevelId." + std::to_wstring(difficulty) + L".AcquisitionMethodId.1.ItemId.344"); // Eggs
		m_leaderboardStatNames[difficulty][eStatsType_Farming].push_back( L"BlockBroken.DifficultyLevelId." + std::to_wstring(difficulty) + L".BlockId.59"); // Wheat
		m_leaderboardStatNames[difficulty][eStatsType_Farming].push_back( L"BlockBroken.DifficultyLevelId." + std::to_wstring(difficulty) + L".BlockId.39"); // Mushroom
		m_leaderboardStatNames[difficulty][eStatsType_Farming].push_back( L"BlockBroken.DifficultyLevelId." + std::to_wstring(difficulty) + L".BlockId.83"); // Sugarcane
		m_leaderboardStatNames[difficulty][eStatsType_Farming].push_back( L"McItemAcquired.DifficultyLevelId." + std::to_wstring(difficulty) + L".AcquisitionMethodId.2.ItemId.335"); // Milk
		m_leaderboardStatNames[difficulty][eStatsType_Farming].push_back( L"McItemAcquired.DifficultyLevelId." + std::to_wstring(difficulty) + L".AcquisitionMethodId.1.ItemId.86"); // Pumpkin

		m_leaderboardStatNames[difficulty][eStatsType_Kills].push_back( L"MobKilledTotal.DifficultyLevelId." + std::to_wstring(difficulty) + L".EnemyRoleId.54"); // Zombie
		m_leaderboardStatNames[difficulty][eStatsType_Kills].push_back( L"MobKilledTotal.DifficultyLevelId." + std::to_wstring(difficulty) + L".EnemyRoleId.51"); // Skeleton
		m_leaderboardStatNames[difficulty][eStatsType_Kills].push_back( L"MobKilledTotal.DifficultyLevelId." + std::to_wstring(difficulty) + L".EnemyRoleId.50"); // Creeper
		m_leaderboardStatNames[difficulty][eStatsType_Kills].push_back( L"MobKilledTotal.DifficultyLevelId." + std::to_wstring(difficulty) + L".EnemyRoleId.52"); // Spider
		m_leaderboardStatNames[difficulty][eStatsType_Kills].push_back( L"MobKilledTotal.DifficultyLevelId." + std::to_wstring(difficulty) + L".EnemyRoleId.49"); // Spider Jockey
		m_leaderboardStatNames[difficulty][eStatsType_Kills].push_back( L"MobKilledTotal.DifficultyLevelId." + std::to_wstring(difficulty) + L".EnemyRoleId.57"); // Zombie Pigman
		m_leaderboardStatNames[difficulty][eStatsType_Kills].push_back( L"MobKilledTotal.DifficultyLevelId." + std::to_wstring(difficulty) + L".EnemyRoleId.55"); // Slime
	}
}

void DurangoLeaderboardManager::Tick()
{
	ReadView view;

	switch( getState() )
	{
	case eStatsState_GettingLeaderboardInfo:
		break;
	case eStatsState_ReceivedLeaderboardInfo:
		{
			setState(eStatsState_GettingStatsInfo);

			// Get the actual display info for the stats
			m_statsAsyncOp = m_xboxLiveContext->UserStatisticsService->GetMultipleUserStatisticsAsync(
				m_xboxUserIds->GetView(),	// the collection of Xbox user IDs whose stats we want to retrieve
				SERVICE_CONFIG_ID,			// the service config that contains the stats we want
				m_statNames->GetView()		// a list of stat names we want
				);

			auto task = concurrency::create_task(m_statsAsyncOp).then( [this] (CC::task<WFC::IVectorView<MXS::UserStatistics::UserStatisticsResult^>^> resultListTask)
			{
				try
				{
					app.DebugPrintf("[LeaderboardManager] Second continuation\n");
					m_statsAsyncOp = nullptr;

					WFC::IVectorView<MXS::UserStatistics::UserStatisticsResult^>^ resultList = resultListTask.get();

					if (m_xboxLiveContext == nullptr) throw(ref new P::Exception(-1));

					int userIndex = 0;
					for( MXS::UserStatistics::UserStatisticsResult^ result : resultList )
					{
						app.DebugPrintf("XboxUserId: %ls\n", result->XboxUserId->Data());

						for( UINT index = 0; index<result->ServiceConfigurationStatistics->Size; index++ )
						{
							MXS::UserStatistics::ServiceConfigurationStatistic^ configStat = result->ServiceConfigurationStatistics->GetAt(index);
							//app.DebugPrintf("ServiceConfigurationId: %ls\n", configStat->ServiceConfigurationId->Data());

							updateStatsInfo(userIndex, m_difficulty, m_type, configStat->Statistics);
						}
						++userIndex;
					}

					app.DebugPrintf("[LeaderboardManager] Setting to ready\n");
					setState(eStatsState_Ready);

				}
				catch (Platform::Exception^ ex)
				{
					m_leaderboardAsyncOp = nullptr;
					setState(eStatsState_Failed);

					if (ex->HResult == HTTP_E_STATUS_NOT_FOUND)	app.DebugPrintf("[LeaderboardManager] ERROR calling GetLeaderboardAsync: 404 Not Found - 0x%0.8x\n", ex->HResult);
					else										app.DebugPrintf("[LeaderboardManager] ERROR calling GetLeaderboardAsync: 0x%0.8x\n", ex->HResult);
				}
				catch (...)
				{
					app.DebugPrintf("[LeaderboardManager] SecondContinuation: Unknown exception.\n");
				}
			});
		}
		break;
	case eStatsState_GettingStatsInfo:
		break;
	case eStatsState_Ready:
		{
			// If we're waiting on profiles, don't return scores just yet
			if (m_waitingForProfiles)
			{
				return;
			}
			else
			{
				if (m_displayNames.size() == m_readCount)
				{
					// Add display names to scores
					for (int i = 0; i < m_displayNames.size(); i++)
					{
						m_scores[i].m_name = m_displayNames[i];
					}
				}
				else
				{
					// This seem to happen if something went wrong with Xbox Live
					app.DebugPrintf("DurangoLeaderboardManager::Tick: Display names count (%i) didn't match read count (%i)", m_displayNames.size(), m_readCount);
				}

				m_displayNames.clear();
			}

			//assert(m_scores != NULL || m_readCount == 0);

			view.m_numQueries = m_readCount;
			view.m_queries = m_scores;

			// 4J-JEV: Debugging.
			//LeaderboardManager::printStats(view);

			eStatsReturn ret = view.m_numQueries > 0 ? eStatsReturn_Success : eStatsReturn_NoResults;

			if (m_readListener != NULL)
			{
				app.DebugPrintf("[LeaderboardManager] OnStatsReadComplete(%i, %i, _)\n", ret, m_readCount);
				m_readListener->OnStatsReadComplete(ret, m_maxRank, view);
			}

			app.DebugPrintf("[LeaderboardManager] Deleting scores\n");
			delete [] m_scores;
			m_scores = NULL;

			setState(eStatsState_Idle);
		}
		break;

	case eStatsState_Failed:
		view.m_numQueries = 0;
		view.m_queries = NULL;

		if ( m_readListener != NULL )
		{
			m_readListener->OnStatsReadComplete(eStatsReturn_NetworkError, 0, view);
		}

		setState(eStatsState_Idle);

		break;

	case eStatsState_Canceled:
		app.DebugPrintf("[LeaderboardManager] Setting canceled\n");
		setState(eStatsState_Idle);
		break;

	default: // Getting or Idle.
		if (m_openSessions == 0 && m_xboxLiveContext != nullptr)
		{
			m_xboxLiveContext = nullptr;

			app.DebugPrintf("[LeaderboardManager] Tick(): Nulled XboxLiveContext\n");
		}
		break;
	}
}

//Open a session
bool DurangoLeaderboardManager::OpenSession()
{
	if (m_openSessions == 0)
	{
		app.DebugPrintf("[LeaderboardManager] OpenSession()\n");

		try
		{
			WXS::User^ user = ProfileManager.GetUser(ProfileManager.GetPrimaryPad());
			if(user != nullptr && user->IsSignedIn && !user->IsGuest)
			{
				m_xboxLiveContext = ref new MXS::XboxLiveContext(user);
			}
			else
			{
				app.DebugPrintf("[LeaderboardManager] OpenSession(): Failed to created new XboxLiveContext, No valid user\n");
				return false;
			}
		}
		catch (Platform::Exception^ ex)
		{
			m_xboxLiveContext = nullptr;
			app.DebugPrintf("[LeaderboardManager] OpenSession(): Failed to created new XboxLiveContext, ret == 0x%08X.\n", ex->HResult);
			return false;
		}
		catch (...)
		{
			app.DebugPrintf("[LeaderboardManager] OpenSession(): Unknown exception.\n");
			return false;
		}
	}
	else app.DebugPrintf("[LeaderboardManager] OpenSession(): Another session opened, total=%i\n", m_openSessions+1);

	m_openSessions++;
	return true;
}

//Close a session
void DurangoLeaderboardManager::CloseSession()
{
	m_openSessions--;

	if (m_openSessions == 0)
	{
		if(isIdle())
		{
			m_xboxLiveContext = nullptr;

			app.DebugPrintf("[LeaderboardManager] CloseSession(): Nulled XboxLiveContext\n");
		}
	}
	else app.DebugPrintf("[LeaderboardManager] CloseSession(): %i sessions still open.\n", m_openSessions);
}

//Delete a session
void DurangoLeaderboardManager::DeleteSession()
{
}

//Write the given stats
//This is called synchronously and will not free any memory allocated for views when it is done

bool DurangoLeaderboardManager::WriteStats(unsigned int viewCount, ViewIn views)
{
	return false;
}

bool DurangoLeaderboardManager::ReadStats_Friends(LeaderboardReadListener *listener, int difficulty, EStatsType type, PlayerUID myUID, unsigned int startIndex, unsigned int readCount)
{
	// Need to cancel read/write operation first.
	if (!isIdle()) return false;
	if (!LeaderboardManager::ReadStats_Friends(listener, difficulty, type, myUID, startIndex, readCount)) return false;
	setState(eStatsState_GettingLeaderboardInfo);

	if( m_xboxLiveContext == nullptr )
	{
		throw ref new Platform::InvalidArgumentException(L"A valid User is required");
	}

	// Request the leaderboard to get ranking information
	auto asyncOp = m_xboxLiveContext->LeaderboardService->GetLeaderboardForSocialGroupWithSkipToRankAsync(
		ref new P::String(myUID.toString().c_str()),
		SERVICE_CONFIG_ID,
		ref new P::String(m_socialLeaderboardNames[difficulty][type].c_str()),
		MXS::Social::SocialGroupConstants::People,
		startIndex,
		ref new P::String(L"descending"),
		readCount
		);

	runLeaderboardRequest(asyncOp, difficulty, type, readCount, EFilterMode::eFM_Friends);

	return true;
}

bool DurangoLeaderboardManager::ReadStats_MyScore(LeaderboardReadListener *listener, int difficulty, EStatsType type, PlayerUID myUID, unsigned int readCount)
{
	// Need to cancel read/write operation first.
	if (!isIdle()) return false;
	if (!LeaderboardManager::ReadStats_MyScore(listener, difficulty, type, myUID, readCount)) return false;
	setState(eStatsState_GettingLeaderboardInfo);

	if( m_xboxLiveContext == nullptr )
	{
		throw ref new Platform::InvalidArgumentException(L"A valid User is required");
	}

	P::String^ leaderboardName = ref new P::String(m_leaderboardNames[difficulty][type].c_str());

	// Request the leaderboard to get ranking information
	auto asyncOp = m_xboxLiveContext->LeaderboardService->GetLeaderboardWithSkipToUserAsync(
		SERVICE_CONFIG_ID,
		leaderboardName,
		ref new P::String(myUID.toString().c_str()), // skip to this user
		readCount
		);

	runLeaderboardRequest(asyncOp, difficulty, type, readCount, EFilterMode::eFM_MyScore);

	return true;
}

bool DurangoLeaderboardManager::ReadStats_TopRank(LeaderboardReadListener *listener, int difficulty, EStatsType type, unsigned int startIndex, unsigned int readCount)
{
	// Need to cancel read/write operation first.
	if (!isIdle()) return false;
	if (!LeaderboardManager::ReadStats_TopRank(listener, difficulty, type, startIndex, readCount)) return false;
	setState(eStatsState_GettingLeaderboardInfo);

	if( m_xboxLiveContext == nullptr )
	{
		throw ref new Platform::InvalidArgumentException(L"A valid User is required");
	}

	P::String^ leaderboardName = ref new P::String(m_leaderboardNames[difficulty][type].c_str());

	// Request the leaderboard to get ranking information
	auto asyncOp = m_xboxLiveContext->LeaderboardService->GetLeaderboardAsync(
		SERVICE_CONFIG_ID,
		leaderboardName,
		startIndex, // skip this many ranks
		readCount
		);

	runLeaderboardRequest(asyncOp, difficulty, type, readCount, EFilterMode::eFM_TopRank);

	return true;
}

//Perform a flush of the stats
void DurangoLeaderboardManager::FlushStats()
{
}

//Cancel the current operation
void DurangoLeaderboardManager::CancelOperation()
{
	m_readListener = NULL;
	setState(eStatsState_Canceled);

	if(m_leaderboardAsyncOp) m_leaderboardAsyncOp->Cancel();
	if(m_statsAsyncOp) m_statsAsyncOp->Cancel();

	//if (m_transactionCtx != 0)
	//{
	//	int ret = sceNpScoreAbortTransaction(m_transactionCtx);
	//
	//	if (ret < 0)
	//	{
	//		app.DebugPrintf("[LeaderboardManager] CancelOperation(): Problem encountered aborting current operation, 0x%X.\n", ret);
	//	}
	//	else
	//	{
	//		app.DebugPrintf("[LeaderboardManager] CancelOperation(): Operation aborted successfully.\n");
	//	}
	//}
	//else app.DebugPrintf("[LeaderboardManager] CancelOperation(): No current operation.\n");
}

//Is the leaderboard manager idle.
bool DurangoLeaderboardManager::isIdle()
{
	return getState() == eStatsState_Idle;
}

void DurangoLeaderboardManager::runLeaderboardRequest(WF::IAsyncOperation<MXSL::LeaderboardResult^>^ asyncOp, int difficulty, EStatsType type, unsigned int readCount, EFilterMode filter)
{
	m_leaderboardAsyncOp = asyncOp;
	m_difficulty = difficulty;
	m_type = type;

	// Build stat names
	m_statNames = ref new PC::Vector<P::String^>();
	m_statNames->Clear();
	for(wstring name : m_leaderboardStatNames[difficulty][type])
	{
		m_statNames->Append(ref new P::String(name.c_str()));
	}

	app.DebugPrintf("[LeaderboardManager] Running request\n");
	CC::create_task(asyncOp)
		.then( [this, readCount, difficulty, type, filter] (CC::task<MXSL::LeaderboardResult^> resultTask)
	{
		try
		{
			app.DebugPrintf("[LeaderboardManager] First continuation.\n");

			m_leaderboardAsyncOp = nullptr;

			MXSL::LeaderboardResult^ lastResult = resultTask.get();

			app.DebugPrintf(
				"Name: %ls - Filter: %ls - Rows: Retrieved=%d Total=%d Requested=%d.\n",
				lastResult->DisplayName->Data(),
				LeaderboardManager::filterNames[ (int) filter ].c_str(),
				lastResult->Rows->Size, lastResult->TotalRowCount, readCount
				);

			//app.DebugPrintf("Column count: %d, Column: %ls, %ls, %d\n", lastResult->Columns->Size, lastResult->Columns->GetAt(0)->DisplayName->Data(), lastResult->Columns->GetAt(0)->StatisticName->Data(), lastResult->Columns->GetAt(0)->StatisticType);

			// 4J-JEV: Fix for Xbox One #162541 - [CRASH] Code: Leaderboards: Title crashes after pressing [B] Back button while changing Leaderboards' filter to 'My Score'
			// 4J-JEV: Fix for X1: #165487 - [CRASH] XR-074: Compliance: The title does not properly handle switching to offline session while browsing the Leaderboards.
			if (m_xboxLiveContext == nullptr) throw(ref new P::Exception(-1));

			// If this is My_Score, check that the user appears
			if (filter == eFM_MyScore)
			{
				bool userIncluded = false;
				for(int i = 0; i < lastResult->Rows->Size; i++)
				{
					if (lastResult->Rows->GetAt(i)->XboxUserId == m_xboxLiveContext->User->XboxUserId) userIncluded = true;
				}

				// If the user isn't included, don't show the results
				if (!userIncluded)
				{
					m_readCount = 0;
					throw(ref new P::Exception(INET_E_DATA_NOT_AVAILABLE));
				}
			}

			m_maxRank = lastResult->TotalRowCount;
			m_readCount = lastResult->Rows->Size;

			if (m_scores != NULL) delete [] m_scores;
			m_scores = new ReadScore[m_readCount];
			ZeroMemory(m_scores, sizeof(ReadScore) * m_readCount);

			m_xboxUserIds->Clear();

			app.DebugPrintf("[LeaderboardManager] Retrieved Scores:\n");
			for( UINT index = 0; index < lastResult->Rows->Size; index++ )
			{
				MXSL::LeaderboardRow^ row = lastResult->Rows->GetAt(index);

				app.DebugPrintf(
					"\tIndex: %d\tRank: %d\tPercentile: %.1f%%\tXboxUserId: %ls\tValue: %ls.\n",
					index,
					row->Rank,
					row->Percentile * 100,
					row->XboxUserId->Data(),
					row->Values->GetAt(0)->Data()
					);

				m_scores[index].m_name = row->Gamertag->Data();
				m_scores[index].m_rank = row->Rank;
				m_scores[index].m_uid  = PlayerUID(row->XboxUserId->Data());

				// 4J-JEV: Added to help determine if this player's score is hidden due to their privacy settings.
				m_scores[index].m_totalScore = (unsigned long) _fromString<long long>(row->Values->GetAt(0)->Data());

				m_xboxUserIds->Append(row->XboxUserId);
			}

			if(m_readCount > 0)
			{
				vector<PlayerUID> xuids = vector<PlayerUID>();
				for(int i = 0; i < lastResult->Rows->Size; i++)
				{
					xuids.push_back(PlayerUID(lastResult->Rows->GetAt(i)->XboxUserId->Data()));
				}
				m_waitingForProfiles = true;
				ProfileManager.GetProfiles(xuids, &GetProfilesCallback, this);
				setState(eStatsState_ReceivedLeaderboardInfo);
			}
			else
			{
				// we hit the end of the list
				app.DebugPrintf("Reach the end\n");
				setState(eStatsState_Ready);
			}
		}
		catch (Platform::Exception^ ex)
		{
			m_leaderboardAsyncOp = nullptr;
			if (ex->HResult == INET_E_DATA_NOT_AVAILABLE)
			{
				// we hit the end of the list
				app.DebugPrintf("ERROR: Reach the end\n");
				setState(eStatsState_Ready);
			}
			else if(ex->HResult == HTTP_E_STATUS_NOT_FOUND)
			{
				app.DebugPrintf("Error calling GetLeaderboardAsync function: 404 Not Found - 0x%0.8x\n", ex->HResult);
				setState(eStatsState_Failed);
			}
			else
			{
				app.DebugPrintf("Error calling GetLeaderboardAsync function: 0x%0.8x\n", ex->HResult);
				setState(eStatsState_Failed);
			}
		}
		catch (...)
		{
			app.DebugPrintf("[LeaderboardManager] Unknown exception.\n");
		}
	});
}

void DurangoLeaderboardManager::updateStatsInfo(int userIndex, int difficulty, EStatsType type, WFC::IVectorView<MXS::UserStatistics::Statistic^>^ statsResult)
{
	if (m_scores)
	{
		LeaderboardManager::ReadScore *userScore = &m_scores[userIndex];

		switch (type)
		{
		case eStatsType_Farming:	userScore->m_statsSize = 6; break;
		case eStatsType_Mining:		userScore->m_statsSize = 7; break;
		case eStatsType_Kills:		userScore->m_statsSize = 7; break;
		case eStatsType_Travelling:	userScore->m_statsSize = 4; break;
		}

		int statIndex = 0, sumScores = 0;
		for(wstring statName : m_leaderboardStatNames[difficulty][type])
		{
			bool found = false;
			for(auto result : statsResult)
			{
				if(statName.compare(result->StatisticName->Data()) == 0)
				{
					userScore->m_statsData[statIndex] = _fromString<unsigned long>(result->Value->Data());
					found = true;
					break;
				}
			}
			if(!found)
			{
				userScore->m_statsData[statIndex] = 0;
			}

			sumScores += userScore->m_statsData[statIndex];
			++statIndex;
		}

		if ( (sumScores == 0) && (userScore->m_totalScore > 0) )
		{
			app.DebugPrintf("[LeaderboardManager] Player '%s' (rank %d) likely has privacy settings enabled.\n", userScore->m_name.c_str(), userScore->m_rank);
			userScore->m_idsErrorMessage = IDS_LEADERBOARD_SCORE_HIDDEN;
		}
	}
}

void DurangoLeaderboardManager::GetProfilesCallback(LPVOID param, std::vector<Microsoft::Xbox::Services::Social::XboxUserProfile^> profiles)
{
	DurangoLeaderboardManager *dlm = (DurangoLeaderboardManager *)param;

	app.DebugPrintf("[LeaderboardManager] GetProfilesCallback, profiles.size() == %d.\n", profiles.size());

	if (profiles.size() > 0)
	{
		dlm->m_displayNames = vector<wstring>();
		for (int i = 0; i < profiles.size(); i++)
		{
			dlm->m_displayNames.push_back(profiles[i]->GameDisplayName->Data());
		}
	}
	else
	{
		dlm->setState(eStatsState_Failed);
	}

	dlm->m_waitingForProfiles = false;
}

DurangoLeaderboardManager::EStatsState DurangoLeaderboardManager::getState()
{
	return m_eStatsState;
}

void DurangoLeaderboardManager::setState(EStatsState newState)
{
	EnterCriticalSection(&m_csStatsState);

	bool validTransition = false;

	switch(m_eStatsState)
	{
	case eStatsState_Idle:
		switch(newState)
		{
		case eStatsState_GettingLeaderboardInfo:
			validTransition = true;
			break;
		};
		break;
	case eStatsState_GettingLeaderboardInfo:
		switch(newState)
		{
		case eStatsState_Ready:
		case eStatsState_ReceivedLeaderboardInfo:
		case eStatsState_Canceled:
		case eStatsState_Failed:
			validTransition = true;
			break;
		};
		break;
		break;
	case eStatsState_ReceivedLeaderboardInfo:
		switch(newState)
		{
		case eStatsState_GettingStatsInfo:
		case eStatsState_Canceled:
		case eStatsState_Failed:
			validTransition = true;
			break;
		};
		break;
	case eStatsState_GettingStatsInfo:
		switch(newState)
		{
		case eStatsState_Ready:
		case eStatsState_Canceled:
		case eStatsState_Failed:
			validTransition = true;
			break;
		};
		break;
	case eStatsState_Failed:
		switch(newState)
		{
		case eStatsState_Idle:
			validTransition = true;
			break;
		};
		break;
	case eStatsState_Ready:
		switch(newState)
		{
		case eStatsState_Canceled:
		case eStatsState_Idle:
		case eStatsState_Failed:
			validTransition = true;
			break;
		};
		break;
	case eStatsState_Canceled:
		switch(newState)
		{
		case eStatsState_Ready:
			newState = eStatsState_Idle;
		case eStatsState_Idle:
			validTransition = true;
			break;
		};
		break;
	};

#ifndef _CONTENT_PACKAGE
	app.DebugPrintf(
		"[LeaderboardManager] %s state transition:\t%ls(%d) -> %ls(%d).\n",
		(validTransition ? "Valid" : "INVALID"),
		stateToString(m_eStatsState).c_str(),	m_eStatsState,
		stateToString(newState).c_str(),		newState
		);
#endif

	if (validTransition)
	{
		m_eStatsState = newState;
	}

	LeaveCriticalSection(&m_csStatsState);
}

wstring DurangoLeaderboardManager::stateToString(EStatsState eState)
{
	switch (eState)
	{
		case eStatsState_Idle:						return L"eStatsState_Idle";
		case eStatsState_GettingLeaderboardInfo:	return L"eStatsState_GettingLeaderboardInfo";
		case eStatsState_ReceivedLeaderboardInfo:	return L"eStatsState_ReceivedLeaderboardInfo";
		case eStatsState_GettingStatsInfo:			return L"eStatsState_GettingStatsInfo";
		case eStatsState_ReceivedStatsInfo:			return L"eStatsState_ReceivedStatsInfo";
		case eStatsState_Failed:					return L"eStatsState_Failed";
		case eStatsState_Ready:						return L"eStatsState_Ready";
		case eStatsState_Canceled:					return L"eStatsState_Canceled";
		case eStatsState_Max:						return L"eStatsState_MAX";
		default:									return L"UNKNOWN";
	}
}