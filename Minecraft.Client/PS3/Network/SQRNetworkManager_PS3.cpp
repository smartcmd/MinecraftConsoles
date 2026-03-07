#include "stdafx.h"
#include "SQRNetworkManager_PS3.h"
#include "Common/Network/Sony/SQRNetworkPlayer.h"
#ifdef __PS3__
#include <sys/random_number.h>
#include <sys/event.h>
#include <cell/rudp.h>
#include <sysutil/sysutil_gamecontent.h>
#else
#include <rudp.h>
#include <np_toolkit.h>
#endif
#include "../Passphrase/ps3__np_conf.h"
#ifdef __PS3__
#include "SonyVoiceChat.h"
#include "Common/Network/Sony/SonyHttp.h"
#endif
#include "../../../Minecraft.World/C4JThread.h"
#include "../PS3Extras/PS3Strings.h"
#include "PS3/Network/SonyRemoteStorage_PS3.h"

int (* SQRNetworkManager_PS3::s_SignInCompleteCallbackFn)(void *pParam, bool bContinue, int pad) = NULL;
void * SQRNetworkManager_PS3::s_SignInCompleteParam = NULL;
SceNpBasicPresenceDetails2 SQRNetworkManager_PS3::s_lastPresenceInfo = { 0 };
int SQRNetworkManager_PS3::s_resendPresenceCountdown = 0;
bool SQRNetworkManager_PS3::s_presenceStatusDirty = false;
bool SQRNetworkManager_PS3::s_presenceDataDirty = false;
bool SQRNetworkManager_PS3::s_signInCompleteCallbackIfFailed = false;
SQRNetworkManager_PS3::PresenceSyncInfo SQRNetworkManager_PS3::s_lastPresenceSyncInfo = { 0 };
SQRNetworkManager_PS3::PresenceSyncInfo SQRNetworkManager_PS3::c_presenceSyncInfoNULL = { 0 };
SceNpBasicAttachmentDataId SQRNetworkManager_PS3::s_lastInviteIdToRetry = SCE_NP_BASIC_INVALID_ATTACHMENT_DATA_ID;
long long SQRNetworkManager_PS3::s_roomStartTime = 0;
bool SQRNetworkManager_PS3::m_bCallPSNSignInCallback=false;


//unsigned int SQRNetworkManager_PS3::RoomSyncData::playerCount = 0;

// This maps internal to extern states, and needs to match element-by-element the eSQRNetworkManagerInternalState enumerated type
const SQRNetworkManager_PS3::eSQRNetworkManagerState SQRNetworkManager_PS3::m_INTtoEXTStateMappings[SQRNetworkManager_PS3::SNM_INT_STATE_COUNT] =
{
	SNM_STATE_INITIALISING,			// SNM_INT_STATE_UNINITIALISED
	SNM_STATE_INITIALISING,			// SNM_INT_STATE_SIGNING_IN
	SNM_STATE_INITIALISING,			// SNM_INT_STATE_STARTING_CONTEXT
	SNM_STATE_INITIALISE_FAILED,	// SNM_INT_STATE_INITIALISE_FAILED
	SNM_STATE_IDLE,					// SNM_INT_STATE_IDLE
	SNM_STATE_IDLE,					// SNM_INT_STATE_IDLE_RECREATING_MATCHING_CONTEXT
	SNM_STATE_HOSTING,				// SNM_INT_STATE_HOSTING_STARTING_MATCHING_CONTEXT
	SNM_STATE_HOSTING,				// SNM_INT_STATE_HOSTING_SEARCHING_FOR_SERVER
	SNM_STATE_HOSTING,				// SNM_INT_STATE_HOSTING_SERVER_SEARCH_SERVER_ERROR
	SNM_STATE_HOSTING,				// SNM_INT_STATE_HOSTING_SERVER_FOUND
	SNM_STATE_HOSTING,				// SNM_INT_STATE_HOSTING_SERVER_SEARCH_CREATING_CONTEXT
	SNM_STATE_HOSTING,				// SNM_INT_STATE_HOSTING_SERVER_SEARCH_FAILED
	SNM_STATE_HOSTING,				// SNM_INT_STATE_HOSTING_CREATE_ROOM_SEARCHING_FOR_WORLD
	SNM_STATE_HOSTING,				// SNM_INT_STATE_HOSTING_CREATE_ROOM_WORLD_FOUND
	SNM_STATE_HOSTING,				// SNM_INT_STATE_HOSTING_CREATE_ROOM_CREATING_ROOM
	SNM_STATE_HOSTING,				// SNM_INT_STATE_HOSTING_CREATE_ROOM_SUCCESS
	SNM_STATE_HOSTING,				// SNM_INT_STATE_HOSTING_CREATE_ROOM_FAILED
	SNM_STATE_HOSTING,				// SNM_INT_STATE_HOSTING_CREATE_ROOM_RESTART_MATCHING_CONTEXT
	SNM_STATE_HOSTING,				// SNM_INT_STATE_HOSTING_WAITING_TO_PLAY
	SNM_STATE_JOINING,				// SNM_INT_STATE_JOINING_STARTING_MATCHING_CONTEXT
	SNM_STATE_JOINING,				// SNM_INT_STATE_JOINING_SEARCHING_FOR_SERVER
	SNM_STATE_JOINING,				// SNM_INT_STATE_JOINING_SERVER_SEARCH_SERVER_ERROR
	SNM_STATE_JOINING,				// SNM_INT_STATE_JOINING_SERVER_FOUND
	SNM_STATE_JOINING,				// SNM_INT_STATE_JOINING_SERVER_SEARCH_CREATING_CONTEXT
	SNM_STATE_JOINING,				// SNM_INT_STATE_JOINING_SERVER_SEARCH_FAILED
	SNM_STATE_JOINING,				// SNM_INT_STATE_JOINING_JOIN_ROOM
	SNM_STATE_JOINING,				// SNM_INT_STATE_JOINING_JOIN_ROOM_FAILED
	SNM_STATE_JOINING,				// SNM_INT_STATE_JOINING_WAITING_FOR_LOCAL_PLAYERS
	SNM_STATE_ENDING,				// SNM_INT_STATE_SERVER_DELETING_CONTEXT
	SNM_STATE_STARTING,				// SNM_INT_STATE_STARTING
	SNM_STATE_PLAYING,				// SNM_INT_STATE_PLAYING
	SNM_STATE_LEAVING,				// SNM_INT_STATE_LEAVING
	SNM_STATE_LEAVING,				// SNM_INT_STATE_LEAVING_FAILED
	SNM_STATE_ENDING,				// SNM_INT_STATE_ENDING
};

SQRNetworkManager_PS3::SQRNetworkManager_PS3(ISQRNetworkManagerListener *listener)
{
	m_state = SNM_INT_STATE_UNINITIALISED;
	m_stateExternal = SNM_STATE_INITIALISING;
	m_nextIdleReasonIsFull = false;
	m_friendSearchState = SNM_FRIEND_SEARCH_STATE_IDLE;
	m_searchResultCount = 0;
	m_serverContextValid = false;
	m_isHosting = false;
	m_currentSmallId = 0;
	memset( m_aRoomSlotPlayers, 0, sizeof(m_aRoomSlotPlayers) );
	memset( m_aSearchResultRoomExtDataReceived, 0, sizeof( void *) * MAX_FRIENDS );
	m_listener = listener;
	m_soc = -1;
	m_resendExternalRoomDataCountdown = 0;
	m_matching2initialised = false;
	m_matchingContextValid = false;
	m_inviteIndex = 0;
	m_doBootInviteCheck = true;
	m_isInSession = false;
	m_offlineGame = false;
	m_offlineSQR = false;
	m_aServerId = NULL;
	m_gameBootInvite = NULL;
	m_onlineStatus = false;
	m_bLinkDisconnected = false;

	InitializeCriticalSection(&m_csRoomSyncData);
	InitializeCriticalSection(&m_csPlayerState);
	InitializeCriticalSection(&m_csStateChangeQueue);

	sys_event_port_create(&m_basicEventPort, SYS_EVENT_PORT_LOCAL, SYS_EVENT_PORT_NO_NAME);

	sys_event_queue_attribute_t queue_attr = {SYS_SYNC_PRIORITY, SYS_PPU_QUEUE};
	sys_event_queue_create(&m_basicEventQueue, &queue_attr, SYS_EVENT_QUEUE_LOCAL, 127);
	sys_event_port_connect_local(m_basicEventPort, m_basicEventQueue);

	m_basicEventThread = new C4JThread(&BasicEventThreadProc,this,"Basic Event Handler");
	m_basicEventThread->Run();
}

// First stage of initialisation. This initialises a few things that don't require the user to be signed in, and then kicks of the network start dialog utility.
// Initialisation continues in InitialiseAfterOnline once this completes.
void SQRNetworkManager_PS3::Initialise()
{
#ifdef __PS3__

#ifdef __PS3__
	sceNpBasicRegisterHandler( &s_npCommunicationId, BasicEventCallback, this );
#endif

	assert( m_state == SNM_INT_STATE_UNINITIALISED );

	//Initialize network
	int ret = sys_net_initialize_network();
	if( ( ret < 0 ) || ForceErrorPoint( SNM_FORCE_ERROR_NET_INITIALIZE_NETWORK ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return;
	}

	//Initialize libnetctl
	ret = cellNetCtlInit();
#else
	int ret = sceNetCtlInit();
#endif
	if( ( ret < 0 && ret != CELL_NET_CTL_ERROR_NOT_TERMINATED ) || ForceErrorPoint( SNM_FORCE_ERROR_NET_CTL_INIT ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return;
	}

	int hid;
	ret = cellNetCtlAddHandler(&NetCtlCallback,this,&hid);

	// Initialise RUDP
#ifdef __PS3__
	ret = cellRudpInit(NULL);
#else
	const int RUDP_POOL_SIZE = (500 * 1024);		// TODO - find out what we need, this size is copied from library reference
	uint8_t *rudp_pool = (uint8_t *)malloc(RUDP_POOL_SIZE);
	ret = sceRudpInit(rudp_pool, RUDP_POOL_SIZE);
#endif
	if( ( ret < 0 ) || ForceErrorPoint( SNM_FORCE_ERROR_RUDP_INIT ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return;
	}

#ifdef __PS3__
	ret = cellRudpEnableInternalIOThread(RUDP_THREAD_STACK_SIZE, RUDP_THREAD_PRIORITY);

	// Register for sys util callbacks... we need this for finding out what state the network dialog stuff has got to

	ret = cellSysutilRegisterCallback( 0, SysUtilCallback, this );

	// Kick off the Network Start Dialog Utility. This interacts with the user to make sure they are signed in & ready for network play.
	struct CellNetCtlNetStartDialogParam netstart_param;

	memset(&netstart_param, 0, sizeof(netstart_param));
	netstart_param.size = sizeof(netstart_param);
	netstart_param.type = CELL_NET_CTL_NETSTART_TYPE_NP;

	SetState(SNM_INT_STATE_SIGNING_IN);
	ret = cellNetCtlNetStartDialogLoadAsync(&netstart_param);
	if( ( ret < 0 ) || ForceErrorPoint( SNM_FORCE_ERROR_NET_START_DIALOG ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return;
	}
	SonyHttp::init();
#else
	ret = sceRudpEnableInternalIOThread(RUDP_THREAD_STACK_SIZE, RUDP_THREAD_PRIORITY);
#endif

	ret = sceNpManagerRegisterCallback(ManagerCallback, this);
	if( ( ret < 0 ) || ForceErrorPoint( SNM_FORCE_ERROR_REGISTER_NP_CALLBACK ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return;
	}

	int status;
	ret = sceNpManagerGetStatus( &status );
	UpdateOnlineStatus(status);

	// Already online? the callback won't catch this, so carry on initialising now
	if( ( ret == 0 ) && ( status == SCE_NP_MANAGER_STATUS_ONLINE ) )
	{
		InitialiseAfterOnline();
	}
}

void SQRNetworkManager_PS3::Terminate()
{
	// If playing, attempt to nicely leave the room before shutting down so that our friends won't still think this game is in progress
	if( ( m_state == SNM_INT_STATE_HOSTING_CREATE_ROOM_SUCCESS ) ||
		( m_state == SNM_INT_STATE_HOSTING_WAITING_TO_PLAY ) ||
		( m_state == SNM_INT_STATE_JOINING_WAITING_FOR_LOCAL_PLAYERS ) ||
		( m_state == SNM_INT_STATE_PLAYING ) )
	{
		if( !m_offlineGame )
		{
			LeaveRoom(true);
			int count = 200;
			do
			{
				Tick();
				Sleep(10);
				count--;
			} while( ( count > 0 ) && ( m_state != SNM_INT_STATE_IDLE ) );
			app.DebugPrintf(CMinecraftApp::USER_RR,"Attempted to leave room, %dms used\n",count * 10);
		}
	}

	int ret = cellRudpEnd();
	ret = sceNpMatching2Term2();
	// Terminate event thread by sending it a non-zero value for data1
	sys_event_port_send(m_basicEventPort, 1, 0, 0);

	do
	{
		Sleep(10);
	} while( m_basicEventThread->isRunning() );
}

// Second stage of initialisation, that requires NP Manager to be online & the player to be signed in. This kicks of the creation of a context
// for Np Matching 2. Initialisation is finally complete when we get a callback to ContextCallback. The SQRNetworkManager_PS3 is then finally moved
// into SNM_INT_STATE_IDLE at this stage.
void SQRNetworkManager_PS3::InitialiseAfterOnline()
{
	SceNpId npId;
	int option = 0;

	// We should only be doing this if we have come in from an initialisation stage (SQRNetworkManager_PS3::Initialise) or we've had a network disconnect and are coming in from an offline state.
	// Don't do anything otherwise - this mainly to catch a bit of a corner case in the initialisation phase where potentially we could register for the callback that would call this with sceNpManagerRegisterCallback.
	// and could then really quickly go online so that the there becomes two paths (via the callback or SQRNetworkManager_PS3::Initialise) by which this could be called
	if( ( m_state != SNM_INT_STATE_SIGNING_IN ) && !(( m_state == SNM_INT_STATE_IDLE ) && m_offlineSQR)  )
	{
		// If we aren't going to continue on with this sign-in but are expecting a callback, then let the game know that we have completed the bit we are expecting to do. This
		// will happen whilst in game, when we want to be able to sign into PSN, but don't expect the full matching stuff to get set up.
		if( s_SignInCompleteCallbackFn )
		{
			s_SignInCompleteCallbackFn(s_SignInCompleteParam,true,0);
			s_SignInCompleteCallbackFn  = NULL;
		}
		return;
	}

	// Initialize matching2 with default settings
#ifdef __PS3__
	int ret = 0;
	if( !m_matching2initialised)
	{
		ret = sceNpMatching2Init2(0, 0, NULL);
	}
#else
	SceNpMatching2InitializeParameter initParam;
	memset(&initParam,0,sizeof(initParam));
	initParam.size = sizeof(initParam);
	int ret = sceNpMatching2Initialize(&initParam);
#endif
	if( ( ret < 0 ) || ForceErrorPoint( SNM_FORCE_ERROR_MATCHING2_INIT ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return;
	}
	app.DebugPrintf("SQRNetworkManager::InitialiseAfterOnline - matching context is now valid\n");
	m_matching2initialised = true;

	// Get NP ID of the signed-in user
	ret = sceNpManagerGetNpId(&npId);
	if( ( ret < 0 ) || ForceErrorPoint( SNM_FORCE_ERROR_GET_NPID ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return;
	}

	//  Use Online Name and avatar to identify user
	option = (SCE_NP_MATCHING2_CONTEXT_OPTION_USE_ONLINENAME | SCE_NP_MATCHING2_CONTEXT_OPTION_USE_AVATARURL);

	// Create context
	ret = sceNpMatching2CreateContext(
			&npId, &s_npCommunicationId, &s_npCommunicationPassphrase, &m_matchingContext, option);
	if( ( ret < 0 ) || ForceErrorPoint( SNM_FORCE_ERROR_CREATE_MATCHING_CONTEXT ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return;
	}
	app.DebugPrintf("SQRNetworkManager::InitialiseAfterOnline - matching context is now valid\n");
	m_matchingContextValid = true;

	bool bRet = RegisterCallbacks();
	if( ( !bRet ) || ForceErrorPoint( SNM_FORCE_ERROR_REGISTER_CALLBACKS ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return;
	}

	// State should be starting context until the callback that this has been created happens
	SetState(SNM_INT_STATE_STARTING_CONTEXT);

	// Start the context
	// Set time-out time to 10 seconds
	ret = sceNpMatching2ContextStartAsync(m_matchingContext, (10*1000*1000));
	if( ( ret < 0 ) || ForceErrorPoint( SNM_FORCE_ERROR_CONTEXT_START_ASYNC ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
	}
}


// General tick function to be called from main game loop - any internal tick functions should be called from here.
void SQRNetworkManager_PS3::Tick()
{
	cellSysutilCheckCallback();
	ServerContextTick();
	RoomCreateTick();
	SonyVoiceChat::tick();
	FriendSearchTick();
	TickRichPresence();

	if( ( m_gameBootInvite ) && ( s_safeToRespondToGameBootInvite ) )
	{
		m_listener->HandleInviteReceived( ProfileManager.GetPrimaryPad(), m_gameBootInvite );
		m_gameBootInvite = NULL;
	}

	ErrorHandlingTick();
	// If we ever fail to send the external room data, we start a countdown so that we attempt to resend. Not sure how likely it is that updating this will fail without the whole network being broken,
	// but if in particular we don't update the flag to say that the session is joinable, then nobody is ever going to see this session.
	if( m_resendExternalRoomDataCountdown )
	{
		if( m_state == SNM_INT_STATE_PLAYING )
		{
			m_resendExternalRoomDataCountdown--;
			if( m_resendExternalRoomDataCountdown == 0 )
			{
				UpdateExternalRoomData();
			}
		}
		else
		{
			m_resendExternalRoomDataCountdown = 0;
		}
	}

	ProfileManager.SetNetworkStatus(GetOnlineStatus());

	// Client only - do the final transition to a starting & playing state once we have fully joined the room, And told the game about all the local players so they are also all valid
	if( m_state == SNM_INT_STATE_JOINING_WAITING_FOR_LOCAL_PLAYERS )
	{
		if( m_localPlayerJoined == m_localPlayerCount )
		{
			// Since we're now fully joined, we can update our presence info so that our friends could find us in this game. This data was set up
			// at the point that we joined the game (either from search info, or an invitation).
			UpdateRichPresenceCustomData(&s_lastPresenceSyncInfo, sizeof(PresenceSyncInfo));
			SetState( SNM_INT_STATE_STARTING);
			SetState( SNM_INT_STATE_PLAYING );
		}
	}

	EnterCriticalSection(&m_csStateChangeQueue);
	while(m_stateChangeQueue.size() > 0 )
	{
		if( m_listener )
		{
			m_listener->HandleStateChange(m_stateChangeQueue.front().m_oldState, m_stateChangeQueue.front().m_newState, m_stateChangeQueue.front().m_idleReasonIsSessionFull);
			if( m_stateChangeQueue.front().m_newState == SNM_STATE_IDLE )
			{
				m_isInSession = false;
			}
		}
		m_stateExternal = m_stateChangeQueue.front().m_newState;
		m_stateChangeQueue.pop();
	}
	LeaveCriticalSection(&m_csStateChangeQueue);

	// 4J-PB - SQRNetworkManager_PS3::AttemptPSNSignIn was causing crashes in Iggy by calling LoadMovie from a callback, so call it frmo the tick instead
	if(m_bCallPSNSignInCallback)
	{
		m_bCallPSNSignInCallback=false;
		if( s_signInCompleteCallbackIfFailed )
		{
			s_SignInCompleteCallbackFn(s_SignInCompleteParam,false,0);
			s_SignInCompleteCallbackFn  = NULL;
		}
		else if(s_SignInCompleteCallbackFn)
		{
			s_SignInCompleteCallbackFn(s_SignInCompleteParam, true, 0);
			s_SignInCompleteCallbackFn = NULL;
		}
	}
}

// Detect any states which reflect internal error states, do anything required, and transition away again
void SQRNetworkManager_PS3::ErrorHandlingTick()
{
	switch( m_state )
	{
		case SNM_INT_STATE_INITIALISE_FAILED:
			if( s_SignInCompleteCallbackFn )
			{
				if( s_signInCompleteCallbackIfFailed )
				{
					s_SignInCompleteCallbackFn(s_SignInCompleteParam,false,0);
				}
				s_SignInCompleteCallbackFn  = NULL;
			}
			app.DebugPrintf("Network error: SNM_INT_STATE_INITIALISE_FAILED\n");
			if( m_isInSession && m_offlineGame) // m_offlineSQR )  // MGH - changed this to m_offlineGame, as m_offlineSQR can be true when running an online game but the init has failed because the servers are down
			{
				// This is a fix for an issue where a player attempts (and fails) to sign in, whilst in an offline game. This was setting the state to idle, which in turn
				// sets the game to Not be in a session anymore (but the game wasn't generally aware of, and so keeps playing). Howoever, the game's connections use
				// their tick to determine whether to empty their queues or not and so no communications (even though they don't actually use this network manager for local connections)
				// were happening.
				SetState(SNM_INT_STATE_PLAYING);
			}
			else
			{
				m_offlineSQR = true;
				SetState(SNM_INT_STATE_IDLE);
			}
			break;
		case SNM_INT_STATE_HOSTING_SERVER_SEARCH_FAILED:
			app.DebugPrintf("Network error: SNM_INT_STATE_HOSTING_SERVER_SEARCH_FAILED\n");
			ResetToIdle();
			break;
		case SNM_INT_STATE_HOSTING_CREATE_ROOM_FAILED:
			app.DebugPrintf("Network error: SNM_INT_STATE_HOSTING_CREATE_ROOM_FAILED\n");
			DeleteServerContext();
			break;
		case SNM_INT_STATE_JOINING_SERVER_SEARCH_FAILED:
			app.DebugPrintf("Network error: SNM_INT_STATE_JOINING_SERVER_SEARCH_FAILED\n");
			ResetToIdle();
			break;
		case SNM_INT_STATE_JOINING_JOIN_ROOM_FAILED:
			app.DebugPrintf("Network error: SNM_INT_STATE_JOINING_JOIN_ROOM_FAILED\n");
			DeleteServerContext();
			break;
		case SNM_INT_STATE_LEAVING_FAILED:
			app.DebugPrintf("Network error: SNM_INT_STATE_LEAVING_FAILED\n");
			DeleteServerContext();
			break;
	}

}

// Start hosting a game, by creating a room & joining it. We explicity create a server context here (via GetServerContext) as Sony suggest that
// this means we have greater control of representing when players are actually "online". The creation of the room is carried out in a callback
// after that server context is made (ServerContextValidCallback_CreateRoom).
// hostIndex is the index of the user that is hosting the session, and localPlayerMask has bit 0 - 3 set to indicate the full set of local players joining the game.
// extData and extDataSize define the initial state of room data that is externally visible (eg by players searching for rooms, but not in it)
void SQRNetworkManager_PS3::CreateAndJoinRoom(int hostIndex, int localPlayerMask, void *extData, int extDataSize, bool offline)
{
	// hostIndex should always be in the mask
	assert( ( ( 1 << hostIndex ) & localPlayerMask ) != 0 );

	m_isHosting = true;
	m_joinExtData = extData;
	m_joinExtDataSize = extDataSize;
	m_offlineGame = offline;
	m_resendExternalRoomDataCountdown = 0;
	m_isInSession= true;

	// Default value for room, which we can use for offline games
	m_room = 0;

	// Initialise room data that will be synchronised. Slot 0 is always reserved for the host. We don't know the
	// room member until the room is actually created so this will be set/updated at that point
	memset( &m_roomSyncData, 0, sizeof(m_roomSyncData) );
	m_roomSyncData.setPlayerCount(1);
	m_roomSyncData.players[0].m_smallId = m_currentSmallId++;
	m_roomSyncData.players[0].m_localIdx = hostIndex;

	// Remove the host player that we've already added, then add any other local players specified in the mask
	localPlayerMask &= ~( ( 1 << hostIndex ) & localPlayerMask );
	for( int i = 0; i < MAX_LOCAL_PLAYER_COUNT; i++ )
	{
		if( localPlayerMask & ( 1 << i ) )
		{
			m_roomSyncData.players[m_roomSyncData.getPlayerCount()].m_smallId = m_currentSmallId++;
			m_roomSyncData.players[m_roomSyncData.getPlayerCount()].m_localIdx = i;
			m_roomSyncData.setPlayerCount(m_roomSyncData.getPlayerCount()+1);
		}
	}
	m_localPlayerCount = m_roomSyncData.getPlayerCount();

	// For offline games, we can jump straight to the state that says we've just created the room (or would have, for an online game)
	if( m_offlineGame )
	{
		SetState(SNM_INT_STATE_HOSTING_CREATE_ROOM_SUCCESS);
	}
	else
	{
		// Kick off the sequence of events required for an online game, starting with getting the server context
		m_isInSession = GetServerContext();
	}
}

// Updates the externally visible data that was associated with the room when it was created with CreateAndJoinRoom.
void SQRNetworkManager_PS3::UpdateExternalRoomData()
{
	if( m_offlineGame ) return;
	if( m_isHosting )
	{
		SceNpMatching2SetRoomDataExternalRequest reqParam;
		memset( &reqParam, 0, sizeof(reqParam) );
		reqParam.roomId = m_room;
		SceNpMatching2BinAttr roomBinAttr;
		memset(&roomBinAttr, 0, sizeof(roomBinAttr));
		roomBinAttr.id = SCE_NP_MATCHING2_ROOM_BIN_ATTR_EXTERNAL_1_ID;
		roomBinAttr.ptr = m_joinExtData;
		roomBinAttr.size = m_joinExtDataSize;
		reqParam.roomBinAttrExternalNum = 1;
		reqParam.roomBinAttrExternal = &roomBinAttr;

		int ret = sceNpMatching2SetRoomDataExternal ( m_matchingContext, &reqParam, NULL, &m_setRoomDataRequestId );
		app.DebugPrintf(CMinecraftApp::USER_RR,"sceNpMatching2SetRoomDataExternal returns 0x%x, number of players %d\n",ret,((char *)m_joinExtData)[174]);
		if( ( ret < 0 ) || ForceErrorPoint( SNM_FORCE_ERROR_SET_EXTERNAL_ROOM_DATA ) )
		{
			// If we ever fail to send the external room data, we start a countdown so that we attempt to resend. Not sure how likely it is that updating this will fail without the whole network being broken,
			// but if in particular we don't update the flag to say that the session is joinable, then nobody is ever going to see this session.
			m_resendExternalRoomDataCountdown = 60;
		}
	}
}

// Determine if the friend room manager is busy. If it isn't busy, then other operations (searching for a friend, reading the found friend's room lists) may safely be performed
bool SQRNetworkManager_PS3::FriendRoomManagerIsBusy()
{
	return (m_friendSearchState != SNM_FRIEND_SEARCH_STATE_IDLE);
}

// Initiate a search for rooms that the signed in user's friends are in. This is an asynchronous operation, this function returns after it kicks off a search across all game servers
// for any of the player's friends.
bool SQRNetworkManager_PS3::FriendRoomManagerSearch()
{
	if( m_state != SNM_INT_STATE_IDLE ) return false;

	// Don't start another search if we're already searching...
	if( m_friendSearchState != SNM_FRIEND_SEARCH_STATE_IDLE )
	{
		return false;
	}

	// Free up any external data that we received from the previous search
	for( int i = 0; i < m_searchResultCount; i++ )
	{
		free(m_aSearchResultRoomExtDataReceived[i]);
		m_aSearchResultRoomExtDataReceived[i] = NULL;
	}

	m_friendSearchState = SNM_FRIEND_SEARCH_STATE_GETTING_FRIEND_COUNT;
	m_friendCount = 0;
	m_searchResultCount = 0;

	// Get friend list - doing this in another thread as it can lock up for a few seconds
	m_getFriendCountThread = new C4JThread(&GetFriendsThreadProc,this,"GetFriendsThreadProc");
	m_getFriendCountThread->Run();

	return true;
}

bool SQRNetworkManager_PS3::FriendRoomManagerSearch2()
{
	SceNpMatching2AttributeId attrId;
	SceNpMatching2GetUserInfoListRequest reqParam;

	if( m_friendCount == 0 )
	{
		m_friendSearchState = SNM_FRIEND_SEARCH_STATE_IDLE;
		return false;
	}

	if( m_searchResultCount )
	{
		// If we have some results, then we also want to make sure that we don't have any duplicate rooms here if more than one friend is playing in the same room.
		unordered_set<SceNpMatching2RoomId> uniqueRooms;
		for( unsigned int i = 0; i < m_searchResultCount; i++ )
		{
			if(m_aSearchResultRoomFound[ i ])
			{
				uniqueRooms.insert( m_aSearchResultRoomId[ i ] );
			}
		}

		// Tidy the results up further based on this
		for( unsigned int i = 0; i < m_searchResultCount; )
		{
			if( uniqueRooms.find(m_aSearchResultRoomId[i]) == uniqueRooms.end() )
			{
				m_searchResultCount--;
				free(m_aSearchResultRoomExtDataReceived[i]);
				m_aSearchResultNpId[ i ]		= m_aSearchResultNpId[ m_searchResultCount ];
				m_aSearchResultRoomId[ i ]		= m_aSearchResultRoomId[ m_searchResultCount ];
				m_aSearchResultServerId[ i ]	= m_aSearchResultServerId[ m_searchResultCount ];
				m_aSearchResultRoomFound[ i ]	= m_aSearchResultRoomFound[ m_searchResultCount ];
				m_aSearchResultRoomExtDataReceived[ i ] = m_aSearchResultRoomExtDataReceived[ m_searchResultCount ];
			}
			else
			{
				uniqueRooms.erase(m_aSearchResultRoomId[i]);
				i++;
			}
		}
	}
	else
	{
		// Done, with no results found
		m_searchResultCount = 0;
	}
	m_friendSearchState = SNM_FRIEND_SEARCH_STATE_IDLE;
	return true;
}

void SQRNetworkManager_PS3::FriendSearchTick()
{
	// Move onto next state if we're done getting our friend count
	if( m_friendSearchState == SNM_FRIEND_SEARCH_STATE_GETTING_FRIEND_COUNT )
	{
		if( !m_getFriendCountThread->isRunning() )
		{
			m_friendSearchState = SNM_FRIEND_SEARCH_STATE_GETTING_FRIEND_INFO;
			delete m_getFriendCountThread;
			m_getFriendCountThread = NULL;
			FriendRoomManagerSearch2();
		}
	}
}

// The handler for basic events can't actually get the events themselves, this has to be done on another thread. Instead, we send a sys_event_t to a queue on This thread,
// which has a single data item used which we can use to determine whether to terminate this thread or get a basic event & handle that.
int SQRNetworkManager_PS3::BasicEventThreadProc( void *lpParameter )
{
	SQRNetworkManager_PS3 *manager = (SQRNetworkManager_PS3 *)lpParameter;

	sys_event_t event;
	do
	{
		sys_event_queue_receive(manager->m_basicEventQueue, &event, 0);

		// If the sys_event_t we've sent here from the handler has a non-zero data1 element, this is to signify that we should terminate the thread
		if( event.data1 == 0 )
		{
			int iEvent;
			SceNpUserInfo from;
			uint8_t buffer[SCE_NP_BASIC_MAX_MESSAGE_SIZE];
			size_t bufferSize = SCE_NP_BASIC_MAX_MESSAGE_SIZE;
			int ret = sceNpBasicGetEvent(&iEvent, &from, &buffer, &bufferSize);
			if( ret == 0 )
			{
				if( iEvent == SCE_NP_BASIC_EVENT_INCOMING_BOOTABLE_INVITATION )
				{
					// 4J Stu - Don't do this here as it can be very disruptive to gameplay. Players can bring this up from LoadOrJoinMenu, PauseMenu and InGameInfoMenu
					//sceNpBasicRecvMessageCustom(SCE_NP_BASIC_MESSAGE_MAIN_TYPE_INVITE, SCE_NP_BASIC_RECV_MESSAGE_OPTIONS_INCLUDE_BOOTABLE, SYS_MEMORY_CONTAINER_ID_INVALID);
				}
				if( iEvent == SCE_NP_BASIC_EVENT_RECV_INVITATION_RESULT )
				{
					SceNpBasicExtendedAttachmentData *result = (SceNpBasicExtendedAttachmentData  *)buffer;
					if(result->userAction == SCE_NP_BASIC_MESSAGE_ACTION_ACCEPT )
					{
						manager->GetInviteDataAndProcess(result->data.id);
					}
				}
				app.DebugPrintf("Incoming basic event of type %d\n",iEvent);
			}
		}

	} while(event.data1 == 0 );
	return 0;
}

int SQRNetworkManager_PS3::GetFriendsThreadProc( void* lpParameter )
{
	SQRNetworkManager_PS3 *manager = (SQRNetworkManager_PS3 *)lpParameter;

	int ret = sceNpBasicGetFriendListEntryCount(&manager->m_friendCount);
	if( ( ret < 0 ) || manager->ForceErrorPoint( SNM_FORCE_ERROR_GET_FRIEND_LIST_ENTRY_COUNT ) )
	{
		// This is likely when friend list hasn't been received from the server yet - will be returning SCE_NP_BASIC_ERROR_BUSY in this case
		manager->m_friendCount = 0;
	}


	// There shouldn't ever be more than 100 friends returned but limit here just in case
	if( manager->m_friendCount > 100 ) manager->m_friendCount = 100;

	// It is possible that the size of the friend list might vary from what we just received, so only add in friends that we successfully get an entry for
	for( unsigned int i = 0; i < manager->m_friendCount; i++ )
	{
		static SceNpBasicPresenceDetails2 presenceDetails;
		static SceNpUserInfo userInfo;

		memset(&userInfo, 0x00, sizeof(userInfo));
		memset(&presenceDetails, 0x00, sizeof(presenceDetails));
		presenceDetails.struct_size = sizeof(presenceDetails);

		int ret = sceNpBasicGetFriendPresenceByIndex2( i, &userInfo, &presenceDetails, 0 );

		if( ( ret == 0 ) && ( !manager->ForceErrorPoint( SNM_FORCE_ERROR_GET_FRIEND_LIST_ENTRY ) ) )
		{
			memcpy(&manager->m_aSearchResultNpId[ manager->m_searchResultCount ], &userInfo.userId, sizeof(SceNpId));
			manager->m_aSearchResultRoomFound[ manager->m_searchResultCount ] = false;

			// Only include the friend's game if its the same network id ( this also filters out generally Zeroed PresenceSyncInfo, which we do when we aren't in an active game session)
			if( presenceDetails.size == sizeof(PresenceSyncInfo) )
			{
				PresenceSyncInfo *pso = (PresenceSyncInfo *)presenceDetails.data;
				if( pso->netVersion == MINECRAFT_NET_VERSION )
				{
					if( !pso->inviteOnly )
					{
						manager->m_aSearchResultRoomFound[manager->m_searchResultCount] = true;
						manager->m_aSearchResultRoomId[manager->m_searchResultCount] = pso->m_RoomId;
						manager->m_aSearchResultServerId[manager->m_searchResultCount] = pso->m_ServerId;

						CPlatformNetworkManagerSony::MallocAndSetExtDataFromSQRPresenceInfo(&manager->m_aSearchResultRoomExtDataReceived[manager->m_searchResultCount], pso);
						manager->m_searchResultCount++;
					}
				}
			}
		}
	}
	return 0;
}

// Get count of rooms that friends are playing in. Only valid when FriendRoomManagerIsBusy() returns false
int	SQRNetworkManager_PS3::FriendRoomManagerGetCount()
{
	assert( m_friendSearchState == SNM_FRIEND_SEARCH_STATE_IDLE );
	return m_searchResultCount;
}

// Get details of a found session that a friend is playing in. 0 < idx < FriendRoomManagerGetCount(). Only valid when FriendRoomManagerIsBusy() returns false
void SQRNetworkManager_PS3::FriendRoomManagerGetRoomInfo(int idx, SQRNetworkManager_PS3::SessionSearchResult *searchResult)
{
	assert( idx < m_searchResultCount );
	assert( m_friendSearchState == SNM_FRIEND_SEARCH_STATE_IDLE );

	searchResult->m_NpId					= m_aSearchResultNpId[idx];
	searchResult->m_sessionId.m_RoomId		= m_aSearchResultRoomId[idx];
	searchResult->m_sessionId.m_ServerId	= m_aSearchResultServerId[idx];
	searchResult->m_extData					= m_aSearchResultRoomExtDataReceived[idx];
}

// Get overall state of the network manager.
SQRNetworkManager_PS3::eSQRNetworkManagerState SQRNetworkManager_PS3::GetState()
{
	return m_stateExternal;;
}

bool SQRNetworkManager_PS3::IsHost()
{
	return m_isHosting;
}

bool SQRNetworkManager_PS3::IsReadyToPlayOrIdle()
{
	return (( m_state == SNM_INT_STATE_HOSTING_WAITING_TO_PLAY ) || ( m_state == SNM_INT_STATE_PLAYING ) || ( m_state == SNM_INT_STATE_IDLE ) );
}


// Consider as "in session" from the moment that a game is created or joined, until the point where the game itself has been told via state change that we are now idle. The
// game code requires IsInSession to return true as soon as it has asked to do one of these things (even if the state system hasn't really caught up with this request yet), and
// it also requires that it is informed of the state changes leading up to not being in the session, before this should report false.
bool SQRNetworkManager_PS3::IsInSession()
{
	return m_isInSession;
}

// Get count of players currently in the session
int	SQRNetworkManager_PS3::GetPlayerCount()
{
	return m_roomSyncData.getPlayerCount();
}

// Get count of players who are in the session, but not local to this machine
int SQRNetworkManager_PS3::GetOnlinePlayerCount()
{
	int onlineCount = 0;
	for( int i = 0; i < m_roomSyncData.getPlayerCount(); i++ )
	{
		if( m_roomSyncData.players[i].m_roomMemberId != m_localMemberId )
		{
			onlineCount++;
		}
	}
	return onlineCount;
}

SQRNetworkPlayer *SQRNetworkManager_PS3::GetPlayerByIndex(int idx)
{
	if( idx < MAX_ONLINE_PLAYER_COUNT )
	{
		return GetPlayerIfReady(m_aRoomSlotPlayers[idx]);
	}
	else
	{
		return NULL;
	}
}

SQRNetworkPlayer *SQRNetworkManager_PS3::GetPlayerBySmallId(int idx)
{
	EnterCriticalSection(&m_csRoomSyncData);
	for( int i = 0; i < m_roomSyncData.getPlayerCount(); i++ )
	{
		if( m_roomSyncData.players[i].m_smallId == idx )
		{
			SQRNetworkPlayer *player = GetPlayerIfReady(m_aRoomSlotPlayers[i]);
			LeaveCriticalSection(&m_csRoomSyncData);
			return player;
		}
	}
	LeaveCriticalSection(&m_csRoomSyncData);
	return NULL;
}

SQRNetworkPlayer *SQRNetworkManager_PS3::GetPlayerByXuid(PlayerUID xuid)
{
	EnterCriticalSection(&m_csRoomSyncData);
	for( int i = 0; i < m_roomSyncData.getPlayerCount(); i++ )
	{
		if( m_roomSyncData.players[i].m_UID == xuid )
		{
			SQRNetworkPlayer *player = GetPlayerIfReady(m_aRoomSlotPlayers[i]);
			LeaveCriticalSection(&m_csRoomSyncData);
			return player;
		}
	}
	LeaveCriticalSection(&m_csRoomSyncData);
	return NULL;
}

SQRNetworkPlayer *SQRNetworkManager_PS3::GetLocalPlayerByUserIndex(int idx)
{
	EnterCriticalSection(&m_csRoomSyncData);
	for( int i = 0; i < m_roomSyncData.getPlayerCount(); i++ )
	{
		if( ( m_roomSyncData.players[i].m_roomMemberId == m_localMemberId ) && ( m_roomSyncData.players[i].m_localIdx == idx ) )
		{
			SQRNetworkPlayer *player = GetPlayerIfReady(m_aRoomSlotPlayers[i]);
			LeaveCriticalSection(&m_csRoomSyncData);
			return player;
		}
	}
	LeaveCriticalSection(&m_csRoomSyncData);
	return NULL;
}

SQRNetworkPlayer *SQRNetworkManager_PS3::GetHostPlayer()
{
	EnterCriticalSection(&m_csRoomSyncData);
	SQRNetworkPlayer *player = GetPlayerIfReady(m_aRoomSlotPlayers[0]);
	LeaveCriticalSection(&m_csRoomSyncData);
	return player;
}

SQRNetworkPlayer *SQRNetworkManager_PS3::GetPlayerIfReady(SQRNetworkPlayer *player)
{
	if( player == NULL ) return NULL;

	if( player->IsReady() ) return player;

	return NULL;
}

// Update state internally
void SQRNetworkManager_PS3::SetState(SQRNetworkManager_PS3::eSQRNetworkManagerInternalState state)
{
	eSQRNetworkManagerState oldState = m_INTtoEXTStateMappings[m_state];
	eSQRNetworkManagerState newState = m_INTtoEXTStateMappings[state];
	bool setIdleReasonSessionFull = false;
	if( ( state == SNM_INT_STATE_IDLE ) && m_nextIdleReasonIsFull )
	{
		setIdleReasonSessionFull = true;
		m_nextIdleReasonIsFull = false;
	}
	m_state = state;
	// Queue any important (ie externally relevant) state changes - we will do a call back for these in our main tick. Don't do it directly here
	// as we could be coming from any thread at this stage, with any stack size etc. and so we don't generally want to expect the game to be able to handle itself in such circumstances.
	if( ( newState != oldState ) || setIdleReasonSessionFull )
	{
		EnterCriticalSection(&m_csStateChangeQueue);
		m_stateChangeQueue.push(StateChangeInfo(oldState,newState,setIdleReasonSessionFull));
		LeaveCriticalSection(&m_csStateChangeQueue);
	}
}

void SQRNetworkManager_PS3::ResetToIdle()
{
	// If we're the client, remove any networked players properly ( this will destory their rupd context etc.)
	if( !m_isHosting )
	{
		RemoveNetworkPlayers((1 << MAX_LOCAL_PLAYER_COUNT)-1);
	}
	m_serverContextValid = false;
	m_isHosting = false;
	m_currentSmallId = 0;
	EnterCriticalSection(&m_csRoomSyncData);
	for(int i = 0; i < m_roomSyncData.getPlayerCount(); i++ )
	{
		delete m_aRoomSlotPlayers[i];
	}
	memset( m_aRoomSlotPlayers, 0, sizeof(m_aRoomSlotPlayers) );
	memset( &m_roomSyncData,0,sizeof(m_roomSyncData));
	LeaveCriticalSection(&m_csRoomSyncData);
	SetState(SNM_INT_STATE_IDLE);
}

// Join a room that was found with FriendRoomManagerSearch. 0 < idx < FriendRoomManagerGetCount(). Only valid when FriendRoomManagerIsBusy() returns false
bool SQRNetworkManager_PS3::JoinRoom(SQRNetworkManager_PS3::SessionSearchResult *searchResult, int localPlayerMask)
{
	// Set up the presence info we would like to synchronise out when we have fully joined the game
	CPlatformNetworkManagerSony::SetSQRPresenceInfoFromExtData(&s_lastPresenceSyncInfo, searchResult->m_extData, searchResult->m_sessionId.m_RoomId, searchResult->m_sessionId.m_ServerId);
	return JoinRoom(searchResult->m_sessionId.m_RoomId, searchResult->m_sessionId.m_ServerId, localPlayerMask, NULL);
}

// Join room with a specified roomId. This is used when joining from an invite, as well as by the previous method
bool SQRNetworkManager_PS3::JoinRoom(SceNpMatching2RoomId roomId, SceNpMatching2ServerId serverId, int localPlayerMask, const SQRNetworkManager_PS3::PresenceSyncInfo *presence)
{
	// The presence info will be directly passed in if we are joining from an invite, otherwise it has already been set up. This is synchronised out when we have fully joined the game.
	if( presence )
	{
		memcpy( &s_lastPresenceSyncInfo, presence, sizeof(PresenceSyncInfo) );
	}

	m_isInSession = true;

	m_isHosting = false;
	m_offlineGame = false;
	m_roomToJoin = roomId;
	m_localPlayerJoinMask = localPlayerMask;
	m_localPlayerCount = 0;
	m_localPlayerJoined = 0;

	for( int i = 0; i < MAX_LOCAL_PLAYER_COUNT; i++ )
	{
		if( localPlayerMask & ( 1 << i ) ) m_localPlayerCount++;
	}

	return GetServerContext( serverId );
}

void SQRNetworkManager_PS3::StartGame()
{
	assert( ( m_state == SNM_INT_STATE_HOSTING_WAITING_TO_PLAY ) || (( m_state == SNM_INT_STATE_IDLE ) && m_offlineSQR) );

	SetState( SNM_INT_STATE_STARTING);
	SetState( SNM_INT_STATE_PLAYING);
}

void SQRNetworkManager_PS3::LeaveRoom(bool bActuallyLeaveRoom)
{
	if( m_offlineGame )
	{
		if( m_state != SNM_INT_STATE_PLAYING ) return;

		SetState(SNM_INT_STATE_LEAVING);
		SetState(SNM_INT_STATE_ENDING);
		ResetToIdle();
		return;
	}

	UpdateRichPresenceCustomData(& c_presenceSyncInfoNULL, sizeof(PresenceSyncInfo) );

	SonyVoiceChat::shutdown();

	// Attempt to leave the room if we are in any of the states we could be in if we have successfully created it
	if( bActuallyLeaveRoom )
	{
		if( ( m_state == SNM_INT_STATE_HOSTING_CREATE_ROOM_SUCCESS ) ||
			( m_state == SNM_INT_STATE_HOSTING_WAITING_TO_PLAY ) ||
			( m_state == SNM_INT_STATE_JOINING_WAITING_FOR_LOCAL_PLAYERS ) ||
			( m_state == SNM_INT_STATE_PLAYING ) )
		{
			SceNpMatching2LeaveRoomRequest reqParam;
			memset( &reqParam, 0, sizeof(reqParam) );
			reqParam.roomId = m_room;

			SetState(SNM_INT_STATE_LEAVING);
			int ret = sceNpMatching2LeaveRoom( m_matchingContext, &reqParam, NULL, &m_leaveRoomRequestId );
			if( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_LEAVE_ROOM) )
			{
				SetState(SNM_INT_STATE_LEAVING_FAILED);
			}
		}
		else if ( m_state == SNM_INT_STATE_HOSTING_CREATE_ROOM_CREATING_ROOM )
		{
			// Haven't created the room yet, but will have created the server context so need to recover from that
			DeleteServerContext();
		}
		else
		{
			SetState(SNM_INT_STATE_IDLE);
		}
	}
	else
	{
		// We have created a room but have now had some kind of connection error which means that we've been dropped out of the room and it has been destroyed, so
		// no need to leave it again since it doesn't exist anymore. Still need to destroy server context which may be valid
		DeleteServerContext();
	}
}

void SQRNetworkManager_PS3::EndGame()
{
}

bool SQRNetworkManager_PS3::SessionHasSpace(int spaceRequired)
{
	return( ( m_roomSyncData.getPlayerCount() + spaceRequired ) <= MAX_ONLINE_PLAYER_COUNT );
}

bool SQRNetworkManager_PS3::AddLocalPlayerByUserIndex(int idx)
{
	if( m_isHosting )
	{
		if( m_roomSyncData.getPlayerCount() == MAX_ONLINE_PLAYER_COUNT ) return false;

		// Host's players are always at the start of the sync data, so we just need to find the first entry that isn't us to determine what we want to insert before
		int insertAtIdx = m_roomSyncData.getPlayerCount();
		for( int i = 0; i < m_roomSyncData.getPlayerCount(); i++ )
		{
			if( m_roomSyncData.players[i].m_roomMemberId != m_localMemberId )
			{
				insertAtIdx = i;
				break;
			}
			else
			{
				// Don't add the same local index twice
				if( m_roomSyncData.players[i].m_localIdx == idx )
				{
					return false;
				}
			}
		}

		// Make room for a new entry...
		for( int i = m_roomSyncData.getPlayerCount(); i > insertAtIdx; i-- )
		{
			m_roomSyncData.players[i] = m_roomSyncData.players[i-1];
		}
		m_roomSyncData.players[insertAtIdx].m_localIdx = idx;
		m_roomSyncData.players[insertAtIdx].m_roomMemberId = m_localMemberId;
		m_roomSyncData.players[insertAtIdx].m_smallId = m_currentSmallId++;

		m_roomSyncData.setPlayerCount(m_roomSyncData.getPlayerCount()+1);

		// And do any adjusting necessary to the mappings from this room data, to the SQRNetworkPlayers.
		// This will also create the required new SQRNetworkPlayer and do all the callbacks that requires etc.
		MapRoomSlotPlayers();

		// Sync this back out to our networked clients...
		SyncRoomData();
		return true;
	}
	else
	{
		// Don't attempt to join if our client's view of the players indicates that there aren't any free slots
		if( m_roomSyncData.getPlayerCount() == MAX_ONLINE_PLAYER_COUNT ) return false;

		// Add the requested player to the mask of local players currently in the game, and update this data - this
		// will also then resync with the server which can respond appropriately
		int mask = 1 << idx;
		if( m_localPlayerJoinMask & mask ) return false;

		m_localPlayerJoinMask |= mask;

		SceNpMatching2SetRoomMemberDataInternalRequest reqParam;
		SceNpMatching2BinAttr binAttr;

		memset(&reqParam, 0, sizeof(reqParam));
		memset(&binAttr, 0, sizeof(binAttr));

		binAttr.id = SCE_NP_MATCHING2_ROOMMEMBER_BIN_ATTR_INTERNAL_1_ID;
		binAttr.ptr = &m_localPlayerJoinMask;
		binAttr.size = sizeof(m_localPlayerJoinMask);

		reqParam.roomId		= m_room;
		reqParam.memberId	= m_localMemberId;
		reqParam.roomMemberBinAttrInternalNum = 1;
		reqParam.roomMemberBinAttrInternal = &binAttr;

		int ret = sceNpMatching2SetRoomMemberDataInternal( m_matchingContext, &reqParam, NULL, &m_setRoomMemberInternalDataRequestId );

		if( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_SET_ROOM_MEMBER_DATA_INTERNAL) )
		{
			return false;
		}

		// Create the client's end of the rudp connections... note that m_roomSyncData.players[0].m_roomMemberId is always be the host's room member id.
		bool rudpOk = CreateRudpConnections(m_room, m_roomSyncData.players[0].m_roomMemberId, mask, m_localMemberId );

		if( rudpOk )
		{
			return true;
		}
		else
		{
			m_localPlayerJoinMask &= (~mask);
			return false;
		}
	}
}

bool SQRNetworkManager_PS3::RemoveLocalPlayerByUserIndex(int idx)
{
	if( m_isHosting )
	{
		EnterCriticalSection(&m_csRoomSyncData);

		int roomSlotPlayerCount = m_roomSyncData.getPlayerCount();

		for( int i = 0; i < m_roomSyncData.getPlayerCount(); i++ )
		{
			if( ( m_roomSyncData.players[i].m_roomMemberId == m_localMemberId ) &&
				( m_roomSyncData.players[i].m_localIdx == idx ) )
			{
				// Shuffle all remaining entries up...
				m_roomSyncData.setPlayerCount(m_roomSyncData.getPlayerCount()-1);
				for( int j = i; j < m_roomSyncData.getPlayerCount(); j++ )
				{
					m_roomSyncData.players[j] = m_roomSyncData.players[j+1];
				}

				// Zero last element that isn't part of the currently sized array anymore
				memset(&m_roomSyncData.players[m_roomSyncData.getPlayerCount()],0,sizeof(PlayerSyncData));

				// And do any adjusting necessary to the mappings from this room data, to the SQRNetworkPlayers.
				// This will also delete the SQRNetworkPlayer and do all the callbacks that requires etc.
				MapRoomSlotPlayers(roomSlotPlayerCount);
				m_aRoomSlotPlayers[m_roomSyncData.getPlayerCount()] = NULL;

				// Sync this back out to our networked clients...
				SyncRoomData();

				LeaveCriticalSection(&m_csRoomSyncData);
				return true;
			}
		}
		LeaveCriticalSection(&m_csRoomSyncData);
		return false;
	}
	else
	{
		// Remove the requested player from the mask of local players currently in the game, and update this data - this
		// will also then resync with the server which can respond appropriately
		int mask = 1 << idx;
		if( ( m_localPlayerJoinMask & mask ) == 0 ) return false;

		m_localPlayerJoinMask &= ~mask;

		SceNpMatching2SetRoomMemberDataInternalRequest reqParam;
		SceNpMatching2BinAttr binAttr;

		memset(&reqParam, 0, sizeof(reqParam));
		memset(&binAttr, 0, sizeof(binAttr));

		binAttr.id = SCE_NP_MATCHING2_ROOMMEMBER_BIN_ATTR_INTERNAL_1_ID;
		binAttr.ptr = &m_localPlayerJoinMask;
		binAttr.size = sizeof(m_localPlayerJoinMask);

		reqParam.roomId		= m_room;
		reqParam.memberId	= m_localMemberId;
		reqParam.roomMemberBinAttrInternalNum = 1;
		reqParam.roomMemberBinAttrInternal = &binAttr;

		int ret = sceNpMatching2SetRoomMemberDataInternal( m_matchingContext, &reqParam, NULL, &m_setRoomMemberInternalDataRequestId );

		if( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_SET_ROOM_MEMBER_DATA_INTERNAL2) )
		{
			return false;
		}

		RemoveNetworkPlayers( mask );

		return true;
	}
}

// Bring up a Gui to send an invite so a player that the user can select. This invite will contain the room Id so that
void SQRNetworkManager_PS3::SendInviteGUI()
{
	SceNpBasicMessageDetails msg;

	//Set invitation information - this is now exactly the same as the presence information that we synchronise out.

	// If we joined a game, we'll have already set s_lastPresenceSyncInfo up (whether we came in from an invite, or joining a game we discovered). If we were hosting,
	// then we'll need to set this up now from the external dasta.
	if( m_isHosting )
	{
		CPlatformNetworkManagerSony::SetSQRPresenceInfoFromExtData(&s_lastPresenceSyncInfo, m_joinExtData, m_room, m_serverId);
	}

	memset(&msg, 0, sizeof(msg));
	msg.msgId = 0;
	msg.mainType = SCE_NP_BASIC_MESSAGE_MAIN_TYPE_INVITE;
	msg.subType = SCE_NP_BASIC_MESSAGE_INVITE_SUBTYPE_ACTION_ACCEPT;
	msg.msgFeatures = SCE_NP_BASIC_MESSAGE_FEATURES_BOOTABLE;
	msg.npids = NULL;
	msg.count = 0;

	uint8_t *subject = mallocAndCreateUTF8ArrayFromString(IDS_INVITATION_SUBJECT_MAX_18_CHARS);
	uint8_t *body = mallocAndCreateUTF8ArrayFromString(IDS_INVITATION_BODY);
	msg.subject = (const char *)subject;
	msg.body = (const char *)body;
	msg.data = &s_lastPresenceSyncInfo;
	msg.size = sizeof(PresenceSyncInfo);

	int ret = sceNpBasicSendMessageGui(&msg, SYS_MEMORY_CONTAINER_ID_INVALID);
	free(subject);
	free(body);
	if (ret < 0)
	{
		app.DebugPrintf("Failed %08x\n", ret);
		//E Error handling
	}
}

// Get the data for an invite into a statically allocated array of invites, and pass a pointer of this back up to the game. Elements in the array are used in a circular fashion, to save any issues with handling freeing of this invite data as the
// qnet equivalent of this seems to just assume that the data persists forever.
void SQRNetworkManager_PS3::GetInviteDataAndProcess(SceNpBasicAttachmentDataId id)
{
	INVITE_INFO *invite = &m_inviteReceived[m_inviteIndex];
	m_inviteIndex = ( m_inviteIndex + 1 ) % MAX_SIMULTANEOUS_INVITES;
	size_t dataSize = sizeof(INVITE_INFO);
	int ret = sceNpBasicRecvMessageAttachmentLoad(id, invite, &dataSize);

	// If we fail ( which we might do if we aren't online at this point) then zero the invite information and we'll try and get it later after (possibly) signing in
	if( ret != 0 )
	{
		memset(invite, 0, sizeof( INVITE_INFO ) );
		s_lastInviteIdToRetry = id;
	}

	m_gameBootInvite = invite;
}

bool SQRNetworkManager_PS3::UpdateInviteData(SQRNetworkManager_PS3::PresenceSyncInfo *invite)
{
	size_t dataSize = sizeof(SQRNetworkManager_PS3::PresenceSyncInfo);
	int ret = sceNpBasicRecvMessageAttachmentLoad(s_lastInviteIdToRetry, invite, &dataSize);
	return (ret == 0);
}

// This method is a helper used in MapRoomSlotPlayers - tries to find a player that matches:
// (1) the playerType
// (2) if playerType is remote, memberId
// (3) localPlayerIdx
// The reason we don't care about memberid when the player isn't remote is that it doesn't matter (since we know the player is either on this machine, or it is the host and there's only one of those),
// and there's a period when starting up the host game where it doesn't accurately know the memberId for its own local players
void SQRNetworkManager_PS3::FindOrCreateNonNetworkPlayer(int slot, int playerType, SceNpMatching2RoomMemberId memberId, int localPlayerIdx, int smallId)
{
    for (auto it = m_vecTempPlayers.begin(); it != m_vecTempPlayers.end(); it++)
    {
		if( ((*it)->m_type == playerType ) && ( (*it)->m_localPlayerIdx == localPlayerIdx ) )
		{
			if( ( playerType != SQRNetworkPlayer::SNP_TYPE_REMOTE ) || ( (*it)->m_roomMemberId == memberId ) )
			{
				SQRNetworkPlayer *player = *it;
				m_vecTempPlayers.erase(it);
				m_aRoomSlotPlayers[ slot ] = player;
				return;
			}
		}
	}
	// Create the player - non-network players can be considered complete as soon as we create them as we aren't waiting on their network connections becoming complete, so can flag them as such and notify via callback
	PlayerUID *pUID = NULL;
	PlayerUID localUID;
	if( ( playerType == SQRNetworkPlayer::SNP_TYPE_LOCAL ) ||
		m_isHosting && ( playerType == SQRNetworkPlayer::SNP_TYPE_HOST ) )
	{
		// Local players can establish their UID at this point
		ProfileManager.GetXUID(localPlayerIdx,&localUID,true);
		pUID = &localUID;
	}
	SQRNetworkPlayer *player = new SQRNetworkPlayer(this, (SQRNetworkPlayer::eSQRNetworkPlayerType)playerType, m_isHosting, memberId, localPlayerIdx, 0, pUID );
	// For offline games, set name directly from gamertag as the PlayerUID will be full of zeroes.
	if( m_offlineGame )
	{
		player->SetName(ProfileManager.GetGamertag(localPlayerIdx));
	}
	NonNetworkPlayerComplete( player, smallId);
	m_aRoomSlotPlayers[ slot ] = player;
	HandlePlayerJoined( player );
}

// For data sending on the local machine, used to send between host and localplayers on the host
void	SQRNetworkManager_PS3::LocalDataSend(SQRNetworkPlayer *playerFrom, SQRNetworkPlayer *playerTo, const void *data, unsigned int dataSize)
{
	assert(m_isHosting);
	if(m_listener)
	{
		m_listener->HandleDataReceived( playerFrom, playerTo, (unsigned char *)data, dataSize );
	}
}

int SQRNetworkManager_PS3::GetSessionIndex(SQRNetworkPlayer *player)
{
	int roomSlotPlayerCount = m_roomSyncData.getPlayerCount();
	for( int i = 0; i < roomSlotPlayerCount; i++  )
	{
		if( m_aRoomSlotPlayers[i] == player ) return i;
	}
	return 0;
}

// Updates m_aRoomSlotPlayers, based on what is in m_roomSyncData. This needs to be updated when room members join & leave, and when any SQRNetworkPlayer is created externally that this should be mapping to
void SQRNetworkManager_PS3::MapRoomSlotPlayers(int roomSlotPlayerCount/*=-1*/)
{
	EnterCriticalSection(&m_csRoomSyncData);

	// If we pass an explicit roomSlotPlayerCount, it is because we are removing a player, and this is the count of slots that there were *before* the removal.
	bool zeroLastSlot = false;
	if( roomSlotPlayerCount == -1 )
	{
		roomSlotPlayerCount = m_roomSyncData.getPlayerCount();
	}
	else
	{
		zeroLastSlot = true;
	}

	if( m_isHosting )
	{
		for( int i = 0; i < roomSlotPlayerCount; i++ )
		{
			if( m_aRoomSlotPlayers[i] )
			{
				// On host, remote players are created and destroyed by the Rudp connections being established and removed, so don't go deleting them here. Other types are managed by this mapping.
				// Note that m_vecTempPlayers is used as a pool of players to consider by FindOrCreateNonNetworkPlayer
				if( m_aRoomSlotPlayers[i]->m_type != SQRNetworkPlayer::SNP_TYPE_REMOTE )
				{
					m_vecTempPlayers.push_back(m_aRoomSlotPlayers[i]);
					m_aRoomSlotPlayers[i] = NULL;
				}
			}
		}
		for( int i = 0; i < m_roomSyncData.getPlayerCount(); i++ )
		{
			if( i == 0 )
			{
				// Special case - slot 0 is always the host
				FindOrCreateNonNetworkPlayer( i, SQRNetworkPlayer::SNP_TYPE_HOST, m_roomSyncData.players[i].m_roomMemberId, m_roomSyncData.players[i].m_localIdx, m_roomSyncData.players[i].m_smallId);
				m_roomSyncData.players[i].m_UID = m_aRoomSlotPlayers[i]->GetUID();		// On host, UIDs flow from player data -> m_roomSyncData
			}
			else
			{
				if( m_roomSyncData.players[i].m_roomMemberId == m_localMemberId )
				{
					FindOrCreateNonNetworkPlayer( i, SQRNetworkPlayer::SNP_TYPE_LOCAL, m_roomSyncData.players[i].m_roomMemberId, m_roomSyncData.players[i].m_localIdx, m_roomSyncData.players[i].m_smallId);
					m_roomSyncData.players[i].m_UID = m_aRoomSlotPlayers[i]->GetUID();	// On host, UIDs flow from player data -> m_roomSyncData
				}
				else
				{
					m_aRoomSlotPlayers[i] = GetPlayerFromRoomMemberAndLocalIdx( m_roomSyncData.players[i].m_roomMemberId, m_roomSyncData.players[i].m_localIdx );
					// If we're the host, then we allocated the small id so can flag now if we've got a player to flag...
					if( m_aRoomSlotPlayers[i] )
					{
						NetworkPlayerSmallIdAllocated(m_aRoomSlotPlayers[i], m_roomSyncData.players[i].m_smallId);
					}
				}
			}
		}

		if( zeroLastSlot )
		{
			if( roomSlotPlayerCount )
			{
				m_aRoomSlotPlayers[ roomSlotPlayerCount - 1 ] = 0;
			}
		}

		// Also update the externally visible room data for the current slots
		if (m_listener )
		{
			m_listener->HandleResyncPlayerRequest(m_aRoomSlotPlayers);
		}
	}
	else
	{
		for( int i = 0; i < m_roomSyncData.getPlayerCount(); i++ )
		{
			if( m_aRoomSlotPlayers[i] )
			{
				// On clients, local players are created and destroyed by the Rudp connections being established and removed, so don't go deleting them here. Other types are managed by this mapping.
				// Note that m_vecTempPlayers is used as a pool of players to consider by FindOrCreateNonNetworkPlayer
				if( m_aRoomSlotPlayers[i]->m_type != SQRNetworkPlayer::SNP_TYPE_LOCAL )
				{
					m_vecTempPlayers.push_back(m_aRoomSlotPlayers[i]);
					m_aRoomSlotPlayers[i] = NULL;
				}
			}
		}
		for( int i = 0; i < m_roomSyncData.getPlayerCount(); i++ )
		{
			if( i == 0 )
			{
				// Special case - slot 0 is always the host
				FindOrCreateNonNetworkPlayer( i, SQRNetworkPlayer::SNP_TYPE_HOST, m_roomSyncData.players[i].m_roomMemberId, m_roomSyncData.players[i].m_localIdx, m_roomSyncData.players[i].m_smallId);
				m_aRoomSlotPlayers[i]->SetUID(m_roomSyncData.players[i].m_UID);  // On client, UIDs flow from m_roomSyncData->player data
			}
			else
			{
				if( m_roomSyncData.players[i].m_roomMemberId == m_localMemberId )
				{
					// This player is local to this machine - don't bother setting UID from sync data, as it will already have been set accurately when we (locally) made this player
					m_aRoomSlotPlayers[i] = GetPlayerFromRoomMemberAndLocalIdx( m_roomSyncData.players[i].m_roomMemberId, m_roomSyncData.players[i].m_localIdx );
					// If we've got the room sync data back from the server, then we've got our smallId. Set flag for this.
					if( m_aRoomSlotPlayers[i] )
					{
						NetworkPlayerSmallIdAllocated(m_aRoomSlotPlayers[i], m_roomSyncData.players[i].m_smallId);
					}
				}
				else
				{
					FindOrCreateNonNetworkPlayer( i, SQRNetworkPlayer::SNP_TYPE_REMOTE, m_roomSyncData.players[i].m_roomMemberId, m_roomSyncData.players[i].m_localIdx, m_roomSyncData.players[i].m_smallId);
					m_aRoomSlotPlayers[i]->SetUID(m_roomSyncData.players[i].m_UID);  // On client, UIDs flow from m_roomSyncData->player data
				}
			}
		}
	}
	// Clear up any non-network players that are no longer required - this would be a good point to notify of players leaving when we support that
	// FindOrCreateNonNetworkPlayer will have pulled any players that we Do need out of m_vecTempPlayers, so the ones that are remaining are no longer in the game
    for (auto it = m_vecTempPlayers.begin(); it != m_vecTempPlayers.end(); it++)
    {
		if( m_listener )
		{
			m_listener->HandlePlayerLeaving(*it);
		}
		delete (*it);
	}
	m_vecTempPlayers.clear();

	LeaveCriticalSection(&m_csRoomSyncData);
}

// On host, update the room sync data with UIDs that are in the players
void SQRNetworkManager_PS3::UpdateRoomSyncUIDsFromPlayers()
{
	EnterCriticalSection(&m_csRoomSyncData);
	if( m_isHosting )
	{
		for( int i = 0; i < m_roomSyncData.getPlayerCount(); i++ )
		{
			if( m_aRoomSlotPlayers[i] )
			{
				m_roomSyncData.players[i].m_UID = m_aRoomSlotPlayers[i]->GetUID();
			}
		}
	}

	LeaveCriticalSection(&m_csRoomSyncData);
}

// On the client, move UIDs from the room sync data out to the players.
void SQRNetworkManager_PS3::UpdatePlayersFromRoomSyncUIDs()
{
	EnterCriticalSection(&m_csRoomSyncData);
	for( int i = 0; i < m_roomSyncData.getPlayerCount(); i++ )
	{
		if( m_aRoomSlotPlayers[i] )
		{
			if( i == 0 )
			{
				// Special case - slot 0 is always the host
				m_aRoomSlotPlayers[i]->SetUID(m_roomSyncData.players[i].m_UID);
			}
			else
			{
				// Don't sync local players as we already set those up with their UID in the first place...
				if( m_roomSyncData.players[i].m_roomMemberId != m_localMemberId )
				{
					m_aRoomSlotPlayers[i]->SetUID(m_roomSyncData.players[i].m_UID);
				}
			}
		}
	}
	LeaveCriticalSection(&m_csRoomSyncData);
}

// Host only - add remote players to our internal storage of player slots, and synchronise this with other room members.
bool SQRNetworkManager_PS3::AddRemotePlayersAndSync( SceNpMatching2RoomMemberId memberId, int playerMask, bool *isFull/*==NULL*/ )
{
	assert( m_isHosting );

	EnterCriticalSection(&m_csRoomSyncData);

	// Establish whether we have enough room to add the players
	int addCount = 0;
	for( int i = 0; i < MAX_LOCAL_PLAYERS; i++ )
	{
		if( playerMask & ( 1 << i ) )
		{
			addCount++;
		}
	}

	if( ( m_roomSyncData.getPlayerCount() + addCount ) > MAX_ONLINE_PLAYER_COUNT )
	{
		if( isFull )
		{
			*isFull = true;
		}
		LeaveCriticalSection(&m_csRoomSyncData);
		return false;
	}

	// We want to keep all players from a particular machine together, so search through the room sync data to see if we can find
	// any pre-existing players from this machine.
	int firstIdx = -1;
	for( int i = 0; i < m_roomSyncData.getPlayerCount(); i++ )
	{
		if( m_roomSyncData.players[i].m_roomMemberId == memberId )
		{
			firstIdx = i;
			break;
		}
	}

	// We'll just be inserting at the end unless we've got a pre-existing player to insert after. Even then there might be no following
	// players.
	int insertIdx = m_roomSyncData.getPlayerCount();
	if( firstIdx > -1 )
	{
		for( int i = firstIdx; i < m_roomSyncData.getPlayerCount(); i++ )
		{
			if( m_roomSyncData.players[i].m_roomMemberId != memberId )
			{
				insertIdx = i;
				break;
			}
		}
	}

	// Add all remote players determined from the player mask to our own slots of active players
	for( int i = 0; i < MAX_LOCAL_PLAYER_COUNT; i++ )
	{
		if( playerMask & ( 1 << i ) )
		{
			// Shift any following players along...
			for( int j = m_roomSyncData.getPlayerCount(); j > insertIdx; j-- )
			{
				m_roomSyncData.players[j] = m_roomSyncData.players[j-1];
			}
			PlayerSyncData *player = &m_roomSyncData.players[ insertIdx ];
			player->m_smallId = m_currentSmallId++;
			player->m_roomMemberId = memberId;
			player->m_localIdx = i;
			m_roomSyncData.setPlayerCount(m_roomSyncData.getPlayerCount()+1);
			insertIdx++;
		}
	}

	// Update mapping from the room slot players to SQRNetworkPlayer instances
	MapRoomSlotPlayers();

	// And then synchronise this out to all other machines
	SyncRoomData();

	LeaveCriticalSection(&m_csRoomSyncData);

	SonyVoiceChat::init(this);

	return true;
}

// Host only - remove all remote players belonging to the supplied memberId, and in the supplied mask, and synchronise this with other room members
void SQRNetworkManager_PS3::RemoveRemotePlayersAndSync( SceNpMatching2RoomMemberId memberId, int mask )
{
	assert( m_isHosting );
	EnterCriticalSection(&m_csRoomSyncData);

	// Remove any applicable players, keeping remaining players in order
	for( int i = 0; i < m_roomSyncData.getPlayerCount(); )
	{
		if( ( m_roomSyncData.players[ i ].m_roomMemberId == memberId ) && ( ( 1 << m_roomSyncData.players[ i ].m_localIdx  ) & mask ) )
		{
			SQRNetworkPlayer *player = GetPlayerFromRoomMemberAndLocalIdx( memberId, m_roomSyncData.players[ i ].m_localIdx );
			if( player )
			{
				// Get Rudp context for this player, close that context down ( which will in turn close the socket if required)
				int ctx = player->m_rudpCtx;
				cellRudpTerminate( ctx );
				if( m_listener )
				{
					m_listener->HandlePlayerLeaving(player);
				}
				// Delete the player itself and the mapping from context to player map as this context is no longer valid
				delete player;
				m_RudpCtxToPlayerMap.erase(ctx);
			}
			m_roomSyncData.setPlayerCount(m_roomSyncData.getPlayerCount()-1);
			// Shuffled entries up into the space that we have just created
			for( int j = i ; j < m_roomSyncData.getPlayerCount(); j++	)
			{
				m_roomSyncData.players[j] = m_roomSyncData.players[j + 1];
				m_aRoomSlotPlayers[j] = m_aRoomSlotPlayers[j + 1];
			}
			// Zero last element, that isn't part of the currently sized array anymore
			memset(&m_roomSyncData.players[m_roomSyncData.getPlayerCount()],0,sizeof(PlayerSyncData));
			m_aRoomSlotPlayers[m_roomSyncData.getPlayerCount()] = NULL;
		}
		else
		{
			i++;
		}
	}
	LeaveCriticalSection(&m_csRoomSyncData);

	// Update mapping from the room slot players to SQRNetworkPlayer instances
	MapRoomSlotPlayers();


	// And then synchronise this out to all other machines
	SyncRoomData();

	if(GetOnlinePlayerCount() == 0)
		SonyVoiceChat::shutdown();
}

// Client only - remove all network players matching the supplied mask
void SQRNetworkManager_PS3::RemoveNetworkPlayers( int mask )
{
	assert( !m_isHosting );

    for (auto it = m_RudpCtxToPlayerMap.begin(); it != m_RudpCtxToPlayerMap.end();)
    {
		SQRNetworkPlayer *player = it->second;
		if( (player->m_roomMemberId == m_localMemberId ) && ( ( 1 << player->m_localPlayerIdx ) & mask ) )
		{
			// Get Rudp context for this player, close that context down ( which will in turn close the socket if required)
			int ctx = it->first;
			cellRudpTerminate( ctx );
			if( m_listener )
			{
				m_listener->HandlePlayerLeaving(player);
			}
			// Delete any reference to this player from the player mappings
			for( int i = 0; i < MAX_ONLINE_PLAYER_COUNT; i++ )
			{
				if( m_aRoomSlotPlayers[i] == player )
				{
					m_aRoomSlotPlayers[i] = NULL;
				}
			}
			// And delete the reference from the ctx->player map
			it = m_RudpCtxToPlayerMap.erase(it);

			// Delete the player itself and the mapping from context to player map as this context is no longer valid
			delete player;
		}
		else
		{
			it++;
		}
	}

}

// Host only - update the memberId of the local players, and synchronise with other room members
void SQRNetworkManager_PS3::SetLocalPlayersAndSync()
{
	assert( m_isHosting );
	for( int i = 0; i < m_localPlayerCount; i++ )
	{
		m_roomSyncData.players[i].m_roomMemberId = m_localMemberId;
	}

	// Update mapping from the room slot players to SQRNetworkPlayer instances
	MapRoomSlotPlayers();

	// And then synchronise this out to all other machines
	SyncRoomData();

}

// Host only - sync the room data with other machines
void SQRNetworkManager_PS3::SyncRoomData()
{
	if( m_offlineGame ) return;

	UpdateRoomSyncUIDsFromPlayers();

	SceNpMatching2SetRoomDataInternalRequest reqParam;
	memset( &reqParam, 0, sizeof(reqParam) );
	reqParam.roomId = m_room;
	SceNpMatching2BinAttr roomBinAttr;
	memset(&roomBinAttr, 0, sizeof(roomBinAttr));
	roomBinAttr.id = SCE_NP_MATCHING2_ROOM_BIN_ATTR_INTERNAL_1_ID;
	roomBinAttr.ptr = &m_roomSyncData;
	roomBinAttr.size = sizeof( m_roomSyncData );
	reqParam.roomBinAttrInternalNum = 1;
	reqParam.roomBinAttrInternal = &roomBinAttr;
	sceNpMatching2SetRoomDataInternal ( m_matchingContext, &reqParam, NULL, &m_setRoomDataRequestId );
}

// Check if the matching context is valid, and if not attempt to create one. If to do this requires starting an asynchronous process, then sets the internal state to the state passed in
// before doing this.
// Returns true on success.
bool SQRNetworkManager_PS3::GetMatchingContext(eSQRNetworkManagerInternalState asyncState)
{
	if( m_matchingContextValid ) return true;

	int ret = 0;
	if( !m_matching2initialised)
	{
		ret = sceNpMatching2Init2(0, 0, NULL);
	}
	if( ret < 0 )
	{
		app.DebugPrintf("SQRNetworkManager::GetMatchingContext - sceNpMatching2Init2 failed with code 0x%08x\n", ret);
		return false;
	}
	m_matching2initialised = true;

	// Get NP ID of the signed-in user
	SceNpId npId;
	ret = sceNpManagerGetNpId(&npId);
	if( ret < 0 )
	{
		app.DebugPrintf("SQRNetworkManager::GetMatchingContext - sceNpManagerGetNpId failed with code 0x%08x\n", ret);
		return false;
	}

	//  Use Online Name and avatar to identify user
	int option = (SCE_NP_MATCHING2_CONTEXT_OPTION_USE_ONLINENAME | SCE_NP_MATCHING2_CONTEXT_OPTION_USE_AVATARURL);

	// Create context
	ret = sceNpMatching2CreateContext(
			&npId, &s_npCommunicationId, &s_npCommunicationPassphrase, &m_matchingContext, option);
	if( ret < 0 )
	{
		app.DebugPrintf("SQRNetworkManager::GetMatchingContext - sceNpMatching2CreateContext failed with code 0x%08x\n", ret);
		return false;
	}

	if( !RegisterCallbacks() )
	{
		app.DebugPrintf("SQRNetworkManager::GetMatchingContext - RegisterCallbacks failed\n");
		return false;
	}

	// Set internal state & kick off async process that will actually start the context.
	SetState(asyncState);
	ret = sceNpMatching2ContextStartAsync(m_matchingContext, (10*1000*1000));
	if( ret < 0 )
	{
		// Put state back so that the caller isn't expecting a callback from sceNpMatching2ContextStartAsync completing to happen
		SetState(SNM_INT_STATE_IDLE);
		app.DebugPrintf("SQRNetworkManager::GetMatchingContext - sceNpMatching2ContextStartAsync failed with code 0x%08x\n", ret);
		return false;
	}

	app.DebugPrintf("SQRNetworkManager::GetMatchingContext - matching context is now valid\n");
	m_matchingContextValid = true;
	return true;
}

// Starts the process of obtaining a server context. This is an asynchronous operation, at the end of which (if successful), we'll be creating
// a room. General procedure followed here is as suggested by Sony - we get a list of servers, then pick a random one, and see if it is available.
// If not we just cycle round trying other random ones until we either find an available one or fail.
bool SQRNetworkManager_PS3::GetServerContext()
{
	assert(m_state == SNM_INT_STATE_IDLE);
	assert(m_serverContextValid == false);

	// Check that the matching context is valid & recreate if necessary
	if( !GetMatchingContext(SNM_INT_STATE_HOSTING_STARTING_MATCHING_CONTEXT) ) return false;
	// If this caused an async thing to be started up, then we've done as much as we can here - the rest of the code will happen when the async matching 2 context starting completes
	// ( event SCE_NP_MATCHING2_CONTEXT_EVENT_Start is received )
	if( m_state == SNM_INT_STATE_HOSTING_STARTING_MATCHING_CONTEXT ) return true;

	return GetServerContext2();
}

// Code split out from previous method, so we can also call from creating matching context if required
bool SQRNetworkManager_PS3::GetServerContext2()
{
	// Get list of server IDs of servers allocated to the application
	int serverCount = sceNpMatching2GetServerIdListLocal( m_matchingContext, NULL, 0 );
	// If an error is returned here, we need to destroy and recerate our server - if this goes ok we should come back through this path again
	if( ( serverCount == SCE_NP_MATCHING2_ERROR_CONTEXT_UNAVAILABLE ) ||		// This error has been seen (occasionally) in a normal working environment
		( serverCount == SCE_NP_MATCHING2_ERROR_CONTEXT_NOT_STARTED ) )				// Also checking for this as a means of simulating the previous error
	{
		sceNpMatching2DestroyContext(m_matchingContext);
		m_matchingContextValid = false;
		if( !GetMatchingContext(SNM_INT_STATE_HOSTING_STARTING_MATCHING_CONTEXT) ) return false;
		// If this caused an async thing to be started up, then we've done as much as we can here - the rest of the code will happen when the async matching 2 context starting completes
		// ( event SCE_NP_MATCHING2_CONTEXT_EVENT_Start is received )
		if( m_state == SNM_INT_STATE_HOSTING_STARTING_MATCHING_CONTEXT )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	if( serverCount <= 0 )
	{
		return false;
	}
	m_aServerId = (SceNpMatching2ServerId *)realloc( m_aServerId, sizeof(SceNpMatching2ServerId) * serverCount );
	m_serverCount = sceNpMatching2GetServerIdListLocal(m_matchingContext, m_aServerId, serverCount);
	m_totalServerCount = m_serverCount;
	if (m_serverCount < 0)
	{
		return false;
	}

	SetState(SNM_INT_STATE_HOSTING_SEARCHING_FOR_SERVER);
	return SelectRandomServer();
}

// Overloaded method for (as before) obtaining a server context. This version is so that can also get a server context for a specific server rather than a random one,
// using mainly the same code by making a single element list. This is used when joining an existing room.
bool SQRNetworkManager_PS3::GetServerContext(SceNpMatching2ServerId serverId)
{
	assert(m_state == SNM_INT_STATE_IDLE);
	assert(m_serverContextValid == false);

	// Check that the matching context is valid & recreate if necessary
	if( !GetMatchingContext(SNM_INT_STATE_JOINING_STARTING_MATCHING_CONTEXT) )
	{
		app.DebugPrintf("SQRNetworkManager::GetServerContext - Failed due to no matching context\n");
		return false;
	}

	// 4J Stu - If this state is set, then we have successfully created a new context but it won't have started yet
	// Therefore the sceNpMatching2GetServerIdListLocal call will fail. If we just skip this check everything should be good.
	if( m_state != SNM_INT_STATE_JOINING_STARTING_MATCHING_CONTEXT )
	{
		// Get list of server IDs of servers allocated to the application. We don't actually need to do this, but it is as good a way as any to try a matching2 service and check that
		// the context *really* is valid.
		int serverCount = sceNpMatching2GetServerIdListLocal( m_matchingContext, NULL, 0 );
		// If an error is returned here, we need to destroy and recerate our server - if this goes ok we should come back through this path again
		if( ( serverCount == SCE_NP_MATCHING2_ERROR_CONTEXT_UNAVAILABLE ) ||		// This error has been seen (occasionally) in a normal working environment
			( serverCount == SCE_NP_MATCHING2_ERROR_CONTEXT_NOT_STARTED ) )				// Also checking for this as a means of simulating the previous error
		{
			app.DebugPrintf("SQRNetworkManager::GetServerContext - sceNpMatching2GetServerIdListLocal failed with error 0x%08x\n", serverCount);
			int ret = sceNpMatching2DestroyContext(m_matchingContext);
			app.DebugPrintf("SQRNetworkManager::GetServerContext - sceNpMatching2DestroyContext returned 0x%08x\n", ret);
			m_matchingContextValid = false;
			if( !GetMatchingContext(SNM_INT_STATE_JOINING_STARTING_MATCHING_CONTEXT) )
			{
				app.DebugPrintf("SQRNetworkManager::GetServerContext - Failed due to no matching context after recreating\n");
				return false;
			}
		}
	}

	m_serverCount = 1;
	m_totalServerCount = m_serverCount;
	m_aServerId = (SceNpMatching2ServerId *)realloc(m_aServerId, sizeof(SceNpMatching2ServerId) * m_serverCount );
	m_aServerId[0] = serverId;

	// If one of the previous GetMatchingContext calls caused an async thing to be started up, then we've done as much as we can here - the rest of the code will happen when the async matching 2 context starting completes
	// ( event SCE_NP_MATCHING2_CONTEXT_EVENT_Start is received )
	if( m_state == SNM_INT_STATE_JOINING_STARTING_MATCHING_CONTEXT ) return true;

	SetState(SNM_INT_STATE_JOINING_SEARCHING_FOR_SERVER);
	return SelectRandomServer();
}

// Tick to update the search for a server which is available, for the creation of a server context.
void SQRNetworkManager_PS3::ServerContextTick()
{
	switch( m_state )
	{
		case SNM_INT_STATE_HOSTING_SEARCHING_FOR_SERVER:
		case SNM_INT_STATE_JOINING_SEARCHING_FOR_SERVER:
			break;
		case SNM_INT_STATE_HOSTING_SERVER_SEARCH_SERVER_ERROR:
		case SNM_INT_STATE_JOINING_SERVER_SEARCH_SERVER_ERROR:
			// Attempt to keep searching if a single server failed
			SetState((m_state==SNM_INT_STATE_HOSTING_SERVER_SEARCH_SERVER_ERROR)?SNM_INT_STATE_HOSTING_SEARCHING_FOR_SERVER:SNM_INT_STATE_JOINING_SEARCHING_FOR_SERVER);
			if(!SelectRandomServer())
			{
				SetState((m_state==SNM_INT_STATE_HOSTING_SERVER_SEARCH_SERVER_ERROR)?SNM_INT_STATE_HOSTING_SERVER_SEARCH_FAILED:SNM_INT_STATE_JOINING_SERVER_SEARCH_FAILED);
			}
			break;
		case SNM_INT_STATE_HOSTING_SERVER_FOUND:
		case SNM_INT_STATE_JOINING_SERVER_FOUND:
			{
				SceNpMatching2CreateServerContextRequest  reqParam;
				reqParam.serverId = m_serverId;
				SetState((m_state==SNM_INT_STATE_HOSTING_SERVER_FOUND)?SNM_INT_STATE_HOSTING_SERVER_SEARCH_CREATING_CONTEXT:SNM_INT_STATE_JOINING_SERVER_SEARCH_CREATING_CONTEXT);
				// Found a server - now try and create a context for it
				int ret = sceNpMatching2CreateServerContext( m_matchingContext, &reqParam, NULL, &m_serverContextRequestId );
				if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_CREATE_SERVER_CONTEXT) )
				{
					SetState((m_state==SNM_INT_STATE_HOSTING_SERVER_FOUND)?SNM_INT_STATE_HOSTING_SERVER_SEARCH_FAILED:SNM_INT_STATE_JOINING_SERVER_SEARCH_FAILED);
				}
			}
			break;
		default:
			break;
	}
}

// Tick the process of creating a room.
void SQRNetworkManager_PS3::RoomCreateTick()
{
	switch( m_state )
	{
		case SNM_INT_STATE_HOSTING_CREATE_ROOM_SEARCHING_FOR_WORLD:
			break;
		case SNM_INT_STATE_HOSTING_CREATE_ROOM_WORLD_FOUND:
			{
				SceNpMatching2CreateJoinRoomRequest reqParam;
				SceNpMatching2SignalingOptParam optSignalingParam;
				SceNpMatching2BinAttr roomBinAttrExt;
				SceNpMatching2BinAttr roomBinAttr;
				memset(&reqParam, 0, sizeof(reqParam));
				memset(&optSignalingParam, 0, sizeof( optSignalingParam) );
				memset(&roomBinAttr, 0, sizeof(roomBinAttr));
				memset(&roomBinAttrExt, 0, sizeof(roomBinAttrExt));

				reqParam.worldId = m_worldId;
				reqParam.flagAttr = SCE_NP_MATCHING2_ROOM_FLAG_ATTR_NAT_TYPE_RESTRICTION;
				reqParam.sigOptParam = &optSignalingParam;
				reqParam.maxSlot = MAX_ONLINE_PLAYER_COUNT;
				reqParam.roomBinAttrInternalNum = 1;
				reqParam.roomBinAttrInternal = &roomBinAttr;
				reqParam.roomBinAttrExternalNum = 1;
				reqParam.roomBinAttrExternal = &roomBinAttrExt;

				roomBinAttr.id = SCE_NP_MATCHING2_ROOM_BIN_ATTR_INTERNAL_1_ID;
				roomBinAttr.ptr = &m_roomSyncData;
				roomBinAttr.size = sizeof( m_roomSyncData );

				roomBinAttrExt.id = SCE_NP_MATCHING2_ROOM_BIN_ATTR_EXTERNAL_1_ID;
				roomBinAttrExt.ptr = m_joinExtData;
				roomBinAttrExt.size = m_joinExtDataSize;

				optSignalingParam.type = SCE_NP_MATCHING2_SIGNALING_TYPE_STAR;
				optSignalingParam.hubMemberId = 0;	// Room owner is the hub of the star
				SetState(SNM_INT_STATE_HOSTING_CREATE_ROOM_CREATING_ROOM);
				app.DebugPrintf(CMinecraftApp::USER_RR,">> Creating room start\n");
				s_roomStartTime = System::currentTimeMillis();
				int ret = sceNpMatching2CreateJoinRoom( m_matchingContext, &reqParam, NULL, &m_createRoomRequestId );
				if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_CREATE_JOIN_ROOM) )
				{
					SetState(SNM_INT_STATE_HOSTING_CREATE_ROOM_FAILED);
				}
			}
			break;
		case SNM_INT_STATE_HOSTING_CREATE_ROOM_CREATING_ROOM:
			break;
		case SNM_INT_STATE_HOSTING_CREATE_ROOM_SUCCESS:
			SetState(SNM_INT_STATE_HOSTING_WAITING_TO_PLAY);

			// Now we know the local member id we can update our local players
			SetLocalPlayersAndSync();
			break;
		case SNM_INT_STATE_HOSTING_CREATE_ROOM_FAILED:
			break;
		default:
			break;
	}
}

// For a player using the network to communicate, flag as having its connection complete. This wraps the player's own functionality, so that we can determine if this
// call is transitioning us from not ready to ready, and call a registered callback.
void SQRNetworkManager_PS3::NetworkPlayerConnectionComplete(SQRNetworkPlayer *player)
{
	EnterCriticalSection(&m_csPlayerState);
	bool wasReady = player->IsReady();
	bool wasClientReady = player->HasConnectionAndSmallId();
	player->ConnectionComplete();
	bool isReady = player->IsReady();
	bool isClientReady = player->HasConnectionAndSmallId();
	if( !m_isHosting )
	{
		// For clients, if we are ready (up the the point of having received our small id) then confirm to the host that this is the case, which makes us now fully ready at this end
		if( ( !wasClientReady ) && ( isClientReady ) )
		{
			player->ConfirmReady();
			isReady = true;
		}
	}
	LeaveCriticalSection(&m_csPlayerState);

	if( ( !wasReady ) && ( isReady ) )
	{
		HandlePlayerJoined( player );
	}
}

// For a player using the network to communicate, set its small id, thereby flagging it as having one allocated
void SQRNetworkManager_PS3::NetworkPlayerSmallIdAllocated(SQRNetworkPlayer *player, unsigned char smallId)
{
	EnterCriticalSection(&m_csPlayerState);
	bool wasReady = player->IsReady();
	bool wasClientReady = player->HasConnectionAndSmallId();
	player->SmallIdAllocated(smallId);
	bool isReady = player->IsReady();
	bool isClientReady = player->HasConnectionAndSmallId();
	if( !m_isHosting )
	{
		// For clients, if we are ready (up the the point of having received our small id) then confirm to the host that this is the case, which makes us now fully ready at this end
		if( ( !wasClientReady ) && ( isClientReady ) )
		{
			player->ConfirmReady();
			isReady = true;
		}
	}
	LeaveCriticalSection(&m_csPlayerState);

	if( ( !wasReady ) && ( isReady ) )
	{
		HandlePlayerJoined( player );
	}
}

// On host, for a player using the network to communicate, confirm that its small id has now been received back
void SQRNetworkManager_PS3::NetworkPlayerInitialDataReceived(SQRNetworkPlayer *player, void *data)
{
	EnterCriticalSection(&m_csPlayerState);
	SQRNetworkPlayer::InitSendData *ISD = (SQRNetworkPlayer::InitSendData *)data;
	bool wasReady = player->IsReady();
	player->InitialDataReceived(ISD);
	bool isReady = player->IsReady();
	LeaveCriticalSection(&m_csPlayerState);
	// Sync room data back out as we've updated a player's UID here
	SyncRoomData();

	if( ( !wasReady ) && ( isReady ) )
	{
		HandlePlayerJoined( player );
	}
}

// For non-network players, flag that it is complete/ready, and assign its small id. We don't want to call any callbacks for these, as that can be explicitly done when local players are added.
// Also, we dynamically destroy & recreate local players quite a lot when remapping player slots which would create a lot of messages we don't want.
void SQRNetworkManager_PS3::NonNetworkPlayerComplete(SQRNetworkPlayer *player, unsigned char smallId)
{
	player->ConnectionComplete();
	player->SmallIdAllocated(smallId);
}

void SQRNetworkManager_PS3::HandlePlayerJoined(SQRNetworkPlayer *player)
{
	if( m_listener )
	{
		m_listener->HandlePlayerJoined( player );
	}
	// On client, keep a count of how many local players we have told the game about. We can only transition to telling the game that we are playing once the room is set up And all the local players are valid to use.
	if( !m_isHosting )
	{
		if( player->IsLocal() )
		{
			m_localPlayerJoined++;
		}
	}
}

// Selects a random server from the current list, removes that server so it won't be searched for again, and then kick off an attempt to find out if that particular server is available.
bool SQRNetworkManager_PS3::SelectRandomServer()
{
	assert( (m_state == SNM_INT_STATE_HOSTING_SEARCHING_FOR_SERVER) || (m_state == SNM_INT_STATE_JOINING_SEARCHING_FOR_SERVER) );

	if( m_serverCount == 0 )
	{
		SetState((m_state == SNM_INT_STATE_HOSTING_SEARCHING_FOR_SERVER) ? SNM_INT_STATE_HOSTING_SERVER_SEARCH_FAILED : SNM_INT_STATE_JOINING_SERVER_SEARCH_FAILED);
		app.DebugPrintf("SQRNetworkManager::SelectRandomServer - Server count is 0\n");
		return false;
	}

	int serverIdx;

	sys_get_random_number(&serverIdx, 4);
	serverIdx &= 0x7fffffff;
	serverIdx = serverIdx % m_serverCount;

	SceNpMatching2GetServerInfoRequest	reqParam;

	//E Request parameters
	memset(&reqParam, 0, sizeof(reqParam));
	reqParam.serverId = m_aServerId[serverIdx]; //E Specify ID of target server

	// Remove this server from ones to try, if we need to try more than one to find an available one
	m_serverCount--;
	m_aServerId[serverIdx] = m_aServerId[m_serverCount];

	// Kick off the search - we'll get a callback to DefaultRequestCallback with the result from this
	app.DebugPrintf(CMinecraftApp::USER_RR,"Kicking off sceNpMatching2GetServerInfo for server id %d\n",reqParam.serverId);
	int ret = sceNpMatching2GetServerInfo( m_matchingContext, &reqParam, NULL, &m_serverSearchRequestId);
	if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_GET_SERVER_INFO) )
	{
		// Jump straight to an error state for this server
		SetState((m_state == SNM_INT_STATE_HOSTING_SEARCHING_FOR_SERVER) ? SNM_INT_STATE_HOSTING_SERVER_SEARCH_SERVER_ERROR : SNM_INT_STATE_JOINING_SERVER_SEARCH_SERVER_ERROR);
		app.DebugPrintf("SQRNetworkManager::SelectRandomServer - An error occured : 0x%08x\n", ret);
		return false;
	}
	return true;
}

// Delete the current server context. Should be called when finished with the current host or client game session.
void SQRNetworkManager_PS3::DeleteServerContext()
{
	if( m_serverContextValid )
	{
		SceNpMatching2DeleteServerContextRequest  reqParam;
		reqParam.serverId = m_serverId;
		m_serverContextValid = false;
		SetState(SNM_INT_STATE_SERVER_DELETING_CONTEXT);
		int ret = sceNpMatching2DeleteServerContext( m_matchingContext, &reqParam, NULL, &m_serverContextRequestId );
		if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_DELETE_SERVER_CONTEXT) )
		{
			ResetToIdle();
		}
	}
	else
	{
		ResetToIdle();
	}
}

// Creates a set of Rudp connections by the "active open" method. This requires that both ends of the connection call cellRudpInitiate to fully create a connection. We
// create one connection per local play on any remote machine.
//
// peerMemberId is the room member Id of the remote end of the connection
// playersMemberId is the room member Id that the players belong to
// ie for the host (when matching incoming connections), these will be the same thing... and for the client, peerMemberId will be the host, whereas playersMemberId will be itself

bool SQRNetworkManager_PS3::CreateRudpConnections(SceNpMatching2RoomId roomId, SceNpMatching2RoomMemberId peerMemberId, int playerMask, SceNpMatching2RoomMemberId playersMemberId)
{
	// First get details of the UDPP2P connection that has been established
	int connStatus;
	sockaddr_in_p2p sinp2pLocal, sinp2pPeer;
	SceNpMatching2SignalingNetInfo netInfo;

	// Local end first...
	memset(&sinp2pLocal, 0, sizeof(sinp2pLocal));
	memset(&netInfo, 0 , sizeof(netInfo));
	netInfo.size = sizeof(netInfo);
	int ret = sceNpMatching2SignalingGetLocalNetInfo(&netInfo);
	if( ret < 0 ) return false;
	sinp2pLocal.sin_family = AF_INET;
	sinp2pLocal.sin_port = htons(SCE_NP_PORT);
//	sinp2pLocal.sin_addr = netInfo.localAddr;

	// ... and then the peer
	memset(&sinp2pPeer, 0, sizeof(sinp2pPeer));
	sinp2pPeer.sin_family = AF_INET;

	ret = sceNpMatching2SignalingGetConnectionStatus(m_matchingContext, roomId, peerMemberId, &connStatus, &sinp2pPeer.sin_addr, &sinp2pPeer.sin_port);
	app.DebugPrintf(CMinecraftApp::USER_RR,"sceNpMatching2SignalingGetConnectionStatus returned 0x%x, connStatus %d peer add:0x%x peer port:0x%x\n",ret, connStatus,sinp2pPeer.sin_addr,sinp2pPeer.sin_port);


	// Set vport for both ends of connection
	sinp2pLocal.sin_vport = htons(1);
	sinp2pPeer.sin_vport = htons(1);

	// Create socket & bind, if we don't already have one
	if( m_soc == -1 )
	{
		ret = socket(AF_INET, SOCK_DGRAM_P2P, 0);
		m_soc = ret;
		int optval = 1;
		ret = setsockopt(m_soc, SOL_SOCKET, SO_USECRYPTO, &optval, sizeof(optval));
		if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_SETSOCKOPT_0) ) return false;
		ret = setsockopt(m_soc, SOL_SOCKET, SO_USESIGNATURE, &optval, sizeof(optval));
		if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_SETSOCKOPT_1) ) return false;
		ret = setsockopt(m_soc, SOL_SOCKET, SO_NBIO, &optval,	sizeof(optval));
		if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_SETSOCKOPT_2) ) return false;

		ret = bind(m_soc, (struct sockaddr *)&sinp2pLocal, sizeof(sinp2pLocal));
		if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_SOCK_BIND) ) return false;
	}

	// Create an Rudp context for each local player that is required. These can be used as individual virtual connections between room members (ie consoles), which are multiplexed
	// over the socket we have just made
	for( int i = 0; i < MAX_LOCAL_PLAYER_COUNT; i++ )
	{
		if( ( playerMask & ( 1 << i ) ) == 0 ) continue;

		int rudpCtx;

		// Socket for the local network node created, now can create an Rupd context.
		ret = cellRudpCreateContext( RudpContextCallback, this, &rudpCtx );
		if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_CREATE_RUDP_CONTEXT) ) return false;
		if( m_isHosting )
		{
			m_RudpCtxToPlayerMap[ rudpCtx ] = new SQRNetworkPlayer( this, SQRNetworkPlayer::SNP_TYPE_REMOTE, true, playersMemberId, i, rudpCtx, NULL );
		}
		else
		{
			// Local players can establish their UID at this point
			PlayerUID localUID;
			ProfileManager.GetXUID(i,&localUID,true);

			m_RudpCtxToPlayerMap[ rudpCtx ] = new SQRNetworkPlayer( this, SQRNetworkPlayer::SNP_TYPE_LOCAL, false, m_localMemberId, i, rudpCtx, &localUID );
		}

		// If we've created a player, then we want to try and patch up any connections that we should have to it
		MapRoomSlotPlayers();

		// TODO - set any non-default options for the context. By default, the context is set to have delivery critical and order critical both on

		// Bind the context to the socket we've just created, and initiate. The initiation needs to happen on both client & host sides of the connection to complete.
		ret = cellRudpBind( rudpCtx, m_soc , 1 + i, CELL_RUDP_MUXMODE_P2P );
		if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_RUDP_BIND) ) return false;

		ret = cellRudpInitiate( rudpCtx, (struct sockaddr *)&sinp2pPeer, sizeof(sinp2pPeer), 0);
		if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_RUDP_INIT2) ) return false;
	}
	return true;
}


SQRNetworkPlayer *SQRNetworkManager_PS3::GetPlayerFromRudpCtx(int rudpCtx)
{
    auto it = m_RudpCtxToPlayerMap.find(rudpCtx);
    if( it != m_RudpCtxToPlayerMap.end() )
	{
		return it->second;
	}
	return NULL;
}

SQRNetworkPlayer *SQRNetworkManager_PS3::GetPlayerFromRoomMemberAndLocalIdx(int roomMember, int localIdx)
{
    for (auto it = m_RudpCtxToPlayerMap.begin(); it != m_RudpCtxToPlayerMap.end(); it++)
    {
		if( (it->second->m_roomMemberId == roomMember ) && ( it->second->m_localPlayerIdx == localIdx ) )
		{
			return it->second;
		}
	}
	return NULL;
}


// This is called as part of the general initialisation of the network manager, to register any callbacks that the sony libraries require.
// Returns true if all were registered successfully.
bool SQRNetworkManager_PS3::RegisterCallbacks()
{
	// Register RUDP event handler
	int ret = cellRudpSetEventHandler(RudpEventCallback, this);
	if (ret < 0)
	{
		app.DebugPrintf("SQRNetworkManager::RegisterCallbacks - cellRudpSetEventHandler failed with code 0x%08x\n", ret);
		return false;
	}

	// Register the context callback function
	ret = sceNpMatching2RegisterContextCallback(m_matchingContext, ContextCallback, this);
	if (ret < 0)
	{
		app.DebugPrintf("SQRNetworkManager::RegisterCallbacks - sceNpMatching2RegisterContextCallback failed with code 0x%08x\n", ret);
		return false;
	}

	// Register the default request callback & parameters
	SceNpMatching2RequestOptParam optParam;

	memset(&optParam, 0, sizeof(optParam));
	optParam.cbFunc = DefaultRequestCallback;
	optParam.cbFuncArg = this;
	optParam.timeout = (30 * 1000 * 1000);
	optParam.appReqId = 0;

	ret = sceNpMatching2SetDefaultRequestOptParam(m_matchingContext, &optParam);
	if (ret < 0)
	{
		app.DebugPrintf("SQRNetworkManager::RegisterCallbacks - sceNpMatching2SetDefaultRequestOptParam failed with code 0x%08x\n", ret);
		return false;
	}

	// Register signalling callback
	ret = sceNpMatching2RegisterSignalingCallback(m_matchingContext, SignallingCallback, this);
	if (ret < 0)
	{
		app.DebugPrintf("SQRNetworkManager::RegisterCallbacks - sceNpMatching2RegisterSignalingCallback failed with code 0x%08x\n", ret);
		return false;
	}

	// Register room event callback
	ret = sceNpMatching2RegisterRoomEventCallback(m_matchingContext, RoomEventCallback, this);
	if (ret < 0)
	{
		app.DebugPrintf("SQRNetworkManager::RegisterCallbacks - sceNpMatching2RegisterRoomEventCallback failed with code 0x%08x\n", ret);
		return false;
	}

	return true;
}

extern bool g_bBootedFromInvite;

// This is an implementation of SceNpMatching2ContextCallback. Used to determine whether the matching 2 context is valid or not.
void SQRNetworkManager_PS3::ContextCallback(SceNpMatching2ContextId  id, SceNpMatching2Event event, SceNpMatching2EventCause eventCause, int errorCode, void *arg)
{
	SQRNetworkManager_PS3 *manager = (SQRNetworkManager_PS3 *)arg;
	if (id != manager->m_matchingContext)
	{
        return;
	}

	switch( event )
	{
		case SCE_NP_MATCHING2_CONTEXT_EVENT_Start:
			if(errorCode < 0)
			{
				if(manager->m_state == SNM_INT_STATE_IDLE_RECREATING_MATCHING_CONTEXT ||
					manager->m_state == SNM_INT_STATE_HOSTING_STARTING_MATCHING_CONTEXT ||
					manager->m_state == SNM_INT_STATE_JOINING_STARTING_MATCHING_CONTEXT)
				{

					// matching context failed to start (this can happen when you block the IP addresses of the matching servers on your router
					// agent-0101.ww.sp-int.matching.playstation.net (198.107.157.191)
					// static-resource.sp-int.community.playstation.net (203.105.77.140)
					manager->SetState(SNM_INT_STATE_INITIALISE_FAILED);
					break;
				}
			}
			// Some special cases to detect when this event is coming in, in case we had to start the matching context because there wasn't a valid context when we went to get a server context. These two
			// responses here complete what should then happen to get the server context in each case (for hosting or joining a game)
			if( manager->m_state == SNM_INT_STATE_IDLE_RECREATING_MATCHING_CONTEXT )
			{

				manager->SetState( SNM_INT_STATE_IDLE );
				manager->GetExtDataForRoom(0, NULL, NULL, NULL);
				break;
			}

			if( manager->m_state == SNM_INT_STATE_HOSTING_STARTING_MATCHING_CONTEXT )
			{
				manager->GetServerContext2();
				break;
			}
			if( manager->m_state == SNM_INT_STATE_JOINING_STARTING_MATCHING_CONTEXT )
			{
				manager->SetState(SNM_INT_STATE_JOINING_SEARCHING_FOR_SERVER);
				manager->SelectRandomServer();
				break;
			}
			// Normal handling of context starting, from standard initialisation procedure
			assert( manager->m_state == SNM_INT_STATE_STARTING_CONTEXT );
			if (errorCode < 0)
			{
				manager->SetState(SNM_INT_STATE_INITIALISE_FAILED);
			}
			else
			{
				manager->m_offlineSQR = false;
				manager->SetState(SNM_INT_STATE_IDLE);

				// 4J-PB - SQRNetworkManager_PS3::AttemptPSNSignIn was causing crashes in Iggy by calling LoadMovie from a callback, so call it from the tick instead
				m_bCallPSNSignInCallback=true;
// 				if(s_SignInCompleteCallbackFn)
// 				{
// 					s_SignInCompleteCallbackFn(s_SignInCompleteParam, true, 0);
// 					s_SignInCompleteCallbackFn = NULL;
// 				}


				// Check to see if we were booted from an invite. Only do this once, the first time we have all our networking stuff set up on boot-up
				if( manager->m_doBootInviteCheck )
				{
					unsigned int type, attributes;
					CellGameContentSize gameSize;
					char dirName[CELL_GAME_DIRNAME_SIZE];

					if( g_bBootedFromInvite )
					{
						manager->GetInviteDataAndProcess(SCE_NP_BASIC_SELECTED_INVITATION_DATA);
						manager->m_doBootInviteCheck  = false;
					}
				}
			}
			break;
		case SCE_NP_MATCHING2_CONTEXT_EVENT_Stop:
			assert(false);
			break;
		case SCE_NP_MATCHING2_CONTEXT_EVENT_StartOver:
			sceNpMatching2DestroyContext(manager->m_matchingContext);
			manager->m_matchingContextValid = false;
			manager->m_offlineSQR = true;
			break;
	}
}

// This is an implementation of SceNpMatching2RequestCallback. This callback is used by default for any matching 2 request functions.
#ifdef __PS3__
void SQRNetworkManager_PS3::DefaultRequestCallback(SceNpMatching2ContextId id, SceNpMatching2RequestId reqId, SceNpMatching2Event event, SceNpMatching2EventKey eventKey, int errorCode, size_t dataSize, void *arg)
#else
void SQRNetworkManager_PS3::DefaultRequestCallback(SceNpMatching2ContextId id, SceNpMatching2RequestId reqId, SceNpMatching2Event event, int errorCode, const void *data, void *arg)
#endif
{
	int ret;
	SQRNetworkManager_PS3 *manager = (SQRNetworkManager_PS3 *)arg;
	if( id != manager->m_matchingContext ) return;

	// PS3 requires a call to get any data that is passed with the callback. On PS3, the data is just passed in directly - do the getting of data here on PS3 to get it into the same state as the PS4
#ifdef __PS3__
	void *data = 0;
	bool dataError = false;

	if( errorCode == 0 )
	{
		switch( event )
		{
			case SCE_NP_MATCHING2_REQUEST_EVENT_GetServerInfo:
				assert(dataSize <= SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_GetServerInfo);
				ret = sceNpMatching2GetEventData( id, eventKey, (void *)(manager->cServerInfoStorage), dataSize );
				if ( (ret >= 0) && !manager->ForceErrorPoint(SNM_FORCE_ERROR_GET_SERVER_INFO_DATA) )
				{
					data = (void *)manager->cServerInfoStorage;
				}
				else
				{
					dataError = true;
				}
				break;
			case SCE_NP_MATCHING2_REQUEST_EVENT_GetWorldInfoList:
				assert( dataSize <= SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_GetWorldInfoList );
				ret = sceNpMatching2GetEventData(manager->m_matchingContext, eventKey, (void *)(manager->cWorldListStorage), SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_GetWorldInfoList);
				if ( (ret >= 0) && !manager->ForceErrorPoint(SNM_FORCE_ERROR_GET_WORLD_INFO_DATA) )
				{
					data = (void *)manager->cWorldListStorage;
				}
				else
				{
					dataError = true;
				}
				break;
			case SCE_NP_MATCHING2_REQUEST_EVENT_CreateJoinRoom:
				assert( dataSize <= SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_CreateJoinRoom );
				ret = sceNpMatching2GetEventData(manager->m_matchingContext, eventKey, (void *)(manager->cCreateJoinRoomStorage), SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_CreateJoinRoom);
				if ( (ret >= 0) && !manager->ForceErrorPoint(SNM_FORCE_ERROR_GET_CREATE_JOIN_ROOM_DATA) )
				{
					data = (void *)manager->cCreateJoinRoomStorage;
				}
				else
				{
					dataError = true;
				}
				break;
			case SCE_NP_MATCHING2_REQUEST_EVENT_GetUserInfoList:
				assert(dataSize <= SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_GetUserInfoList);
				ret = sceNpMatching2GetEventData(manager->m_matchingContext, eventKey, (void *)(manager->cUserInfoListStorage), SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_GetUserInfoList);
				if ( (ret >= 0) && !manager->ForceErrorPoint(SNM_FORCE_ERROR_GET_USER_INFO_LIST_DATA) )
				{
					data = (void *)manager->cUserInfoListStorage;
				}
				else
				{
					dataError = true;
				}
				break;
			case SCE_NP_MATCHING2_REQUEST_EVENT_JoinRoom:
				assert( dataSize <= SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_JoinRoom);
				ret = sceNpMatching2GetEventData(manager->m_matchingContext, eventKey, (void *)(manager->cJoinRoomStorage), SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_JoinRoom);
				if ( (ret >= 0) && !manager->ForceErrorPoint(SNM_FORCE_ERROR_GET_JOIN_ROOM_DATA) )
				{
					data = (void *)manager->cJoinRoomStorage;
				}
				else
				{
					dataError = true;
				}
				break;
			case SCE_NP_MATCHING2_REQUEST_EVENT_GetRoomMemberDataInternal:
				assert( dataSize <= SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_GetRoomMemberDataInternal );
				ret = sceNpMatching2GetEventData(manager->m_matchingContext, eventKey, (void *)(manager->cRoomMemberDataInternal), SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_GetRoomMemberDataInternal);
				if ( (ret >= 0) && !manager->ForceErrorPoint(SNM_FORCE_ERROR_GET_ROOM_MEMBER_DATA_INTERNAL) )
				{
					data = (void *)manager->cRoomMemberDataInternal;
				}
				else
				{
					dataError = true;
				}
				break;
			case SCE_NP_MATCHING2_REQUEST_EVENT_GetRoomDataExternalList:
				assert( dataSize <= SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_GetRoomDataExternalList );
				ret = sceNpMatching2GetEventData(manager->m_matchingContext, eventKey, (void *)(manager->cRoomDataExternalList), SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_GetRoomDataExternalList);
				if ( (ret >= 0) && !manager->ForceErrorPoint(SNM_FORCE_ERROR_GET_ROOM_EXTERNAL_DATA2) )
				{
					data = (void *)manager->cRoomDataExternalList;
				}
				else
				{
					dataError = true;
				}
				break;
		}
	}

	// If there was some problem getting data, then silently ignore as we don't want to go any further with a NULL data pointer, and this will just be indicating that the networking context
	// is invalid which will be picked up elsewhere to shut everything down properly
	if( dataError )
	{
		return;
	}

	// If we didn't get the event data, then we need to clear it, or the system even queue will overflow
	if( data == 0 )
	{
		sceNpMatching2ClearEventData(manager->m_matchingContext, eventKey);
	}
#endif

	switch( event )
	{
		// This is the responese to a call to sceNpMatching2GetServerInfo, used when we are trying to establish an available server to get an explicit server context for, which is the process
		// kicked off by the GetServerContext methods.
		case SCE_NP_MATCHING2_REQUEST_EVENT_GetServerInfo:
			if( errorCode == SCE_NP_MATCHING2_ERROR_NP_SIGNED_OUT )
			{
				// If we've already signed out, then we should have detected this already elsewhere and so can silently ignore any errors coming in for pending requests here
				break;
			}
			assert( (manager->m_state == SNM_INT_STATE_HOSTING_SEARCHING_FOR_SERVER) || (manager->m_state == SNM_INT_STATE_JOINING_SEARCHING_FOR_SERVER) );
			app.DebugPrintf(CMinecraftApp::USER_RR,"Response to server search received, errorCode 0x%x\n",errorCode);
			if( errorCode == 0 )
			{
				if( data != 0 )
				{
					SceNpMatching2GetServerInfoResponse *pServerInfo = (SceNpMatching2GetServerInfoResponse *)data;
					if (pServerInfo->server.status == SCE_NP_MATCHING2_SERVER_STATUS_AVAILABLE)
					{
						app.DebugPrintf(CMinecraftApp::USER_RR,"Server status available\n");
						// This server is available
							manager->SetState((manager->m_state == SNM_INT_STATE_HOSTING_SEARCHING_FOR_SERVER) ? SNM_INT_STATE_HOSTING_SERVER_FOUND : SNM_INT_STATE_JOINING_SERVER_FOUND);
							manager->m_serverId = pServerInfo->server.serverId;
							break;
					}
				}
			}
			// A problem of some kind with this particular server - let the tick decide what to do
			manager->SetState((manager->m_state == SNM_INT_STATE_HOSTING_SEARCHING_FOR_SERVER) ? SNM_INT_STATE_HOSTING_SERVER_SEARCH_SERVER_ERROR : SNM_INT_STATE_JOINING_SERVER_SEARCH_SERVER_ERROR);
			break;
		// This is the response to sceNpMatching2GetWorldInfoList, which is called as part of the process to create a room (which needs a world to be created in). We aren't anticipating
		// using worlds in a meaningful way so just getting the first world we find on the server here, and then advancing the state so that the tick can get on with the rest of the process.
		case SCE_NP_MATCHING2_REQUEST_EVENT_GetWorldInfoList:
			if( errorCode == SCE_NP_MATCHING2_ERROR_NP_SIGNED_OUT )
			{
				// If we've already signed out, then we should have detected this already elsewhere and so can silently ignore any errors coming in for pending requests here
				break;
			}
			assert( manager->m_state == SNM_INT_STATE_HOSTING_CREATE_ROOM_SEARCHING_FOR_WORLD );
			if( errorCode == 0 )
			{
				if( data != 0 )
				{
					// Currently just using first world - this may well be all that we need anyway
					SceNpMatching2GetWorldInfoListResponse *pWorldList = (SceNpMatching2GetWorldInfoListResponse *)data;
					if( pWorldList->worldNum >= 1 )
					{
						manager->m_worldId = pWorldList->world[0].worldId;
						manager->SetState(SNM_INT_STATE_HOSTING_CREATE_ROOM_WORLD_FOUND);
						break;
					}
				}
			}
			manager->SetState(SNM_INT_STATE_HOSTING_CREATE_ROOM_FAILED);
			break;
		// This is the response to sceNpMatching2CreateJoinRoom, which if successful means that we are just about ready to move to an online state as host of a game. The final
		// transition actually occurs in the create room tick, on detecting that the state has transitioned to SNM_INT_STATE_HOSTING_CREATE_ROOM_SUCCESS here.
		case SCE_NP_MATCHING2_REQUEST_EVENT_CreateJoinRoom:
			if( errorCode == SCE_NP_MATCHING2_ERROR_NP_SIGNED_OUT )
			{
				// If we've already signed out, then we should have detected this already elsewhere and so can silently ignore any errors coming in for pending requests here
				break;
			}
			app.DebugPrintf(CMinecraftApp::USER_RR,">> Creating room complete, time taken %d, error 0x%x\n",System::currentTimeMillis()-s_roomStartTime, errorCode);
			assert( manager->m_state == SNM_INT_STATE_HOSTING_CREATE_ROOM_CREATING_ROOM );
			if( errorCode == 0 )
			{
				if( data != 0 )
				{
					SceNpMatching2CreateJoinRoomResponse *roomData = (SceNpMatching2CreateJoinRoomResponse *)data;
					manager->m_localMemberId = roomData->roomDataInternal->memberList.me->memberId;

					manager->SetState(SNM_INT_STATE_HOSTING_CREATE_ROOM_SUCCESS);
					manager->m_room = roomData->roomDataInternal->roomId;
					break;
				}
			}
			manager->SetState(SNM_INT_STATE_HOSTING_CREATE_ROOM_FAILED);
			break;
		// This is the response to sceNpMatching2CreateServerContext, which is the final thing that happens in the chain of states kicked off by GetServerContext, and happens
		// after we've found an available server. If this context creation is successful, then we can proceed to create or join a room - we can determine which to
		// do by virtue of the current state.
		case SCE_NP_MATCHING2_REQUEST_EVENT_CreateServerContext:
			if( errorCode == SCE_NP_MATCHING2_ERROR_NP_SIGNED_OUT )
			{
				// If we've already signed out, then we should have detected this already elsewhere and so can silently ignore any errors coming in for pending requests here
				break;
			}
			assert( ( manager->m_state == SNM_INT_STATE_HOSTING_SERVER_SEARCH_CREATING_CONTEXT ) || ( manager->m_state == SNM_INT_STATE_JOINING_SERVER_SEARCH_CREATING_CONTEXT ) );
			if( manager->m_state == SNM_INT_STATE_HOSTING_SERVER_SEARCH_CREATING_CONTEXT )
			{
				if(( errorCode == 0 ) && !manager->ForceErrorPoint(SNM_FORCE_ERROR_CREATE_SERVER_CONTEXT_CALLBACK) )
				{
					manager->m_serverContextValid = true;
					manager->ServerContextValid_CreateRoom();
					break;
				}
				manager->SetState(SNM_INT_STATE_HOSTING_SERVER_SEARCH_FAILED);
			}
			else
			{
				if(( errorCode == 0 ) && !manager->ForceErrorPoint(SNM_FORCE_ERROR_CREATE_SERVER_CONTEXT_CALLBACK) )
				{
					manager->m_serverContextValid = true;
						manager->ServerContextValid_JoinRoom();
					break;
				}
				manager->SetState(SNM_INT_STATE_JOINING_SERVER_SEARCH_FAILED);
			}
			break;
		// This is the response to sceNpMatching2DeleteServerContext.
		case SCE_NP_MATCHING2_REQUEST_EVENT_DeleteServerContext:
//			assert( manager->m_state == SNM_INT_STATE_SERVER_DELETING_CONTEXT );
			manager->ResetToIdle();
			break;
		// This is the response to sceNpMatching2JoinRoom, which is called as the final stage of the process started when calling the JoinRoom method. If this is successful, then
		// the state can change to SNM_INT_STATE_JOINING_WAITING_FOR_LOCAL_PLAYERS. We can transition out of that state once we have told the application that all the local players
		// have joined.
		case SCE_NP_MATCHING2_REQUEST_EVENT_JoinRoom:
			assert( manager->m_state == SNM_INT_STATE_JOINING_JOIN_ROOM);
			if( errorCode == 0 )
			{
				if( data != 0 )
				{
					SceNpMatching2JoinRoomResponse *roomData = (SceNpMatching2JoinRoomResponse *)data;

					manager->m_localMemberId = roomData->roomDataInternal->memberList.me->memberId;
					manager->m_room = roomData->roomDataInternal->roomId;
					SonyVoiceChat::init(manager);
					// Copy over initial room sync data
					for( int i = 0; i < roomData->roomDataInternal->roomBinAttrInternalNum; i++ )
					{
						if( roomData->roomDataInternal->roomBinAttrInternal[i].data.id == SCE_NP_MATCHING2_ROOM_BIN_ATTR_INTERNAL_1_ID )
						{
							assert( roomData->roomDataInternal->roomBinAttrInternal[i].data.size == sizeof( manager->m_roomSyncData ) );
							memcpy( &manager->m_roomSyncData, roomData->roomDataInternal[i].roomBinAttrInternal[0].data.ptr, sizeof( manager->m_roomSyncData ) );

//							manager->UpdatePlayersFromRoomSyncUIDs();
							// Update mapping from the room slot players to SQRNetworkPlayer instances
							manager->MapRoomSlotPlayers();
							break;
						}
					}
					manager->SetState(SNM_INT_STATE_JOINING_WAITING_FOR_LOCAL_PLAYERS);
					break;
				}
			}
			manager->SetState(SNM_INT_STATE_JOINING_JOIN_ROOM_FAILED);
			if(errorCode == SCE_NP_MATCHING2_SERVER_ERROR_ROOM_FULL) // MGH - added to fix "host has exited" error when 2 players go after the final slot
			{
				Minecraft::GetInstance()->connectionDisconnected(ProfileManager.GetPrimaryPad(), DisconnectPacket::eDisconnect_ServerFull);
			}
			break;
		// This is the response to sceNpMatching2GetRoomMemberDataInternal.This only happens on the host, as a response to an incoming connection being established, when we
		// kick off the request for room member internal data so that we can determine what local players that remote machine is intending to bring into the game. At this point we can
		// activate the host end of each of the Rupd connection that this machine requires. We can also update our player slot data (which gets syncronised back out to other room members) at this point.
		case SCE_NP_MATCHING2_REQUEST_EVENT_GetRoomMemberDataInternal:
			if( errorCode == 0 )
			{
				assert( manager->m_isHosting );
				if( data != 0 )
				{
					SceNpMatching2GetRoomMemberDataInternalResponse *pRoomMemberData = (SceNpMatching2GetRoomMemberDataInternalResponse *)data;
					assert( pRoomMemberData->roomMemberDataInternal->roomMemberBinAttrInternalNum == 1 );
					int playerMask = *((int *)(pRoomMemberData->roomMemberDataInternal->roomMemberBinAttrInternal->data.ptr));

					bool isFull = false;
					bool success = manager->AddRemotePlayersAndSync( pRoomMemberData->roomMemberDataInternal->memberId, playerMask, &isFull );
					if( success )
					{
						success = manager->CreateRudpConnections(manager->m_room, pRoomMemberData->roomMemberDataInternal->memberId, playerMask, pRoomMemberData->roomMemberDataInternal->memberId);
						if( success ) break;
					}
					// Something has gone wrong adding these players to the room - kick out the player
					SceNpMatching2KickoutRoomMemberRequest reqParam;
					SceNpMatching2PresenceOptionData optParam;
					memset(&reqParam,0,sizeof(reqParam));
					reqParam.roomId = manager->m_room;
					reqParam.target = pRoomMemberData->roomMemberDataInternal->memberId;
					// Set flag to indicate whether we were kicked for being out of room or not
					reqParam.optData.data[0] = isFull ? 1 : 0;
					reqParam.optData.len = 1;
					int ret = sceNpMatching2KickoutRoomMember(manager->m_matchingContext, &reqParam, NULL, &manager->m_kickRequestId);
					app.DebugPrintf(CMinecraftApp::USER_RR,"sceNpMatching2KickoutRoomMember returns error 0x%x\n",ret);
					break;
				}
			}
			break;
		case SCE_NP_MATCHING2_REQUEST_EVENT_LeaveRoom:
			// This is the response to sceNpMatching2LeaveRoom - from the Sony docs, this doesn't ever fail so no need to do error checking here
			SonyVoiceChat::signalDisconnected();
			assert(manager->m_state == SNM_INT_STATE_LEAVING );
			manager->DeleteServerContext();
			break;
		// This is the response to SceNpMatching2GetRoomDataExternalListRequest, which happens when we request the full details of a room we are interested in joining
		case SCE_NP_MATCHING2_REQUEST_EVENT_GetRoomDataExternalList:
			if( errorCode == 0 )
			{
				if( data != 0 )
				{
					SceNpMatching2GetRoomDataExternalListResponse *pExternalData = (SceNpMatching2GetRoomDataExternalListResponse *)data;
					SceNpMatching2RoomDataExternal *pRoomExtData = pExternalData->roomDataExternal;
					if( pExternalData->roomDataExternalNum == 1 )
					{
						if(pRoomExtData->roomBinAttrExternalNum == 1 )
						{
							memcpy(manager->m_pExtDataToUpdate, pRoomExtData->roomBinAttrExternal[0].ptr,pRoomExtData->roomBinAttrExternal[0].size);
							manager->m_FriendSessionUpdatedFn(true, manager->m_pParamFriendSessionUpdated);
						}
						else
						{
							manager->m_FriendSessionUpdatedFn(false, manager->m_pParamFriendSessionUpdated);
						}
					}
					else
					{
						manager->m_FriendSessionUpdatedFn(true, manager->m_pParamFriendSessionUpdated);
					}
				}
				else
				{
					manager->m_FriendSessionUpdatedFn(false, manager->m_pParamFriendSessionUpdated);
				}
			}
			else
			{
				manager->m_FriendSessionUpdatedFn(false, manager->m_pParamFriendSessionUpdated);
			}
			break;
		case SCE_NP_MATCHING2_REQUEST_EVENT_SetRoomDataExternal:
			if( ( errorCode != 0 ) || manager->ForceErrorPoint(SNM_FORCE_ERROR_SET_ROOM_DATA_CALLBACK) )
			{
				app.DebugPrintf(CMinecraftApp::USER_RR,"Error updating external data 0x%x (from SCE_NP_MATCHING2_REQUEST_EVENT_SetRoomDataExternal event in callback)\n",errorCode);
				// If we ever fail to send the external room data, we start a countdown so that we attempt to resend. Not sure how likely it is that updating this will fail without the whole network being broken,
				// but if in particular we don't update the flag to say that the session is joinable, then nobody is ever going to see this session.
				manager->m_resendExternalRoomDataCountdown = 60;
			}
			break;
	};
}

// This is an implementation of SceNpMatching2RoomEventCallback.
#ifdef __PS3__
void SQRNetworkManager_PS3::RoomEventCallback(SceNpMatching2ContextId id, SceNpMatching2RoomId roomId, SceNpMatching2Event event, SceNpMatching2EventKey eventKey, int errorCode, size_t dataSize, void *arg)
#else
void SQRNetworkManager_PS3::RoomEventCallback(SceNpMatching2ContextId id, SceNpMatching2RoomId roomId, SceNpMatching2Event event, int errorCode, const void *data, void *arg)
#endif
{
	SQRNetworkManager_PS3 *manager = (SQRNetworkManager_PS3 *)arg;

	bool gotEventData = false;
	switch( event )
	{
		case SCE_NP_MATCHING2_ROOM_EVENT_MemberJoined:
			break;
		case SCE_NP_MATCHING2_ROOM_EVENT_MemberLeft:
			break;
		case SCE_NP_MATCHING2_ROOM_EVENT_Kickedout:
			{
				SonyVoiceChat::signalRoomKickedOut();
				// We've been kicked out. This server has rejected our attempt to join, most likely because there wasn't enough space in the server to have us. There's a flag set
				// so we can determine which thing has happened
				assert ( dataSize <= SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomUpdateInfo );
				int ret = sceNpMatching2GetEventData( manager->m_matchingContext, eventKey, manager->cRoomDataUpdateInfo, SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomUpdateInfo);
				app.DebugPrintf(CMinecraftApp::USER_RR,"SCE_NP_MATCHING2_ROOM_EVENT_Kickedout, sceNpMatching2GetEventData returning 0x%x\n",ret);

				bool bIsFull = false;
				if( ( ret >= 0 ) && !manager->ForceErrorPoint(SNM_FORCE_ERROR_UPDATED_ROOM_DATA) )
				{
					gotEventData = true;
					SceNpMatching2RoomUpdateInfo *pUpdateInfo = (SceNpMatching2RoomUpdateInfo *)(manager->cRoomDataUpdateInfo);
					if( pUpdateInfo->optData.len == 1 )
					{
						if( pUpdateInfo->optData.data[0] == 1 )
						{
							bIsFull = true;
						}
					}
				}
				app.DebugPrintf(CMinecraftApp::USER_RR,"IsFull determined to be %d\n",bIsFull);
				if( bIsFull )
				{
					manager->m_nextIdleReasonIsFull = true;
				}
				manager->ResetToIdle();
			}
			break;
		case SCE_NP_MATCHING2_ROOM_EVENT_RoomDestroyed:
			{
				SonyVoiceChat::signalRoomDestroyed();
				SceNpMatching2RoomUpdateInfo *pUpdateInfo=NULL;

				if( dataSize <= SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomUpdateInfo )
				{
					int ret = sceNpMatching2GetEventData( manager->m_matchingContext, eventKey, manager->cRoomDataUpdateInfo, SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomUpdateInfo);
					app.DebugPrintf("Room destroyed, error code 0x%x\n",errorCode);
					app.DebugPrintf("SCE_NP_MATCHING2_ROOM_EVENT_RoomDestroyed, sceNpMatching2GetEventData returning 0x%x\n",ret);
					if( ret >= 0 )
					{
						pUpdateInfo = (SceNpMatching2RoomUpdateInfo *)(manager->cRoomDataUpdateInfo);
						app.DebugPrintf("Further info: Error 0x%x, cause %d\n",pUpdateInfo->errorCode,pUpdateInfo->eventCause);
					}
				}

				// If we're hosting, then handle this a bit like a disconnect, in that we will shift the game into an offline game - but don't need to actually leave the room
				// since that has been destroyed and so isn't there to be left anymore. Don't do this if we are disconnected though, as we've already handled this.
				if( ( manager->m_isHosting ) && !manager->m_bLinkDisconnected )
				{
					// MGH - we're not receiving an SCE_NP_MATCHING2_SIGNALING_EVENT_DEAD after this so we have to remove all the remote players
					while(manager->m_RudpCtxToPlayerMap.size())
					{
						SQRNetworkPlayer* pRemotePlayer = manager->m_RudpCtxToPlayerMap.begin()->second;
						manager->RemoveRemotePlayersAndSync( pRemotePlayer->m_roomMemberId, 15 );
					}

					if(pUpdateInfo && (pUpdateInfo->eventCause==SCE_NP_MATCHING2_EVENT_CAUSE_NP_SIGNED_OUT))
					{
						manager->m_listener->HandleDisconnect(true,true);
					}
					else
					{
						manager->m_listener->HandleDisconnect(true);
					}
				}

			}
			break;
		case SCE_NP_MATCHING2_ROOM_EVENT_RoomOwnerChanged:
			break;
		case SCE_NP_MATCHING2_ROOM_EVENT_UpdatedRoomDataInternal:
			// We are using the room internal data to synchronise the player data stored in m_roomSyncData from the host to clients.
			// The host is the thing creating the internal room data, so it doesn't need to update itself.
			if( !manager->m_isHosting )
			{
				assert ( dataSize <= SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomDataInternalUpdateInfo );
				int ret = sceNpMatching2GetEventData( manager->m_matchingContext, eventKey, manager->cRoomDataInternal, SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomDataInternalUpdateInfo);
				if( ( ret >= 0 ) && !manager->ForceErrorPoint(SNM_FORCE_ERROR_UPDATED_ROOM_DATA) )
				{
					gotEventData = true;
					SceNpMatching2RoomDataInternalUpdateInfo *pRoomData = (SceNpMatching2RoomDataInternalUpdateInfo *)(manager->cRoomDataInternal);
					for(int i = 0; i < pRoomData->newRoomBinAttrInternalNum; i++)
					{
						if( pRoomData->newRoomBinAttrInternal[i]->data.id == SCE_NP_MATCHING2_ROOM_BIN_ATTR_INTERNAL_1_ID )
						{
							assert( pRoomData->newRoomBinAttrInternal[i]->data.size == sizeof( manager->m_roomSyncData ) );
							memcpy( &manager->m_roomSyncData, pRoomData->newRoomBinAttrInternal[i]->data.ptr, sizeof( manager->m_roomSyncData ) );

//							manager->UpdatePlayersFromRoomSyncUIDs();
							// Update mapping from the room slot players to SQRNetworkPlayer instances
							manager->MapRoomSlotPlayers();
#if 0
							{
								printf("New player sync data arrived\n");
								for(int i = 0; i < manager->m_roomSyncData.getPlayerCount(); i++ )
								{
									printf("%d: small %d, machine %d, local %d\n",i, manager->m_roomSyncData.players[i].m_smallId, manager->m_roomSyncData.players[i].m_roomMemberId, manager->m_roomSyncData.players[i].m_localIdx);
								}
							}
#endif
							break;
						}
					}
					break;
				}
				// TODO - handle error here? What could we do?
			}

			break;
		case SCE_NP_MATCHING2_ROOM_EVENT_UpdatedRoomMemberDataInternal:
			if( ( errorCode == 0 )  && (!manager->ForceErrorPoint(SNM_FORCE_ERROR_UPDATED_ROOM_MEMBER_DATA_INTERNAL1) ) )
			{
				// We'll get this sync'd round all the connected clients, but we only care about it on the host where we can use it to work out if any RUDP connections need to be made or released
				if( manager->m_isHosting )
				{
					assert( dataSize <= SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomMemberDataInternalUpdateInfo );
					int ret = sceNpMatching2GetEventData(manager->m_matchingContext, eventKey, (void *)(manager->cRoomMemberDataInternalUpdate), SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomMemberDataInternalUpdateInfo);
					if( ( ret >= 0 ) && (!manager->ForceErrorPoint(SNM_FORCE_ERROR_UPDATED_ROOM_MEMBER_DATA_INTERNAL2) ) )
					{
						gotEventData = true;
						SceNpMatching2RoomMemberDataInternalUpdateInfo *pRoomMemberData = (SceNpMatching2RoomMemberDataInternalUpdateInfo *)(manager->cRoomMemberDataInternalUpdate);
						assert( pRoomMemberData->newRoomMemberBinAttrInternalNum == 1 );

						int playerMask = *((int *)(pRoomMemberData->newRoomMemberBinAttrInternal[0]->data.ptr));
						int oldMask = manager->GetOldMask( pRoomMemberData->newRoomMemberDataInternal->memberId );
						int addedMask = manager->GetAddedMask(playerMask, oldMask );
						int removedMask = manager->GetRemovedMask(playerMask, oldMask );

						if( addedMask != 0 )
						{
							bool success = manager->AddRemotePlayersAndSync( pRoomMemberData->newRoomMemberDataInternal->memberId, addedMask );
							if( success )
							{
								success = manager->CreateRudpConnections(manager->m_room, pRoomMemberData->newRoomMemberDataInternal->memberId, addedMask, pRoomMemberData->newRoomMemberDataInternal->memberId);
							}
							if( ( !success ) || (manager->ForceErrorPoint(SNM_FORCE_ERROR_UPDATED_ROOM_MEMBER_DATA_INTERNAL3) ) )
							{
								// Failed for some reason - signal back to the client that this is the case, by updating its internal data back again, rather than have
								// it wait for a timeout on its rudp connection initialisation.
								SceNpMatching2SetRoomMemberDataInternalRequest reqParam;
								SceNpMatching2BinAttr binAttr;

								memset(&reqParam, 0, sizeof(reqParam));
								memset(&binAttr, 0, sizeof(binAttr));

								binAttr.id = SCE_NP_MATCHING2_ROOMMEMBER_BIN_ATTR_INTERNAL_1_ID;
								binAttr.ptr = &oldMask;
								binAttr.size = sizeof(oldMask);

								reqParam.roomId		= manager->m_room;
								reqParam.memberId	= pRoomMemberData->newRoomMemberDataInternal->memberId;
								reqParam.roomMemberBinAttrInternalNum = 1;
								reqParam.roomMemberBinAttrInternal = &binAttr;

								int ret = sceNpMatching2SetRoomMemberDataInternal( manager->m_matchingContext, &reqParam, NULL, &manager->m_setRoomMemberInternalDataRequestId );
							}
						}

						if( removedMask != 0 )
						{
							manager->RemoveRemotePlayersAndSync( pRoomMemberData->newRoomMemberDataInternal->memberId, removedMask );
						}

						break;
					}
				}
				else
				{
					// If, as a client, we receive an updated room member data this could be for two reason.
					// (1) Another client in the game has updated their own data as someone has joined/left the session
					// (2) The server has set someone's data back, due to a failed attempt to join a game
					// We're only interested in scenario (2), when the data that has been updated is our own, in which case we know to abandon creating rudp connections etc. for a new player
					assert( dataSize <= SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomMemberDataInternalUpdateInfo );
					int ret = sceNpMatching2GetEventData(manager->m_matchingContext, eventKey, (void *)(manager->cRoomMemberDataInternalUpdate), SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomMemberDataInternalUpdateInfo);
					if( ( ret >= 0 ) && (!manager->ForceErrorPoint(SNM_FORCE_ERROR_UPDATED_ROOM_MEMBER_DATA_INTERNAL4) ) )
					{
						gotEventData = true;
						SceNpMatching2RoomMemberDataInternalUpdateInfo *pRoomMemberData = (SceNpMatching2RoomMemberDataInternalUpdateInfo *)(manager->cRoomMemberDataInternalUpdate);
						assert( pRoomMemberData->newRoomMemberBinAttrInternalNum == 1 );
						if(  pRoomMemberData->newRoomMemberDataInternal->memberId == manager->m_localMemberId )
						{
							int playerMask = *((int *)(pRoomMemberData->newRoomMemberBinAttrInternal[0]->data.ptr));
							if( playerMask != manager->m_localPlayerJoinMask )
							{
								int playersToRemove = manager->m_localPlayerJoinMask & (~playerMask);
								manager->RemoveNetworkPlayers( playersToRemove );
								if( manager->m_listener )
								{
									for( int i = 0; i < MAX_LOCAL_PLAYER_COUNT; i++ )
									{
										if( playersToRemove & ( 1 << i )  )
										{
											manager->m_listener->HandleAddLocalPlayerFailed(i);
											break;
										}
									}
								}
							}
						}
					}

				}
			}
			break;
		case SCE_NP_MATCHING2_ROOM_EVENT_UpdatedSignalingOptParam:
			break;
	};

	// If we didn't get the event data, then we need to clear it, or the system even queue will overflow
	if( !gotEventData )
	{
		sceNpMatching2ClearEventData(manager->m_matchingContext, eventKey);
	}
}

// This is an implementation of SceNpMatching2SignalingCallback. We configure our too automatically create a star network of connections with the host at the hub, and can respond here to
// the connections being set up to layer sockets and Rudp on top.
void SQRNetworkManager_PS3::SignallingCallback(SceNpMatching2ContextId ctxId, SceNpMatching2RoomId roomId, SceNpMatching2RoomMemberId peerMemberId, SceNpMatching2Event event, int errorCode, void *arg)
{
	SQRNetworkManager_PS3 *manager = (SQRNetworkManager_PS3 *)arg;

	switch( event )
	{
		case SCE_NP_MATCHING2_SIGNALING_EVENT_Dead:
			if( manager->m_isHosting )
			{
				// Remove any players associated with this peer
				manager->RemoveRemotePlayersAndSync( peerMemberId, 15 );
			}
			else
			{
				// Host has left the game... so its all over for this client too. Finish everything up now, including deleting the server context which belongs to this gaming session
				// This also might be a response to a request to leave the game from our end too so don't need to do anything in that case
				if( manager->m_state != SNM_INT_STATE_LEAVING )
				{
					manager->DeleteServerContext();
				}
			}
			break;
		case SCE_NP_MATCHING2_SIGNALING_EVENT_Established:
			{
				if( manager->m_isHosting )
				{
					// If we're the host, then we need to get the data associated with the connecting peer to know what connections we should be trying to match. So
					// the actual creation of connections happens when the response for this request is processed.

					SceNpMatching2GetRoomMemberDataInternalRequest  reqParam;
					memset( &reqParam, 0, sizeof(reqParam));
					reqParam.roomId = roomId;
					reqParam.memberId = peerMemberId;
					SceNpMatching2AttributeId attrs[1] = {SCE_NP_MATCHING2_ROOMMEMBER_BIN_ATTR_INTERNAL_1_ID};
					reqParam.attrId = attrs;
					reqParam.attrIdNum = 1;

					sceNpMatching2GetRoomMemberDataInternal( manager->m_matchingContext, &reqParam, NULL, &manager->m_roomMemberDataRequestId);
				}
				else
				{
					// If we are the client, then we locally know what Rupd connections we need (from m_localPlayerJoinMask) and can kick this off.
					bool ret = manager->CreateRudpConnections( roomId, peerMemberId, manager->m_localPlayerJoinMask, manager->m_localMemberId);
					if( ret == false )
					{
						manager->DeleteServerContext();
					}
				}
			}
			break;
	}
}


// Implementation of SceNpBasicEventHandler
int SQRNetworkManager_PS3::BasicEventCallback(int event, int retCode, uint32_t reqId, void *arg)
{
	SQRNetworkManager_PS3 *manager = (SQRNetworkManager_PS3 *)arg;
	// We aren't allowed to actually get the event directly from this callback, so send our own internal event to a thread dedicated to doing this
	sys_event_port_send(manager->m_basicEventPort, 0, 0, 0);
	return 0;
}

// Implementation of SceNpManagerCallback
void SQRNetworkManager_PS3::ManagerCallback(int event, int result, void *arg)
{
	SQRNetworkManager_PS3 *manager = (SQRNetworkManager_PS3 *)arg;
	switch( event )
	{
		case SCE_NP_MANAGER_STATUS_GETTING_TICKET:
			break;
		case SCE_NP_MANAGER_STATUS_GETTING_PROFILE:
			break;
		case SCE_NP_MANAGER_STATUS_LOGGING_IN:
			break;
		case SCE_NP_MANAGER_STATUS_ONLINE:
			manager->InitialiseAfterOnline();
			break;
		case SCE_NP_MANAGER_EVENT_GOT_TICKET:
			{
				// Let the profile Manager deal with any tickets
				SonyRemoteStorage_PS3* pRemote = (SonyRemoteStorage_PS3*)app.getRemoteStorage();
				if(pRemote->isWaitingForTicket())
				{
					pRemote->npauthhandler(event, result, arg);
				}
				else
				{
					ProfileManager.HandleNetworkTicket(result,arg);
				}
			}
			break;
		case SCE_NP_MANAGER_STATUS_OFFLINE:
		break;
	}
	manager->UpdateOnlineStatus(event);
}

// Implementation of CellSysutilCallback
void SQRNetworkManager_PS3::SysUtilCallback(uint64_t status, uint64_t param, void *userdata)
{
	SQRNetworkManager_PS3 *manager = (SQRNetworkManager_PS3 *)userdata;
    struct CellNetCtlNetStartDialogResult netstart_result;
    int ret = 0;
    netstart_result.size = sizeof(netstart_result);
    switch(status)
	{
		case CELL_SYSUTIL_NET_CTL_NETSTART_FINISHED:
			ret = cellNetCtlNetStartDialogUnloadAsync(&netstart_result);
			if(ret < 0)
			{
				manager->SetState(SNM_INT_STATE_INITIALISE_FAILED);
				if( s_SignInCompleteCallbackFn )
				{
					if( s_signInCompleteCallbackIfFailed )
					{
						s_SignInCompleteCallbackFn(s_SignInCompleteParam,false,0);
					}
					s_SignInCompleteCallbackFn  = NULL;
				}
				return;
			}

			if( netstart_result.result != 0 )
			{
				// Failed, or user may have decided not to sign in - maybe need to differentiate here
				manager->SetState(SNM_INT_STATE_INITIALISE_FAILED);
				if( s_SignInCompleteCallbackFn )
				{
					if( s_signInCompleteCallbackIfFailed )
					{
						s_SignInCompleteCallbackFn(s_SignInCompleteParam,false,0);
					}
					s_SignInCompleteCallbackFn  = NULL;
				}
			}

			break;
		case CELL_SYSUTIL_NET_CTL_NETSTART_UNLOADED:
			break;
		case CELL_SYSUTIL_NP_INVITATION_SELECTED:
			manager->GetInviteDataAndProcess(SCE_NP_BASIC_SELECTED_INVITATION_DATA);
			break;
		default:
			break;
    }
}

// Implementation of CellRudpContextEventHandler. This is associate with an Rudp context every time one is created, and can be used to determine the status of each
// Rudp connection. We create one context/connection per local player on the non-hosting consoles.
void SQRNetworkManager_PS3::RudpContextCallback(int ctx_id, int event_id, int error_code, void *arg)
{
	SQRNetworkManager_PS3 *manager = (SQRNetworkManager_PS3 *)arg;
	switch(event_id)
	{
		case CELL_RUDP_CONTEXT_EVENT_CLOSED:
			app.DebugPrintf(CMinecraftApp::USER_RR,"RUDP closed - event error 0x%x\n",error_code);
			if( !manager->m_isHosting )
			{
				if( manager->m_state == SNM_INT_STATE_JOINING_WAITING_FOR_LOCAL_PLAYERS )
				{
					manager->LeaveRoom(true);
				}
			}
			break;
		case CELL_RUDP_CONTEXT_EVENT_ESTABLISHED:
			{
				SQRNetworkPlayer *player = manager->GetPlayerFromRudpCtx(ctx_id);
				if( player )
				{
					// Flag connection stage as being completed for this player
					manager->NetworkPlayerConnectionComplete(player);
				}
				else
				{
					assert(false);
				}
			}
			break;
		case CELL_RUDP_CONTEXT_EVENT_ERROR:
			break;
		case CELL_RUDP_CONTEXT_EVENT_WRITABLE:
			{
				SQRNetworkPlayer *player = manager->GetPlayerFromRudpCtx(ctx_id);
				// This event signifies that room has opened up in the write buffer, so attempt to send something
				if( player )
				{
					player->SendMoreInternal();
				}
			}
			break;
		case CELL_RUDP_CONTEXT_EVENT_READABLE:
			if( manager->m_listener )
			{
				unsigned int dataSize = cellRudpGetSizeReadable(ctx_id);
				// If we're the host, and this player hasn't yet had its small id confirmed, then the first byte sent to us should be this id
				if( manager->m_isHosting )
				{
					SQRNetworkPlayer *playerFrom = manager->GetPlayerFromRudpCtx( ctx_id );
					if( playerFrom && !playerFrom->HasSmallIdConfirmed() )
					{
						if( dataSize >= sizeof(SQRNetworkPlayer::InitSendData) )
						{
							SQRNetworkPlayer::InitSendData ISD;
							unsigned int bytesRead = cellRudpRead( ctx_id, &ISD, sizeof(SQRNetworkPlayer::InitSendData), 0, NULL );
							if( bytesRead == sizeof(SQRNetworkPlayer::InitSendData) )
							{
								manager->NetworkPlayerInitialDataReceived(playerFrom, &ISD);
								dataSize -= sizeof(SQRNetworkPlayer::InitSendData);
							}
							else
							{
								assert(false);
							}
						}
						else
						{
							assert(false);
						}
					}
				}

				if( dataSize > 0 )
				{
					unsigned char *data = new unsigned char [ dataSize ];
					unsigned int bytesRead = cellRudpRead( ctx_id, data, dataSize, 0, NULL );
					if( bytesRead > 0 )
					{
						SQRNetworkPlayer *playerFrom, *playerTo;
						if( manager->m_isHosting )
						{
							// Data always going from a remote player, to the host
							playerFrom = manager->GetPlayerFromRudpCtx( ctx_id );
							playerTo = manager->m_aRoomSlotPlayers[0];
						}
						else
						{
							// Data always going from host player, to a local player
							playerFrom = manager->m_aRoomSlotPlayers[0];
							playerTo = manager->GetPlayerFromRudpCtx( ctx_id );
						}
						if( ( playerFrom != NULL ) && ( playerTo != NULL ) )
						{
							manager->m_listener->HandleDataReceived( playerFrom, playerTo, data, bytesRead );
						}
					}
					delete [] data;
				}
			}
			break;
		case CELL_RUDP_CONTEXT_EVENT_FLUSHED:
			break;
	}
}

// Implementation of CellRudpEventHandler
#ifdef __PS3__
int SQRNetworkManager_PS3::RudpEventCallback(int event_id, int soc, uint8_t const *data, size_t datalen, struct sockaddr const *addr, socklen_t addrlen, void *arg)
#else
int SQRNetworkManager_PS3::RudpEventCallback(int event_id, int soc, uint8_t const *data, size_t datalen, struct SceNetSockaddr  const *addr, SceNetSocklen_t addrlen, void *arg)
#endif
{
	SQRNetworkManager_PS3 *manager = (SQRNetworkManager_PS3 *)arg;

	if( event_id == CELL_RUDP_EVENT_SOCKET_RELEASED )
	{
		assert( soc == manager->m_soc );
		socketclose(soc);
		manager->m_soc = -1;
	}
	return 0;
}

void SQRNetworkManager_PS3::NetCtlCallback(int prev_state, int new_state, int event, int error_code, void *arg)
{
	SQRNetworkManager_PS3 *manager = (SQRNetworkManager_PS3 *)arg;
	// Oddly, the disconnect event comes in with a new state of "CELL_NET_CTL_STATE_Connecting"... looks like the event is more important than the state to
	// determine what has just happened
	if( event == CELL_NET_CTL_EVENT_LINK_DISCONNECTED )
	{
		manager->m_bLinkDisconnected = true;
		manager->m_listener->HandleDisconnect(false);
	}
	else if( event == CELL_NET_CTL_EVENT_ESTABLISH )
	{
		manager->m_bLinkDisconnected = false;
	}
}

// Called when the context has been created, and we are intending to create a room.
void SQRNetworkManager_PS3::ServerContextValid_CreateRoom()
{
	// First find a world
	assert( m_state == SNM_INT_STATE_HOSTING_SERVER_SEARCH_CREATING_CONTEXT );

	SetState(SNM_INT_STATE_HOSTING_CREATE_ROOM_SEARCHING_FOR_WORLD);

	SceNpMatching2GetWorldInfoListRequest reqParam;

	//  Request parameters
	memset(&reqParam, 0, sizeof(reqParam));
	reqParam.serverId = m_serverId;

	int ret = -1;
	if( !ForceErrorPoint(SNM_FORCE_ERROR_GET_WORLD_INFO_LIST) )
	{
		ret = sceNpMatching2GetWorldInfoList( m_matchingContext, &reqParam, NULL, &m_getWorldRequestId);
	}
	if (ret < 0)
	{
		SetState(SNM_INT_STATE_HOSTING_CREATE_ROOM_FAILED);
		return;
	}
}

// Called when the context has been created, and we are intending to join a pre-existing room.
void SQRNetworkManager_PS3::ServerContextValid_JoinRoom()
{
	assert( m_state == SNM_INT_STATE_JOINING_SERVER_SEARCH_CREATING_CONTEXT );

	SetState(SNM_INT_STATE_JOINING_JOIN_ROOM);

	// Join the room, passing the local player mask as initial binary data so that the host knows what local players are here
	SceNpMatching2JoinRoomRequest reqParam;
	SceNpMatching2BinAttr binAttr;
	memset(&reqParam, 0, sizeof(reqParam));
	memset(&binAttr, 0, sizeof(binAttr));
	binAttr.id = SCE_NP_MATCHING2_ROOMMEMBER_BIN_ATTR_INTERNAL_1_ID;
	binAttr.ptr = &m_localPlayerJoinMask;
	binAttr.size = sizeof(m_localPlayerJoinMask);

	reqParam.roomId = m_roomToJoin;
	reqParam.roomMemberBinAttrInternalNum = 1;
	reqParam.roomMemberBinAttrInternal = &binAttr;

	int ret = sceNpMatching2JoinRoom( m_matchingContext, &reqParam, NULL, &m_joinRoomRequestId );
	if ( (ret < 0) || ForceErrorPoint(SNM_FORCE_ERROR_JOIN_ROOM) )
	{
		if( ret == SCE_NP_MATCHING2_SERVER_ERROR_NAT_TYPE_MISMATCH)
		{
			app.SetDisconnectReason( DisconnectPacket::eDisconnect_NATMismatch );
		}
		SetState(SNM_INT_STATE_JOINING_JOIN_ROOM_FAILED);
	}
}

const SceNpCommunicationId* SQRNetworkManager_PS3::GetSceNpCommsId()
{
	return &s_npCommunicationId;
}

const SceNpCommunicationSignature* SQRNetworkManager_PS3::GetSceNpCommsSig()
{
	return &s_npCommunicationSignature;
}

int	SQRNetworkManager_PS3::GetOldMask(SceNpMatching2RoomMemberId memberId)
{
	int oldMask = 0;
	for( int i = 0; i < m_roomSyncData.getPlayerCount(); i++ )
	{
		if( m_roomSyncData.players[i].m_roomMemberId == memberId )
		{
			oldMask |= (1 << m_roomSyncData.players[i].m_localIdx);
		}
	}
	return oldMask;
}

int	SQRNetworkManager_PS3::GetAddedMask(int newMask, int oldMask)
{
	return newMask & ~oldMask;
}

int	SQRNetworkManager_PS3::GetRemovedMask(int newMask, int oldMask)
{
	return oldMask & ~newMask;
}


void SQRNetworkManager_PS3::GetExtDataForRoom( SceNpMatching2RoomId roomId, void *extData, void (* FriendSessionUpdatedFn)(bool success, void *pParam), void *pParam )
{
	static SceNpMatching2GetRoomDataExternalListRequest reqParam;
	static SceNpMatching2RoomId aRoomId[1];
	static SceNpMatching2AttributeId attr[1];

	// All parameters will be NULL if this is being called a second time, after creating a new matching context via one of the paths below (using GetMatchingContext).
	// NULL parameters therefore basically represents an attempt to retry the last sceNpMatching2GetRoomDataExternalList
	if( extData != NULL )
	{
		aRoomId[0] = roomId;
		attr[0] = SCE_NP_MATCHING2_ROOM_BIN_ATTR_EXTERNAL_1_ID;

		memset(&reqParam, 0, sizeof(reqParam));
		reqParam.roomId = aRoomId;
		reqParam.roomIdNum = 1;
		reqParam.attrIdNum = 1;
		reqParam.attrId = attr;

		m_FriendSessionUpdatedFn = FriendSessionUpdatedFn;
		m_pParamFriendSessionUpdated = pParam;
		m_pExtDataToUpdate = extData;
	}

	// Check there's a valid matching context and possibly recreate here
	if( !GetMatchingContext(SNM_INT_STATE_IDLE_RECREATING_MATCHING_CONTEXT) )
	{
		// No matching context, and failed to try and make one. We're really broken here.
		m_FriendSessionUpdatedFn(false, m_pParamFriendSessionUpdated);
		return;
	}

	// Kicked off an asynchronous thing that will create a matching context, and then call this method back again (with NULL params) once done, so we can reattempt. Don't do anything more now.
	if( m_state == SNM_INT_STATE_IDLE_RECREATING_MATCHING_CONTEXT )
	{
		app.DebugPrintf("Having to recreate matching context, setting state to SNM_INT_STATE_IDLE_RECREATING_MATCHING_CONTEXT\n");
		return;
	}

	int ret = sceNpMatching2GetRoomDataExternalList( m_matchingContext, &reqParam, NULL, &m_roomDataExternalListRequestId );

	// If we hadn't properly detected that a matching context was unvailable, we might still get an error indicating that it is from the previous call. Handle similarly, but we need
	// to destroy the context first.
	if( ( ret == SCE_NP_MATCHING2_ERROR_CONTEXT_UNAVAILABLE ) ||		// This error has been seen (occasionally) in a normal working environment
		( ret == SCE_NP_MATCHING2_ERROR_CONTEXT_NOT_STARTED ) )			// Also checking for this as a means of simulating the previous error
	{
		sceNpMatching2DestroyContext(m_matchingContext);
		m_matchingContextValid = false;
		if( !GetMatchingContext(SNM_INT_STATE_IDLE_RECREATING_MATCHING_CONTEXT) )
		{
			// No matching context, and failed to try and make one. We're really broken here.
			m_FriendSessionUpdatedFn(false, m_pParamFriendSessionUpdated);
			return;
		};
		// Kicked off an asynchronous thing that will create a matching context, and then call this method back again (with NULL params) once done, so we can reattempt. Don't do anything more now.
		if( m_state == SNM_INT_STATE_IDLE_RECREATING_MATCHING_CONTEXT )
		{
			return;
		}
	}

	if( ret != 0 )
	{
		m_FriendSessionUpdatedFn(false, m_pParamFriendSessionUpdated);
	}
}


#ifdef _CONTENT_PACKAGE
bool SQRNetworkManager_PS3::ForceErrorPoint(eSQRForceError error)
{
	return false;
}
#else
bool SQRNetworkManager_PS3::aForceError[SNM_FORCE_ERROR_COUNT] =
{
		false, // SNM_FORCE_ERROR_NP2_INIT
		false, // SNM_FORCE_ERROR_NET_INITIALIZE_NETWORK
		false, // SNM_FORCE_ERROR_NET_CTL_INIT
		false, // SNM_FORCE_ERROR_RUDP_INIT
		false, // SNM_FORCE_ERROR_NET_START_DIALOG
		false, // SNM_FORCE_ERROR_MATCHING2_INIT
		false, // SNM_FORCE_ERROR_REGISTER_NP_CALLBACK
		false, // SNM_FORCE_ERROR_GET_NPID
		false, // SNM_FORCE_ERROR_CREATE_MATCHING_CONTEXT
		false, // SNM_FORCE_ERROR_REGISTER_CALLBACKS
		false, // SNM_FORCE_ERROR_CONTEXT_START_ASYNC
		false, // SNM_FORCE_ERROR_SET_EXTERNAL_ROOM_DATA
		false, // SNM_FORCE_ERROR_GET_FRIEND_LIST_ENTRY_COUNT
		false, // SNM_FORCE_ERROR_GET_FRIEND_LIST_ENTRY
		false, // SNM_FORCE_ERROR_GET_USER_INFO_LIST
		false, // SNM_FORCE_ERROR_LEAVE_ROOM
		false, // SNM_FORCE_ERROR_SET_ROOM_MEMBER_DATA_INTERNAL
		false, // SNM_FORCE_ERROR_SET_ROOM_MEMBER_DATA_INTERNAL2
		false, // SNM_FORCE_ERROR_CREATE_SERVER_CONTEXT
		false, // SNM_FORCE_ERROR_CREATE_JOIN_ROOM
		false, // SNM_FORCE_ERROR_GET_SERVER_INFO
		false, // SNM_FORCE_ERROR_DELETE_SERVER_CONTEXT
		false, // SNM_FORCE_ERROR_SETSOCKOPT_0
		false, // SNM_FORCE_ERROR_SETSOCKOPT_1
		false, // SNM_FORCE_ERROR_SETSOCKOPT_2
		false, // SNM_FORCE_ERROR_SOCK_BIND
		false, // SNM_FORCE_ERROR_CREATE_RUDP_CONTEXT
		false, // SNM_FORCE_ERROR_RUDP_BIND
		false, // SNM_FORCE_ERROR_RUDP_INIT2
		false, // SNM_FORCE_ERROR_GET_ROOM_EXTERNAL_DATA
		false, // SNM_FORCE_ERROR_GET_SERVER_INFO_DATA
		false, // SNM_FORCE_ERROR_GET_WORLD_INFO_DATA
		false, // SNM_FORCE_ERROR_GET_CREATE_JOIN_ROOM_DATA
		false, // SNM_FORCE_ERROR_GET_USER_INFO_LIST_DATA
		false, // SNM_FORCE_ERROR_GET_JOIN_ROOM_DATA
		false, // SNM_FORCE_ERROR_GET_ROOM_MEMBER_DATA_INTERNAL
		false, // SNM_FORCE_ERROR_GET_ROOM_EXTERNAL_DATA2
		false, // SNM_FORCE_ERROR_CREATE_SERVER_CONTEXT_CALLBACK
		false, // SNM_FORCE_ERROR_SET_ROOM_DATA_CALLBACK
		false, // SNM_FORCE_ERROR_UPDATED_ROOM_DATA
		false, // SNM_FORCE_ERROR_UPDATED_ROOM_MEMBER_DATA_INTERNAL1
		false, // SNM_FORCE_ERROR_UPDATED_ROOM_MEMBER_DATA_INTERNAL2
		false, // SNM_FORCE_ERROR_UPDATED_ROOM_MEMBER_DATA_INTERNAL3
		false, // SNM_FORCE_ERROR_UPDATED_ROOM_MEMBER_DATA_INTERNAL4
		false, // SNM_FORCE_ERROR_GET_WORLD_INFO_LIST
		false, // SNM_FORCE_ERROR_JOIN_ROOM
};

bool SQRNetworkManager_PS3::ForceErrorPoint(eSQRForceError err)
{
	return aForceError[err];
}
#endif

void SQRNetworkManager_PS3::AttemptPSNSignIn(int (*SignInCompleteCallbackFn)(void *pParam, bool bContinue, int pad), void *pParam, bool callIfFailed/*=false*/)
{
	s_SignInCompleteCallbackFn = SignInCompleteCallbackFn;
	s_signInCompleteCallbackIfFailed = callIfFailed;
	s_SignInCompleteParam = pParam;

	CellNetCtlNetStartDialogParam param;
	param.cid = 0;
	param.size = sizeof(param);
	param.type = CELL_NET_CTL_NETSTART_TYPE_NP;
	int ret = cellNetCtlNetStartDialogLoadAsync(&param);

	if( ret < 0 )
	{
		if(s_SignInCompleteCallbackFn) // MGH - added after crash on PS4
		{
			if( s_signInCompleteCallbackIfFailed )
			{
				s_SignInCompleteCallbackFn(s_SignInCompleteParam,false,0);
			}
			s_SignInCompleteCallbackFn  = NULL;
		}
	}
}

int SQRNetworkManager_PS3::SetRichPresence(const void *data, unsigned int options)
{
	const SceNpBasicPresenceDetails2 *newPresenceInfo = (const SceNpBasicPresenceDetails2 *)data;

	s_lastPresenceInfo.struct_size  = sizeof( SceNpBasicPresenceDetails2);
	memcpy( s_lastPresenceInfo.status, newPresenceInfo->status, SCE_NP_BASIC_PRESENCE_EXTENDED_STATUS_SIZE_MAX);

	s_presenceStatusDirty = true;
	SendLastPresenceInfo();

	// Return as if no error happened no matter what, as we'll be resending ourselves if we need to and don't want the calling system to retry
	return 0;
}

void SQRNetworkManager_PS3::UpdateRichPresenceCustomData(void *data, unsigned int dataBytes)
{
	assert(dataBytes <= SCE_NP_BASIC_MAX_PRESENCE_SIZE );
	memcpy(s_lastPresenceInfo.data, data, dataBytes);
	s_lastPresenceInfo.size = dataBytes;

	s_presenceDataDirty = true;
	SendLastPresenceInfo();
}

void SQRNetworkManager_PS3::TickRichPresence()
{
	if( s_resendPresenceCountdown )
	{
		s_resendPresenceCountdown--;
		if( s_resendPresenceCountdown == 0 )
		{
			SendLastPresenceInfo();
		}
	}
}

void SQRNetworkManager_PS3::SendLastPresenceInfo()
{
	// Don't attempt to send if we are already waiting to resend
	if( s_resendPresenceCountdown ) return;

	unsigned int options = 0;
	if( s_presenceStatusDirty ) options |= SCE_NP_BASIC_PRESENCE_OPTIONS_SET_STATUS;
	if( s_presenceDataDirty ) options |= SCE_NP_BASIC_PRESENCE_OPTIONS_SET_DATA;

	int err = sceNpBasicSetPresenceDetails2( &s_lastPresenceInfo, options);

	if( err == 0 )
	{
		s_presenceStatusDirty = false;
		s_presenceDataDirty = false;
	}
	else
	{
		s_resendPresenceCountdown = (20 * 65);		// Bit over a minute before attempting to resend, we should get a new token every minute
	}
}

void SQRNetworkManager_PS3::SetPresenceDataStartHostingGame()
{
	if( m_offlineGame )
	{
		SQRNetworkManager_PS3::UpdateRichPresenceCustomData(&c_presenceSyncInfoNULL, sizeof(SQRNetworkManager_PS3::PresenceSyncInfo) );
	}
	else
	{
		SQRNetworkManager_PS3::PresenceSyncInfo presenceInfo;
		CPlatformNetworkManagerSony::SetSQRPresenceInfoFromExtData( &presenceInfo, m_joinExtData, m_room, m_serverId );
		SQRNetworkManager_PS3::UpdateRichPresenceCustomData(&presenceInfo, sizeof(SQRNetworkManager_PS3::PresenceSyncInfo) );
	}
}

int SQRNetworkManager_PS3::GetJoiningReadyPercentage()
{
	if ( (m_state == SNM_INT_STATE_HOSTING_SEARCHING_FOR_SERVER) || (m_state == SNM_INT_STATE_JOINING_SEARCHING_FOR_SERVER) )
	{
		int completed = ( m_totalServerCount - m_serverCount ) - 1;
		int pc = ( completed * 100 ) / m_totalServerCount;
		if( pc < 0 ) pc = 0;
		if( pc > 100 ) pc = 100;
		return pc;
	}
	else
	{
		return 100;
	}
}