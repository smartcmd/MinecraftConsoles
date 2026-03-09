#include "stdafx.h"
#include "..\..\..\Minecraft.World\Socket.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "PlatformNetworkManagerSony.h"
#include "NetworkPlayerSony.h"
#include "..\..\Common\Network\GameNetworkManager.h"

CPlatformNetworkManagerSony *g_pPlatformNetworkManager;

bool CPlatformNetworkManagerSony::IsLocalGame()
{
	return m_bIsOfflineGame;
}
bool CPlatformNetworkManagerSony::IsPrivateGame()
{
	return m_bIsPrivateGame;
}
bool CPlatformNetworkManagerSony::IsLeavingGame()
{
	return m_bLeavingGame;
}
void CPlatformNetworkManagerSony::ResetLeavingGame()
{
	m_bLeavingGame = false;
}


void CPlatformNetworkManagerSony::HandleStateChange(SQRNetworkManager::eSQRNetworkManagerState oldState, SQRNetworkManager::eSQRNetworkManagerState newState, bool idleReasonIsSessionFull)
{
	static const char * c_apszStateNames[] =
    {
		"SNM_STATE_INITIALISING",
		"SNM_STATE_INITIALISE_FAILED",
		"SNM_STATE_IDLE",
		"SNM_STATE_HOSTING",
		"SNM_STATE_JOINING",
		"SNM_STATE_STARTING",
		"SNM_STATE_PLAYING",
		"SNM_STATE_LEAVING",
		"SNM_STATE_ENDING",
    };

    app.DebugPrintf( "Network State: %s ==> %s\n",
        c_apszStateNames[ oldState ],
        c_apszStateNames[ newState ] );

	if( newState == SQRNetworkManager::SNM_STATE_HOSTING )
	{
		m_bLeavingGame = false;
		m_bLeaveGameOnTick = false;
		m_bHostChanged = false;
		g_NetworkManager.StateChange_AnyToHosting();
	}
	else if( newState == SQRNetworkManager::SNM_STATE_JOINING )
	{
		// 4J Stu - We may be accepting an invite from the DLC menu, so hide the icon
#if defined __ORBIS__ || defined __PSVITA__
		app.GetCommerce()->HidePsStoreIcon();
#endif
		m_bLeavingGame = false;
		m_bLeaveGameOnTick = false;
		m_bHostChanged = false;
		g_NetworkManager.StateChange_AnyToJoining();
	}
	else if( newState == SQRNetworkManager::SNM_STATE_IDLE && oldState == SQRNetworkManager::SNM_STATE_JOINING )
	{
		if( idleReasonIsSessionFull )
		{
			g_NetworkManager.StateChange_JoiningToIdle(JOIN_FAILED_SERVER_FULL);
		}
		else
		{
			g_NetworkManager.StateChange_JoiningToIdle(JOIN_FAILED_NONSPECIFIC);
		}
	}
	else if( newState == SQRNetworkManager::SNM_STATE_IDLE && oldState == SQRNetworkManager::SNM_STATE_HOSTING )
	{
		m_bLeavingGame = true;
	}
	else if( newState == SQRNetworkManager::SNM_STATE_STARTING )
	{
		m_lastPlayerEventTimeStart = app.getAppTime();

		g_NetworkManager.StateChange_AnyToStarting();
	}
	// Fix for #93148 - TCR 001: BAS Game Stability: Title will crash for the multiplayer client if host of the game will exit during the clients loading to created world.
	// 4J Stu - If the client joins just as the host is exiting, then they can skip to leaving without passing through ending
	else if( newState == SQRNetworkManager::SNM_STATE_ENDING )
	{
		g_NetworkManager.StateChange_AnyToEnding( oldState == SQRNetworkManager::SNM_STATE_PLAYING );

		// 4J-PB - Only the host can leave here - the clients will hang if m_bLeavingGame is set to true here
		if( m_pSQRNet->IsHost() )
		{
			m_bLeavingGame = true;
		}
	}

	if( newState == SQRNetworkManager::SNM_STATE_IDLE )
	{
		// On PS3, sometimes we're getting a SNM_STATE_ENDING transition to SNM_STATE_IDLE on joining, because the server context being deleted sets the state away from SNM_STATE_JOINING before we detect
		// the cause for the disconnection. This means we don't pick up on the joining->idle transition. Set disconnection reason here too for this case.
		if( idleReasonIsSessionFull )
		{
			app.SetDisconnectReason( DisconnectPacket::eDisconnect_ServerFull );
		}
		g_NetworkManager.StateChange_AnyToIdle();
	}
}

void CPlatformNetworkManagerSony::HandleDataReceived(SQRNetworkPlayer *playerFrom, SQRNetworkPlayer *playerTo, unsigned char *data, unsigned int dataSize)
{
	if(m_pSQRNet->GetState() == SQRNetworkManager::SNM_STATE_ENDING)
	{
		return;
	}

	if( playerTo->IsHost() )
	{
		// If we are the host we care who this came from
		//app.DebugPrintf( "Pushing data into host read queue for user \"%ls\"\n", pPlayerFrom->GetGamertag());
		// Push this data into the read queue for the player that sent it
		INetworkPlayer *pPlayerFrom = getNetworkPlayer(playerFrom);
		Socket *socket = pPlayerFrom->GetSocket();

		if(socket != nullptr)
			socket->pushDataToQueue(data, dataSize, false);
	}
	else
	{
		// If we are not the host the message must have come from the host, so we care more about who it is addressed to
		INetworkPlayer *pPlayerTo = getNetworkPlayer(playerTo);
		Socket *socket = pPlayerTo->GetSocket();
		//app.DebugPrintf( "Pushing data into read queue for user \"%ls\"\n", apPlayersTo[dwPlayer]->GetGamertag());
		if(socket != nullptr)
			socket->pushDataToQueue(data, dataSize);
	}
}

void CPlatformNetworkManagerSony::HandlePlayerJoined(SQRNetworkPlayer *              pSQRPlayer)
{
	const char * pszDescription;

	// 4J Stu - We create a fake socket for every where that we need an INBOUND queue of game data. Outbound
	// is all handled by QNet so we don't need that. Therefore each client player has one, and the host has one
	// for each client player.
	bool createFakeSocket = false;
	bool localPlayer = false;

	NetworkPlayerSony *networkPlayer = (NetworkPlayerSony *)addNetworkPlayer(pSQRPlayer);

    if( pSQRPlayer->IsLocal() )
    {
		localPlayer = true;
        if( pSQRPlayer->IsHost() )
        {
            pszDescription = "local host";
			// 4J Stu - No socket for the localhost as it uses a special loopback queue

			m_machineSQRPrimaryPlayers.push_back( pSQRPlayer );
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
        if( pSQRPlayer->IsHost() )
        {
            pszDescription = "remote host";
        }
        else
        {
            pszDescription = "remote";

			// If we are the host, then create a fake socket for every remote player
			if( m_pSQRNet->IsHost() )
			{
				createFakeSocket = true;
			}
        }

		if( m_pSQRNet->IsHost() && !m_bHostChanged )
		{
			// Do we already have a primary player for this system?
			bool systemHasPrimaryPlayer = false;
			for( SQRNetworkPlayer *pQNetPrimaryPlayer : m_machineSQRPrimaryPlayers )
			{
				if( pSQRPlayer->IsSameSystem(pQNetPrimaryPlayer) )
				{
					systemHasPrimaryPlayer = true;
					break;
				}
			}
			if( !systemHasPrimaryPlayer )
				m_machineSQRPrimaryPlayers.push_back( pSQRPlayer );
		}
    }
	g_NetworkManager.PlayerJoining( networkPlayer );

	if( createFakeSocket == true && !m_bHostChanged )
	{
		g_NetworkManager.CreateSocket( networkPlayer, localPlayer );
	}

#if 0
    app.DebugPrintf( "Player 0x%p \"%ls\" joined; %s; voice %i; camera %i.\n",
        pSQRPlayer,
        pSQRPlayer->GetGamertag(),
        pszDescription,
        (int) pSQRPlayer->HasVoice(),
        (int) pSQRPlayer->HasCamera() );
#endif


	if( m_pSQRNet->IsHost() )
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

	if(true)	// TODO m_pSQRNet->GetState() == QNET_STATE_GAME_PLAY)
	{
		int localPlayerCount = 0;
		for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
		{
			if( m_pSQRNet->GetLocalPlayerByUserIndex(idx) != nullptr ) ++localPlayerCount;
		}

		float appTime = app.getAppTime();

		// Only record stats for the primary player here
		m_lastPlayerEventTimeStart = appTime;
	}
}

void CPlatformNetworkManagerSony::HandlePlayerLeaving(SQRNetworkPlayer *pSQRPlayer)
{
	//__debugbreak();

	app.DebugPrintf( "Player 0x%p leaving.\n",
        pSQRPlayer );

	INetworkPlayer *networkPlayer = getNetworkPlayer(pSQRPlayer);

	if( networkPlayer )
	{
		// Get our wrapper object associated with this player.
		Socket *socket = networkPlayer->GetSocket();
		if( socket != nullptr )
		{
			// If we are in game then remove this player from the game as well.
			// We may get here either from the player requesting to exit the game,
			// in which case we they will already have left the game server, or from a disconnection
			// where we then have to remove them from the game server
			if( m_pSQRNet->IsHost() && !m_bHostChanged )
			{
				g_NetworkManager.CloseConnection(networkPlayer);
			}

			// Free the wrapper object memory.
			// TODO 4J Stu - We may still be using this at the point that the player leaves the session.
			// We need this as long as the game server still needs to communicate with the player
			//delete socket;

			networkPlayer->SetSocket( nullptr );
		}

		if( m_pSQRNet->IsHost() && !m_bHostChanged )
		{
			if( isSystemPrimaryPlayer(pSQRPlayer) )
			{
				SQRNetworkPlayer *pNewSQRPrimaryPlayer = nullptr;
				for(unsigned int i = 0; i < m_pSQRNet->GetPlayerCount(); ++i )
				{
					SQRNetworkPlayer *pSQRPlayer2 = m_pSQRNet->GetPlayerByIndex( i );

					if ( pSQRPlayer2 != nullptr && pSQRPlayer2 != pSQRPlayer && pSQRPlayer2->IsSameSystem( pSQRPlayer ) )
					{
						pNewSQRPrimaryPlayer = pSQRPlayer2;
						break;
					}
				}
				auto it = find( m_machineSQRPrimaryPlayers.begin(), m_machineSQRPrimaryPlayers.end(), pSQRPlayer);
				if( it != m_machineSQRPrimaryPlayers.end() )
				{
					m_machineSQRPrimaryPlayers.erase( it );
				}

				if( pNewSQRPrimaryPlayer != nullptr )
					m_machineSQRPrimaryPlayers.push_back( pNewSQRPrimaryPlayer );
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

		if(m_pSQRNet->GetState() == SQRNetworkManager::SNM_STATE_PLAYING)
		{
			int localPlayerCount = 0;
			for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
			{
				if( m_pSQRNet->GetLocalPlayerByUserIndex(idx) != nullptr ) ++localPlayerCount;
			}

			float appTime = app.getAppTime();
			m_lastPlayerEventTimeStart = appTime;
		}

		removeNetworkPlayer(pSQRPlayer);
	}
}

// Update our external data to match the current internal player slots, and resync back out (host only)
void CPlatformNetworkManagerSony::HandleResyncPlayerRequest(SQRNetworkPlayer **aPlayers)
{
	m_hostGameSessionData.playerCount = 0;
	for(int i = 0; i < SQRNetworkManager::MAX_ONLINE_PLAYER_COUNT; i++ )
	{
		if( aPlayers[i] )
		{
			m_hostGameSessionData.players[i] = aPlayers[i]->GetUID();
			m_hostGameSessionData.playerCount++;
		}
		else
		{
			memset(&m_hostGameSessionData.players[i],0,sizeof(m_hostGameSessionData.players[i]));
		}
	}
	m_pSQRNet->UpdateExternalRoomData();
}

void CPlatformNetworkManagerSony::HandleAddLocalPlayerFailed(int idx)
{
	g_NetworkManager.AddLocalPlayerFailed(idx);
}

void CPlatformNetworkManagerSony::HandleDisconnect(bool bLostRoomOnly,bool bPSNSignOut)
{
	g_NetworkManager.HandleDisconnect(bLostRoomOnly,bPSNSignOut);
}

void CPlatformNetworkManagerSony::HandleInviteReceived( int userIndex, const SQRNetworkManager::PresenceSyncInfo *pInviteInfo)
{
	g_NetworkManager.GameInviteReceived( userIndex, pInviteInfo );
}

extern SQRNetworkManager *testSQRNetworkManager;

bool CPlatformNetworkManagerSony::Initialise(CGameNetworkManager *pGameNetworkManager, int flagIndexSize)
{
	// Create a sony network manager, and go online
#ifdef __ORBIS__
	m_pSQRNet = new SQRNetworkManager_Orbis(this);
	m_pSQRNet->Initialise();
#elif defined __PS3__
	m_pSQRNet = new SQRNetworkManager_PS3(this);
	m_pSQRNet->Initialise();
#else // __PSVITA__
	//	m_pSQRNet = new SQRNetworkManager_Vita(this);
	m_bUsingAdhocMode = false;
	m_pSQRNet_Vita_Adhoc = new SQRNetworkManager_AdHoc_Vita(this);
	m_pSQRNet_Vita = new SQRNetworkManager_Vita(this);

	m_pSQRNet = m_pSQRNet_Vita;

	// 4J-PB - seems we can't initialise both adhoc and psn comms - from Rohan - "having adhoc matching and matching2 library initialised together results in undesired behaviour", but probably having other parts initialised also is 'undesirable'

 	m_pSQRNet_Vita->Initialise();

	if(ProfileManager.IsSignedInPSN(ProfileManager.GetPrimaryPad()))
	{
		// we're signed into the PSN, but we won't be online yet, force a sign-in online here
		m_pSQRNet_Vita->AttemptPSNSignIn(nullptr, nullptr);
	}


#endif

	m_pGameNetworkManager = pGameNetworkManager;
	m_flagIndexSize = flagIndexSize;
	g_pPlatformNetworkManager = this;
	for( int i = 0; i < XUSER_MAX_COUNT; i++ )
	{
		playerChangedCallback[ i ] = nullptr;
	}

	m_bLeavingGame = false;
	m_bLeaveGameOnTick = false;
	m_bHostChanged = false;
	m_bLeaveRoomWhenLeavingGame = true;

	m_bSearchPending = false;

	m_bIsOfflineGame = false;
	m_pSearchParam = nullptr;
	m_SessionsUpdatedCallback = nullptr;

	m_searchResultsCount = 0;
	m_pSearchResults = nullptr;

	m_lastSearchStartTime = 0;

    // Success!
    return true;
}

void CPlatformNetworkManagerSony::Terminate()
{
	m_pSQRNet->Terminate();
}

int CPlatformNetworkManagerSony::GetJoiningReadyPercentage()
{
	return m_pSQRNet->GetJoiningReadyPercentage();
}

int CPlatformNetworkManagerSony::CorrectErrorIDS(int IDS)
{
	// Attempts to remap the following messages to provide something that PS3 TCRs are happier with
	//
	//  IDS_CONNECTION_LOST				  - "Connection lost"
	//  IDS_CONNECTION_FAILED			  - "Connection failed"
	//	IDS_CONNECTION_LOST_LIVE		  - "Connection to "PSN" was lost. Exiting to the main menu."
	//  IDS_CONNECTION_LOST_LIVE_NO_EXIT  - "Connection to "PSN" was lost."
	//  IDS_CONNECTION_LOST_SERVER		  - "Connection to the server was lost. Exiting to the main menu."
	//
	// Map to:
	//
	// IDS_ERROR_NETWORK				    - "A network error has occurred"
	// IDS_ERROR_NETWORK_TITLE			    - "Network Error"
	// IDS_ERROR_NETWORK_EXIT			    - "A network error has occurred. Exiting to Main Menu."
	// IDS_ERROR_PSN_SIGN_OUT				- You have been signed out from the "PSN".
	// IDS_ERROR_PSN_SIGN_OUT_EXIT			- You have been signed out from the "PSN". Exiting to Main Menu

	// Determine if we'd prefer to present errors as a signing out issue, rather than a network issue, based on whether we have a network connection at all or not
	bool preferSignoutError = false;
	int state;

#if defined __PSVITA__			// MGH - to fix devtrack #6258
	if(!ProfileManager.IsSignedInPSN(ProfileManager.GetPrimaryPad()))
		preferSignoutError = true;
#elif defined __ORBIS__
	if(!ProfileManager.isSignedInPSN(ProfileManager.GetPrimaryPad()))
		preferSignoutError = true;
#elif defined __PS3__
	int ret = cellNetCtlGetState( &state );
	int IPObtainedState = CELL_NET_CTL_STATE_IPObtained;
	if( ret == 0 )
	{
		if( state == IPObtainedState )
		{
			preferSignoutError = true;
		}
	}
#endif

#ifdef __PSVITA__
	// If we're in ad-hoc mode this problem definitely wasn't PSN related
	if (usingAdhocMode()) preferSignoutError = false;
#endif

	// If we're the host we haven't lost connection to the server
	if (IDS == IDS_CONNECTION_LOST_SERVER && g_NetworkManager.IsHost())
	{
		IDS = IDS_CONNECTION_LOST_LIVE;
	}

	switch(IDS)
	{
		case IDS_CONNECTION_LOST:
		case IDS_CONNECTION_FAILED:
			return IDS_ERROR_NETWORK_TITLE;
		case IDS_CONNECTION_LOST_LIVE:
			if( preferSignoutError )
			{
				return IDS_ERROR_PSN_SIGN_OUT_EXIT;
			}
			else
			{
				return IDS_ERROR_NETWORK_EXIT;
			}
		case IDS_CONNECTION_LOST_LIVE_NO_EXIT:
			if( preferSignoutError )
			{
				return IDS_ERROR_PSN_SIGN_OUT;
			}
			else
			{
				return IDS_ERROR_NETWORK_TITLE;
			}
			break;
#ifdef __PSVITA__
		case IDS_CONNECTION_LOST_SERVER:
			if(preferSignoutError)
			{
				if(ProfileManager.IsSignedInPSN(ProfileManager.GetPrimaryPad()) == false)
					return IDS_ERROR_PSN_SIGN_OUT_EXIT;
			}
#endif
		default:
			return IDS;
	}


}

bool CPlatformNetworkManagerSony::isSystemPrimaryPlayer(SQRNetworkPlayer *pSQRPlayer)
{
	bool playerIsSystemPrimary = false;
	for( SQRNetworkPlayer *pSQRPrimaryPlayer : m_machineSQRPrimaryPlayers )
	{
		if( pSQRPrimaryPlayer == pSQRPlayer )
		{
			playerIsSystemPrimary = true;
			break;
		}
	}
	return playerIsSystemPrimary;
}

// We call this twice a frame, either side of the render call so is a good place to "tick" things
void CPlatformNetworkManagerSony::DoWork()
{
#if 0
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
#else
	TickSearch();

	if( m_bLeaveGameOnTick )
	{
		m_pSQRNet->LeaveRoom(m_bLeaveRoomWhenLeavingGame);
		m_bLeaveGameOnTick = false;
	}

	m_pSQRNet->Tick();
#endif
}

int CPlatformNetworkManagerSony::GetPlayerCount()
{
	return m_pSQRNet->GetPlayerCount();
}

bool CPlatformNetworkManagerSony::ShouldMessageForFullSession()
{
	return false;
}

int CPlatformNetworkManagerSony::GetOnlinePlayerCount()
{
	return m_pSQRNet->GetOnlinePlayerCount();
}

int CPlatformNetworkManagerSony::GetLocalPlayerMask(int playerIndex)
{
	return 1 << playerIndex;
}

bool CPlatformNetworkManagerSony::AddLocalPlayerByUserIndex( int userIndex )
{
	return  m_pSQRNet->AddLocalPlayerByUserIndex(userIndex);
}

bool CPlatformNetworkManagerSony::RemoveLocalPlayerByUserIndex( int userIndex )
{
	SQRNetworkPlayer *pSQRPlayer = m_pSQRNet->GetLocalPlayerByUserIndex(userIndex);
	INetworkPlayer *pNetworkPlayer = getNetworkPlayer(pSQRPlayer);

	if(pNetworkPlayer != nullptr)
	{
		Socket *socket = pNetworkPlayer->GetSocket();

		if( socket != nullptr )
		{
			// We can't remove the player from qnet until we have stopped using it to communicate
			C4JThread* thread = new C4JThread(&CPlatformNetworkManagerSony::RemovePlayerOnSocketClosedThreadProc, pNetworkPlayer, "RemovePlayerOnSocketClosed");
			thread->SetProcessor( CPU_CORE_REMOVE_PLAYER );
			thread->Run();
		}
		else
		{
			// Safe to remove the player straight away
			return m_pSQRNet->RemoveLocalPlayerByUserIndex(userIndex);
		}
	}
	return true;
}

bool CPlatformNetworkManagerSony::IsInStatsEnabledSession()
{
#if 0
	DWORD dataSize = sizeof(QNET_LIVE_STATS_MODE);
	QNET_LIVE_STATS_MODE statsMode;
	m_pIQNet->GetOpt(QNET_OPTION_LIVE_STATS_MODE, &statsMode , &dataSize );

	// Use QNET_LIVE_STATS_MODE_AUTO if there is another way to check if stats are enabled or not
	bool statsEnabled = statsMode == QNET_LIVE_STATS_MODE_ENABLED;
	return m_pIQNet->GetState() != QNET_STATE_IDLE && statsEnabled;
#endif
	return true;
}

bool CPlatformNetworkManagerSony::SessionHasSpace(unsigned int spaceRequired /*= 1*/)
{
	return m_pSQRNet->SessionHasSpace(spaceRequired);
#if 0
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
#else
	return true;
#endif
}

void CPlatformNetworkManagerSony::SendInviteGUI(int quadrant)
{
	m_pSQRNet->SendInviteGUI();
}

bool CPlatformNetworkManagerSony::IsAddingPlayer()
{
	return false;
}

bool CPlatformNetworkManagerSony::LeaveGame(bool bMigrateHost)
{
	if( m_bLeavingGame ) return true;

	m_bLeavingGame = true;

	// If we are a client, wait for all client connections to close
	// TODO Possibly need to do multiple objects depending on how split screen online works
	SQRNetworkPlayer *pSQRPlayer = m_pSQRNet->GetLocalPlayerByUserIndex(g_NetworkManager.GetPrimaryPad());
	INetworkPlayer *pNetworkPlayer = getNetworkPlayer(pSQRPlayer);

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
				pSQRPlayer = m_pSQRNet->GetLocalPlayerByUserIndex(g_NetworkManager.GetPrimaryPad());
				pNetworkPlayer = getNetworkPlayer(pSQRPlayer);
				pNetworkPlayer->SetSocket( nullptr );
			}
			delete socket;
		}
		else
		{
			//printf("Socket is already nullptr\n");
		}
	}

	// If we are the host wait for the game server to end
	if(m_pSQRNet->IsHost() && g_NetworkManager.ServerStoppedValid())
	{
		m_pSQRNet->EndGame();
		g_NetworkManager.ServerStoppedWait();
		g_NetworkManager.ServerStoppedDestroy();
	}

	return _LeaveGame(bMigrateHost, true);
}

bool CPlatformNetworkManagerSony::_LeaveGame(bool bMigrateHost, bool bLeaveRoom)
{
	// 4J Stu - Fix for #10490 - TCR 001 BAS Game Stability: When a party of four players leave a world to join another world without saving the title will crash.
	// Changed this to make it threadsafe
	m_bLeavingGame = true;		// Added for Sony platforms but unsure why the 360 doesn't need it - without this, the leaving triggered by this causes the game to respond by leaving again when it transitions to the SNM_STATE_ENDING state
	m_bLeaveRoomWhenLeavingGame = bLeaveRoom;
	m_bLeaveGameOnTick = true;
	m_migrateHostOnLeave = bMigrateHost;

	return true;
}

void CPlatformNetworkManagerSony::HostGame(int localUsersMask, bool bOnlineGame, bool bIsPrivate, unsigned char publicSlots /*= MINECRAFT_NET_MAX_PLAYERS*/, unsigned char privateSlots /*= 0*/)
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

void CPlatformNetworkManagerSony::_HostGame(int usersMask, unsigned char publicSlots /*= MINECRAFT_NET_MAX_PLAYERS*/, unsigned char privateSlots /*= 0*/)
{
	// Start hosting a new game

	memset(&m_hostGameSessionData,0,sizeof(m_hostGameSessionData));
	m_hostGameSessionData.netVersion = MINECRAFT_NET_VERSION;
	m_hostGameSessionData.isJoinable = !IsPrivateGame();
	m_hostGameSessionData.isReadyToJoin = false;
	m_hostGameSessionData.playerCount = 0;
	m_hostGameSessionData.m_uiGameHostSettings = app.GetGameHostOption(eGameHostOption_All);
	for( int i = 0; i < SQRNetworkManager::MAX_LOCAL_PLAYER_COUNT; i++ )
	{
		if( usersMask & ( 1 << i ) )
		{
			m_hostGameSessionData.playerCount++;
		}
	}

	m_pSQRNet->CreateAndJoinRoom(g_NetworkManager.GetPrimaryPad(),usersMask, &m_hostGameSessionData, sizeof(m_hostGameSessionData), IsLocalGame());		// Should be using: g_NetworkManager.GetLockedProfile() but that isn't being set currently
}

bool CPlatformNetworkManagerSony::_StartGame()
{
#if 0
	// Set the options that now allow players to join this game
	BOOL enableJip = TRUE; // Must always be true othewise nobody can join the game while in the PLAY state
	m_pIQNet->SetOpt( QNET_OPTION_JOIN_IN_PROGRESS_ALLOWED, &enableJip, sizeof BOOL );
	BOOL enableInv = !IsLocalGame();
	m_pIQNet->SetOpt( QNET_OPTION_INVITES_ALLOWED, &enableInv, sizeof BOOL );
	BOOL enablePres = !IsPrivateGame() && !IsLocalGame();
	m_pIQNet->SetOpt( QNET_OPTION_PRESENCE_JOIN_MODE, &enablePres, sizeof BOOL );

	return ( m_pIQNet->StartGame() == S_OK );
#else
	m_pSQRNet->StartGame();
	return true;
#endif
}

int CPlatformNetworkManagerSony::JoinGame(FriendSessionInfo *searchResult, int localUsersMask, int primaryUserIndex)
{
	int joinPlayerCount = 0;
	for( int i = 0; i < SQRNetworkManager::MAX_LOCAL_PLAYER_COUNT; i++ )
	{
		if( localUsersMask & ( 1 << i ) )
		{
			joinPlayerCount++;
		}
	}
	GameSessionData *gameSession = (GameSessionData *)(&searchResult->data);
	if( ( gameSession->playerCount + joinPlayerCount ) > SQRNetworkManager::MAX_ONLINE_PLAYER_COUNT )
	{
		return CGameNetworkManager::JOINGAME_FAIL_SERVER_FULL;
	}

	if( m_pSQRNet->JoinRoom(&searchResult->searchResult, localUsersMask) )
	{
		return CGameNetworkManager::JOINGAME_SUCCESS;
	}
	else
	{
		return CGameNetworkManager::JOINGAME_FAIL_GENERAL;
	}
}

bool CPlatformNetworkManagerSony::SetLocalGame(bool isLocal)
{
	if( m_pSQRNet->GetState() == SQRNetworkManager::SNM_STATE_IDLE)
	{
#if 0
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
#endif

		m_bIsOfflineGame = isLocal;
		app.DebugPrintf("Setting as local game: %s\n", isLocal ? "yes" : "no" );
	}
	else
	{
		app.DebugPrintf("Tried to change session type while not in idle or offline state\n");
	}

	return true;
}

void CPlatformNetworkManagerSony::SetPrivateGame(bool isPrivate)
{
	app.DebugPrintf("Setting as private game: %s\n", isPrivate ? "yes" : "no" );
	m_bIsPrivateGame = isPrivate;
}

void CPlatformNetworkManagerSony::RegisterPlayerChangedCallback(int iPad, void (*callback)(void *callbackParam, INetworkPlayer *pPlayer, bool leaving), void *callbackParam)
{
	playerChangedCallback[iPad] = callback;
	playerChangedCallbackParam[iPad] = callbackParam;
}

void CPlatformNetworkManagerSony::UnRegisterPlayerChangedCallback(int iPad, void (*callback)(void *callbackParam, INetworkPlayer *pPlayer, bool leaving), void *callbackParam)
{
	if(playerChangedCallbackParam[iPad] == callbackParam)
	{
		playerChangedCallback[iPad] = nullptr;
		playerChangedCallbackParam[iPad] = nullptr;
	}
}

void CPlatformNetworkManagerSony::HandleSignInChange()
{
	return;
}

bool CPlatformNetworkManagerSony::_RunNetworkGame()
{
#if 0
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
#endif
	if( IsHost() )
	{
		m_pSQRNet->StartGame();
		m_hostGameSessionData.isReadyToJoin = true;
		m_pSQRNet->UpdateExternalRoomData();
		m_pSQRNet->SetPresenceDataStartHostingGame();
	}

	return true;
}

// Note that this does less than the xbox equivalent as we have HandleResyncPlayerRequest that is called by the underlying SQRNetworkManager when players are added/removed etc., so this
// call is only used to update the game host settings & then do the final push out of the data.
void CPlatformNetworkManagerSony::UpdateAndSetGameSessionData(INetworkPlayer *pNetworkPlayerLeaving /*= nullptr*/)
{
	if( this->m_bLeavingGame )
		return;

	m_hostGameSessionData.hostPlayerUID = GetHostPlayer()->GetUID();
#ifdef __PSVITA__
	if(usingAdhocMode())
	{
		m_hostGameSessionData.hostPlayerUID.setForAdhoc();
	}
#endif

	m_hostGameSessionData.m_uiGameHostSettings = app.GetGameHostOption(eGameHostOption_All);

	// If this is called With a pNetworkPlayerLeaving, then the call has ultimately started within SQRNetworkManager::RemoveRemotePlayersAndSync, so we don't need to sync each change
	// as that function does a sync at the end of all changes.
	if( pNetworkPlayerLeaving == nullptr )
	{
		m_pSQRNet->UpdateExternalRoomData();
	}
}

int CPlatformNetworkManagerSony::RemovePlayerOnSocketClosedThreadProc( void* lpParam )
{
	INetworkPlayer *pNetworkPlayer = (INetworkPlayer *)lpParam;

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

bool CPlatformNetworkManagerSony::RemoveLocalPlayer( INetworkPlayer *pNetworkPlayer )
{
	if( pNetworkPlayer->IsLocal() )
	{
		return m_pSQRNet->RemoveLocalPlayerByUserIndex( pNetworkPlayer->GetUserIndex() );
	}

	return true;
}

CPlatformNetworkManagerSony::PlayerFlags::PlayerFlags(INetworkPlayer *pNetworkPlayer, unsigned int count)
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
CPlatformNetworkManagerSony::PlayerFlags::~PlayerFlags()
{
	delete [] flags;
}

// Add a player to the per system flag storage - if we've already got a player from that system, copy its flags over
void CPlatformNetworkManagerSony::SystemFlagAddPlayer(INetworkPlayer *pNetworkPlayer)
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
void CPlatformNetworkManagerSony::SystemFlagRemovePlayer(INetworkPlayer *pNetworkPlayer)
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

void CPlatformNetworkManagerSony::SystemFlagReset()
{
	for( unsigned int i = 0; i < m_playerFlags.size(); i++ )
	{
		delete m_playerFlags[i];
	}
	m_playerFlags.clear();
}

// Set a per system flag - this is done by setting the flag on every player that shares that system
void CPlatformNetworkManagerSony::SystemFlagSet(INetworkPlayer *pNetworkPlayer, int index)
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
bool CPlatformNetworkManagerSony::SystemFlagGet(INetworkPlayer *pNetworkPlayer, int index)
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

wstring CPlatformNetworkManagerSony::GatherStats()
{
#if 0
	return L"Queue messages: " + std::to_wstring(((NetworkPlayerXbox *)GetHostPlayer())->GetQNetPlayer()->GetSendQueueSize( nullptr, QNET_GETSENDQUEUESIZE_MESSAGES ) )
		+ L" Queue bytes: " + std::to_wstring( ((NetworkPlayerXbox *)GetHostPlayer())->GetQNetPlayer()->GetSendQueueSize( nullptr, QNET_GETSENDQUEUESIZE_BYTES  ) );
#else
	return L"";
#endif
}

wstring CPlatformNetworkManagerSony::GatherRTTStats()
{
#if 0
	wstring stats(L"Rtt: ");

	wchar_t stat[32];

	for(unsigned int i = 0; i < GetPlayerCount(); ++i)
	{
		SQRNetworkPlayer *pSQRPlayer = ((NetworkPlayerXbox *)GetPlayerByIndex( i ))->GetQNetPlayer();

		if(!pSQRPlayer->IsLocal())
		{
			ZeroMemory(stat,32);
			swprintf(stat, 32, L"%d: %d/", i, pSQRPlayer->GetCurrentRtt() );
			stats.append(stat);
		}
	}
	return stats;
#else
	return L"";
#endif
}

void CPlatformNetworkManagerSony::TickSearch()
{
	if( m_bSearchPending )
	{
		if( !m_pSQRNet->FriendRoomManagerIsBusy() )
		{
			m_searchResultsCount = m_pSQRNet->FriendRoomManagerGetCount();
			delete m_pSearchResults;
			m_pSearchResults = new SQRNetworkManager::SessionSearchResult[m_searchResultsCount];

			for( int i = 0; i < m_searchResultsCount; i++ )
			{
				m_pSQRNet->FriendRoomManagerGetRoomInfo(i, &m_pSearchResults[i] );
			}
			m_bSearchPending = false;

			if( m_SessionsUpdatedCallback != nullptr ) m_SessionsUpdatedCallback(m_pSearchParam);
		}
	}
	else
	{
		if( !m_pSQRNet->FriendRoomManagerIsBusy() )
		{
			// Don't start searches unless we have registered a callback
			int searchDelay = MINECRAFT_PS3ROOM_SEARCH_DELAY_MILLISECONDS;
#ifdef __PSVITA__
			// in adhoc mode we can keep searching, as the friend list is populated in callbacks
			// 4J Stu - Every second seems a bit much as it makes the friend list flash every time it updates. Changed this to 5 seconds.
			if( usingAdhocMode())
				searchDelay = 5000;
#endif
			if( m_SessionsUpdatedCallback != nullptr && (m_lastSearchStartTime + searchDelay) < GetTickCount() )
			{
				if( m_pSQRNet->FriendRoomManagerSearch() )
				{
					m_bSearchPending = true;
					m_lastSearchStartTime = GetTickCount();
				}
			}
		}
	}
}

vector<FriendSessionInfo *> *CPlatformNetworkManagerSony::GetSessionList(int iPad, int localPlayers, bool partyOnly)
{
	vector<FriendSessionInfo *> *filteredList = new vector<FriendSessionInfo *>();
	for( int i = 0; i < m_searchResultsCount; i++ )
	{
		if( m_pSearchResults[i].m_extData )
		{
			FriendSessionInfo *newInfo = new FriendSessionInfo();
			newInfo->displayLabel = new wchar_t[17];
			ZeroMemory(newInfo->displayLabel, sizeof(wchar_t)*17);
			// TODO - this mbstowcs shouldn't encounter any non-ascii characters, but I imagine we'll want to actually use the online name here which is UTF-8
			mbstowcs(newInfo->displayLabel, m_pSearchResults[i].m_NpId.handle.data, 17);
			newInfo->displayLabelLength = strlen(m_pSearchResults[i].m_NpId.handle.data);
			newInfo->hasPartyMember = false;
			newInfo->searchResult = m_pSearchResults[i];
			newInfo->sessionId = m_pSearchResults[i].m_sessionId;
			memcpy(&newInfo->data, m_pSearchResults[i].m_extData, sizeof(GameSessionData));
			if( ( newInfo->data.isReadyToJoin ) &&
				( newInfo->data.isJoinable ) &&
				( newInfo->data.netVersion == MINECRAFT_NET_VERSION ) )
			{
				filteredList->push_back(newInfo);
			}
			else
			{
				delete newInfo;
			}
		}
	}

	return filteredList;
}

bool CPlatformNetworkManagerSony::GetGameSessionInfo(int iPad, SessionID sessionId, FriendSessionInfo *foundSessionInfo)
{
#if 0
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
			auto itFriendSession = friendsSessions[iPad].begin();
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
#else
	return false;
#endif
}

void CPlatformNetworkManagerSony::SetSessionsUpdatedCallback( void (*SessionsUpdatedCallback)(LPVOID pParam), LPVOID pSearchParam )
{
	m_SessionsUpdatedCallback = SessionsUpdatedCallback; m_pSearchParam = pSearchParam;
}

void CPlatformNetworkManagerSony::GetFullFriendSessionInfo( FriendSessionInfo *foundSession, void (* FriendSessionUpdatedFn)(bool success, void *pParam), void *pParam )
{
	m_pSQRNet->GetExtDataForRoom( foundSession->sessionId.m_RoomId, &foundSession->data, FriendSessionUpdatedFn, pParam);
}

void CPlatformNetworkManagerSony::ForceFriendsSessionRefresh()
{
	app.DebugPrintf("Resetting friends session search data\n");
	m_lastSearchStartTime = 0;
	m_searchResultsCount = 0;
	delete m_pSearchResults;
	m_pSearchResults = nullptr;
}

INetworkPlayer *CPlatformNetworkManagerSony::addNetworkPlayer(SQRNetworkPlayer *pSQRPlayer)
{
	NetworkPlayerSony *pNetworkPlayer = new NetworkPlayerSony(pSQRPlayer);
	pSQRPlayer->SetCustomDataValue((ULONG_PTR)pNetworkPlayer);
	currentNetworkPlayers.push_back( pNetworkPlayer );
	return pNetworkPlayer;
}

void CPlatformNetworkManagerSony::removeNetworkPlayer(SQRNetworkPlayer *pSQRPlayer)
{
	INetworkPlayer *pNetworkPlayer = getNetworkPlayer(pSQRPlayer);
	for( auto it = currentNetworkPlayers.begin(); it != currentNetworkPlayers.end(); it++ )
	{
		if( *it == pNetworkPlayer )
		{
			currentNetworkPlayers.erase(it);
			return;
		}
	}
}

INetworkPlayer *CPlatformNetworkManagerSony::getNetworkPlayer(SQRNetworkPlayer *pSQRPlayer)
{
	return pSQRPlayer ? (INetworkPlayer *)(pSQRPlayer->GetCustomDataValue()) : nullptr;
}


INetworkPlayer *CPlatformNetworkManagerSony::GetLocalPlayerByUserIndex(int userIndex )
{
	return getNetworkPlayer(m_pSQRNet->GetLocalPlayerByUserIndex(userIndex));
}

INetworkPlayer *CPlatformNetworkManagerSony::GetPlayerByIndex(int playerIndex)
{
	return getNetworkPlayer(m_pSQRNet->GetPlayerByIndex(playerIndex));
}

INetworkPlayer * CPlatformNetworkManagerSony::GetPlayerByXuid(PlayerUID xuid)
{
	return getNetworkPlayer(m_pSQRNet->GetPlayerByXuid(xuid));
}

INetworkPlayer * CPlatformNetworkManagerSony::GetPlayerBySmallId(unsigned char smallId)
{
	return getNetworkPlayer(m_pSQRNet->GetPlayerBySmallId(smallId));
}

INetworkPlayer *CPlatformNetworkManagerSony::GetHostPlayer()
{
	return getNetworkPlayer(m_pSQRNet->GetHostPlayer());
}

bool CPlatformNetworkManagerSony::IsHost()
{
	return m_pSQRNet->IsHost() && !m_bHostChanged;
}

bool CPlatformNetworkManagerSony::JoinGameFromInviteInfo( int userIndex, int userMask, const INVITE_INFO *pInviteInfo)
{
	return m_pSQRNet->JoinRoom( pInviteInfo->m_RoomId, pInviteInfo->m_ServerId, userMask, pInviteInfo );
}

void CPlatformNetworkManagerSony::SetSessionTexturePackParentId( int id )
{
	m_hostGameSessionData.texturePackParentId = id;
}

void CPlatformNetworkManagerSony::SetSessionSubTexturePackId( int id )
{
	m_hostGameSessionData.subTexturePackId = id;
}

void CPlatformNetworkManagerSony::Notify(int ID, ULONG_PTR Param)
{
#if 0
	m_pSQRNet->Notify( ID, Param );
#endif
}

bool CPlatformNetworkManagerSony::IsInSession()
{
	return m_pSQRNet->IsInSession();
}

bool CPlatformNetworkManagerSony::IsInGameplay()
{
	return m_pSQRNet->GetState() == SQRNetworkManager::SNM_STATE_PLAYING;
}

bool CPlatformNetworkManagerSony::IsReadyToPlayOrIdle()
{
	return m_pSQRNet->IsReadyToPlayOrIdle();
}

void CPlatformNetworkManagerSony::SetSQRPresenceInfoFromExtData(SQRNetworkManager::PresenceSyncInfo *presence, void *pExtData, SceNpMatching2RoomId roomId, SceNpMatching2ServerId serverId)
{
	GameSessionData *gsd = (GameSessionData *)pExtData;

	memcpy(&presence->hostPlayerUID, &gsd->hostPlayerUID, sizeof(GameSessionUID) );
	presence->m_RoomId   = roomId;
	presence->m_ServerId = serverId;
	presence->texturePackParentId = gsd->texturePackParentId;
	presence->subTexturePackId	  = gsd->subTexturePackId;
	presence->netVersion = gsd->netVersion;
	presence->inviteOnly = !gsd->isJoinable;
}

void CPlatformNetworkManagerSony::MallocAndSetExtDataFromSQRPresenceInfo(void **pExtData, SQRNetworkManager::PresenceSyncInfo *presence)
{
	GameSessionData *gsd = (GameSessionData *)malloc(sizeof(GameSessionData));
	memset(gsd, 0, sizeof(GameSessionData));
	if( presence->netVersion != 0 )
	{
		memcpy(&gsd->hostPlayerUID, &presence->hostPlayerUID, sizeof(GameSessionUID) );
		gsd->texturePackParentId = presence->texturePackParentId;
		gsd->subTexturePackId = presence->subTexturePackId;
		gsd->netVersion = presence->netVersion;
		gsd->isJoinable = !presence->inviteOnly;
		gsd->isReadyToJoin = true;
	}
	*pExtData = gsd;
}

#ifdef __PSVITA__
bool CPlatformNetworkManagerSony::setAdhocMode( bool bAdhoc )
{
	if(m_bUsingAdhocMode != bAdhoc)
	{
		m_bUsingAdhocMode = bAdhoc;
		if(m_bUsingAdhocMode)
		{
			// uninit the PSN, and init adhoc
			if(m_pSQRNet_Vita->IsInitialised())
			{
				m_pSQRNet_Vita->UnInitialise();
			}

			if(m_pSQRNet_Vita_Adhoc->IsInitialised()==false)
			{
				m_pSQRNet_Vita_Adhoc->Initialise();
			}

			m_pSQRNet = m_pSQRNet_Vita_Adhoc;
		}
		else
		{
			if(m_pSQRNet_Vita_Adhoc->IsInitialised())
			{
				int ret = sceNetCtlAdhocDisconnect();
				// uninit the adhoc, and init psn
				m_pSQRNet_Vita_Adhoc->UnInitialise();
			}

			if(m_pSQRNet_Vita->IsInitialised()==false)
			{
				m_pSQRNet_Vita->Initialise();
			}

			m_pSQRNet = m_pSQRNet_Vita;
		}
	}

	return true;
}

void CPlatformNetworkManagerSony::startAdhocMatching( )
{
	assert(m_pSQRNet == m_pSQRNet_Vita_Adhoc);
	((SQRNetworkManager_AdHoc_Vita*)m_pSQRNet_Vita_Adhoc)->startMatching();
}

bool CPlatformNetworkManagerSony::checkValidInviteData(const INVITE_INFO* pInviteInfo)
{
	SQRNetworkManager_Vita* pSQR = (SQRNetworkManager_Vita*)m_pSQRNet_Vita;
	if(pSQR->IsOnlineGame() && !pSQR->IsHost()&& (pSQR->GetHostUID() == pInviteInfo->hostPlayerUID))
	{
		// we're trying to join a game we're already in, so we just ignore this
		return false;
	}
	else
	{
		return true;
	}
}



#endif // __PSVITA__
