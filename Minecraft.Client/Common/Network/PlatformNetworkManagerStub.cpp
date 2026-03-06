#include "stdafx.h"
#include "..\..\..\Minecraft.World\Socket.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "PlatformNetworkManagerStub.h"
#include "..\..\Xbox\Network\NetworkPlayerXbox.h"
#ifdef _WINDOWS64
#include "..\..\Windows64\Network\WinsockNetLayer.h"
#include "..\..\Windows64\Windows64_Xuid.h"
#include "..\..\Minecraft.h"
#include "..\..\User.h"
#include <iostream>
#endif

CPlatformNetworkManagerStub *g_pPlatformNetworkManager;


void CPlatformNetworkManagerStub::NotifyPlayerJoined(IQNetPlayer *pQNetPlayer	)
{
	const char * pszDescription;

	// 4J Stu - We create a fake socket for every where that we need an INBOUND queue of game data. Outbound
	// is all handled by QNet so we don't need that. Therefore each client player has one, and the host has one
	// for each client player.
	bool createFakeSocket = false;
	bool localPlayer = false;

	NetworkPlayerXbox *networkPlayer = (NetworkPlayerXbox *)addNetworkPlayer(pQNetPlayer);

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
            for (auto& pQNetPrimaryPlayer : m_machineQNetPrimaryPlayers)
            {
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
//		g_NetworkManager.UpdateAndSetGameSessionData();
		SystemFlagAddPlayer( networkPlayer );
	}

	for( int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
	{
		if(playerChangedCallback[idx] != NULL)
			playerChangedCallback[idx]( playerChangedCallbackParam[idx], networkPlayer, false );
	}

	if(m_pIQNet->GetState() == QNET_STATE_GAME_PLAY)
	{
		int localPlayerCount = 0;
		for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
		{
			if( m_pIQNet->GetLocalPlayerByUserIndex(idx) != NULL ) ++localPlayerCount;
		}

		float appTime = app.getAppTime();

		// Only record stats for the primary player here
		m_lastPlayerEventTimeStart = appTime;
	}
}

void CPlatformNetworkManagerStub::NotifyPlayerLeaving(IQNetPlayer* pQNetPlayer)
{
	app.DebugPrintf("Player 0x%p \"%ls\" leaving.\n", pQNetPlayer, pQNetPlayer->GetGamertag());

	INetworkPlayer* networkPlayer = getNetworkPlayer(pQNetPlayer);
	if (networkPlayer == NULL)
		return;

	Socket* socket = networkPlayer->GetSocket();
	if (socket != NULL)
	{
		if (m_pIQNet->IsHost())
			g_NetworkManager.CloseConnection(networkPlayer);
	}

	if (m_pIQNet->IsHost())
	{
		SystemFlagRemovePlayer(networkPlayer);
	}

	g_NetworkManager.PlayerLeaving(networkPlayer);

	for (int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
	{
		if (playerChangedCallback[idx] != NULL)
			playerChangedCallback[idx](playerChangedCallbackParam[idx], networkPlayer, true);
	}

	removeNetworkPlayer(pQNetPlayer);
}


bool CPlatformNetworkManagerStub::Initialise(CGameNetworkManager *pGameNetworkManager, int flagIndexSize)
{
	m_pGameNetworkManager = pGameNetworkManager;
	m_flagIndexSize = flagIndexSize;
	m_pIQNet = new IQNet();
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

	for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		m_searchResultsCount[i] = 0;
		m_lastSearchStartTime[i] = 0;

		// The results that will be filled in with the current search
		m_pSearchResults[i] = NULL;
		m_pQoSResult[i] = NULL;
		m_pCurrentSearchResults[i] = NULL;
		m_pCurrentQoSResult[i] = NULL;
		m_currentSearchResultsCount[i] = 0;
	}

    // Success!
    return true;
}

void CPlatformNetworkManagerStub::Terminate()
{
}

int CPlatformNetworkManagerStub::GetJoiningReadyPercentage()
{
	return 100;
}

int CPlatformNetworkManagerStub::CorrectErrorIDS(int IDS)
{
	return IDS;
}

bool CPlatformNetworkManagerStub::isSystemPrimaryPlayer(IQNetPlayer *pQNetPlayer)
{
	return true;
}

// We call this twice a frame, either side of the render call so is a good place to "tick" things
void CPlatformNetworkManagerStub::DoWork()
{
#ifdef _WINDOWS64
	extern QNET_STATE _iQNetStubState;
	if (_iQNetStubState == QNET_STATE_SESSION_STARTING && app.GetGameStarted())
	{
		_iQNetStubState = QNET_STATE_GAME_PLAY;
		if (m_pIQNet->IsHost())
			WinsockNetLayer::UpdateAdvertiseJoinable(true);
	}
	if (_iQNetStubState == QNET_STATE_IDLE)
		TickSearch();
	if (_iQNetStubState == QNET_STATE_GAME_PLAY && m_pIQNet->IsHost())
	{
		BYTE disconnectedSmallId;
		while (WinsockNetLayer::PopDisconnectedSmallId(&disconnectedSmallId))
		{
			IQNetPlayer* qnetPlayer = m_pIQNet->GetPlayerBySmallId(disconnectedSmallId);
			if (qnetPlayer != NULL && qnetPlayer->m_smallId == disconnectedSmallId)
			{
				NotifyPlayerLeaving(qnetPlayer);
				qnetPlayer->m_smallId = 0;
				qnetPlayer->m_isRemote = false;
				qnetPlayer->m_isHostPlayer = false;
				qnetPlayer->m_resolvedXuid = INVALID_XUID;
				qnetPlayer->m_gamertag[0] = 0;
				qnetPlayer->SetCustomDataValue(0);
				WinsockNetLayer::PushFreeSmallId(disconnectedSmallId);
				if (IQNet::s_playerCount > 1)
					IQNet::s_playerCount--;
			}
		}
	}
#endif
}

int CPlatformNetworkManagerStub::GetPlayerCount()
{
	return m_pIQNet->GetPlayerCount();
}

bool CPlatformNetworkManagerStub::ShouldMessageForFullSession()
{
	return false;
}

int CPlatformNetworkManagerStub::GetOnlinePlayerCount()
{
	return 1;
}

int CPlatformNetworkManagerStub::GetLocalPlayerMask(int playerIndex)
{
	return 1 << playerIndex;
}

bool CPlatformNetworkManagerStub::AddLocalPlayerByUserIndex( int userIndex )
{
	NotifyPlayerJoined(m_pIQNet->GetLocalPlayerByUserIndex(userIndex));
	return ( m_pIQNet->AddLocalPlayerByUserIndex(userIndex) == S_OK );
}

bool CPlatformNetworkManagerStub::RemoveLocalPlayerByUserIndex( int userIndex )
{
	return true;
}

bool CPlatformNetworkManagerStub::IsInStatsEnabledSession()
{
	return true;
}

bool CPlatformNetworkManagerStub::SessionHasSpace(unsigned int spaceRequired /*= 1*/)
{
	return true;
}

void CPlatformNetworkManagerStub::SendInviteGUI(int quadrant)
{
}

bool CPlatformNetworkManagerStub::IsAddingPlayer()
{
	return false;
}

bool CPlatformNetworkManagerStub::LeaveGame(bool bMigrateHost)
{
	if( m_bLeavingGame ) return true;

	m_bLeavingGame = true;

#ifdef _WINDOWS64
	WinsockNetLayer::StopAdvertising();
#endif

	// If we are the host wait for the game server to end
	if(m_pIQNet->IsHost() && g_NetworkManager.ServerStoppedValid())
	{
		m_pIQNet->EndGame();
		g_NetworkManager.ServerStoppedWait();
		g_NetworkManager.ServerStoppedDestroy();
	}
	else
	{
		m_pIQNet->EndGame();
	}

	for (auto & it : currentNetworkPlayers)
		delete it;
	currentNetworkPlayers.clear();
	m_machineQNetPrimaryPlayers.clear();
	SystemFlagReset();

#ifdef _WINDOWS64
	WinsockNetLayer::Shutdown();
	WinsockNetLayer::Initialize();
#endif

	return true;
}

bool CPlatformNetworkManagerStub::_LeaveGame(bool bMigrateHost, bool bLeaveRoom)
{
	return true;
}

void CPlatformNetworkManagerStub::HostGame(int localUsersMask, bool bOnlineGame, bool bIsPrivate, unsigned char publicSlots /*= MINECRAFT_NET_MAX_PLAYERS*/, unsigned char privateSlots /*= 0*/)
{
// #ifdef _XBOX
	// 4J Stu - We probably did this earlier as well, but just to be sure!
	SetLocalGame( !bOnlineGame );
	SetPrivateGame( bIsPrivate );
	SystemFlagReset();

	// Make sure that the Primary Pad is in by default
	localUsersMask |= GetLocalPlayerMask( g_NetworkManager.GetPrimaryPad() );

	m_bLeavingGame = false;

	m_pIQNet->HostGame();

#ifdef _WINDOWS64
	IQNet::m_player[0].m_smallId = 0;
	IQNet::m_player[0].m_isRemote = false;
	// world host is pinned to legacy host XUID to keep old player data compatibility.
	IQNet::m_player[0].m_isHostPlayer = true;
	IQNet::m_player[0].m_resolvedXuid = Win64Xuid::GetLegacyEmbeddedHostXuid();
	IQNet::s_playerCount = 1;
#endif

	_HostGame( localUsersMask, publicSlots, privateSlots );

#ifdef _WINDOWS64
	int port = WIN64_NET_DEFAULT_PORT;
	const char* bindIp = NULL;
	if (g_Win64DedicatedServer)
	{
		if (g_Win64DedicatedServerPort > 0)
			port = g_Win64DedicatedServerPort;
		if (g_Win64DedicatedServerBindIP[0] != 0)
			bindIp = g_Win64DedicatedServerBindIP;
	}
	if (!WinsockNetLayer::IsActive())
		WinsockNetLayer::HostGame(port, bindIp);

	if (WinsockNetLayer::IsActive())
	{
		const wchar_t* hostName = IQNet::m_player[0].m_gamertag;
		unsigned int settings = app.GetGameHostOption(eGameHostOption_All);
		WinsockNetLayer::StartAdvertising(port, hostName, settings, 0, 0, MINECRAFT_NET_VERSION);
	}
#endif
//#endif
}

void CPlatformNetworkManagerStub::_HostGame(int usersMask, unsigned char publicSlots /*= MINECRAFT_NET_MAX_PLAYERS*/, unsigned char privateSlots /*= 0*/)
{
}

bool CPlatformNetworkManagerStub::_StartGame()
{
	return true;
}

int CPlatformNetworkManagerStub::JoinGame(FriendSessionInfo* searchResult, int localUsersMask, int primaryUserIndex)
{
#ifdef _WINDOWS64
	if (searchResult == NULL)
		return CGameNetworkManager::JOINGAME_FAIL_GENERAL;

	const char* hostIP = searchResult->data.hostIP;
	int hostPort = searchResult->data.hostPort;

	if (hostPort <= 0 || hostIP[0] == 0)
		return CGameNetworkManager::JOINGAME_FAIL_GENERAL;

	m_bLeavingGame = false;
	IQNet::s_isHosting = false;
	m_pIQNet->ClientJoinGame();

	IQNet::m_player[0].m_smallId = 0;
	IQNet::m_player[0].m_isRemote = true;
	IQNet::m_player[0].m_isHostPlayer = true;
	// Remote host still maps to legacy host XUID in mixed old/new sessions.
	IQNet::m_player[0].m_resolvedXuid = Win64Xuid::GetLegacyEmbeddedHostXuid();
	wcsncpy_s(IQNet::m_player[0].m_gamertag, 32, searchResult->data.hostName, _TRUNCATE);

	WinsockNetLayer::StopDiscovery();

	if (!WinsockNetLayer::JoinGame(hostIP, hostPort))
	{
		app.DebugPrintf("Win64 LAN: Failed to connect to %s:%d\n", hostIP, hostPort);
		return CGameNetworkManager::JOINGAME_FAIL_GENERAL;
	}

	BYTE localSmallId = WinsockNetLayer::GetLocalSmallId();

	IQNet::m_player[localSmallId].m_smallId = localSmallId;
	IQNet::m_player[localSmallId].m_isRemote = false;
	IQNet::m_player[localSmallId].m_isHostPlayer = false;
	// Local non-host identity is the persistent uid.dat XUID.
	IQNet::m_player[localSmallId].m_resolvedXuid = Win64Xuid::ResolvePersistentXuid();

	Minecraft* pMinecraft = Minecraft::GetInstance();
	wcscpy_s(IQNet::m_player[localSmallId].m_gamertag, 32, pMinecraft->user->name.c_str());
	IQNet::s_playerCount = localSmallId + 1;

	NotifyPlayerJoined(&IQNet::m_player[0]);
	NotifyPlayerJoined(&IQNet::m_player[localSmallId]);

	m_pGameNetworkManager->StateChange_AnyToStarting();

	return CGameNetworkManager::JOINGAME_SUCCESS;
#else
	return CGameNetworkManager::JOINGAME_SUCCESS;
#endif
}

bool CPlatformNetworkManagerStub::SetLocalGame(bool isLocal)
{
	m_bIsOfflineGame = isLocal;

	return true;
}

void CPlatformNetworkManagerStub::SetPrivateGame(bool isPrivate)
{
	app.DebugPrintf("Setting as private game: %s\n", isPrivate ? "yes" : "no" );
	m_bIsPrivateGame = isPrivate;
}

void CPlatformNetworkManagerStub::RegisterPlayerChangedCallback(int iPad, void (*callback)(void *callbackParam, INetworkPlayer *pPlayer, bool leaving), void *callbackParam)
{
	playerChangedCallback[iPad] = callback;
	playerChangedCallbackParam[iPad] = callbackParam;
}

void CPlatformNetworkManagerStub::UnRegisterPlayerChangedCallback(int iPad, void (*callback)(void *callbackParam, INetworkPlayer *pPlayer, bool leaving), void *callbackParam)
{
	if(playerChangedCallbackParam[iPad] == callbackParam)
	{
		playerChangedCallback[iPad] = NULL;
		playerChangedCallbackParam[iPad] = NULL;
	}
}

void CPlatformNetworkManagerStub::HandleSignInChange()
{
	return;
}

bool CPlatformNetworkManagerStub::_RunNetworkGame()
{
#ifdef _WINDOWS64
	extern QNET_STATE _iQNetStubState;
	_iQNetStubState = QNET_STATE_GAME_PLAY;

	for (DWORD i = 0; i < IQNet::s_playerCount; i++)
	{
		if (IQNet::m_player[i].m_isRemote)
		{
			INetworkPlayer* pNetworkPlayer = getNetworkPlayer(&IQNet::m_player[i]);
			if (pNetworkPlayer != NULL && pNetworkPlayer->GetSocket() != NULL)
			{
				Socket::addIncomingSocket(pNetworkPlayer->GetSocket());
			}
		}
	}
#endif
	return true;
}

void CPlatformNetworkManagerStub::UpdateAndSetGameSessionData(INetworkPlayer *pNetworkPlayerLeaving /*= NULL*/)
{
// 	DWORD playerCount = m_pIQNet->GetPlayerCount();
//
// 	if( this->m_bLeavingGame )
// 		return;
//
// 	if( GetHostPlayer() == NULL )
// 		return;
//
// 	for(unsigned int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; ++i)
// 	{
// 		if( i < playerCount )
// 		{
// 			INetworkPlayer *pNetworkPlayer = GetPlayerByIndex(i);
//
// 			// We can call this from NotifyPlayerLeaving but at that point the player is still considered in the session
// 			if( pNetworkPlayer != pNetworkPlayerLeaving )
// 			{
// 				m_hostGameSessionData.players[i] = ((NetworkPlayerXbox *)pNetworkPlayer)->GetUID();
//
// 				char *temp;
// 				temp = (char *)wstringtofilename( pNetworkPlayer->GetOnlineName() );
// 				memcpy(m_hostGameSessionData.szPlayers[i],temp,XUSER_NAME_SIZE);
// 			}
// 			else
// 			{
// 				m_hostGameSessionData.players[i] = NULL;
// 				memset(m_hostGameSessionData.szPlayers[i],0,XUSER_NAME_SIZE);
// 			}
// 		}
// 		else
// 		{
// 			m_hostGameSessionData.players[i] = NULL;
// 			memset(m_hostGameSessionData.szPlayers[i],0,XUSER_NAME_SIZE);
// 		}
// 	}
//
// 	m_hostGameSessionData.hostPlayerUID = ((NetworkPlayerXbox *)GetHostPlayer())->GetQNetPlayer()->GetXuid();
// 	m_hostGameSessionData.m_uiGameHostSettings = app.GetGameHostOption(eGameHostOption_All);
}

int CPlatformNetworkManagerStub::RemovePlayerOnSocketClosedThreadProc( void* lpParam )
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

bool CPlatformNetworkManagerStub::RemoveLocalPlayer( INetworkPlayer *pNetworkPlayer )
{
	return true;
}

CPlatformNetworkManagerStub::PlayerFlags::PlayerFlags(INetworkPlayer *pNetworkPlayer, unsigned int count)
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
CPlatformNetworkManagerStub::PlayerFlags::~PlayerFlags()
{
	delete [] flags;
}

// Add a player to the per system flag storage - if we've already got a player from that system, copy its flags over
void CPlatformNetworkManagerStub::SystemFlagAddPlayer(INetworkPlayer *pNetworkPlayer)
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
void CPlatformNetworkManagerStub::SystemFlagRemovePlayer(INetworkPlayer *pNetworkPlayer)
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

void CPlatformNetworkManagerStub::SystemFlagReset()
{
	for( unsigned int i = 0; i < m_playerFlags.size(); i++ )
	{
		delete m_playerFlags[i];
	}
	m_playerFlags.clear();
}

// Set a per system flag - this is done by setting the flag on every player that shares that system
void CPlatformNetworkManagerStub::SystemFlagSet(INetworkPlayer *pNetworkPlayer, int index)
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
bool CPlatformNetworkManagerStub::SystemFlagGet(INetworkPlayer *pNetworkPlayer, int index)
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

wstring CPlatformNetworkManagerStub::GatherStats()
{
	return L"";
}

wstring CPlatformNetworkManagerStub::GatherRTTStats()
{
	wstring stats(L"Rtt: ");

	wchar_t stat[32];

	for(unsigned int i = 0; i < GetPlayerCount(); ++i)
	{
		IQNetPlayer *pQNetPlayer = ((NetworkPlayerXbox *)GetPlayerByIndex( i ))->GetQNetPlayer();

		if(!pQNetPlayer->IsLocal())
		{
			ZeroMemory(stat,32*sizeof(WCHAR));
			swprintf(stat, 32, L"%d: %d/", i, pQNetPlayer->GetCurrentRtt() );
			stats.append(stat);
		}
	}
	return stats;
}

void CPlatformNetworkManagerStub::TickSearch()
{
#ifdef _WINDOWS64
	if (m_SessionsUpdatedCallback == NULL)
		return;

	static DWORD lastSearchTime = 0;
	DWORD now = GetTickCount();
	if (now - lastSearchTime < 2000)
		return;
	lastSearchTime = now;

	SearchForGames();
#endif
}

void CPlatformNetworkManagerStub::SearchForGames()
{
#ifdef _WINDOWS64
	std::vector<Win64LANSession> lanSessions = WinsockNetLayer::GetDiscoveredSessions();

	//THEY GET DELETED HERE DAMMIT
	for (size_t i = 0; i < friendsSessions[0].size(); i++)
		delete friendsSessions[0][i];
	friendsSessions[0].clear();

	for (size_t i = 0; i < lanSessions.size(); i++)
	{
		FriendSessionInfo* info = new FriendSessionInfo();
		size_t nameLen = wcslen(lanSessions[i].hostName);
		info->displayLabel = new wchar_t[nameLen + 1];
		wcscpy_s(info->displayLabel, nameLen + 1, lanSessions[i].hostName);
		info->displayLabelLength = (unsigned char)nameLen;
		info->displayLabelViewableStartIndex = 0;

		info->data.netVersion = lanSessions[i].netVersion;
		info->data.m_uiGameHostSettings = lanSessions[i].gameHostSettings;
		info->data.texturePackParentId = lanSessions[i].texturePackParentId;
		info->data.subTexturePackId = lanSessions[i].subTexturePackId;
		info->data.isReadyToJoin = lanSessions[i].isJoinable;
		info->data.isJoinable = lanSessions[i].isJoinable;
		strncpy_s(info->data.hostIP, sizeof(info->data.hostIP), lanSessions[i].hostIP, _TRUNCATE);
		info->data.hostPort = lanSessions[i].hostPort;
		wcsncpy_s(info->data.hostName, XUSER_NAME_SIZE, lanSessions[i].hostName, _TRUNCATE);
		info->data.playerCount = lanSessions[i].playerCount;
		info->data.maxPlayers = lanSessions[i].maxPlayers;

		info->sessionId = (SessionID)((uint64_t)inet_addr(lanSessions[i].hostIP) | ((uint64_t)lanSessions[i].hostPort << 32));

		friendsSessions[0].push_back(info);
	}

	std::FILE* file = std::fopen("servers.txt", "r");

	if (file) {
		wstring wline;
		int phase = 0;

		string ip;
		wstring port;
		wstring name;

		char buffer[512];
		while (std::fgets(buffer, sizeof(buffer), file)) {
			if (phase == 0) {
				ip = buffer;
				if (!ip.empty() && (ip.back() == '\n' || ip.back() == '\r'))
					ip.pop_back();
				phase = 1;
			}
			else if (phase == 1) {
				wline = convStringToWstring(buffer);
				port = wline;
				phase = 2;
			}
			else if (phase == 2) {
				wline = convStringToWstring(buffer);
				name = wline;
				phase = 0;

				//THEY GET DELETED AFTER USE LIKE 30 LINES UP!!
				FriendSessionInfo* info = new FriendSessionInfo();
				wchar_t label[128];
				wcsncpy_s(label, sizeof(label)/sizeof(wchar_t), name.c_str(), _TRUNCATE);
				size_t nameLen = wcslen(label);
				info->displayLabel = new wchar_t[nameLen+1];
				wcscpy_s(info->displayLabel, nameLen + 1, label);
				info->displayLabelLength = (unsigned char)nameLen;
				info->displayLabelViewableStartIndex = 0;
				info->data.isReadyToJoin = true;
				info->data.isJoinable = true;
				strncpy_s(info->data.hostIP, sizeof(info->data.hostIP), ip.c_str(), _TRUNCATE);
				info->data.hostPort = stoi(port);
				info->sessionId = (SessionID)(static_cast<uint64_t>(inet_addr(ip.c_str())) | (static_cast<uint64_t>(stoi(port)) << 32));
				friendsSessions[0].push_back(info);
			}
		}

		std::fclose(file);
	}

	m_searchResultsCount[0] = (int)friendsSessions[0].size();

	if (m_SessionsUpdatedCallback != NULL)
		m_SessionsUpdatedCallback(m_pSearchParam);
#endif
}

int CPlatformNetworkManagerStub::SearchForGamesThreadProc( void* lpParameter )
{
	return 0;
}

void CPlatformNetworkManagerStub::SetSearchResultsReady(int resultCount)
{
	m_bSearchResultsReady = true;
	m_searchResultsCount[m_lastSearchPad] = resultCount;
}

vector<FriendSessionInfo *> *CPlatformNetworkManagerStub::GetSessionList(int iPad, int localPlayers, bool partyOnly)
{
	vector<FriendSessionInfo*>* filteredList = new vector<FriendSessionInfo*>();
	for (size_t i = 0; i < friendsSessions[0].size(); i++)
		filteredList->push_back(friendsSessions[0][i]);
	return filteredList;
}

bool CPlatformNetworkManagerStub::GetGameSessionInfo(int iPad, SessionID sessionId, FriendSessionInfo *foundSessionInfo)
{
	return false;
}

void CPlatformNetworkManagerStub::SetSessionsUpdatedCallback( void (*SessionsUpdatedCallback)(LPVOID pParam), LPVOID pSearchParam )
{
	m_SessionsUpdatedCallback = SessionsUpdatedCallback; m_pSearchParam = pSearchParam;
}

void CPlatformNetworkManagerStub::GetFullFriendSessionInfo( FriendSessionInfo *foundSession, void (* FriendSessionUpdatedFn)(bool success, void *pParam), void *pParam )
{
	FriendSessionUpdatedFn(true, pParam);
}

void CPlatformNetworkManagerStub::ForceFriendsSessionRefresh()
{
	app.DebugPrintf("Resetting friends session search data\n");

	for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		m_searchResultsCount[i] = 0;
		m_lastSearchStartTime[i] = 0;
		delete m_pSearchResults[i];
		m_pSearchResults[i] = NULL;
	}
}

INetworkPlayer *CPlatformNetworkManagerStub::addNetworkPlayer(IQNetPlayer *pQNetPlayer)
{
	NetworkPlayerXbox *pNetworkPlayer = new NetworkPlayerXbox(pQNetPlayer);
	pQNetPlayer->SetCustomDataValue((ULONG_PTR)pNetworkPlayer);
	currentNetworkPlayers.push_back( pNetworkPlayer );
	return pNetworkPlayer;
}

void CPlatformNetworkManagerStub::removeNetworkPlayer(IQNetPlayer *pQNetPlayer)
{
	INetworkPlayer *pNetworkPlayer = getNetworkPlayer(pQNetPlayer);
    for (auto it = currentNetworkPlayers.begin(); it != currentNetworkPlayers.end(); it++)
    {
		if( *it == pNetworkPlayer )
		{
			currentNetworkPlayers.erase(it);
			return;
		}
	}
}

INetworkPlayer *CPlatformNetworkManagerStub::getNetworkPlayer(IQNetPlayer *pQNetPlayer)
{
	return pQNetPlayer ? (INetworkPlayer *)(pQNetPlayer->GetCustomDataValue()) : NULL;
}


INetworkPlayer *CPlatformNetworkManagerStub::GetLocalPlayerByUserIndex(int userIndex )
{
	return getNetworkPlayer(m_pIQNet->GetLocalPlayerByUserIndex(userIndex));
}

INetworkPlayer *CPlatformNetworkManagerStub::GetPlayerByIndex(int playerIndex)
{
	return getNetworkPlayer(m_pIQNet->GetPlayerByIndex(playerIndex));
}

INetworkPlayer * CPlatformNetworkManagerStub::GetPlayerByXuid(PlayerUID xuid)
{
	return getNetworkPlayer( m_pIQNet->GetPlayerByXuid(xuid)) ;
}

INetworkPlayer * CPlatformNetworkManagerStub::GetPlayerBySmallId(unsigned char smallId)
{
	return getNetworkPlayer(m_pIQNet->GetPlayerBySmallId(smallId));
}

INetworkPlayer *CPlatformNetworkManagerStub::GetHostPlayer()
{
	return getNetworkPlayer(m_pIQNet->GetHostPlayer());
}

bool CPlatformNetworkManagerStub::IsHost()
{
	return m_pIQNet->IsHost() && !m_bHostChanged;
}

bool CPlatformNetworkManagerStub::JoinGameFromInviteInfo( int userIndex, int userMask, const INVITE_INFO *pInviteInfo)
{
	return ( m_pIQNet->JoinGameFromInviteInfo( userIndex, userMask, pInviteInfo ) == S_OK);
}

void CPlatformNetworkManagerStub::SetSessionTexturePackParentId( int id )
{
	m_hostGameSessionData.texturePackParentId = id;
}

void CPlatformNetworkManagerStub::SetSessionSubTexturePackId( int id )
{
	m_hostGameSessionData.subTexturePackId = id;
}

void CPlatformNetworkManagerStub::Notify(int ID, ULONG_PTR Param)
{
}

bool CPlatformNetworkManagerStub::IsInSession()
{
	return m_pIQNet->GetState() != QNET_STATE_IDLE;
}

bool CPlatformNetworkManagerStub::IsInGameplay()
{
	return m_pIQNet->GetState() == QNET_STATE_GAME_PLAY;
}

bool CPlatformNetworkManagerStub::IsReadyToPlayOrIdle()
{
	return true;
}
