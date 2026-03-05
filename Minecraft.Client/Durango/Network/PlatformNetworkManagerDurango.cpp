#include "stdafx.h"
#include "..\..\..\Minecraft.World\Socket.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "PlatformNetworkManagerDurango.h"
#include "NetworkPlayerDurango.h"

CPlatformNetworkManagerDurango *g_pPlatformNetworkManager;

void CPlatformNetworkManagerDurango::HandleStateChange(DQRNetworkManager::eDQRNetworkManagerState oldState, DQRNetworkManager::eDQRNetworkManagerState newState)
{
	static const char * c_apszStateNames[] =
    {
		"DNM_STATE_INITIALISING",
		"DNM_STATE_INITIALISE_FAILED",
		"DNM_STATE_IDLE",
		"DNM_STATE_HOSTING",
		"DNM_STATE_JOINING",
		"DNM_STATE_STARTING",
		"DNM_STATE_PLAYING",
		"DNM_STATE_LEAVING",
		"DNM_STATE_ENDING",
    };

    app.DebugPrintf( "Network State: %s ==> %s\n",
        c_apszStateNames[ oldState ],
        c_apszStateNames[ newState ] );

	if( newState == DQRNetworkManager::DNM_STATE_HOSTING )
	{
		m_bLeavingGame = false;
		m_bLeaveGameOnTick = false;
		m_bHostChanged = false;
		g_NetworkManager.StateChange_AnyToHosting();
	}
	else if( newState == DQRNetworkManager::DNM_STATE_JOINING )
	{
		m_bLeavingGame = false;
		m_bLeaveGameOnTick = false;
		m_bHostChanged = false;
		g_NetworkManager.StateChange_AnyToJoining();
	}
	else if( newState == DQRNetworkManager::DNM_STATE_IDLE && oldState == DQRNetworkManager::DNM_STATE_JOINING )
	{
		g_NetworkManager.StateChange_JoiningToIdle(JOIN_FAILED_NONSPECIFIC);
	}
	else if( newState == DQRNetworkManager::DNM_STATE_IDLE && oldState == DQRNetworkManager::DNM_STATE_HOSTING )
	{
		m_bLeavingGame = true;
	}
	else if( newState == DQRNetworkManager::DNM_STATE_STARTING )
	{
		m_lastPlayerEventTimeStart = app.getAppTime();

		g_NetworkManager.StateChange_AnyToStarting();
	}
	// Fix for #93148 - TCR 001: BAS Game Stability: Title will crash for the multiplayer client if host of the game will exit during the clients loading to created world.
	// 4J Stu - If the client joins just as the host is exiting, then they can skip to leaving without passing through ending
	else if( newState == DQRNetworkManager::DNM_STATE_ENDING )
	{
		g_NetworkManager.StateChange_AnyToEnding( oldState == DQRNetworkManager::DNM_STATE_PLAYING );

		if( m_pDQRNet->IsHost() )
		{
			m_bLeavingGame = true;
		}
	}

	if( newState == DQRNetworkManager::DNM_STATE_IDLE )
	{
		g_NetworkManager.StateChange_AnyToIdle();
	}
}

void CPlatformNetworkManagerDurango::HandlePlayerJoined(DQRNetworkPlayer *pDQRPlayer)
{
	const char * pszDescription;

	// If this is a local player, we need to inform the chat system that it has joined
	if( pDQRPlayer->IsLocal() )
	{
		m_pDQRNet->ChatPlayerJoined(pDQRPlayer->GetLocalPlayerIndex());
	}

	// 4J Stu - We create a fake socket for every where that we need an INBOUND queue of game data. Outbound
	// is all handled by QNet so we don't need that. Therefore each client player has one, and the host has one
	// for each client player.
	bool createFakeSocket = false;
	bool localPlayer = false;

	NetworkPlayerDurango *networkPlayer = (NetworkPlayerDurango *)addNetworkPlayer(pDQRPlayer);

	// Request full display name for this player
	m_pDQRNet->RequestDisplayName(pDQRPlayer);

    if( pDQRPlayer->IsLocal() )
    {
		localPlayer = true;
        if( pDQRPlayer->IsHost() )
        {
            pszDescription = "local host";
			// 4J Stu - No socket for the localhost as it uses a special loopback queue

			m_machineDQRPrimaryPlayers.push_back( pDQRPlayer );
        }
        else
        {
            pszDescription = "local";

			// We need an inbound queue on all local players to receive data from the host
			createFakeSocket = true;
        }
    }
    else
    {
        if( pDQRPlayer->IsHost() )
        {
            pszDescription = "remote host";
        }
        else
        {
            pszDescription = "remote";

			// If we are the host, then create a fake socket for every remote player
			if( m_pDQRNet->IsHost() )
			{
				createFakeSocket = true;
			}
        }

		if( m_pDQRNet->IsHost() && !m_bHostChanged )
		{
			// Do we already have a primary player for this system?
			bool systemHasPrimaryPlayer = false;
            for ( DQRNetworkPlayer *pQNetPrimaryPlayer : m_machineDQRPrimaryPlayers )
            {
				if( pDQRPlayer->IsSameSystem(pQNetPrimaryPlayer) )
				{
					systemHasPrimaryPlayer = true;
					break;
				}
			}
			if( !systemHasPrimaryPlayer )
				m_machineDQRPrimaryPlayers.push_back( pDQRPlayer );
		}
    }
	g_NetworkManager.PlayerJoining( networkPlayer );

	if( createFakeSocket == true && !m_bHostChanged )
	{
		g_NetworkManager.CreateSocket( networkPlayer, localPlayer );
	}

    app.DebugPrintf( "Player 0x%p \"%ls\" joined; %s; voice %i; camera %i.\n",
        pDQRPlayer,
        pDQRPlayer->GetGamertag(),
        pszDescription,
        (int) pDQRPlayer->HasVoice(),
        (int) pDQRPlayer->HasCamera() );


	if( m_pDQRNet->IsHost() )
	{
		g_NetworkManager.UpdateAndSetGameSessionData();
		SystemFlagAddPlayer( networkPlayer );
	}

	for( int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
	{
		if(playerChangedCallback[idx] != NULL)
			playerChangedCallback[idx]( playerChangedCallbackParam[idx], networkPlayer, false );
	}

	if(m_pDQRNet->GetState() == QNET_STATE_GAME_PLAY)
	{
		int localPlayerCount = 0;
		for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
		{
			if( m_pDQRNet->GetLocalPlayerByUserIndex(idx) != NULL ) ++localPlayerCount;
		}

		float appTime = app.getAppTime();

		// Only record stats for the primary player here
		m_lastPlayerEventTimeStart = appTime;
	}
}

void CPlatformNetworkManagerDurango::HandlePlayerLeaving(DQRNetworkPlayer *pDQRPlayer)
{
	//__debugbreak();

	app.DebugPrintf( "Player 0x%p leaving.\n",
        pDQRPlayer );

	INetworkPlayer *networkPlayer = getNetworkPlayer(pDQRPlayer);

	if( networkPlayer )
	{
		// Get our wrapper object associated with this player.
		Socket *socket = networkPlayer->GetSocket();
		if( socket != NULL )
		{
			// If we are in game then remove this player from the game as well.
			// We may get here either from the player requesting to exit the game,
			// in which case we they will already have left the game server, or from a disconnection
			// where we then have to remove them from the game server
			if( m_pDQRNet->IsHost() && !m_bHostChanged )
			{
				g_NetworkManager.CloseConnection(networkPlayer);
			}

			// Free the wrapper object memory.
			// TODO 4J Stu - We may still be using this at the point that the player leaves the session.
			// We need this as long as the game server still needs to communicate with the player
			//delete socket;

			networkPlayer->SetSocket( NULL );
		}

		if( m_pDQRNet->IsHost() && !m_bHostChanged )
		{
			if( isSystemPrimaryPlayer(pDQRPlayer) )
			{
				DQRNetworkPlayer *pNewDQRPrimaryPlayer = NULL;
				for(unsigned int i = 0; i < m_pDQRNet->GetPlayerCount(); ++i )
				{
					DQRNetworkPlayer *pDQRPlayer2 = m_pDQRNet->GetPlayerByIndex( i );

					if( pDQRPlayer2 != pDQRPlayer && pDQRPlayer2->IsSameSystem( pDQRPlayer ) )
					{
						pNewDQRPrimaryPlayer = pDQRPlayer2;
						break;
					}
				}
                auto it = find(m_machineDQRPrimaryPlayers.begin(), m_machineDQRPrimaryPlayers.end(), pDQRPlayer);
                if( it != m_machineDQRPrimaryPlayers.end() )
				{
					m_machineDQRPrimaryPlayers.erase( it );
				}

				if( pNewDQRPrimaryPlayer != NULL )
					m_machineDQRPrimaryPlayers.push_back( pNewDQRPrimaryPlayer );
			}

			g_NetworkManager.UpdateAndSetGameSessionData( networkPlayer );
			SystemFlagRemovePlayer( networkPlayer );

		}

		g_NetworkManager.PlayerLeaving( networkPlayer );

		for( int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
		{
			if(playerChangedCallback[idx] != NULL)
				playerChangedCallback[idx]( playerChangedCallbackParam[idx], networkPlayer, true );
		}

		if(m_pDQRNet->GetState() == DQRNetworkManager::DNM_STATE_PLAYING)
		{
			int localPlayerCount = 0;
			for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
			{
				if( m_pDQRNet->GetLocalPlayerByUserIndex(idx) != NULL ) ++localPlayerCount;
			}

			float appTime = app.getAppTime();
			m_lastPlayerEventTimeStart = appTime;
		}

		removeNetworkPlayer(pDQRPlayer);
	}
}

void CPlatformNetworkManagerDurango::HandleDataReceived(DQRNetworkPlayer *playerFrom, DQRNetworkPlayer *playerTo, unsigned char *data, unsigned int dataSize)
{
#if 0
	// TODO - put this back in
	if(m_pSQRNet->GetState() == SQRNetworkManager::SNM_STATE_ENDING)
	{
		return;
	}
#endif

	if( playerTo->IsHost() )
	{
		// If we are the host we care who this came from
		//app.DebugPrintf( "Pushing data into host read queue for user \"%ls\"\n", pPlayerFrom->GetGamertag());
		// Push this data into the read queue for the player that sent it
		INetworkPlayer *pPlayerFrom = getNetworkPlayer(playerFrom);
		Socket *socket = pPlayerFrom->GetSocket();

		if(socket != NULL)
			socket->pushDataToQueue(data, dataSize, false);
	}
	else
	{
		// If we are not the host the message must have come from the host, so we care more about who it is addressed to
		INetworkPlayer *pPlayerTo = getNetworkPlayer(playerTo);
		Socket *socket = pPlayerTo->GetSocket();
		//app.DebugPrintf( "Pushing data into read queue for user \"%ls\"\n", apPlayersTo[dwPlayer]->GetGamertag());
		if(socket != NULL)
			socket->pushDataToQueue(data, dataSize);
	}
}

void CPlatformNetworkManagerDurango::HandleInviteReceived(int playerIndex, DQRNetworkManager::SessionInfo *pInviteInfo)
{
	g_NetworkManager.GameInviteReceived(playerIndex, pInviteInfo);
}

bool CPlatformNetworkManagerDurango::Initialise(CGameNetworkManager *pGameNetworkManager, int flagIndexSize)
{
	m_pDQRNet = new DQRNetworkManager(this);
	m_pDQRNet->Initialise();

	m_hostGameSessionIsJoinable = false;
	m_pGameNetworkManager = pGameNetworkManager;
	m_flagIndexSize = flagIndexSize;
	g_pPlatformNetworkManager = this;
	for( int i = 0; i < XUSER_MAX_COUNT; i++ )
	{
		playerChangedCallback[ i ] = NULL;
	}

	m_bLeavingGame = false;
	m_bLeaveGameOnTick = false;
	m_bHostChanged = false;

	m_bSearchResultsReady = false;
	m_bSearchPending = false;

	m_bIsOfflineGame = false;
	m_pSearchParam = NULL;
	m_SessionsUpdatedCallback = NULL;

	m_searchResultsCount = 0;
	m_lastSearchStartTime = 0;

	// The results that will be filled in with the current search
	m_pSearchResults = NULL;

	Windows::Networking::Connectivity::NetworkInformation::NetworkStatusChanged += ref new Windows::Networking::Connectivity::NetworkStatusChangedEventHandler( []( Platform::Object^ pxObject )
	{
		app.DebugPrintf("NetworkStatusChanged callback\n" );
		auto internetProfile = Windows::Networking::Connectivity::NetworkInformation::GetInternetConnectionProfile();
		if (internetProfile != nullptr)
		{
			auto connectionLevel = internetProfile->GetNetworkConnectivityLevel();
			app.DebugPrintf("Connection level has changed to (%d)\n", connectionLevel);

			//int iPrimaryPlayer = g_NetworkManager.GetPrimaryPad();
			//bool bConnected	= (connectionLevel == Windows::Networking::Connectivity::NetworkConnectivityLevel::XboxLiveAccess)?true:false;
			//if((g_NetworkManager.GetLockedProfile()!=-1) && iPrimaryPlayer!=-1 && bConnected == false && g_NetworkManager.IsInSession() )
			//{
			//	app.SetAction(iPrimaryPlayer,eAppAction_EthernetDisconnected);
			//}
		}
	});

	// Success!
    return true;
}

void CPlatformNetworkManagerDurango::Terminate()
{
}

int CPlatformNetworkManagerDurango::GetJoiningReadyPercentage()
{
	return 100;
}

int CPlatformNetworkManagerDurango::CorrectErrorIDS(int IDS)
{
	return IDS;
}

bool CPlatformNetworkManagerDurango::isSystemPrimaryPlayer(DQRNetworkPlayer *pDQRPlayer)
{
	bool playerIsSystemPrimary = false;
	for(auto it = m_machineDQRPrimaryPlayers.begin(); it < m_machineDQRPrimaryPlayers.end(); ++it)
	{
		DQRNetworkPlayer *pDQRPrimaryPlayer = *it;
		if( pDQRPrimaryPlayer == pDQRPlayer )
		{
			playerIsSystemPrimary = true;
			break;
		}
	}
	return playerIsSystemPrimary;
}

// We call this twice a frame, either side of the render call so is a good place to "tick" things
void CPlatformNetworkManagerDurango::DoWork()
{
	m_pDQRNet->Tick();

	TickSearch();

	if( m_bLeaveGameOnTick )
	{
		m_pDQRNet->LeaveRoom();
		m_bLeaveGameOnTick = false;
	}
}

int CPlatformNetworkManagerDurango::GetPlayerCount()
{
	return m_pDQRNet->GetPlayerCount();
}

bool CPlatformNetworkManagerDurango::ShouldMessageForFullSession()
{
	return m_pDQRNet->ShouldMessageForFullSession();
}

int CPlatformNetworkManagerDurango::GetOnlinePlayerCount()
{
	return m_pDQRNet->GetOnlinePlayerCount();
}

int CPlatformNetworkManagerDurango::GetLocalPlayerMask(int playerIndex)
{
	return 1 << playerIndex;
}

bool CPlatformNetworkManagerDurango::AddLocalPlayerByUserIndex( int userIndex )
{
	return m_pDQRNet->AddLocalPlayerByUserIndex( userIndex );
}

bool CPlatformNetworkManagerDurango::RemoveLocalPlayerByUserIndex( int userIndex )
{
	return m_pDQRNet->RemoveLocalPlayerByUserIndex( userIndex );
}

bool CPlatformNetworkManagerDurango::IsInStatsEnabledSession()
{
	return true;
}

bool CPlatformNetworkManagerDurango::SessionHasSpace(unsigned int spaceRequired /*= 1*/)
{
	return true;
}

void CPlatformNetworkManagerDurango::SendInviteGUI(int quadrant)
{
	m_pDQRNet->SendInviteGUI(quadrant);
}

bool CPlatformNetworkManagerDurango::IsAddingPlayer()
{
	return m_pDQRNet->IsAddingPlayer();
}

bool CPlatformNetworkManagerDurango::LeaveGame(bool bMigrateHost)
{
	if( m_bLeavingGame ) return true;

	m_bLeavingGame = true;

	// If we are the host wait for the game server to end
	if(m_pDQRNet->IsHost() && g_NetworkManager.ServerStoppedValid())
	{
//		m_pDQRNet->EndGame();
		g_NetworkManager.ServerStoppedWait();
		g_NetworkManager.ServerStoppedDestroy();
	}
	return _LeaveGame(bMigrateHost, true);;
}

bool CPlatformNetworkManagerDurango::_LeaveGame(bool bMigrateHost, bool bLeaveRoom)
{
	m_bLeavingGame = true;
	m_bLeaveGameOnTick = true;
	m_migrateHostOnLeave = bMigrateHost;
	return true;
}

void CPlatformNetworkManagerDurango::HostGame(int localUsersMask, bool bOnlineGame, bool bIsPrivate, unsigned char publicSlots /*= MINECRAFT_NET_MAX_PLAYERS*/, unsigned char privateSlots /*= 0*/)
{
// #ifdef _XBOX
	// 4J Stu - We probably did this earlier as well, but just to be sure!
	SetLocalGame( !bOnlineGame );
	SetPrivateGame( bIsPrivate );
	SystemFlagReset();

	// Make sure that the Primary Pad is in by default
	localUsersMask |= GetLocalPlayerMask( g_NetworkManager.GetPrimaryPad() );

	m_bLeavingGame = false;

	_HostGame( localUsersMask, publicSlots, privateSlots );
//#endif
}

void CPlatformNetworkManagerDurango::_HostGame(int usersMask, unsigned char publicSlots /*= MINECRAFT_NET_MAX_PLAYERS*/, unsigned char privateSlots /*= 0*/)
{
	memset(&m_hostGameSessionData,0,sizeof(m_hostGameSessionData));
	m_hostGameSessionData.netVersion = MINECRAFT_NET_VERSION;
	m_hostGameSessionData.isReadyToJoin = false;
	m_hostGameSessionData.m_uiGameHostSettings = app.GetGameHostOption(eGameHostOption_All);
	m_hostGameSessionIsJoinable = !IsPrivateGame();

	m_pDQRNet->CreateAndJoinSession(usersMask, (unsigned char *)&m_hostGameSessionData, sizeof(m_hostGameSessionData), IsLocalGame() );
}

bool CPlatformNetworkManagerDurango::_StartGame()
{
	return true;
}

int CPlatformNetworkManagerDurango::JoinGame(FriendSessionInfo *searchResult, int localUsersMask, int primaryUserIndex)
{
	app.DebugPrintf("Joining game party from search result\n");

	int joinPlayerCount = 0;
	for( int i = 0; i < DQRNetworkManager::MAX_LOCAL_PLAYER_COUNT; i++ )
	{
		if( localUsersMask & ( 1 << i ) )
		{
			// Check if this joining user is already in the session, in which case we don't need to count it
			bool isJoiningUser = false;
			for( int j = 0; j < searchResult->searchResult.m_usedSlotCount; j++ )
			{
				Platform::String^ xuid = ref new Platform::String(searchResult->searchResult.m_sessionXuids[j].c_str());
				if( xuid == ProfileManager.GetUser(i)->XboxUserId )
				{
					app.DebugPrintf("Joining user found to be already in session, so won't be counting to our total\n");
					isJoiningUser = true;
					break;
				}
			}
			if( !isJoiningUser )
			{
				joinPlayerCount++;
			}
		}
	}
	app.DebugPrintf("Used slots: %d, fully playing players: %d, trying to join %d\n", searchResult->searchResult.m_usedSlotCount, searchResult->searchResult.m_playerCount, joinPlayerCount);
	GameSessionData *gameSession = (GameSessionData *)(&searchResult->data);
	if( ( searchResult->searchResult.m_usedSlotCount + joinPlayerCount ) > DQRNetworkManager::MAX_ONLINE_PLAYER_COUNT )
	{
		return CGameNetworkManager::JOINGAME_FAIL_SERVER_FULL;
	}

	if(m_pDQRNet->JoinPartyFromSearchResult(&searchResult->searchResult, localUsersMask))
	{
		app.DebugPrintf("Join success\n");
		return CGameNetworkManager::JOINGAME_SUCCESS;
	}
	else
	{
		app.DebugPrintf("Join fail\n");
		return CGameNetworkManager::JOINGAME_FAIL_GENERAL;
	}
}

void CPlatformNetworkManagerDurango::CancelJoinGame()
{
	m_pDQRNet->CancelJoinPartyFromSearchResult();
}

bool CPlatformNetworkManagerDurango::SetLocalGame(bool isLocal)
{
	m_bIsOfflineGame = isLocal;

	return true;
}

void CPlatformNetworkManagerDurango::SetPrivateGame(bool isPrivate)
{
	app.DebugPrintf("Setting as private game: %s\n", isPrivate ? "yes" : "no" );
	m_bIsPrivateGame = isPrivate;
}

void CPlatformNetworkManagerDurango::RegisterPlayerChangedCallback(int iPad, void (*callback)(void *callbackParam, INetworkPlayer *pPlayer, bool leaving), void *callbackParam)
{
	playerChangedCallback[iPad] = callback;
	playerChangedCallbackParam[iPad] = callbackParam;
}

void CPlatformNetworkManagerDurango::UnRegisterPlayerChangedCallback(int iPad, void (*callback)(void *callbackParam, INetworkPlayer *pPlayer, bool leaving), void *callbackParam)
{
	if(playerChangedCallbackParam[iPad] == callbackParam)
	{
		playerChangedCallback[iPad] = NULL;
		playerChangedCallbackParam[iPad] = NULL;
	}
}

void CPlatformNetworkManagerDurango::HandleSignInChange()
{
	return;
}

void CPlatformNetworkManagerDurango::HandleAddLocalPlayerFailed(int idx, bool serverFull)
{
	g_NetworkManager.AddLocalPlayerFailed(idx, serverFull);
}

void CPlatformNetworkManagerDurango::HandleDisconnect(bool bLostRoomOnly)
{
	g_NetworkManager.HandleDisconnect(bLostRoomOnly);
}

bool CPlatformNetworkManagerDurango::_RunNetworkGame()
{
	m_pDQRNet->StartGame();
	m_hostGameSessionData.isReadyToJoin = true;
	m_pDQRNet->UpdateCustomSessionData();
	return true;
}

void CPlatformNetworkManagerDurango::UpdateAndSetGameSessionData(INetworkPlayer *pNetworkPlayerLeaving /*= NULL*/)
{
 	if( this->m_bLeavingGame )
 		return;

 	if( GetHostPlayer() == NULL )
 		return;

 	m_hostGameSessionData.m_uiGameHostSettings = app.GetGameHostOption(eGameHostOption_All);

	m_pDQRNet->UpdateCustomSessionData();
}

int CPlatformNetworkManagerDurango::RemovePlayerOnSocketClosedThreadProc( void* lpParam )
{
	INetworkPlayer *pNetworkPlayer = (INetworkPlayer *)lpParam;

	Socket *socket = pNetworkPlayer->GetSocket();

	if( socket != NULL )
	{
		//printf("Waiting for socket closed event\n");
		socket->m_socketClosedEvent->WaitForSignal(INFINITE);

		//printf("Socket closed event has fired\n");
		// 4J Stu - Clear our reference to this socket
		pNetworkPlayer->SetSocket( NULL );
		delete socket;
	}

	return g_pPlatformNetworkManager->RemoveLocalPlayer( pNetworkPlayer );
}

bool CPlatformNetworkManagerDurango::RemoveLocalPlayer( INetworkPlayer *pNetworkPlayer )
{
	return true;
}

CPlatformNetworkManagerDurango::PlayerFlags::PlayerFlags(INetworkPlayer *pNetworkPlayer, unsigned int count)
{
	// 4J Stu - Don't assert, just make it a multiple of 8! This count is calculated from a load of separate values,
	// and makes tweaking world/render sizes a pain if we hit an assert here
	count = (count + 8 - 1) & ~(8 - 1);
	//assert( ( count % 8 ) == 0 );
	this->m_pNetworkPlayer = pNetworkPlayer;
	this->flags = new unsigned char [ count / 8 ];
	memset( this->flags, 0, count / 8 );
	this->count = count;
}

CPlatformNetworkManagerDurango::PlayerFlags::~PlayerFlags()
{
	delete [] flags;
}

// Add a player to the per system flag storage - if we've already got a player from that system, copy its flags over
void CPlatformNetworkManagerDurango::SystemFlagAddPlayer(INetworkPlayer *pNetworkPlayer)
{
	PlayerFlags *newPlayerFlags = new PlayerFlags( pNetworkPlayer,  m_flagIndexSize);
	// If any of our existing players are on the same system, then copy over flags from that one
	for( unsigned int i = 0; i < m_playerFlags.size(); i++ )
	{
		if( pNetworkPlayer->IsSameSystem(m_playerFlags[i]->m_pNetworkPlayer) )
		{
			memcpy( newPlayerFlags->flags, m_playerFlags[i]->flags, m_playerFlags[i]->count / 8 );
			break;
		}
	}
	m_playerFlags.push_back(newPlayerFlags);
}

// Remove a player from the per system flag storage - just maintains the m_playerFlags vector without any gaps in it
void CPlatformNetworkManagerDurango::SystemFlagRemovePlayer(INetworkPlayer *pNetworkPlayer)
{
	for( unsigned int i = 0; i < m_playerFlags.size(); i++ )
	{
		if( m_playerFlags[i]->m_pNetworkPlayer == pNetworkPlayer )
		{
			delete m_playerFlags[i];
			m_playerFlags[i] = m_playerFlags.back();
			m_playerFlags.pop_back();
			return;
		}
	}
}

void CPlatformNetworkManagerDurango::SystemFlagReset()
{
	for( unsigned int i = 0; i < m_playerFlags.size(); i++ )
	{
		delete m_playerFlags[i];
	}
	m_playerFlags.clear();
}

// Set a per system flag - this is done by setting the flag on every player that shares that system
void CPlatformNetworkManagerDurango::SystemFlagSet(INetworkPlayer *pNetworkPlayer, int index)
{
	if( ( index < 0 ) || ( index >= m_flagIndexSize ) ) return;
	if( pNetworkPlayer == NULL ) return;

	for( unsigned int i = 0; i < m_playerFlags.size(); i++ )
	{
		if( pNetworkPlayer->IsSameSystem(m_playerFlags[i]->m_pNetworkPlayer) )
		{
			m_playerFlags[i]->flags[ index / 8 ] |= ( 128 >> ( index % 8 ) );
		}
	}
}

// Get value of a per system flag - can be read from the flags of the passed in player as anything else sent to that
// system should also have been duplicated here
bool CPlatformNetworkManagerDurango::SystemFlagGet(INetworkPlayer *pNetworkPlayer, int index)
{
	if( ( index < 0 ) || ( index >= m_flagIndexSize ) ) return false;
	if( pNetworkPlayer == NULL )
	{
		return false;
	}

	for( unsigned int i = 0; i < m_playerFlags.size(); i++ )
	{
		if( m_playerFlags[i]->m_pNetworkPlayer == pNetworkPlayer )
		{
			return ( ( m_playerFlags[i]->flags[ index / 8 ] & ( 128 >> ( index % 8 ) ) ) != 0 );
		}
	}
	return false;
}

wstring CPlatformNetworkManagerDurango::GatherStats()
{
	return L"";
}

wstring CPlatformNetworkManagerDurango::GatherRTTStats()
{
	return L"";
}

void CPlatformNetworkManagerDurango::TickSearch()
{
	if( m_bSearchPending )
	{
		if( !m_pDQRNet->FriendPartyManagerIsBusy() )
		{
			m_searchResultsCount = m_pDQRNet->FriendPartyManagerGetCount();
			delete [] m_pSearchResults;
			m_pSearchResults = new DQRNetworkManager::SessionSearchResult[m_searchResultsCount];

			for( int i = 0; i < m_searchResultsCount; i++ )
			{
				m_pDQRNet->FriendPartyManagerGetSessionInfo(i, &m_pSearchResults[i] );
			}
			m_bSearchPending = false;

			if( m_SessionsUpdatedCallback != NULL ) m_SessionsUpdatedCallback(m_pSearchParam);
		}
	}
	else
	{
		if( !m_pDQRNet->FriendPartyManagerIsBusy() )
		{
			// Don't start searches unless we have registered a callback
			if( m_SessionsUpdatedCallback != NULL && (m_lastSearchStartTime + MINECRAFT_DURANGO_PARTY_SEARCH_DELAY_MILLISECONDS) < GetTickCount() )
			{
				if( m_pDQRNet->FriendPartyManagerSearch() );
				{
					m_bSearchPending = true;
					m_lastSearchStartTime = GetTickCount();
				}
			}
		}
	}
}

vector<FriendSessionInfo *> *CPlatformNetworkManagerDurango::GetSessionList(int iPad, int localPlayers, bool partyOnly)
{
	vector<FriendSessionInfo *> *filteredList = new vector<FriendSessionInfo *>();
	for( int i = 0; i < m_searchResultsCount; i++ )
	{
		GameSessionData *gameSessionData = (GameSessionData *)m_pSearchResults[i].m_extData;
		if( ( gameSessionData->netVersion == MINECRAFT_NET_VERSION ) &&
			( gameSessionData->isReadyToJoin ) )
		{
			FriendSessionInfo *session = new FriendSessionInfo();
			session->searchResult = m_pSearchResults[i];
			session->searchResult.m_extData = NULL;			// We have another copy of the GameSessionData, so don't need to make another copy of this here
			session->displayLabelLength = session->searchResult.m_playerNames[0].size();
			session->displayLabel = new wchar_t[ session->displayLabelLength + 1 ];
			memcpy(&session->data, gameSessionData, sizeof(GameSessionData));
			wcscpy_s(session->displayLabel, session->displayLabelLength + 1, session->searchResult.m_playerNames[0].c_str() );
			filteredList->push_back(session);
		}
	}
	return filteredList;
}

bool CPlatformNetworkManagerDurango::GetGameSessionInfo(int iPad, SessionID sessionId, FriendSessionInfo *foundSessionInfo)
{
	return false;
}

void CPlatformNetworkManagerDurango::SetSessionsUpdatedCallback( void (*SessionsUpdatedCallback)(LPVOID pParam), LPVOID pSearchParam )
{
	m_SessionsUpdatedCallback = SessionsUpdatedCallback; m_pSearchParam = pSearchParam;
}

void CPlatformNetworkManagerDurango::GetFullFriendSessionInfo( FriendSessionInfo *foundSession, void (* FriendSessionUpdatedFn)(bool success, void *pParam), void *pParam )
{
	FriendSessionUpdatedFn(true, pParam);
}

void CPlatformNetworkManagerDurango::ForceFriendsSessionRefresh()
{
	app.DebugPrintf("Resetting friends session search data\n");
	m_lastSearchStartTime = 0;
	m_searchResultsCount = 0;
	delete [] m_pSearchResults;
	m_pSearchResults = NULL;
}

INetworkPlayer *CPlatformNetworkManagerDurango::addNetworkPlayer(DQRNetworkPlayer *pDQRPlayer)
{
	NetworkPlayerDurango *pNetworkPlayer = new NetworkPlayerDurango(pDQRPlayer);
	pDQRPlayer->SetCustomDataValue((ULONG_PTR)pNetworkPlayer);
	currentNetworkPlayers.push_back( pNetworkPlayer );
	return pNetworkPlayer;
}

void CPlatformNetworkManagerDurango::removeNetworkPlayer(DQRNetworkPlayer *pDQRPlayer)
{
	INetworkPlayer *pNetworkPlayer = getNetworkPlayer(pDQRPlayer);
    for (auto it = currentNetworkPlayers.begin(); it != currentNetworkPlayers.end(); ++it)
    {
		if( *it == pNetworkPlayer )
		{
			currentNetworkPlayers.erase(it);
			return;
		}
	}
}

INetworkPlayer *CPlatformNetworkManagerDurango::getNetworkPlayer(DQRNetworkPlayer *pDQRPlayer)
{
	return pDQRPlayer ? (INetworkPlayer *)(pDQRPlayer->GetCustomDataValue()) : NULL;
}


INetworkPlayer *CPlatformNetworkManagerDurango::GetLocalPlayerByUserIndex(int userIndex )
{
	return getNetworkPlayer(m_pDQRNet->GetLocalPlayerByUserIndex(userIndex));
}

INetworkPlayer *CPlatformNetworkManagerDurango::GetPlayerByIndex(int playerIndex)
{
	return getNetworkPlayer(m_pDQRNet->GetPlayerByIndex(playerIndex));
}

INetworkPlayer * CPlatformNetworkManagerDurango::GetPlayerByXuid(PlayerUID xuid)
{
	return getNetworkPlayer(m_pDQRNet->GetPlayerByXuid(xuid)) ;
}

INetworkPlayer * CPlatformNetworkManagerDurango::GetPlayerBySmallId(unsigned char smallId)
{
	return getNetworkPlayer(m_pDQRNet->GetPlayerBySmallId(smallId));
}

wstring CPlatformNetworkManagerDurango::GetDisplayNameByGamertag(wstring gamertag)
{
	return m_pDQRNet->GetDisplayNameByGamertag(gamertag);
}

INetworkPlayer *CPlatformNetworkManagerDurango::GetHostPlayer()
{
	return getNetworkPlayer(m_pDQRNet->GetHostPlayer());
}

bool CPlatformNetworkManagerDurango::IsHost()
{
	return m_pDQRNet->IsHost() && !m_bHostChanged;
}

bool CPlatformNetworkManagerDurango::JoinGameFromInviteInfo( int userIndex, int userMask, const INVITE_INFO *pInviteInfo)
{
	m_pDQRNet->m_currentUserMask = userMask;
	m_pDQRNet->JoinSessionFromInviteInfo(userMask);
	return true;
}

void CPlatformNetworkManagerDurango::SetSessionTexturePackParentId( int id )
{
	m_hostGameSessionData.texturePackParentId = id;
}

void CPlatformNetworkManagerDurango::SetSessionSubTexturePackId( int id )
{
	m_hostGameSessionData.subTexturePackId = id;
}

void CPlatformNetworkManagerDurango::Notify(int ID, ULONG_PTR Param)
{
}

bool CPlatformNetworkManagerDurango::IsInSession()
{
	return m_pDQRNet->IsInSession();
}

bool CPlatformNetworkManagerDurango::IsInGameplay()
{
	return m_pDQRNet->GetState() == DQRNetworkManager::DNM_STATE_PLAYING;
}

bool CPlatformNetworkManagerDurango::IsReadyToPlayOrIdle()
{
	return m_pDQRNet->IsReadyToPlayOrIdle();
}

bool CPlatformNetworkManagerDurango::IsSessionJoinable()
{
	return m_hostGameSessionIsJoinable;
}