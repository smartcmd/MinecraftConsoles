#include "stdafx.h"
#include "../../../Minecraft.World/Socket.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "PlatformNetworkManagerXbox.h"
#include "NetworkPlayerXbox.h"
#include "../../Common/Network/GameNetworkManager.h"

CPlatformNetworkManagerXbox *g_pPlatformNetworkManager;

VOID CPlatformNetworkManagerXbox::NotifyStateChanged(
	__in QNET_STATE                 OldState,
	__in QNET_STATE                 NewState,
	__in HRESULT                    hrInfo
	)
{
	static const char * c_apszStateNames[] =
    {
        "QNET_STATE_IDLE",
        "QNET_STATE_SESSION_HOSTING",
        "QNET_STATE_SESSION_JOINING",
        "QNET_STATE_GAME_LOBBY",
        "QNET_STATE_SESSION_REGISTERING",
        "QNET_STATE_SESSION_STARTING",
        "QNET_STATE_GAME_PLAY",
        "QNET_STATE_SESSION_ENDING",
        "QNET_STATE_SESSION_LEAVING",
        "QNET_STATE_SESSION_DELETING",
    };

    app.DebugPrintf( "State: %s ==> %s, result 0x%08x.\n",
        c_apszStateNames[ OldState ],
        c_apszStateNames[ NewState ],
        hrInfo );
	if( NewState == QNET_STATE_SESSION_HOSTING )
	{
		m_bLeavingGame = false;
		m_bLeaveGameOnTick = false;
		m_bHostChanged = false;
		g_NetworkManager.StateChange_AnyToHosting();
	}
	else if( NewState == QNET_STATE_SESSION_JOINING )
	{
		m_bLeavingGame = false;
		m_bLeaveGameOnTick = false;
		m_bHostChanged = false;
		g_NetworkManager.StateChange_AnyToJoining();
	}
	else if( NewState == QNET_STATE_IDLE && OldState == QNET_STATE_SESSION_JOINING )
	{
		// 4J-PB - now and then we get ERROR_DEVICE_REMOVED when qnet says
		// " Couldn't join, removed from session!" or
		//[qnet]: Received data change notification from partially connected player!
		//[qnet]: Couldn't join, removed from session!
		// instead of a QNET_E_SESSION_FULL as should be reported

		eJoinFailedReason reason;
		switch( hrInfo )
		{
		case QNET_E_INSUFFICIENT_PRIVILEGES:
			reason = JOIN_FAILED_INSUFFICIENT_PRIVILEGES;
			break;
		case QNET_E_SESSION_FULL:
			reason = JOIN_FAILED_SERVER_FULL;
			break;
		default:
			reason = JOIN_FAILED_NONSPECIFIC;
			break;
		}
		g_NetworkManager.StateChange_JoiningToIdle(reason);
	}
	else if( NewState == QNET_STATE_IDLE && OldState == QNET_STATE_SESSION_HOSTING )
	{
		m_bLeavingGame = true;
	}
	else if( NewState == QNET_STATE_SESSION_STARTING )
	{
		m_lastPlayerEventTimeStart = app.getAppTime();

		g_NetworkManager.StateChange_AnyToStarting();
	}
	// Fix for #93148 - TCR 001: BAS Game Stability: Title will crash for the multiplayer client if host of the game will exit during the clients loading to created world.
	// 4J Stu - If the client joins just as the host is exiting, then they can skip to leaving without passing through ending
	else if( NewState == QNET_STATE_SESSION_ENDING || (NewState == QNET_STATE_SESSION_LEAVING && OldState == QNET_STATE_GAME_PLAY) )
	{
		g_NetworkManager.StateChange_AnyToEnding( OldState == QNET_STATE_GAME_PLAY );

		if( m_pIQNet->IsHost() )
		{
			m_bLeavingGame = true;
		}
	}

	if( NewState == QNET_STATE_IDLE )
	{
		g_NetworkManager.StateChange_AnyToIdle();
	}
}


VOID CPlatformNetworkManagerXbox::NotifyPlayerJoined(
	__in IQNetPlayer *              pQNetPlayer
	)
{
	const char * pszDescription;

	// 4J Stu - We create a fake socket for every where that we need an INBOUND queue of game data. Outbound
	// is all handled by QNet so we don't need that. Therefore each client player has one, and the host has one
	// for each client player.
	bool createFakeSocket = false;
	bool localPlayer = false;

	NetworkPlayerXbox *networkPlayer = static_cast<NetworkPlayerXbox *>(addNetworkPlayer(pQNetPlayer));

    if( pQNetPlayer->IsLocal() )
    {
		localPlayer = true;
        if( pQNetPlayer->IsHost() )
        {
            pszDescription = "local host";
			// 4J Stu - No socket for the localhost as it uses a special loopback queue

			m_machineQNetPrimaryPlayers.push_back( pQNetPlayer );
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
        if( pQNetPlayer->IsHost() )
        {
            pszDescription = "remote host";
        }
        else
        {
            pszDescription = "remote";

			// If we are the host, then create a fake socket for every remote player
			if( m_pIQNet->IsHost() )
			{
				createFakeSocket = true;
			}
        }

		if( m_pIQNet->IsHost() && !m_bHostChanged )
		{
			// Do we already have a primary player for this system?
			bool systemHasPrimaryPlayer = false;
            for (auto it = m_machineQNetPrimaryPlayers.begin(); it < m_machineQNetPrimaryPlayers.end(); ++it)
            {
				IQNetPlayer *pQNetPrimaryPlayer = *it;
				if( pQNetPlayer->IsSameSystem(pQNetPrimaryPlayer) )
				{
					systemHasPrimaryPlayer = true;
					break;
				}
			}
			if( !systemHasPrimaryPlayer )
				m_machineQNetPrimaryPlayers.push_back( pQNetPlayer );
		}
    }
	g_NetworkManager.PlayerJoining( networkPlayer );

	if( createFakeSocket == true && !m_bHostChanged )
	{
		g_NetworkManager.CreateSocket( networkPlayer, localPlayer );
	}

    app.DebugPrintf( "Player 0x%p \"%ls\" joined; %s; voice %i; camera %i.\n",
        pQNetPlayer,
        pQNetPlayer->GetGamertag(),
        pszDescription,
        (int) pQNetPlayer->HasVoice(),
        (int) pQNetPlayer->HasCamera() );


	if( m_pIQNet->IsHost() )
	{
		// 4J-PB - only the host should do this
		g_NetworkManager.UpdateAndSetGameSessionData();
		SystemFlagAddPlayer( networkPlayer );
	}

	for( int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
	{
		if(playerChangedCallback[idx] != nullptr)
			playerChangedCallback[idx]( playerChangedCallbackParam[idx], networkPlayer, false );
	}

	if(m_pIQNet->GetState() == QNET_STATE_GAME_PLAY)
	{
		int localPlayerCount = 0;
		for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
		{
			if( m_pIQNet->GetLocalPlayerByUserIndex(idx) != nullptr ) ++localPlayerCount;
		}

		float appTime = app.getAppTime();

		// Only record stats for the primary player here
		m_lastPlayerEventTimeStart = appTime;
	}
}


VOID CPlatformNetworkManagerXbox::NotifyPlayerLeaving(
	__in IQNetPlayer *              pQNetPlayer
	)
{
	//__debugbreak();

	app.DebugPrintf( "Player 0x%p \"%ls\" leaving.\n",
        pQNetPlayer,
        pQNetPlayer->GetGamertag() );

	INetworkPlayer *networkPlayer = getNetworkPlayer(pQNetPlayer);

	// Get our wrapper object associated with this player.
    Socket *socket = networkPlayer->GetSocket();
    if( socket != nullptr )
    {
		// If we are in game then remove this player from the game as well.
		// We may get here either from the player requesting to exit the game,
		// in which case we they will already have left the game server, or from a disconnection
		// where we then have to remove them from the game server
		if( m_pIQNet->IsHost() && !m_bHostChanged )
		{
			g_NetworkManager.CloseConnection(networkPlayer);
		}

        // Free the wrapper object memory.
		// TODO 4J Stu - We may still be using this at the point that the player leaves the session.
		// We need this as long as the game server still needs to communicate with the player
        //delete socket;

        networkPlayer->SetSocket( nullptr );
    }

	if( m_pIQNet->IsHost() && !m_bHostChanged )
	{
		if( isSystemPrimaryPlayer(pQNetPlayer) )
		{
			IQNetPlayer *pNewQNetPrimaryPlayer = nullptr;
			for(unsigned int i = 0; i < m_pIQNet->GetPlayerCount(); ++i )
			{
				IQNetPlayer *pQNetPlayer2 = m_pIQNet->GetPlayerByIndex( i );

				if( pQNetPlayer2 != pQNetPlayer && pQNetPlayer2->IsSameSystem( pQNetPlayer ) )
				{
					pNewQNetPrimaryPlayer = pQNetPlayer2;
					break;
				}
			}
            auto it = find(m_machineQNetPrimaryPlayers.begin(), m_machineQNetPrimaryPlayers.end(), pQNetPlayer);
            if( it != m_machineQNetPrimaryPlayers.end() )
			{
				m_machineQNetPrimaryPlayers.erase( it );
			}

			if( pNewQNetPrimaryPlayer != nullptr )
				m_machineQNetPrimaryPlayers.push_back( pNewQNetPrimaryPlayer );
		}

		g_NetworkManager.UpdateAndSetGameSessionData( networkPlayer );
		SystemFlagRemovePlayer( networkPlayer );

	}

	g_NetworkManager.PlayerLeaving( networkPlayer );

	for( int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
	{
		if(playerChangedCallback[idx] != nullptr)
			playerChangedCallback[idx]( playerChangedCallbackParam[idx], networkPlayer, true );
	}

	if(m_pIQNet->GetState() == QNET_STATE_GAME_PLAY)
	{
		int localPlayerCount = 0;
		for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
		{
			if( m_pIQNet->GetLocalPlayerByUserIndex(idx) != nullptr ) ++localPlayerCount;
		}

		float appTime = app.getAppTime();
		m_lastPlayerEventTimeStart = appTime;
	}

	removeNetworkPlayer(pQNetPlayer);
}


VOID CPlatformNetworkManagerXbox::NotifyNewHost(
	__in IQNetPlayer *              pQNetPlayer
	)
{
	app.DebugPrintf( "Player 0x%p \"%ls\" (local %i) is new host.\n",
        pQNetPlayer,
        pQNetPlayer->GetGamertag(),
        (int) pQNetPlayer->IsLocal() );

	m_bHostChanged = true;

	if( m_pIQNet->IsHost() && !IsLeavingGame() )
    {
		m_pGameNetworkManager->HostChanged();
	}
}


VOID CPlatformNetworkManagerXbox::NotifyDataReceived(
	__in IQNetPlayer *              pQNetPlayerFrom,
	__in DWORD                      dwNumPlayersTo,
	__in_ecount(dwNumPlayersTo) IQNetPlayer ** apQNetPlayersTo,
	__in_bcount(dwDataSize) const BYTE * pbData,
	__in DWORD                      dwDataSize
	)
{
	if(m_pIQNet->GetState() == QNET_STATE_SESSION_ENDING)
		return;

    DWORD dwPlayer;

	// Loop through all the local players that were targeted and print info
    // regarding this message.
	/*
    for( dwPlayer = 0; dwPlayer < dwNumPlayersTo; dwPlayer++ )
    {
        app.DebugPrintf( "Received %u bytes of data from \"%ls\" to \"%ls\"\n",
            dwDataSize,
            pPlayerFrom->GetGamertag(),
            apPlayersTo[ dwPlayer ]->GetGamertag());

	}
	*/

	// Loop through all the players and push this data into their read queue
	for( dwPlayer = 0; dwPlayer < dwNumPlayersTo; dwPlayer++ )
	{
		if( apQNetPlayersTo[dwPlayer]->IsHost() )
		{
			// If we are the host we care who this came from
			//app.DebugPrintf( "Pushing data into host read queue for user \"%ls\"\n", pPlayerFrom->GetGamertag());
			// Push this data into the read queue for the player that sent it
			INetworkPlayer *pPlayerFrom = getNetworkPlayer(pQNetPlayerFrom);
			Socket *socket = pPlayerFrom->GetSocket();

			if(socket != nullptr)
				socket->pushDataToQueue(pbData, dwDataSize, false);
		}
		else
		{
			// If we are not the host the message must have come from the host, so we care more about who it is addressed to
			INetworkPlayer *pPlayerTo = getNetworkPlayer(apQNetPlayersTo[dwPlayer]);
			Socket *socket = pPlayerTo->GetSocket();
			//app.DebugPrintf( "Pushing data into read queue for user \"%ls\"\n", apPlayersTo[dwPlayer]->GetGamertag());
			if(socket != nullptr)
				socket->pushDataToQueue(pbData, dwDataSize);
		}
	}
}


VOID CPlatformNetworkManagerXbox::NotifyWriteStats(
	__in IQNetPlayer *              pQNetPlayer
	)
{
	app.DebugPrintf( "QNet: NotifyWriteStats\n" );

	g_NetworkManager.WriteStats( getNetworkPlayer( pQNetPlayer ) );
}


VOID CPlatformNetworkManagerXbox::NotifyReadinessChanged(
	__in IQNetPlayer *              pQNetPlayer,
	__in BOOL                       bReady
	)
{
	app.DebugPrintf( "Player 0x%p readiness is now %i.\n", pQNetPlayer, static_cast<int>(bReady) );
}


VOID CPlatformNetworkManagerXbox::NotifyCommSettingsChanged(
	__in IQNetPlayer *              pQNetPlayer
	)
{
}


VOID CPlatformNetworkManagerXbox::NotifyGameSearchComplete(
	__in IQNetGameSearch *          pGameSearch,
	__in HRESULT                    hrComplete,
	__in DWORD                      dwNumResults
	)
{
	// Not currently used
}


VOID CPlatformNetworkManagerXbox::NotifyGameInvite(
	__in DWORD                      dwUserIndex,
	__in const INVITE_INFO *       pInviteInfo
	)
{
	g_NetworkManager.GameInviteReceived( dwUserIndex, pInviteInfo );
}


VOID CPlatformNetworkManagerXbox::NotifyContextChanged(
	__in const XUSER_CONTEXT *      pContext
	)
{
	app.DebugPrintf( "Context 0x%p changed.\n", pContext );
}


VOID CPlatformNetworkManagerXbox::NotifyPropertyChanged(
	__in const XUSER_PROPERTY *     pProperty
	)
{
	app.DebugPrintf( "Property 0x%p changed.\n", pProperty );
}

bool CPlatformNetworkManagerXbox::Initialise(CGameNetworkManager *pGameNetworkManager, int flagIndexSize)
{
	m_pGameNetworkManager = pGameNetworkManager;
	m_flagIndexSize = flagIndexSize;
	g_pPlatformNetworkManager = this;
	for( int i = 0; i < XUSER_MAX_COUNT; i++ )
	{
		playerChangedCallback[ i ] = nullptr;
	}

	HRESULT hr;
	int iResult;
	DWORD dwResult;

    // Start up XNet with default settings.
    iResult = XNetStartup( nullptr );
    if( iResult != 0 )
    {
        app.DebugPrintf( "Starting up XNet failed (err = %i)!\n", iResult );
        return false;
    }

    // Start up XOnline.
    dwResult = XOnlineStartup();
    if( dwResult != ERROR_SUCCESS )
    {
        app.DebugPrintf( "Starting up XOnline failed (err = %u)!\n", dwResult );
        XNetCleanup();
        return false;
    }

    // Create the QNet object.
    hr = QNetCreateUsingXAudio2( QNET_SESSIONTYPE_LIVE_STANDARD, this, nullptr, g_pXAudio2, &m_pIQNet );
    if( FAILED( hr ) )
    {
        app.DebugPrintf( "Creating QNet object failed (err = 0x%08x)!\n", hr );
        XOnlineCleanup();
        XNetCleanup();
        return false;
    }

	BOOL enableNotify = FALSE;
	m_pIQNet->SetOpt( QNET_OPTION_NOTIFY_LISTENER, &enableNotify, sizeof BOOL );

	BOOL enableJip = FALSE;
	m_pIQNet->SetOpt( QNET_OPTION_JOIN_IN_PROGRESS_ALLOWED, &enableJip, sizeof BOOL );
	BOOL enableInv = FALSE;
	m_pIQNet->SetOpt( QNET_OPTION_INVITES_ALLOWED, &enableInv, sizeof BOOL );
	BOOL enablePres = FALSE;
	m_pIQNet->SetOpt( QNET_OPTION_PRESENCE_JOIN_MODE, &enablePres, sizeof BOOL );

	// We DO NOT want QNet to handle XN_SYS_SIGNINCHANGED but so far everything else should be fine
	// We DO WANT QNet to handle XN_LIVE_INVITE_ACCEPTED at a minimum
	// Receive all types that QNet needs, and filter out the specific ones we don't want later
	m_notificationListener = XNotifyCreateListener(XNOTIFY_SYSTEM | XNOTIFY_FRIENDS | XNOTIFY_LIVE);

	m_bLeavingGame = false;
	m_bLeaveGameOnTick = false;
	m_bHostChanged = false;

	m_bSearchResultsReady = false;
	m_bSearchPending = false;

	m_bIsOfflineGame = false;
	m_pSearchParam = nullptr;
	m_SessionsUpdatedCallback = nullptr;

	for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		m_searchResultsCount[i] = 0;
		m_lastSearchStartTime[i] = 0;

		// The results that will be filled in with the current search
		m_pSearchResults[i] = nullptr;
		m_pQoSResult[i] = nullptr;
		m_pCurrentSearchResults[i] = nullptr;
		m_pCurrentQoSResult[i] = nullptr;
		m_currentSearchResultsCount[i] = 0;
	}

    // Success!
    return true;
}

void CPlatformNetworkManagerXbox::Terminate()
{
}

int CPlatformNetworkManagerXbox::GetJoiningReadyPercentage()
{
	return 100;
}

int CPlatformNetworkManagerXbox::CorrectErrorIDS(int IDS)
{
	return IDS;
}

bool CPlatformNetworkManagerXbox::isSystemPrimaryPlayer(IQNetPlayer *pQNetPlayer)
{
	bool playerIsSystemPrimary = false;
    for (auto it = m_machineQNetPrimaryPlayers.begin(); it < m_machineQNetPrimaryPlayers.end(); ++it)
    {
		IQNetPlayer *pQNetPrimaryPlayer = *it;
		if( pQNetPrimaryPlayer == pQNetPlayer )
		{
			playerIsSystemPrimary = true;
			break;
		}
	}
	return playerIsSystemPrimary;
}

// We call this twice a frame, either side of the render call so is a good place to "tick" things
void CPlatformNetworkManagerXbox::DoWork()
{
	DWORD dwNotifyId;
	ULONG_PTR ulpNotifyParam;

	while( XNotifyGetNext(
         m_notificationListener,
         0,							// Any notification
         &dwNotifyId,
         &ulpNotifyParam)
		)
	{

		switch(dwNotifyId)
		{

		case XN_SYS_SIGNINCHANGED:
			app.DebugPrintf("Signinchanged - %d\n", ulpNotifyParam);
			break;
		case XN_LIVE_INVITE_ACCEPTED:
			// ignore these - we're catching them from the game listener, so we can get the one from the dashboard
			break;
	default:
			m_pIQNet->Notify(dwNotifyId,ulpNotifyParam);
			break;
		}

	}

	TickSearch();

	if( m_bLeaveGameOnTick )
	{
		m_pIQNet->LeaveGame(m_migrateHostOnLeave);
		m_bLeaveGameOnTick = false;
	}

	m_pIQNet->DoWork();
}

int CPlatformNetworkManagerXbox::GetPlayerCount()
{
	return m_pIQNet->GetPlayerCount();
}

bool CPlatformNetworkManagerXbox::ShouldMessageForFullSession()
{
	return false;
}

int CPlatformNetworkManagerXbox::GetOnlinePlayerCount()
{
	DWORD playerCount = GetPlayerCount();
	DWORD onlinePlayerCount = 0;

	for(DWORD i = 0; i < playerCount; ++i)
	{
		IQNetPlayer *pQNetPlayer = m_pIQNet->GetPlayerByIndex(i);
		if(!pQNetPlayer->IsLocal())++onlinePlayerCount;
	}
	return onlinePlayerCount;
}

int CPlatformNetworkManagerXbox::GetLocalPlayerMask(int playerIndex)
{
	switch(playerIndex)
	{
		case 0:
			return QNET_USER_MASK_USER0;
		case 1:
			return QNET_USER_MASK_USER1;
		case 2:
			return QNET_USER_MASK_USER2;
		case 3:
			return QNET_USER_MASK_USER3;
		default:
			return 0;
	}
}

bool CPlatformNetworkManagerXbox::AddLocalPlayerByUserIndex( int userIndex )
{
	return ( m_pIQNet->AddLocalPlayerByUserIndex(userIndex) == S_OK );
}

bool CPlatformNetworkManagerXbox::RemoveLocalPlayerByUserIndex( int userIndex )
{
	IQNetPlayer *pQNetPlayer = m_pIQNet->GetLocalPlayerByUserIndex(userIndex);
	INetworkPlayer *pNetworkPlayer = getNetworkPlayer(pQNetPlayer);

	if(pNetworkPlayer != nullptr)
	{
		Socket *socket = pNetworkPlayer->GetSocket();

		if( socket != nullptr )
		{
			// We can't remove the player from qnet until we have stopped using it to communicate
			C4JThread* thread = new C4JThread(&CPlatformNetworkManagerXbox::RemovePlayerOnSocketClosedThreadProc, pNetworkPlayer, "RemovePlayerOnSocketClosed");
			thread->SetProcessor( CPU_CORE_REMOVE_PLAYER );
			thread->Run();
		}
		else
		{
			// Safe to remove the player straight away
			return ( m_pIQNet->RemoveLocalPlayerByUserIndex(userIndex) == S_OK );
		}
	}
	return true;
}

bool CPlatformNetworkManagerXbox::IsInStatsEnabledSession()
{

	DWORD dataSize = sizeof(QNET_LIVE_STATS_MODE);
	QNET_LIVE_STATS_MODE statsMode;
	m_pIQNet->GetOpt(QNET_OPTION_LIVE_STATS_MODE, &statsMode , &dataSize );

	// Use QNET_LIVE_STATS_MODE_AUTO if there is another way to check if stats are enabled or not
	bool statsEnabled = statsMode == QNET_LIVE_STATS_MODE_ENABLED;
	return m_pIQNet->GetState() != QNET_STATE_IDLE && statsEnabled;
}

bool CPlatformNetworkManagerXbox::SessionHasSpace(unsigned int spaceRequired /*= 1*/)
{
	// This function is used while a session is running, so all players trying to join
	// should use public slots,
	DWORD publicSlots = 0;
	DWORD filledPublicSlots = 0;
	DWORD privateSlots = 0;
	DWORD filledPrivateSlots = 0;

	DWORD dataSize = sizeof(DWORD);
	m_pIQNet->GetOpt(QNET_OPTION_TOTAL_PUBLIC_SLOTS, &publicSlots, &dataSize );
	m_pIQNet->GetOpt(QNET_OPTION_FILLED_PUBLIC_SLOTS, &filledPublicSlots, &dataSize );
	m_pIQNet->GetOpt(QNET_OPTION_TOTAL_PRIVATE_SLOTS, &privateSlots, &dataSize );
	m_pIQNet->GetOpt(QNET_OPTION_FILLED_PRIVATE_SLOTS, &filledPrivateSlots, &dataSize );

	DWORD spaceLeft = (publicSlots - filledPublicSlots) + (privateSlots - filledPrivateSlots);

	return spaceLeft >= spaceRequired;
}

void CPlatformNetworkManagerXbox::SendInviteGUI(int quadrant)
{
}

bool CPlatformNetworkManagerXbox::IsAddingPlayer()
{
	return false;
}

bool CPlatformNetworkManagerXbox::LeaveGame(bool bMigrateHost)
{
	if( m_bLeavingGame ) return true;

	m_bLeavingGame = true;

	// If we are a client, wait for all client connections to close
	// TODO Possibly need to do multiple objects depending on how split screen online works
	IQNetPlayer *pQNetPlayer = m_pIQNet->GetLocalPlayerByUserIndex(g_NetworkManager.GetPrimaryPad());
	INetworkPlayer *pNetworkPlayer = getNetworkPlayer(pQNetPlayer);

	if(pNetworkPlayer != nullptr)
	{
		Socket *socket = pNetworkPlayer->GetSocket();

		if( socket != nullptr )
		{
			//printf("Waiting for socket closed event\n");
			DWORD result = socket->m_socketClosedEvent->WaitForSignal(INFINITE);

			// The session might be gone once the socket releases
			if( IsInSession() )
			{
				//printf("Socket closed event has fired\n");
				// 4J Stu - Clear our reference to this socket
				pQNetPlayer = m_pIQNet->GetLocalPlayerByUserIndex(g_NetworkManager.GetPrimaryPad());
				pNetworkPlayer = getNetworkPlayer(pQNetPlayer);
				if(pNetworkPlayer) pNetworkPlayer->SetSocket( nullptr );
			}
			delete socket;
		}
		else
		{
			//printf("Socket is already nullptr\n");
		}
	}

	// If we are the host wait for the game server to end
	if(m_pIQNet->IsHost() && g_NetworkManager.ServerStoppedValid())
	{
		m_pIQNet->EndGame();
		g_NetworkManager.ServerStoppedWait();
		g_NetworkManager.ServerStoppedDestroy();
	}

	return _LeaveGame(bMigrateHost, true);
}

bool CPlatformNetworkManagerXbox::_LeaveGame(bool bMigrateHost, bool bLeaveRoom)
{
	// 4J Stu - Fix for #10490 - TCR 001 BAS Game Stability: When a party of four players leave a world to join another world without saving the title will crash.
	// Changed this to make it threadsafe
	m_bLeaveGameOnTick = true;
	m_migrateHostOnLeave = bMigrateHost;

	return true;
}

void CPlatformNetworkManagerXbox::HostGame(int localUsersMask, bool bOnlineGame, bool bIsPrivate, unsigned char publicSlots /*= MINECRAFT_NET_MAX_PLAYERS*/, unsigned char privateSlots /*= 0*/)
{
// #ifdef _XBOX
	// 4J Stu - We probably did this earlier as well, but just to be sure!
	SetLocalGame( !bOnlineGame );
	SetPrivateGame( bIsPrivate );
	SystemFlagReset();

	// Make sure that the Primary Pad is in by default
	localUsersMask |= GetLocalPlayerMask( g_NetworkManager.GetPrimaryPad() );

	_HostGame( localUsersMask, publicSlots, privateSlots );
//#endif
}

void CPlatformNetworkManagerXbox::_HostGame(int usersMask, unsigned char publicSlots /*= MINECRAFT_NET_MAX_PLAYERS*/, unsigned char privateSlots /*= 0*/)
{
	HRESULT hr;
	// Create a session using the standard game type, in multiplayer game mode,
	// The constants used to specify game mode, context ID and context value are
	// defined in the title's .spa.h file, generated using the XLAST tool.
	// TODO 4J Stu - Game mode should be CONTEXT_GAME_MODE_MULTIPLAYER?
	XUSER_CONTEXT aXUserContexts[] = { { X_CONTEXT_GAME_TYPE, X_CONTEXT_GAME_TYPE_STANDARD },
									   { X_CONTEXT_GAME_MODE, CONTEXT_GAME_MODE_GAMEMODE } };


	// We need at least one other slot otherwise it's not multiplayer, is it!
	if(publicSlots==1 && privateSlots==0)
		privateSlots = 1;

	//printf("Hosting game with %d public slots and %d private slots\n", publicSlots, privateSlots);

	BOOL enableJip = FALSE;
	m_pIQNet->SetOpt( QNET_OPTION_JOIN_IN_PROGRESS_ALLOWED, &enableJip, sizeof BOOL );
	BOOL enableInv = FALSE;
	m_pIQNet->SetOpt( QNET_OPTION_INVITES_ALLOWED, &enableInv, sizeof BOOL );
	BOOL enablePres = FALSE;
	m_pIQNet->SetOpt( QNET_OPTION_PRESENCE_JOIN_MODE, &enablePres, sizeof BOOL );

	// Start hosting a new game
	// Use only the contexts defined above, and no properties.
	hr = m_pIQNet->HostGame(
		g_NetworkManager.GetLockedProfile(),          // dwUserIndex
		usersMask,            // dwUserMask
		publicSlots,                   // dwPublicSlots
		privateSlots,                  // dwPrivateSlots
		0,                             // cProperties
		nullptr,                          // pProperties
		ARRAYSIZE( aXUserContexts ),   // cContexts
		aXUserContexts );              // pContexts

	m_hostGameSessionData.netVersion = MINECRAFT_NET_VERSION;
	m_hostGameSessionData.isJoinable = !IsPrivateGame();

	char* hostName = new char[XUSER_NAME_SIZE];
	hostName = g_NetworkManager.GetOnlineName( g_NetworkManager.GetPrimaryPad() );
	memcpy(m_hostGameSessionData.hostName,hostName,XUSER_NAME_SIZE);

	hr = m_pIQNet->SetOpt(
         QNET_OPTION_QOS_DATA_BUFFER,
         &m_hostGameSessionData,
         sizeof(GameSessionData)
		);
}

bool CPlatformNetworkManagerXbox::_StartGame()
{
	// Set the options that now allow players to join this game
	BOOL enableJip = TRUE; // Must always be true othewise nobody can join the game while in the PLAY state
	m_pIQNet->SetOpt( QNET_OPTION_JOIN_IN_PROGRESS_ALLOWED, &enableJip, sizeof BOOL );
	BOOL enableInv = !IsLocalGame();
	m_pIQNet->SetOpt( QNET_OPTION_INVITES_ALLOWED, &enableInv, sizeof BOOL );
	BOOL enablePres = !IsPrivateGame() && !IsLocalGame();
	m_pIQNet->SetOpt( QNET_OPTION_PRESENCE_JOIN_MODE, &enablePres, sizeof BOOL );

	return ( m_pIQNet->StartGame() == S_OK );
}

int CPlatformNetworkManagerXbox::JoinGame(FriendSessionInfo *searchResult, int localUsersMask, int primaryUserIndex)
{
	// Being a bit over-cautious here, but the xbox code pre-refactoring took a copy of XSESSION_SEARCHRESULT (although not in static memory)
	// so seems safest to replicate this kind of thing here rather than risk data being pointed to by the searchResult being altered whilst
	// JoinGameFromSearchResult is running.
	static XSESSION_SEARCHRESULT searchResultCopy;
	searchResultCopy = searchResult->searchResult;
	HRESULT hr = m_pIQNet->JoinGameFromSearchResult(
                primaryUserIndex, // dwUserIndex
                localUsersMask,   // dwUserMask
				&searchResultCopy );      // pSearchResult


    if( FAILED( hr ) )
    {
        app.DebugPrintf( "Failed joining game (err = 0x%08x)!\n", hr );
    }

	switch( hr )
	{
		case S_OK:
			return CGameNetworkManager::JOINGAME_SUCCESS;
		case QNET_E_SESSION_FULL:
			return CGameNetworkManager::JOINGAME_FAIL_SERVER_FULL;
		default:
			return CGameNetworkManager::JOINGAME_FAIL_GENERAL;
	}
}

bool CPlatformNetworkManagerXbox::SetLocalGame(bool isLocal)
{
	if( m_pIQNet->GetState() == QNET_STATE_IDLE )
	{
		QNET_SESSIONTYPE sessionType = isLocal ? QNET_SESSIONTYPE_LOCAL : QNET_SESSIONTYPE_LIVE_STANDARD;
		m_pIQNet->SetOpt(QNET_OPTION_TYPE_SESSIONTYPE, &sessionType , sizeof QNET_SESSIONTYPE);

		// The default value for this is QNET_LIVE_STATS_MODE_AUTO, but that decides based on the players
		// in when the game starts. As we may want a non-live player to join the game we cannot have stats enabled
		// when we create the sessions. As a result of this, the NotifyWriteStats callback will not be called for
		// LIVE players that are connected to LIVE so we write their stats data on a state change.
		QNET_LIVE_STATS_MODE statsMode = isLocal ? QNET_LIVE_STATS_MODE_DISABLED : QNET_LIVE_STATS_MODE_ENABLED;
		m_pIQNet->SetOpt(QNET_OPTION_LIVE_STATS_MODE, &statsMode , sizeof QNET_LIVE_STATS_MODE);

		// Also has a default of QNET_LIVE_PRESENCE_MODE_AUTO as above, although the effects are less of an issue
		QNET_LIVE_PRESENCE_MODE presenceMode = isLocal ? QNET_LIVE_PRESENCE_MODE_NOT_ADVERTISED : QNET_LIVE_PRESENCE_MODE_ADVERTISED;
		m_pIQNet->SetOpt(QNET_OPTION_LIVE_PRESENCE_MODE, &presenceMode , sizeof QNET_LIVE_PRESENCE_MODE);

		m_bIsOfflineGame = isLocal;
		app.DebugPrintf("Setting as local game: %s\n", isLocal ? "yes" : "no" );
	}
	else
	{
		app.DebugPrintf("Tried to change QNet Session type while not in idle state\n");
	}
	return true;
}

void CPlatformNetworkManagerXbox::SetPrivateGame(bool isPrivate)
{
	app.DebugPrintf("Setting as private game: %s\n", isPrivate ? "yes" : "no" );
	m_bIsPrivateGame = isPrivate;
}

void CPlatformNetworkManagerXbox::RegisterPlayerChangedCallback(int iPad, void (*callback)(void *callbackParam, INetworkPlayer *pPlayer, bool leaving), void *callbackParam)
{
	playerChangedCallback[iPad] = callback;
	playerChangedCallbackParam[iPad] = callbackParam;
}

void CPlatformNetworkManagerXbox::UnRegisterPlayerChangedCallback(int iPad, void (*callback)(void *callbackParam, INetworkPlayer *pPlayer, bool leaving), void *callbackParam)
{
	if(playerChangedCallbackParam[iPad] == callbackParam)
	{
		playerChangedCallback[iPad] = nullptr;
		playerChangedCallbackParam[iPad] = nullptr;
	}
}

void CPlatformNetworkManagerXbox::HandleSignInChange()
{
	return;
}

bool CPlatformNetworkManagerXbox::_RunNetworkGame()
{
	// We delay actually starting the session so that we know the game server is running by the time the clients try to join
	// This does result in a host advantage
	HRESULT hr = m_pIQNet->StartGame();
	if(FAILED(hr)) return false;

	// Set the options that now allow players to join this game
	BOOL enableJip = TRUE; // Must always be true othewise nobody can join the game while in the PLAY state
	m_pIQNet->SetOpt( QNET_OPTION_JOIN_IN_PROGRESS_ALLOWED, &enableJip, sizeof BOOL );
	BOOL enableInv = !IsLocalGame();
	m_pIQNet->SetOpt( QNET_OPTION_INVITES_ALLOWED, &enableInv, sizeof BOOL );
	BOOL enablePres = !IsPrivateGame() && !IsLocalGame();
	m_pIQNet->SetOpt( QNET_OPTION_PRESENCE_JOIN_MODE, &enablePres, sizeof BOOL );

	return true;
}

void CPlatformNetworkManagerXbox::UpdateAndSetGameSessionData(INetworkPlayer *pNetworkPlayerLeaving /*= nullptr*/)
{
	DWORD playerCount = m_pIQNet->GetPlayerCount();

	if( this->m_bLeavingGame )
		return;

	if( GetHostPlayer() == nullptr )
		return;

	for(unsigned int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; ++i)
	{
		if( i < playerCount )
		{
			INetworkPlayer *pNetworkPlayer = GetPlayerByIndex(i);

			// We can call this from NotifyPlayerLeaving but at that point the player is still considered in the session
			if( pNetworkPlayer != pNetworkPlayerLeaving )
			{
				m_hostGameSessionData.players[i] = static_cast<NetworkPlayerXbox *>(pNetworkPlayer)->GetUID();

				char *temp;
				temp = (char *)wstringtofilename( pNetworkPlayer->GetOnlineName() );
				memcpy(m_hostGameSessionData.szPlayers[i],temp,XUSER_NAME_SIZE);
			}
			else
			{
				m_hostGameSessionData.players[i] = nullptr;
				memset(m_hostGameSessionData.szPlayers[i],0,XUSER_NAME_SIZE);
			}
		}
		else
		{
			m_hostGameSessionData.players[i] = nullptr;
			memset(m_hostGameSessionData.szPlayers[i],0,XUSER_NAME_SIZE);
		}
	}

	m_hostGameSessionData.hostPlayerUID = static_cast<NetworkPlayerXbox *>(GetHostPlayer())->GetQNetPlayer()->GetXuid();
	m_hostGameSessionData.m_uiGameHostSettings = app.GetGameHostOption(eGameHostOption_All);

	HRESULT hr = S_OK;
	hr = m_pIQNet->SetOpt(
		QNET_OPTION_QOS_DATA_BUFFER,
		&m_hostGameSessionData,
		sizeof(GameSessionData)
		);
}

int CPlatformNetworkManagerXbox::RemovePlayerOnSocketClosedThreadProc( void* lpParam )
{
	INetworkPlayer *pNetworkPlayer = static_cast<INetworkPlayer *>(lpParam);

	Socket *socket = pNetworkPlayer->GetSocket();

	if( socket != nullptr )
	{
		//printf("Waiting for socket closed event\n");
		socket->m_socketClosedEvent->WaitForSignal(INFINITE);

		//printf("Socket closed event has fired\n");
		// 4J Stu - Clear our reference to this socket
		pNetworkPlayer->SetSocket( nullptr );
		delete socket;
	}

	return g_pPlatformNetworkManager->RemoveLocalPlayer( pNetworkPlayer );
}

bool CPlatformNetworkManagerXbox::RemoveLocalPlayer( INetworkPlayer *pNetworkPlayer )
{
	if( pNetworkPlayer->IsLocal() )
	{
		return ( m_pIQNet->RemoveLocalPlayerByUserIndex( pNetworkPlayer->GetUserIndex() ) == S_OK );
	}

	return true;
}

CPlatformNetworkManagerXbox::PlayerFlags::PlayerFlags(INetworkPlayer *pNetworkPlayer, unsigned int count)
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
CPlatformNetworkManagerXbox::PlayerFlags::~PlayerFlags()
{
	delete [] flags;
}

// Add a player to the per system flag storage - if we've already got a player from that system, copy its flags over
void CPlatformNetworkManagerXbox::SystemFlagAddPlayer(INetworkPlayer *pNetworkPlayer)
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
void CPlatformNetworkManagerXbox::SystemFlagRemovePlayer(INetworkPlayer *pNetworkPlayer)
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

void CPlatformNetworkManagerXbox::SystemFlagReset()
{
	for( unsigned int i = 0; i < m_playerFlags.size(); i++ )
	{
		delete m_playerFlags[i];
	}
	m_playerFlags.clear();
}

// Set a per system flag - this is done by setting the flag on every player that shares that system
void CPlatformNetworkManagerXbox::SystemFlagSet(INetworkPlayer *pNetworkPlayer, int index)
{
	if( ( index < 0 ) || ( index >= m_flagIndexSize ) ) return;
	if( pNetworkPlayer == nullptr ) return;

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
bool CPlatformNetworkManagerXbox::SystemFlagGet(INetworkPlayer *pNetworkPlayer, int index)
{
	if( ( index < 0 ) || ( index >= m_flagIndexSize ) ) return false;
	if( pNetworkPlayer == nullptr )
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

wstring CPlatformNetworkManagerXbox::GatherStats()
{
	return L"Queue messages: " + std::to_wstring(static_cast<NetworkPlayerXbox *>(GetHostPlayer())->GetQNetPlayer()->GetSendQueueSize( nullptr, QNET_GETSENDQUEUESIZE_MESSAGES ) )
		+ L" Queue bytes: " + std::to_wstring( static_cast<NetworkPlayerXbox *>(GetHostPlayer())->GetQNetPlayer()->GetSendQueueSize( nullptr, QNET_GETSENDQUEUESIZE_BYTES  ) );
}

wstring CPlatformNetworkManagerXbox::GatherRTTStats()
{
	wstring stats(L"Rtt: ");

	wchar_t stat[32];

	for(unsigned int i = 0; i < GetPlayerCount(); ++i)
	{
		IQNetPlayer *pQNetPlayer = static_cast<NetworkPlayerXbox *>(GetPlayerByIndex(i))->GetQNetPlayer();

		if(!pQNetPlayer->IsLocal())
		{
			ZeroMemory(stat,sizeof(WCHAR)*32);
			swprintf(stat, 32, L"%d: %d/", i, pQNetPlayer->GetCurrentRtt() );
			stats.append(stat);
		}
	}
	return stats;
}

void CPlatformNetworkManagerXbox::TickSearch()
{
	if( m_bSearchPending )
	{
		if( m_bSearchResultsReady )
		{
			m_currentSearchResultsCount[m_lastSearchPad] = m_searchResultsCount[m_lastSearchPad];

			// Store the current search results so that we don't delete them too early
			if( m_pCurrentSearchResults[m_lastSearchPad] != nullptr )
			{
				delete m_pCurrentSearchResults[m_lastSearchPad];
				m_pCurrentSearchResults[m_lastSearchPad] = nullptr;
			}
			m_pCurrentSearchResults[m_lastSearchPad] = m_pSearchResults[m_lastSearchPad];
			m_pSearchResults[m_lastSearchPad] = nullptr;

			if( m_pCurrentQoSResult[m_lastSearchPad] != nullptr )
			{
				XNetQosRelease(m_pCurrentQoSResult[m_lastSearchPad]);
				m_pCurrentQoSResult[m_lastSearchPad] = nullptr;
			}
			m_pCurrentQoSResult[m_lastSearchPad] = m_pQoSResult[m_lastSearchPad];
			m_pQoSResult[m_lastSearchPad] = nullptr;

			if( m_SessionsUpdatedCallback != nullptr ) m_SessionsUpdatedCallback(m_pSearchParam);
			m_bSearchResultsReady = false;
			m_bSearchPending = false;
		}
	}
	else
	{
		// Don't start searches unless we have registered a callback
		if( m_SessionsUpdatedCallback != nullptr && (m_lastSearchStartTime[g_NetworkManager.GetPrimaryPad()] + MINECRAFT_XSESSION_SEARCH_DELAY_MILLISECONDS) < GetTickCount() )
		{
			SearchForGames();
		}
	}
}

void CPlatformNetworkManagerXbox::SearchForGames()
{
	// Don't start a new search until we have finished the last one
	if(m_bSearchPending) return;

	m_lastSearchPad = g_NetworkManager.GetPrimaryPad();

	m_lastSearchStartTime[m_lastSearchPad] = GetTickCount();
	m_bSearchPending = true;
	m_bSearchResultsReady = false;

    for (auto it = friendsSessions[m_lastSearchPad].begin(); it < friendsSessions[m_lastSearchPad].end(); ++it)
    {
		delete (*it);
	}
	friendsSessions[m_lastSearchPad].clear();

	if( m_pSearchResults[m_lastSearchPad] != nullptr )
	{
		delete m_pSearchResults[m_lastSearchPad];
		m_pSearchResults[m_lastSearchPad] = nullptr;
	}
	if( m_pQoSResult[m_lastSearchPad] != nullptr )
	{
		XNetQosRelease(m_pQoSResult[m_lastSearchPad]);
		m_pQoSResult[m_lastSearchPad] = nullptr;
	}

	bool bMultiplayerAllowed = g_NetworkManager.IsSignedInLive( g_NetworkManager.GetPrimaryPad() ) && g_NetworkManager.AllowedToPlayMultiplayer( g_NetworkManager.GetPrimaryPad() );

	if( bMultiplayerAllowed )
	{
		// PARTY
		XPARTY_USER_LIST partyUserList;
		HRESULT partyResult = XPartyGetUserList( &partyUserList );
		if((partyResult != XPARTY_E_NOT_IN_PARTY) && (partyUserList.dwUserCount>1))
		{
			for(unsigned int i = 0; i<partyUserList.dwUserCount; i++)
			{
				if(
					( (partyUserList.Users[i].dwFlags & XPARTY_USER_ISLOCAL ) !=  XPARTY_USER_ISLOCAL ) &&
					( (partyUserList.Users[i].dwFlags & XPARTY_USER_ISINGAMESESSION) ==  XPARTY_USER_ISINGAMESESSION ) &&
					partyUserList.Users[i].dwTitleId == TITLEID_MINECRAFT
					)
				{
					bool sessionAlreadyAdded = false;
                    for (auto it = friendsSessions[m_lastSearchPad].begin(); it < friendsSessions[m_lastSearchPad].end(); ++it)
                    {
						FriendSessionInfo *current = *it;
						if( memcmp( &partyUserList.Users[i].SessionInfo.sessionID, &current->sessionId, sizeof(SessionID) ) == 0 )
						{
							//printf("We already have this session from another player.\n");
							sessionAlreadyAdded = true;
							break;
						}
					}

					if(!sessionAlreadyAdded)
					{
						FriendSessionInfo *sessionInfo = new FriendSessionInfo();
						sessionInfo->sessionId = partyUserList.Users[i].SessionInfo.sessionID;
						sessionInfo->hasPartyMember = true;
						friendsSessions[m_lastSearchPad].push_back(sessionInfo);
					}
				}
			}
		}

		// FRIENDS

		DWORD bufferSize = 0;
		HANDLE hFriendsEnumerator;
		DWORD hr = XFriendsCreateEnumerator(
			 g_NetworkManager.GetPrimaryPad(),
			 0,
			 MAX_FRIENDS,
			 &bufferSize,
			 &hFriendsEnumerator
			);

		char *buffer = new char[bufferSize];
		DWORD itemsReturned;
		DWORD result = XEnumerate(
			 hFriendsEnumerator,
			 buffer,
			 bufferSize,
			 &itemsReturned,
			 nullptr
		);

		DWORD flagPlayingOnline = XONLINE_FRIENDSTATE_FLAG_ONLINE; // | XONLINE_FRIENDSTATE_FLAG_PLAYING;

		XONLINE_FRIEND *friends = (XONLINE_FRIEND *)buffer;
		for(unsigned int i = 0; i<itemsReturned; i++)
		{
			//printf("%s\n",friends[i].szGamertag);
			if( (friends[i].dwFriendState & flagPlayingOnline) == flagPlayingOnline &&
				( (friends[i].dwFriendState & XONLINE_FRIENDSTATE_FLAG_JOINABLE) == XONLINE_FRIENDSTATE_FLAG_JOINABLE ||
				(friends[i].dwFriendState & XONLINE_FRIENDSTATE_FLAG_JOINABLE_FRIENDS_ONLY) == XONLINE_FRIENDSTATE_FLAG_JOINABLE_FRIENDS_ONLY)
				&& ( friends[i].dwFriendState & ( XONLINE_FRIENDSTATE_FLAG_SENTREQUEST | XONLINE_FRIENDSTATE_FLAG_RECEIVEDREQUEST ) ) == 0
				&& friends[i].dwTitleID == TITLEID_MINECRAFT)
			{
				//printf("Valid game to join\n");

				bool sessionAlreadyAdded = false;
                for (auto it = friendsSessions[m_lastSearchPad].begin(); it < friendsSessions[m_lastSearchPad].end(); ++it)
                {
					FriendSessionInfo *current = *it;
					if( memcmp( &friends[i].sessionID, &current->sessionId, sizeof(SessionID) ) == 0 )
					{
						//printf("We already have this session from another player.\n");
						sessionAlreadyAdded = true;
						break;
					}
				}

				if(!sessionAlreadyAdded)
				{
					FriendSessionInfo *sessionInfo = new FriendSessionInfo();
					sessionInfo->sessionId = friends[i].sessionID;
					friendsSessions[m_lastSearchPad].push_back(sessionInfo);
					//g_NetworkManager.SearchForGameById(friends[i].sessionID,&SearchForGameCallback, m_hObj);
					//++m_searches;
				}
			}
		}
		delete [] buffer;
	}

	if( friendsSessions[m_lastSearchPad].empty() )
	{
		SetSearchResultsReady();
		return;
	}

	DWORD sessionIDCount = min( XSESSION_SEARCH_MAX_IDS, friendsSessions[m_lastSearchPad].size() );
	SessionID *sessionIDList = new SessionID[sessionIDCount];

	for(DWORD i = 0; i < sessionIDCount; ++i)
	{
		sessionIDList[i] = friendsSessions[m_lastSearchPad].at(i)->sessionId;
	}

	DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   cbResults = 0;
	// In this first call, explicitly pass in a null pointer and a buffer size
	// of 0. This forces the function to set cbResults to the correct buffer
	// size.

	dwStatus = XSessionSearchByIds(
		sessionIDCount,
		sessionIDList,
		g_NetworkManager.GetPrimaryPad(),
		&cbResults,         // Pass in the address of the size variable
		nullptr,
		nullptr                       // This example uses the synchronous model
		);

	XOVERLAPPED *pOverlapped = new XOVERLAPPED();
	ZeroMemory(pOverlapped, sizeof(XOVERLAPPED));

	// If the function returns ERROR_INSUFFICIENT_BUFFER cbResults has been
	// changed to reflect the size buffer that will be necessary for this call. Use
	// the new size to allocate a buffer of the appropriate size.
	if (ERROR_INSUFFICIENT_BUFFER == dwStatus && cbResults > 0)
	{
		// Allocate this on the main thread rather in the search thread which might run out of memory
		m_pSearchResults[m_lastSearchPad] = (XSESSION_SEARCHRESULT_HEADER *) new BYTE[cbResults];

		if (!m_pSearchResults[m_lastSearchPad])
		{
			dwStatus = ERROR_OUTOFMEMORY;
			// Handle this "out of title memory" case and abort the read.
		}

		ZeroMemory(m_pSearchResults[m_lastSearchPad], cbResults);

		// Next, call the function again with the exact same parameters, except
		// this time use the modified buffer size and a pointer to a buffer that
		// matches it.
		dwStatus = XSessionSearchByIds(
			sessionIDCount,
			sessionIDList,
			g_NetworkManager.GetPrimaryPad(),
			&cbResults,       // Pass in the address of the size variable
			m_pSearchResults[m_lastSearchPad],
			pOverlapped
			);
	}

	// Test the result of either the first call (if it failed with
	// something other than ERROR_INSUFFICIENT_BUFFER) or the subsequent call.
	// If the function does not succeed after allocating a buffer of the appropriate size
	// succeed, something else is wrong.
	if (ERROR_IO_PENDING != dwStatus)//ERROR_SUCCESS != dwStatus)
	{
		// Handle other errors.
		app.DebugPrintf("An error occured while enumerating sessions\n");
		SetSearchResultsReady();

		delete [] sessionIDList;
	}
	else if ( cbResults > 0 )
	{
		SearchForGamesData *threadData = new SearchForGamesData();
		threadData->sessionIDCount = sessionIDCount;
		threadData->searchBuffer = m_pSearchResults[m_lastSearchPad];
		threadData->ppQos = &m_pQoSResult[m_lastSearchPad];
		threadData->pOverlapped = pOverlapped;
		threadData->sessionIDList = sessionIDList;

		m_SearchingThread = new C4JThread(&CPlatformNetworkManagerXbox::SearchForGamesThreadProc, threadData, "SearchForGames");
		m_SearchingThread->SetProcessor( 2 );
		m_SearchingThread->Run();
	}
	else
	{
		SetSearchResultsReady();
	}
}

int CPlatformNetworkManagerXbox::SearchForGamesThreadProc( void* lpParameter )
{
	SearchForGamesData *threadData = static_cast<SearchForGamesData *>(lpParameter);

	DWORD sessionIDCount = threadData->sessionIDCount;

	XOVERLAPPED *pOverlapped = threadData->pOverlapped;

	DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   cbResults = sessionIDCount;
	XSESSION_SEARCHRESULT_HEADER *pSearchResults = (XSESSION_SEARCHRESULT_HEADER *)threadData->searchBuffer;

	while( !XHasOverlappedIoCompleted(pOverlapped) )
	{
		Sleep(100);
	}
	delete pOverlapped;
	delete [] threadData->sessionIDList;

	if( pSearchResults->dwSearchResults == 0 )
	{
		g_pPlatformNetworkManager->SetSearchResultsReady();
		return 0;
	}

	// TODO 4J Stu - Is there a nicer way to allocate less here?
	const XNADDR *QoSxnaddr[XSESSION_SEARCH_MAX_IDS];// = new XNADDR*[sessionIDCount];
	const SessionID *QoSxnkid[XSESSION_SEARCH_MAX_IDS];// = new XNKID*[sessionIDCount];		// Note SessionID is just typedef'd to be a XNKID on xbox
	const XNKEY *QoSxnkey[XSESSION_SEARCH_MAX_IDS];// = new XNKEY*[sessionIDCount];


	for(DWORD i = 0; i < pSearchResults->dwSearchResults; ++i)
	{
		QoSxnaddr[i] = &pSearchResults->pResults[i].info.hostAddress;
		QoSxnkid[i] = &pSearchResults->pResults[i].info.sessionID;
		QoSxnkey[i] = &pSearchResults->pResults[i].info.keyExchangeKey;
	}
	// Create an event object that is autoreset with an initial state of "not signaled".
	// Pass this event handle to the QoSLookup to receive notification of each QoS lookup.
	HANDLE QoSLookupHandle = CreateEvent(nullptr, false, false, nullptr);

	*threadData->ppQos = new XNQOS();

	INT iRet = XNetQosLookup(
		pSearchResults->dwSearchResults, // Number of remote Xbox 360 consoles to probe
		QoSxnaddr,                       // Array of pointers to XNADDR structures
		QoSxnkid,                        // Array of pointers to XNKID structures that contain session IDs for the remote Xbox 360 consoles
		QoSxnkey,                        // Array of pointers to XNKEY structures that contain key-exchange keys for the remote Xbox 360 consoles
		0,                                 // Number of security gateways to probe
		nullptr,                              // Pointer to an array of IN_ADDR structures that contain the IP addresses of the security gateways
		nullptr,                              // Pointer to an array of service IDs for the security gateway
		8,                                 // Number of desired probe replies to receive
		0,                                 // Maximum upstream bandwidth that the outgoing QoS probe packets can consume
		0,                                 // Flags
		QoSLookupHandle,                   // Event handle
		threadData->ppQos );              // Pointer to a pointer to an XNQOS structure that receives the results from the QoS probes

	if( 0 != iRet )
	{
		app.DebugPrintf( "XNetQosLookup failed with error 0x%08x", iRet);
		g_pPlatformNetworkManager->SetSearchResultsReady();
	}
	else
	{

		//m_bQoSTesting = TRUE;

		// Wait for results to all complete.  cxnqosPending will eventually hit zero.
		// Pause thread waiting for QosLookup events to be triggered.
		while ( (*threadData->ppQos)->cxnqosPending != 0 )
		{
			// 4J Stu - We could wait for INFINITE if we weren't watching for the kill flag
			WaitForSingleObject(QoSLookupHandle, 100);
		}

		// Close handle
		CloseHandle( QoSLookupHandle );

		g_pPlatformNetworkManager->SetSearchResultsReady(pSearchResults->dwSearchResults);
	}

	return 0;
}

void CPlatformNetworkManagerXbox::SetSearchResultsReady(int resultCount )
{
	m_bSearchResultsReady = true;
	m_searchResultsCount[m_lastSearchPad] = resultCount;
}

vector<FriendSessionInfo *> *CPlatformNetworkManagerXbox::GetSessionList(int iPad, int localPlayers, bool partyOnly)
{
	vector<FriendSessionInfo *> *filteredList = new vector<FriendSessionInfo *>();;

	const XSESSION_SEARCHRESULT *pSearchResult;
    const XNQOSINFO * pxnqi;

	if( m_currentSearchResultsCount[iPad] > 0 )
	{
		// Loop through all the results.
		for( DWORD dwResult = 0; dwResult < m_currentSearchResultsCount[iPad]; dwResult++ )
		{
			pSearchResult = &m_pCurrentSearchResults[iPad]->pResults[dwResult];

			// No room for us, so ignore it
			// 4J Stu - pSearchResult should never be nullptr, but just in case...
			if(pSearchResult == nullptr || pSearchResult->dwOpenPublicSlots < localPlayers) continue;

			bool foundSession = false;
			FriendSessionInfo *sessionInfo = nullptr;
            auto itFriendSession = friendsSessions[iPad].begin();
            for(itFriendSession = friendsSessions[iPad].begin(); itFriendSession < friendsSessions[iPad].end(); ++itFriendSession)
			{
				sessionInfo = *itFriendSession;
				if(memcmp( &pSearchResult->info.sessionID, &sessionInfo->sessionId, sizeof(SessionID) ) == 0 && (!partyOnly || (partyOnly && sessionInfo->hasPartyMember) ) )
				{
					sessionInfo->searchResult = *pSearchResult;
					sessionInfo->displayLabel = new wchar_t[100];
					ZeroMemory( sessionInfo->displayLabel, 100 * sizeof(wchar_t) );
					foundSession = true;
					break;
				}
			}

			// We received a search result for a session no longer in our list of friends sessions
			if(!foundSession) continue;

			// Print some info about this result.
			app.DebugPrintf( "Search result %u:\n", dwResult );
			//app.DebugPrintf( "    public slots open = %u, filled = %u\n", pSearchResult->dwOpenPublicSlots, pSearchResult->dwFilledPublicSlots );
			//app.DebugPrintf( "    private slots open = %u, filled = %u\n", pSearchResult->dwOpenPrivateSlots, pSearchResult->dwFilledPrivateSlots );

			// See if this result was contacted successfully via QoS probes.
			pxnqi = &m_pCurrentQoSResult[iPad]->axnqosinfo[dwResult];
			if( pxnqi->bFlags & XNET_XNQOSINFO_TARGET_CONTACTED )
			{
				// Print the round trip time and the rough estimation of
				// bandwidth.
				//app.DebugPrintf( "    RTT min = %u, med = %u\n", pxnqi->wRttMinInMsecs, pxnqi->wRttMedInMsecs );
				//app.DebugPrintf( "    bps up = %u, down = %u\n", pxnqi->dwUpBitsPerSec, pxnqi->dwDnBitsPerSec );

				if(pxnqi->cbData > 0)
				{
					sessionInfo->data = *(GameSessionData *)pxnqi->pbData;

					wstring gamerName = convStringToWstring(sessionInfo->data.hostName);
#ifndef _CONTENT_PACKAGE
					if(app.DebugSettingsOn() && (app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_DebugLeaderboards)))
					{
						swprintf(sessionInfo->displayLabel,app.GetString(IDS_GAME_HOST_NAME),L"WWWWWWWWWWWWWWWW");
					}
					else
#endif
					{
						swprintf(sessionInfo->displayLabel,app.GetString(IDS_GAME_HOST_NAME),gamerName.c_str() );
					}
				}
				else
				{
					swprintf(sessionInfo->displayLabel,app.GetString(IDS_GAME_HOST_NAME_UNKNOWN));
				}
				sessionInfo->displayLabelLength = wcslen( sessionInfo->displayLabel );

				// If this host wasn't disabled use this one.
				if( !( pxnqi->bFlags & XNET_XNQOSINFO_TARGET_DISABLED ) &&
					sessionInfo->data.netVersion == MINECRAFT_NET_VERSION &&
					sessionInfo->data.isJoinable)
				{
					//printf("This game is valid\n");
					//if( foundSession ) friendsSessions.erase(itFriendSession);
					FriendSessionInfo *newInfo = new FriendSessionInfo();
					newInfo->data = sessionInfo->data;
					newInfo->displayLabel = new wchar_t[100];
					memcpy(newInfo->displayLabel, sessionInfo->displayLabel, 100 * sizeof(wchar_t) );
					newInfo->displayLabelLength = sessionInfo->displayLabelLength;
					newInfo->hasPartyMember = sessionInfo->hasPartyMember;
					newInfo->searchResult = sessionInfo->searchResult;
					newInfo->sessionId = sessionInfo->sessionId;
					filteredList->push_back(newInfo);
				}
	#ifndef _CONTENT_PACKAGE
				if( sessionInfo->data.netVersion != MINECRAFT_NET_VERSION )
				{
					wprintf(L"%ls version of %d does not match our version of %d\n", sessionInfo->displayLabel, sessionInfo->data.netVersion, MINECRAFT_NET_VERSION);
				}
	#endif
			}
		}
	}

	return filteredList;
}

// This runs through the search results for a session matching sessionId, then returns the full details in foundSessionInfo
bool CPlatformNetworkManagerXbox::GetGameSessionInfo(int iPad, SessionID sessionId, FriendSessionInfo *foundSessionInfo)
{
	HRESULT hr = E_FAIL;

	const XSESSION_SEARCHRESULT *pSearchResult;
    const XNQOSINFO * pxnqi;

	if( m_currentSearchResultsCount[iPad] > 0 )
	{
		// Loop through all the results.
		for( DWORD dwResult = 0; dwResult < m_currentSearchResultsCount[iPad]; dwResult++ )
		{
			pSearchResult = &m_pCurrentSearchResults[iPad]->pResults[dwResult];

			if(memcmp( &pSearchResult->info.sessionID, &sessionId, sizeof(SessionID) ) != 0) continue;

			bool foundSession = false;
			FriendSessionInfo *sessionInfo = nullptr;
			auto = itFriendSession, friendsSessions[iPad].begin();
			for(itFriendSession = friendsSessions[iPad].begin(); itFriendSession < friendsSessions[iPad].end(); ++itFriendSession)
			{
				sessionInfo = *itFriendSession;
				if(memcmp( &pSearchResult->info.sessionID, &sessionInfo->sessionId, sizeof(SessionID) ) == 0)
				{
					sessionInfo->searchResult = *pSearchResult;
					sessionInfo->displayLabel = new wchar_t[100];
					ZeroMemory( sessionInfo->displayLabel, 100 * sizeof(wchar_t) );
					foundSession = true;
					break;
				}
			}

			// We received a search result for a session no longer in our list of friends sessions
			if(!foundSession) break;

			// See if this result was contacted successfully via QoS probes.
			pxnqi = &m_pCurrentQoSResult[iPad]->axnqosinfo[dwResult];
			if( pxnqi->bFlags & XNET_XNQOSINFO_TARGET_CONTACTED )
			{

				if(pxnqi->cbData > 0)
				{
					sessionInfo->data = *(GameSessionData *)pxnqi->pbData;

					wstring gamerName = convStringToWstring(sessionInfo->data.hostName);
					swprintf(sessionInfo->displayLabel,app.GetString(IDS_GAME_HOST_NAME),L"MWWWWWWWWWWWWWWM");// gamerName.c_str() );
				}
				else
				{
					swprintf(sessionInfo->displayLabel,app.GetString(IDS_GAME_HOST_NAME_UNKNOWN));
				}
				sessionInfo->displayLabelLength = wcslen( sessionInfo->displayLabel );

				// If this host wasn't disabled use this one.
				if( !( pxnqi->bFlags & XNET_XNQOSINFO_TARGET_DISABLED ) &&
					sessionInfo->data.netVersion == MINECRAFT_NET_VERSION &&
					sessionInfo->data.isJoinable)
				{
					foundSessionInfo->data = sessionInfo->data;
					if(foundSessionInfo->displayLabel != nullptr) delete [] foundSessionInfo->displayLabel;
					foundSessionInfo->displayLabel = new wchar_t[100];
					memcpy(foundSessionInfo->displayLabel, sessionInfo->displayLabel, 100 * sizeof(wchar_t) );
					foundSessionInfo->displayLabelLength = sessionInfo->displayLabelLength;
					foundSessionInfo->hasPartyMember = sessionInfo->hasPartyMember;
					foundSessionInfo->searchResult = sessionInfo->searchResult;
					foundSessionInfo->sessionId = sessionInfo->sessionId;

					hr = S_OK;
				}
			}
		}
	}

	return ( hr == S_OK );
}

void CPlatformNetworkManagerXbox::SetSessionsUpdatedCallback( void (*SessionsUpdatedCallback)(LPVOID pParam), LPVOID pSearchParam )
{
	m_SessionsUpdatedCallback = SessionsUpdatedCallback; m_pSearchParam = pSearchParam;
}

void CPlatformNetworkManagerXbox::GetFullFriendSessionInfo( FriendSessionInfo *foundSession, void (* FriendSessionUpdatedFn)(bool success, void *pParam), void *pParam )
{
	FriendSessionUpdatedFn(true, pParam);
}

void CPlatformNetworkManagerXbox::ForceFriendsSessionRefresh()
{
	app.DebugPrintf("Resetting friends session search data\n");
	for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		m_searchResultsCount[i] = 0;
		m_lastSearchStartTime[i] = 0;
		delete m_pSearchResults[i];
		m_pSearchResults[i] = nullptr;
	}
}

INetworkPlayer *CPlatformNetworkManagerXbox::addNetworkPlayer(IQNetPlayer *pQNetPlayer)
{
	NetworkPlayerXbox *pNetworkPlayer = new NetworkPlayerXbox(pQNetPlayer);
	pQNetPlayer->SetCustomDataValue((ULONG_PTR)pNetworkPlayer);
	currentNetworkPlayers.push_back( pNetworkPlayer );
	return pNetworkPlayer;
}

void CPlatformNetworkManagerXbox::removeNetworkPlayer(IQNetPlayer *pQNetPlayer)
{
	INetworkPlayer *pNetworkPlayer = getNetworkPlayer(pQNetPlayer);
	for( auto it = currentNetworkPlayers.begin(); it != currentNetworkPlayers.end(); it++ )
	{
		if( *it == pNetworkPlayer )
		{
			currentNetworkPlayers.erase(it);
			return;
		}
	}
}

INetworkPlayer *CPlatformNetworkManagerXbox::getNetworkPlayer(IQNetPlayer *pQNetPlayer)
{
	return pQNetPlayer ? (INetworkPlayer *)(pQNetPlayer->GetCustomDataValue()) : nullptr;
}


INetworkPlayer *CPlatformNetworkManagerXbox::GetLocalPlayerByUserIndex(int userIndex )
{
	return getNetworkPlayer(m_pIQNet->GetLocalPlayerByUserIndex(userIndex));
}

INetworkPlayer *CPlatformNetworkManagerXbox::GetPlayerByIndex(int playerIndex)
{
	return getNetworkPlayer(m_pIQNet->GetPlayerByIndex(playerIndex));
}

INetworkPlayer * CPlatformNetworkManagerXbox::GetPlayerByXuid(PlayerUID xuid)
{
	return getNetworkPlayer( m_pIQNet->GetPlayerByXuid(xuid)) ;
}

INetworkPlayer * CPlatformNetworkManagerXbox::GetPlayerBySmallId(unsigned char smallId)
{
	return getNetworkPlayer(m_pIQNet->GetPlayerBySmallId(smallId));
}

INetworkPlayer *CPlatformNetworkManagerXbox::GetHostPlayer()
{
	return getNetworkPlayer(m_pIQNet->GetHostPlayer());
}

bool CPlatformNetworkManagerXbox::IsHost()
{
	return m_pIQNet->IsHost() && !m_bHostChanged;
}

bool CPlatformNetworkManagerXbox::JoinGameFromInviteInfo( int userIndex, int userMask, const INVITE_INFO *pInviteInfo)
{
	return ( m_pIQNet->JoinGameFromInviteInfo( userIndex, userMask, pInviteInfo ) == S_OK);
}

void CPlatformNetworkManagerXbox::SetSessionTexturePackParentId( int id )
{
	m_hostGameSessionData.texturePackParentId = id;
}

void CPlatformNetworkManagerXbox::SetSessionSubTexturePackId( int id )
{
	m_hostGameSessionData.subTexturePackId = id;
}

void CPlatformNetworkManagerXbox::Notify(int ID, ULONG_PTR Param)
{
	m_pIQNet->Notify( ID, Param );
}

bool CPlatformNetworkManagerXbox::IsInSession()
{
	return m_pIQNet->GetState() != QNET_STATE_IDLE;
}

bool CPlatformNetworkManagerXbox::IsInGameplay()
{
	return m_pIQNet->GetState() == QNET_STATE_GAME_PLAY;
}

bool CPlatformNetworkManagerXbox::IsReadyToPlayOrIdle()
{
	return true;
}
