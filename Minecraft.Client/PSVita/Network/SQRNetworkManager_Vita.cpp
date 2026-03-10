#include "stdafx.h"
#include "SQRNetworkManager_Vita.h"
#include "SonyVoiceChat_Vita.h"
#include "Common/Network/Sony/PlatformNetworkManagerSony.h"

#include <np.h>
#include <rudp.h>
#include <np/np_basic.h>
#include <libnetctl.h>
#include <netcheck_dialog.h>

#include "PSVita\PSVitaExtras\Conf.h"
#include "Common\Network\Sony\SonyHttp.h"
#include "..\..\..\Minecraft.World\C4JThread.h"

// image used for the invite gui, filesize must be smaller than SCE_NP_MESSAGE_DIALOG_MAX_INDEX_ICON_SIZE ( 64K )
#define SESSION_IMAGE_PATH				"app0:PSVita/session_image.png"

int (* SQRNetworkManager_Vita::s_SignInCompleteCallbackFn)(void *pParam, bool bContinue, int pad) = nullptr;
void * SQRNetworkManager_Vita::s_SignInCompleteParam = nullptr;
sce::Toolkit::NP::PresenceDetails SQRNetworkManager_Vita::s_lastPresenceInfo;
int SQRNetworkManager_Vita::s_resendPresenceCountdown = 0;
bool SQRNetworkManager_Vita::s_presenceStatusDirty = false;
bool SQRNetworkManager_Vita::s_signInCompleteCallbackIfFailed = false;
SQRNetworkManager_Vita::PresenceSyncInfo SQRNetworkManager_Vita::s_lastPresenceSyncInfo = { 0 };
SQRNetworkManager_Vita::PresenceSyncInfo SQRNetworkManager_Vita::c_presenceSyncInfoNULL = { 0 };
//SceNpBasicAttachmentDataId SQRNetworkManager_Vita::s_lastInviteIdToRetry = SCE_NP_BASIC_INVALID_ATTACHMENT_DATA_ID;
long long SQRNetworkManager_Vita::s_roomStartTime = 0;
bool SQRNetworkManager_Vita::b_inviteRecvGUIRunning = false;
SQRNetworkManager_Vita::PresenceSyncInfo* SQRNetworkManager_Vita::m_gameBootInvite;
SQRNetworkManager_Vita::PresenceSyncInfo SQRNetworkManager_Vita::m_gameBootInvite_data;
bool SQRNetworkManager_Vita::m_bCallPSNSignInCallback=false;
bool SQRNetworkManager_Vita::m_bJoinablePresenceWaitingForOnline = false;
SceAppUtilNpBasicJoinablePresenceParam	SQRNetworkManager_Vita::m_joinablePresenceParam;
bool									SQRNetworkManager_Vita::m_bSendingInviteMessage;

// static const int sc_UserEventHandle = 0;

//unsigned int SQRNetworkManager_Vita::RoomSyncData::playerCount = 0;

// This maps internal to extern states, and needs to match element-by-element the eSQRNetworkManagerInternalState enumerated type
const SQRNetworkManager_Vita::eSQRNetworkManagerState SQRNetworkManager_Vita::m_INTtoEXTStateMappings[SQRNetworkManager_Vita::SNM_INT_STATE_COUNT] =
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

SQRNetworkManager_Vita::SQRNetworkManager_Vita(ISQRNetworkManagerListener *listener)
{
	m_state = SNM_INT_STATE_UNINITIALISED;
	m_stateExternal = SNM_STATE_INITIALISING;
	m_nextIdleReasonIsFull = false;
	m_friendSearchState = SNM_FRIEND_SEARCH_STATE_IDLE;
	m_serverContextValid = false;
	m_isHosting = false;
	m_currentSmallId = 0;
	memset( m_aRoomSlotPlayers, 0, sizeof(m_aRoomSlotPlayers) );
	m_listener = listener;
	m_soc = -1;
	m_resendExternalRoomDataCountdown = 0;
	m_matching2initialised = false;
	m_matchingContextValid = false;
	m_inviteIndex = 0;
	m_doBootInviteCheck = true;
	m_bSendingInviteMessage = false;
	m_isInSession = false;
	m_offlineGame = false;
	m_offlineSQR = false;
	m_aServerId = nullptr;
	m_gameBootInvite = nullptr;
	m_onlineStatus = false;
	m_bLinkDisconnected = false;
	m_bShuttingDown = false;

	InitializeCriticalSection(&m_csRoomSyncData);
	InitializeCriticalSection(&m_csPlayerState);
	InitializeCriticalSection(&m_csStateChangeQueue);
	InitializeCriticalSection(&m_csMatching);
	InitializeCriticalSection(&m_csAckQueue);

	memset( &m_roomSyncData,0,sizeof(m_roomSyncData));		// MGH -  added to fix problem when joining a full room, and the sync data wasn't populated

	// 	int ret = sceKernelCreateEqueue(&m_basicEventQueue, "SQRNetworkManager_Vita EQ");
	// 	assert(ret == SCE_OK);
	// 	ret = sceKernelAddUserEvent(m_basicEventQueue, sc_UserEventHandle);
	// 	assert(ret == SCE_OK);
	//
	// 	m_basicEventThread = new C4JThread(&BasicEventThreadProc,this,"Basic Event Handler");
	// 	m_basicEventThread->Run();
}

// First stage of initialisation. This initialises a few things that don't require the user to be signed in, and then kicks of the network start dialog utility.
// Initialisation continues in InitialiseAfterOnline once this completes.
void SQRNetworkManager_Vita::Initialise()
{
#define NP_IN_GAME_MESSAGE_POOL_SIZE ( 16 * 1024 )
	m_bShuttingDown = false;
	int32_t ret = 0;
	// 	int32_t libCtxId = 0;
	// 	ret = sceNpInGameMessageInitialize(NP_IN_GAME_MESSAGE_POOL_SIZE, nullptr);
	// 	assert (ret >= 0);
	// 	libCtxId = ret;

	assert( m_state == SNM_INT_STATE_UNINITIALISED );


	//Initialize libnetctl - already done in SonyHttp_Vita::init
	// 	app.DebugPrintf("sceNetCtlInit\n");
	// 	ret = sceNetCtlInit();
	// 	if( ( ret < 0 && ret != SCE_NET_CTL_ERROR_NOT_TERMINATED ) || ForceErrorPoint( SNM_FORCE_ERROR_NET_CTL_INIT ) )
	// 	{
	// 		SetState(SNM_INT_STATE_INITIALISE_FAILED);
	// 		return;
	// 	}

	m_hid=0;
	app.DebugPrintf("sceNetCtlInetRegisterCallback\n");
	ret = sceNetCtlInetRegisterCallback(&NetCtlCallback,this,&m_hid);
	assert(ret == SCE_OK);

	// Initialise RUDP
	const int RUDP_POOL_SIZE = (500 * 1024);		// TODO - find out what we need, this size is copied from library reference
	uint8_t *rudp_pool = (uint8_t *)malloc(RUDP_POOL_SIZE);
	app.DebugPrintf("sceRudpInit\n");
	ret = sceRudpInit(rudp_pool, RUDP_POOL_SIZE);
	if( ( ret < 0 ) || ForceErrorPoint( SNM_FORCE_ERROR_RUDP_INIT ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return;
	}

	SetState(SNM_INT_STATE_SIGNING_IN);
	// 	AttemptPSNSignIn(nullptr, nullptr);

	// 	SonyHttp::init();

	// 	SceNpCommunicationConfig npConf ;
	// 	npConf.commId			= &s_npCommunicationId;
	// 	npConf.commPassphrase	= &s_npCommunicationPassphrase;
	// 	npConf.commSignature	= &s_npCommunicationSignature;
	// 	ret = sceNpInit(&npConf, nullptr);
	// 	if (ret < 0 && ret != SCE_NP_ERROR_ALREADY_INITIALIZED)
	// 	{
	// 		app.DebugPrintf("sceNpInit failed, ret=%x\n", ret);
	// 		assert(0);
	// 	}

	app.DebugPrintf("sceRudpEnableInternalIOThread\n");
	ret = sceRudpEnableInternalIOThread(RUDP_THREAD_STACK_SIZE, SCE_KERNEL_DEFAULT_PRIORITY);
	if(ret < 0)
	{
		app.DebugPrintf("sceRudpEnableInternalIOThread failed with error code 0x%08x\n", ret);
		assert(0);
	}
	// Already online? the callback won't catch this, so carry on initialising now
	if(ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()))
	{
		InitialiseAfterOnline();
	}
	else
	{
		// On PS3 we'd be running the netstart dialog here, but we don't want to do that on Vita since we could be switching from ad-hoc mode
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		m_offlineSQR = true;
	}
	SonyVoiceChat_Vita::init();

	m_bIsInitialised = true;
}

bool SQRNetworkManager_Vita::IsInitialised()
{
	return m_bIsInitialised;
}
void SQRNetworkManager_Vita::UnInitialise()
{
	int ret;
	// on changing to adhoc, we need to shutdown all PSN networking init

	// shutdown voice chat
	SonyVoiceChat_Vita::shutdown();

	app.DebugPrintf("sceNpMatching2ContextStop\n");
	int err = sceNpMatching2ContextStop(m_matchingContext);

	app.DebugPrintf("sceNpMatching2Term\n");
	sceNpMatching2DestroyContext(m_matchingContext);

	app.DebugPrintf("sceNpMatching2Term\n");
	ret = sceNpMatching2Term();

	app.DebugPrintf("sceRudpEnd\n");
	ret = sceRudpEnd();

	// shut down NP lib
	app.DebugPrintf("sceNetCtlInetUnregisterCallback\n");
	ret = sceNetCtlInetUnregisterCallback(m_hid);

	//app.DebugPrintf("sceNetCtlTerm\n");
	//sceNetCtlTerm();

	SetState(SNM_INT_STATE_UNINITIALISED);
	m_bIsInitialised = false;
	m_bShuttingDown = false;

}

void SQRNetworkManager_Vita::Terminate()
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

	app.DebugPrintf("sceRudpEnd\n");
	int ret = sceRudpEnd();
	app.DebugPrintf("sceNpMatching2Term\n");
	ret = sceNpMatching2Term();
	// Terminate event thread by sending it a non-zero value for data
	// 	sceKernelTriggerUserEvent(m_basicEventQueue, sc_UserEventHandle, (void*)1);


	// 	do
	// 	{
	// 		Sleep(10);
	// 	} while( m_basicEventThread->isRunning() );
}

// Second stage of initialisation, that requires NP Manager to be online & the player to be signed in. This kicks of the creation of a context
// for Np Matching 2. Initialisation is finally complete when we get a callback to ContextCallback. The SQRNetworkManager_Vita is then finally moved
// into SNM_INT_STATE_IDLE at this stage.
void SQRNetworkManager_Vita::InitialiseAfterOnline()
{
	// MGH -  added, so we don't init the matching2 stuff in trial mode - devtrack #5921
	if(!ProfileManager.IsFullVersion())
		return;

	// 	SceNpId npId;
	// 	int option = 0;

	// We should only be doing this if we have come in from an initialisation stage (SQRNetworkManager_Vita::Initialise) or we've had a network disconnect and are coming in from an offline state.
	// Don't do anything otherwise - this mainly to catch a bit of a corner case in the initialisation phase where potentially we could register for the callback that would call this with sceNpManagerRegisterCallback.
	// and could then really quickly go online so that the there becomes two paths (via the callback or SQRNetworkManager_Vita::Initialise) by which this could be called
	if( ( m_state != SNM_INT_STATE_SIGNING_IN ) && !(( m_state == SNM_INT_STATE_IDLE ) && m_offlineSQR)  )
	{
		// If we aren't going to continue on with this sign-in but are expecting a callback, then let the game know that we have completed the bit we are expecting to do. This
		// will happen whilst in game, when we want to be able to sign into PSN, but don't expect the full matching stuff to get set up.
		if( s_SignInCompleteCallbackFn )
		{
			s_SignInCompleteCallbackFn(s_SignInCompleteParam,true,0);
			s_SignInCompleteCallbackFn  = nullptr;
		}
		return;
	}

	// Initialize matching2 with default settings
	//int sceNetAdhocMatchingInit(SceSize poolsize,void *poolptr);

	app.DebugPrintf("sceNpMatching2Init\n");
	int ret = sceNpMatching2Init(0, 0, SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT, 0);

	if( ( ret < 0 ) || ForceErrorPoint( SNM_FORCE_ERROR_MATCHING2_INIT ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return;
	}
	app.DebugPrintf("SQRNetworkManager::InitialiseAfterOnline - matching context is now valid\n");
	m_matching2initialised = true;

	// Get NP ID of the signed-in user
	SceNpId npID;
	int primaryPad = ProfileManager.GetPrimaryPad();
	if(primaryPad >=0 && ProfileManager.IsSignedInLive(primaryPad))
	{
		ProfileManager.GetSceNpId(primaryPad, &npID);
	}
	else
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return;
	}

	app.DebugPrintf("sceNpMatching2CreateContext\n");
	ret = sceNpMatching2CreateContext(&npID, GetSceNpCommsId(), &s_npCommunicationPassphrase, &m_matchingContext);
	//ret = sceNpMatching2CreateContext(&npID, nullptr, nullptr, &m_matchingContext);

	if( ( ret < 0 ) || ForceErrorPoint( SNM_FORCE_ERROR_CREATE_MATCHING_CONTEXT ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return;
	}
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
	app.DebugPrintf("sceNpMatching2ContextStart\n");
	ret = sceNpMatching2ContextStart(m_matchingContext, (10*1000*1000));

	if( ( ret < 0 ) || ForceErrorPoint( SNM_FORCE_ERROR_CONTEXT_START_ASYNC ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
	}
}


// General tick function to be called from main game loop - any internal tick functions should be called from here.
void SQRNetworkManager_Vita::Tick()
{
	TickWriteAcks();
	OnlineCheck();
	sceNetCtlCheckCallback();
	updateNetCheckDialog();
	ServerContextTick();
	RoomCreateTick();
	FriendSearchTick();
	TickRichPresence();
	TickJoinablePresenceData();
	// 	TickInviteGUI();  //  TODO

	if( ( m_gameBootInvite ) && ( s_safeToRespondToGameBootInvite ) )
	{
		m_listener->HandleInviteReceived( ProfileManager.GetPrimaryPad(), m_gameBootInvite );
		m_gameBootInvite = nullptr;
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

	// 	ProfileManager.SetNetworkStatus(GetOnlineStatus());

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

	if( m_state == SNM_INT_STATE_SERVER_DELETING_CONTEXT )
	{
		// make sure we've removed all the remote players and killed the udp connections before we bail out
		if(m_RudpCtxToPlayerMap.size() == 0)
			ResetToIdle();
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
			s_SignInCompleteCallbackFn  = nullptr;
		}
		else if(s_SignInCompleteCallbackFn)
		{
			s_SignInCompleteCallbackFn(s_SignInCompleteParam, true, 0);
			s_SignInCompleteCallbackFn = nullptr;
		}
	}

}

// Detect any states which reflect internal error states, do anything required, and transition away again
void SQRNetworkManager_Vita::ErrorHandlingTick()
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
			s_SignInCompleteCallbackFn  = nullptr;
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
void SQRNetworkManager_Vita::CreateAndJoinRoom(int hostIndex, int localPlayerMask, void *extData, int extDataSize, bool offline)
{
	// hostIndex should always be in the mask
	assert( ( ( 1 << hostIndex ) & localPlayerMask ) != 0 );

	m_isHosting = true;
	m_joinExtData = extData;
	m_joinExtDataSize = extDataSize;
	m_offlineGame = offline;
	m_resendExternalRoomDataCountdown = 0;
	m_isInSession= true;

	// Default value for room, which we can use for offlinae games
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
void SQRNetworkManager_Vita::UpdateExternalRoomData()
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

		app.DebugPrintf("sceNpMatching2SetRoomDataExternal\n");
		int ret = sceNpMatching2SetRoomDataExternal ( m_matchingContext, &reqParam, nullptr, &m_setRoomDataRequestId );
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
bool SQRNetworkManager_Vita::FriendRoomManagerIsBusy()
{
	return (m_friendSearchState != SNM_FRIEND_SEARCH_STATE_IDLE);
}

// Initiate a search for rooms that the signed in user's friends are in. This is an asynchronous operation, this function returns after it kicks off a search across all game servers
// for any of the player's friends.
bool SQRNetworkManager_Vita::FriendRoomManagerSearch()
{
	if( m_state != SNM_INT_STATE_IDLE ) return false;

	// Don't start another search if we're already searching...
	if( m_friendSearchState != SNM_FRIEND_SEARCH_STATE_IDLE )
	{
		return false;
	}

	// Free up any external data that we received from the previous search
	for( size_t i = 0; i < m_aFriendSearchResults.size(); i++ )
	{
		if(m_aFriendSearchResults[i].m_RoomExtDataReceived)
			free(m_aFriendSearchResults[i].m_RoomExtDataReceived);
		m_aFriendSearchResults[i].m_RoomExtDataReceived = nullptr;
	}

	m_friendSearchState = SNM_FRIEND_SEARCH_STATE_GETTING_FRIEND_COUNT;
	m_friendCount = 0;
	m_aFriendSearchResults.clear();

	// Get friend list - doing this in another thread as it can lock up for a few seconds
	m_getFriendCountThread = new C4JThread(&GetFriendsThreadProc,this,"GetFriendsThreadProc");
	m_getFriendCountThread->Run();

	return true;
}

bool SQRNetworkManager_Vita::FriendRoomManagerSearch2()
{
	if( m_friendCount == 0 )
	{
		m_friendSearchState = SNM_FRIEND_SEARCH_STATE_IDLE;
		return false;
	}

	if( m_aFriendSearchResults.size() > 0 )
	{
		// If we have some results, then we also want to make sure that we don't have any duplicate rooms here if more than one friend is playing in the same room.
		unordered_set<SceNpMatching2RoomId> uniqueRooms;
		for( unsigned int i = 0; i < m_aFriendSearchResults.size(); i++ )
		{
			if(m_aFriendSearchResults[i].m_RoomFound)
			{
				uniqueRooms.insert( m_aFriendSearchResults[i].m_RoomId );
			}
		}

		// Tidy the results up further based on this
		for( unsigned int i = 0; i < m_aFriendSearchResults.size(); )
		{
			if( uniqueRooms.find(m_aFriendSearchResults[i].m_RoomId) == uniqueRooms.end() )
			{
				free(m_aFriendSearchResults[i].m_RoomExtDataReceived);
				m_aFriendSearchResults[i] = m_aFriendSearchResults.back();
				m_aFriendSearchResults.pop_back();
			}
			else
			{
				uniqueRooms.erase(m_aFriendSearchResults[i].m_RoomId);
				i++;
			}
		}
	}
	m_friendSearchState = SNM_FRIEND_SEARCH_STATE_IDLE;
	return true;
}

void SQRNetworkManager_Vita::FriendSearchTick()
{
	// Move onto next state if we're done getting our friend count
	if( m_friendSearchState == SNM_FRIEND_SEARCH_STATE_GETTING_FRIEND_COUNT )
	{
		if( !m_getFriendCountThread->isRunning() )
		{
			m_friendSearchState = SNM_FRIEND_SEARCH_STATE_GETTING_FRIEND_INFO;
			delete m_getFriendCountThread;
			m_getFriendCountThread = nullptr;
			FriendRoomManagerSearch2();
		}
	}
}

// The handler for basic events can't actually get the events themselves, this has to be done on another thread. Instead, we send a sys_event_t to a queue on This thread,
// which has a single data item used which we can use to determine whether to terminate this thread or get a basic event & handle that.
int SQRNetworkManager_Vita::BasicEventThreadProc( void *lpParameter )
{
	PSVITA_STUBBED;
	return 0;
	// 	SQRNetworkManager_Vita *manager = (SQRNetworkManager_Vita *)lpParameter;
	//
	// 	int ret = SCE_OK;
	// 	SceKernelEvent event;
	// 	int outEv;
	//
	// 	do
	// 	{
	// 		ret = sceKernelWaitEqueue(manager->m_basicEventQueue, &event, 1, &outEv, nullptr);
	//
	// 		// If the sys_event_t we've sent here from the handler has a non-zero data1 element, this is to signify that we should terminate the thread
	// 		if( event.udata == 0 )
	// 		{
	// 			// 			int iEvent;
	// 			// 			SceNpUserInfo from;
	// 			// 			uint8_t buffer[SCE_NP_BASIC_MAX_MESSAGE_SIZE];
	// 			// 			size_t bufferSize = SCE_NP_BASIC_MAX_MESSAGE_SIZE;
	// 			// 			int ret = sceNpBasicGetEvent(&iEvent, &from, &buffer, &bufferSize);
	// 			// 			if( ret == 0 )
	// 			// 			{
	// 			// 				if( iEvent == SCE_NP_BASIC_EVENT_INCOMING_BOOTABLE_INVITATION )
	// 			// 				{
	// 			// 					// 4J Stu - Don't do this here as it can be very disruptive to gameplay. Players can bring this up from LoadOrJoinMenu, PauseMenu and InGameInfoMenu
	// 			// 					//sceNpBasicRecvMessageCustom(SCE_NP_BASIC_MESSAGE_MAIN_TYPE_INVITE, SCE_NP_BASIC_RECV_MESSAGE_OPTIONS_INCLUDE_BOOTABLE, SYS_MEMORY_CONTAINER_ID_INVALID);
	// 			// 				}
	// 			// 				if( iEvent == SCE_NP_BASIC_EVENT_RECV_INVITATION_RESULT )
	// 			// 				{
	// 			// 					SceNpBasicExtendedAttachmentData *result = (SceNpBasicExtendedAttachmentData  *)buffer;
	// 			// 					if(result->userAction == SCE_NP_BASIC_MESSAGE_ACTION_ACCEPT )
	// 			// 					{
	// 			// 						manager->GetInviteDataAndProcess(result->data.id);
	// 			// 					}
	// 			// 				}
	// 			// 				app.DebugPrintf("Incoming basic event of type %d\n",iEvent);
	// 			// 			}
	// 		}
	//
	// 	} while(event.udata == 0 );
	// 	return 0;
}

int SQRNetworkManager_Vita::GetFriendsThreadProc( void* lpParameter )
{
	SQRNetworkManager_Vita *manager = (SQRNetworkManager_Vita *)lpParameter;

	int ret = 0;
	manager->m_aFriendSearchResults.clear();
	manager->m_friendCount = 0;
	if(!ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()))
	{
		app.DebugPrintf("getFriendslist failed, not signed into Live! \n");
		return 0;
	}


	ret = sceNpBasicGetFriendListEntryCount(&manager->m_friendCount);
	if( ( ret < 0 ) || manager->ForceErrorPoint( SNM_FORCE_ERROR_GET_FRIEND_LIST_ENTRY_COUNT ) )
	{
		// This is likely when friend list hasn't been received from the server yet - will be returning SCE_NP_BASIC_ERROR_BUSY in this case
		manager->m_friendCount = 0;
	}


	// There shouldn't ever be more than 100 friends returned but limit here just in case
	if( manager->m_friendCount > 100 ) manager->m_friendCount = 100;

	SceNpId* friendIDs = nullptr;
	if(manager->m_friendCount > 0)
	{
		// grab all the friend IDs first
		friendIDs = new SceNpId[manager->m_friendCount];
		SceSize numRecieved;
		ret = sceNpBasicGetFriendListEntries(0, friendIDs, manager->m_friendCount, &numRecieved);
		if (ret < 0)
		{
			app.DebugPrintf("sceNpBasicGetFriendListEntries() failed: ret = 0x%x\n", ret);
			manager->m_friendCount = 0;
		}
		else
		{
			assert(numRecieved == manager->m_friendCount);
		}
	}


	// It is possible that the size of the friend list might vary from what we just received, so only add in friends that we successfully get an entry for
	for( unsigned int i = 0; i < manager->m_friendCount; i++ )
	{
		static SceNpBasicGamePresence presenceDetails;
		static SceNpBasicFriendContextState contextState;
		int ret = sceNpBasicGetFriendContextState(&friendIDs[i], &contextState);
		if (ret < 0)
		{
			app.DebugPrintf("sceNpBasicGetFriendContextState() failed: ret = 0x%x\n", ret);
			contextState = SCE_NP_BASIC_FRIEND_CONTEXT_STATE_UNKNOWN;
		}
		if(contextState == SCE_NP_BASIC_FRIEND_CONTEXT_STATE_IN_CONTEXT) // using the same SceNpCommunicationId, so playing Minecraft
		{
			ret = sceNpBasicGetGamePresenceOfFriend(&friendIDs[i], &presenceDetails);
			if( ( ret == 0 ) && ( !manager->ForceErrorPoint( SNM_FORCE_ERROR_GET_FRIEND_LIST_ENTRY ) ) )
			{
				FriendSearchResult result;
				memcpy(&result.m_NpId, &friendIDs[i], sizeof(SceNpId));
				result.m_RoomFound = false;

				// Only include the friend's game if its the same network id ( this also filters out generally Zeroed PresenceSyncInfo, which we do when we aren't in an active game session)
				// 				if( presenceDetails.size == sizeof(PresenceSyncInfo) )
				{
					PresenceSyncInfo *pso = (PresenceSyncInfo *)presenceDetails.inGamePresence.data;
					if( pso->netVersion == MINECRAFT_NET_VERSION )
					{
						if( !pso->inviteOnly )
						{
							result.m_RoomFound = true;
							result.m_RoomId = pso->m_RoomId;
							result.m_ServerId = pso->m_ServerId;

							CPlatformNetworkManagerSony::MallocAndSetExtDataFromSQRPresenceInfo(&result.m_RoomExtDataReceived, pso);
							manager->m_aFriendSearchResults.push_back(result);
						}
					}
				}
			}
		}
	}

	if(friendIDs)
		delete friendIDs;
	return 0;
}

// Get count of rooms that friends are playing in. Only valid when FriendRoomManagerIsBusy() returns false
int	SQRNetworkManager_Vita::FriendRoomManagerGetCount()
{
	assert( m_friendSearchState == SNM_FRIEND_SEARCH_STATE_IDLE );
	return m_aFriendSearchResults.size();
}

// Get details of a found session that a friend is playing in. 0 < idx < FriendRoomManagerGetCount(). Only valid when FriendRoomManagerIsBusy() returns false
void SQRNetworkManager_Vita::FriendRoomManagerGetRoomInfo(int idx, SQRNetworkManager_Vita::SessionSearchResult *searchResult)
{
	assert( idx < m_aFriendSearchResults.size() );
	assert( m_friendSearchState == SNM_FRIEND_SEARCH_STATE_IDLE );

	searchResult->m_NpId					= m_aFriendSearchResults[idx].m_NpId;
	searchResult->m_sessionId.m_RoomId		= m_aFriendSearchResults[idx].m_RoomId;
	searchResult->m_sessionId.m_ServerId	= m_aFriendSearchResults[idx].m_ServerId;
	searchResult->m_extData					= m_aFriendSearchResults[idx].m_RoomExtDataReceived;
}

// Get overall state of the network manager.
SQRNetworkManager_Vita::eSQRNetworkManagerState SQRNetworkManager_Vita::GetState()
{
	return m_stateExternal;;
}

bool SQRNetworkManager_Vita::IsHost()
{
	return m_isHosting;
}

bool SQRNetworkManager_Vita::IsReadyToPlayOrIdle()
{
	return (( m_state == SNM_INT_STATE_HOSTING_WAITING_TO_PLAY ) || ( m_state == SNM_INT_STATE_PLAYING ) || ( m_state == SNM_INT_STATE_IDLE ) );
}


// Consider as "in session" from the moment that a game is created or joined, until the point where the game itself has been told via state change that we are now idle. The
// game code requires IsInSession to return true as soon as it has asked to do one of these things (even if the state system hasn't really caught up with this request yet), and
// it also requires that it is informed of the state changes leading up to not being in the session, before this should report false.
bool SQRNetworkManager_Vita::IsInSession()
{
	return m_isInSession;
}

// Get count of players currently in the session
int	SQRNetworkManager_Vita::GetPlayerCount()
{
	return m_roomSyncData.getPlayerCount();
}

// Get count of players who are in the session, but not local to this machine
int SQRNetworkManager_Vita::GetOnlinePlayerCount()
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

SQRNetworkPlayer *SQRNetworkManager_Vita::GetPlayerByIndex(int idx)
{
	if( idx < MAX_ONLINE_PLAYER_COUNT )
	{
		return GetPlayerIfReady(m_aRoomSlotPlayers[idx]);
	}
	else
	{
		return nullptr;
	}
}

SQRNetworkPlayer *SQRNetworkManager_Vita::GetPlayerBySmallId(int idx)
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
	return nullptr;
}

SQRNetworkPlayer *SQRNetworkManager_Vita::GetLocalPlayerByUserIndex(int idx)
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
	return nullptr;
}

SQRNetworkPlayer *SQRNetworkManager_Vita::GetHostPlayer()
{
	EnterCriticalSection(&m_csRoomSyncData);
	SQRNetworkPlayer *player = GetPlayerIfReady(m_aRoomSlotPlayers[0]);
	LeaveCriticalSection(&m_csRoomSyncData);
	return player;
}

SQRNetworkPlayer *SQRNetworkManager_Vita::GetPlayerIfReady(SQRNetworkPlayer *player)
{
	if( player == nullptr ) return nullptr;

	if( player->IsReady() ) return player;

	return nullptr;
}

// Update state internally

#ifdef _DEBUG
static const char szNetState[35][60]=
{
	"SNM_INT_STATE_UNINITIALISED",
	"SNM_INT_STATE_SIGNING_IN",
	"SNM_INT_STATE_STARTING_CONTEXT",
	"SNM_INT_STATE_INITIALISE_FAILED",
	"SNM_INT_STATE_IDLE",
	"SNM_INT_STATE_IDLE_RECREATING_MATCHING_CONTEXT",
	"SNM_INT_STATE_HOSTING_STARTING_MATCHING_CONTEXT",
	"SNM_INT_STATE_HOSTING_SEARCHING_FOR_SERVER",
	"SNM_INT_STATE_HOSTING_SERVER_SEARCH_SERVER_ERROR",
	"SNM_INT_STATE_HOSTING_SERVER_FOUND",
	"SNM_INT_STATE_HOSTING_SERVER_SEARCH_CREATING_CONTEXT",
	"SNM_INT_STATE_HOSTING_SERVER_SEARCH_FAILED",
	"SNM_INT_STATE_HOSTING_CREATE_ROOM_SEARCHING_FOR_WORLD",
	"SNM_INT_STATE_HOSTING_CREATE_ROOM_WORLD_FOUND",
	"SNM_INT_STATE_HOSTING_CREATE_ROOM_CREATING_ROOM",
	"SNM_INT_STATE_HOSTING_CREATE_ROOM_SUCCESS",
	"SNM_INT_STATE_HOSTING_CREATE_ROOM_FAILED",
	"SNM_INT_STATE_HOSTING_CREATE_ROOM_RESTART_MATCHING_CONTEXT",
	"SNM_INT_STATE_HOSTING_WAITING_TO_PLAY",
	"SNM_INT_STATE_JOINING_STARTING_MATCHING_CONTEXT",
	"SNM_INT_STATE_JOINING_SEARCHING_FOR_SERVER",
	"SNM_INT_STATE_JOINING_SERVER_SEARCH_SERVER_ERROR",
	"SNM_INT_STATE_JOINING_SERVER_FOUND",
	"SNM_INT_STATE_JOINING_SERVER_SEARCH_CREATING_CONTEXT",
	"SNM_INT_STATE_JOINING_SERVER_SEARCH_FAILED",
	"SNM_INT_STATE_JOINING_JOIN_ROOM",
	"SNM_INT_STATE_JOINING_JOIN_ROOM_FAILED",
	"SNM_INT_STATE_JOINING_WAITING_FOR_LOCAL_PLAYERS",
	"SNM_INT_STATE_SERVER_DELETING_CONTEXT",
	"SNM_INT_STATE_STARTING",
	"SNM_INT_STATE_PLAYING",
	"SNM_INT_STATE_LEAVING",
	"SNM_INT_STATE_LEAVING_FAILED",
	"SNM_INT_STATE_ENDING",
	"SNM_INT_STATE_COUNT"
};

#endif
void SQRNetworkManager_Vita::SetState(SQRNetworkManager_Vita::eSQRNetworkManagerInternalState state)
{
#ifdef _DEBUG
	app.DebugPrintf("SQRNetworkManager_Vita::SetState [%s]\n",szNetState[state]);
#endif

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

void SQRNetworkManager_Vita::ResetToIdle()
{
	app.DebugPrintf("------------------ResetToIdle--------------------\n");
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
	SonyVoiceChat_Vita::checkFinished();
}

// Join a room that was found with FriendRoomManagerSearch. 0 < idx < FriendRoomManagerGetCount(). Only valid when FriendRoomManagerIsBusy() returns false
bool SQRNetworkManager_Vita::JoinRoom(SQRNetworkManager_Vita::SessionSearchResult *searchResult, int localPlayerMask)
{
	// Set up the presence info we would like to synchronise out when we have fully joined the game
	CPlatformNetworkManagerSony::SetSQRPresenceInfoFromExtData(&s_lastPresenceSyncInfo, searchResult->m_extData, searchResult->m_sessionId.m_RoomId, searchResult->m_sessionId.m_ServerId);
	return JoinRoom(searchResult->m_sessionId.m_RoomId, searchResult->m_sessionId.m_ServerId, localPlayerMask, nullptr);
}

// Join room with a specified roomId. This is used when joining from an invite, as well as by the previous method
bool SQRNetworkManager_Vita::JoinRoom(SceNpMatching2RoomId roomId, SceNpMatching2ServerId serverId, int localPlayerMask, const SQRNetworkManager_Vita::PresenceSyncInfo *presence)
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

void SQRNetworkManager_Vita::StartGame()
{
	assert( ( m_state == SNM_INT_STATE_HOSTING_WAITING_TO_PLAY ) || (( m_state == SNM_INT_STATE_IDLE ) && m_offlineSQR) );

	SetState( SNM_INT_STATE_STARTING);
	SetState( SNM_INT_STATE_PLAYING);
}

void SQRNetworkManager_Vita::LeaveRoom(bool bActuallyLeaveRoom)
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

	// 	SonyVoiceChat::shutdown();

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
			app.DebugPrintf("sceNpMatching2LeaveRoom\n");
			int ret = sceNpMatching2LeaveRoom( m_matchingContext, &reqParam, nullptr, &m_leaveRoomRequestId );
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

void SQRNetworkManager_Vita::EndGame()
{
}

bool SQRNetworkManager_Vita::SessionHasSpace(int spaceRequired)
{
	return( ( m_roomSyncData.getPlayerCount() + spaceRequired ) <= MAX_ONLINE_PLAYER_COUNT );
}

bool SQRNetworkManager_Vita::AddLocalPlayerByUserIndex(int idx)
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

		// no connections being made because we're all on the host, so add this player to the existing connections
		SonyVoiceChat_Vita::connectPlayerToAll(idx);
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

		app.DebugPrintf("sceNpMatching2SetRoomMemberDataInternal\n");
		int ret = sceNpMatching2SetRoomMemberDataInternal( m_matchingContext, &reqParam, nullptr, &m_setRoomMemberInternalDataRequestId );

		if( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_SET_ROOM_MEMBER_DATA_INTERNAL) )
		{
			return false;
		}

		// Create the client's end of the rudp connections... note that m_roomSyncData.players[0].m_roomMemberId is always be the host's room member id.
		bool rudpOk = CreateRudpConnections(m_room, m_roomSyncData.players[0].m_roomMemberId, mask, m_localMemberId );

		if( rudpOk )
		{
			bool ret = CreateVoiceRudpConnections( m_room, m_roomSyncData.players[0].m_roomMemberId, mask);
			assert(ret);
			return true;
		}
		else
		{
			m_localPlayerJoinMask &= (~mask);
			return false;
		}
	}
}

bool SQRNetworkManager_Vita::RemoveLocalPlayerByUserIndex(int idx)
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
				m_aRoomSlotPlayers[m_roomSyncData.getPlayerCount()] = nullptr;

				// Sync this back out to our networked clients...
				SyncRoomData();

				SonyVoiceChat_Vita::disconnectLocalPlayer(idx);

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

		int ret = sceNpMatching2SetRoomMemberDataInternal( m_matchingContext, &reqParam, nullptr, &m_setRoomMemberInternalDataRequestId );

		if( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_SET_ROOM_MEMBER_DATA_INTERNAL2) )
		{
			return false;
		}

		RemoveNetworkPlayers( mask );

		return true;
	}
}


extern uint8_t *mallocAndCreateUTF8ArrayFromString(int iID);

// Bring up a Gui to send an invite so a player that the user can select. This invite will contain the room Id so that
void SQRNetworkManager_Vita::SendInviteGUI()
{
	if(ProfileManager.IsSystemUIDisplayed())
	{
		app.DebugPrintf("SendInviteGUI failed, SysUI is already up \n");
		return;
	}

	//Set invitation information - this is now exactly the same as the presence information that we synchronise out.

	// If we joined a game, we'll have already set s_lastPresenceSyncInfo up (whether we came in from an invite, or joining a game we discovered). If we were hosting,
	// then we'll need to set this up now from the external dasta.
	if( m_isHosting )
	{
		CPlatformNetworkManagerSony::SetSQRPresenceInfoFromExtData(&s_lastPresenceSyncInfo, m_joinExtData, m_room, m_serverId);
	}

	sce::Toolkit::NP::MessageData messData;
	memset(&messData,0,sizeof(messData));

	char *subject = (char*)mallocAndCreateUTF8ArrayFromString(IDS_INVITATION_SUBJECT_MAX_18_CHARS);
	char *body = (char*)mallocAndCreateUTF8ArrayFromString(IDS_INVITATION_BODY);
	messData.attachment = (SceChar8*)&s_lastPresenceSyncInfo;;
	messData.attachmentSize = sizeof(PresenceSyncInfo);
	messData.body.assign(body);
	messData.iconPath.assign(SESSION_IMAGE_PATH);
	messData.expireMinutes = 0;

	int ret = sce::Toolkit::NP::Messaging::Interface::sendMessage(&messData, SCE_TOOLKIT_NP_MESSAGE_TYPE_CUSTOM_DATA);
	if(ret < SCE_TOOLKIT_NP_SUCCESS )
	{
		app.DebugPrintf("Send Message failed 0x%x ...\n",ret);
		assert(0);
		return;
	}
	else
	{
		m_bSendingInviteMessage = true;
		ProfileManager.SetSysUIShowing( true );
	}
}


void SQRNetworkManager_Vita::RecvInviteGUI()
{
	if(ProfileManager.IsSystemUIDisplayed())
	{
		app.DebugPrintf("RecvInviteGUI failed, SysUI is already up \n");
		return;
	}

	int ret = sce::Toolkit::NP::Messaging::Interface::displayReceivedMessages(SCE_TOOLKIT_NP_MESSAGE_TYPE_CUSTOM_DATA);
	if(ret < SCE_TOOLKIT_NP_SUCCESS )
	{
		app.DebugPrintf("displayReceivedMessages 0x%x ...\n",ret);
		assert(0);
		return;
	}
	else
	{
		ProfileManager.SetSysUIShowing( true );
	}


	// 	int ret = sceGameCustomDataDialogInitialize();
	// 	if(ret != SCE_OK)
	// 	{
	// 		app.DebugPrintf("sceGameCustomDataDialogInitialize() failed. ret = 0x%x\n", ret);
	// 	}
	// 	else
	// 	{
	//
	// 		SceGameCustomDataDialogParam		dialogParam;
	// 		SceGameCustomDataDialogDataParam	dataParam;
	//
	// 		sceGameCustomDataDialogParamInit( &dialogParam );
	// 		memset( &dataParam,	0x00,		sizeof( SceGameCustomDataDialogDataParam ) );
	// 		dialogParam.mode = SCE_GAME_CUSTOM_DATA_DIALOG_MODE_RECV;
	// 		dialogParam.dataParam = &dataParam;
	// 		dialogParam.userId = ProfileManager.getUserID(ProfileManager.GetPrimaryPad());
	// 		ret = sceGameCustomDataDialogOpen( &dialogParam );
	//
	// 		if( SCE_OK != ret )
	// 		{
	// 			app.DebugPrintf("sceGameCustomDataDialogOpen() failed. ret = 0x%x\n", ret);
	// 		}
	// 		else
	// 		{
	// 			b_inviteRecvGUIRunning = true;
	// 		}
	// 	}
}


void SQRNetworkManager_Vita::TickInviteGUI()
{
	PSVITA_STUBBED;
	// 	if(b_inviteRecvGUIRunning)
	// 	{
	// 		SceCommonDialogStatus	status	= sceGameCustomDataDialogUpdateStatus();
	//
	// 		if( SCE_COMMON_DIALOG_STATUS_FINISHED == status )
	// 		{
	// 			SceGameCustomDataDialogOnlineIdList sentOnlineIdList;
	// 			memset( &sentOnlineIdList, 0x0, sizeof(SceGameCustomDataDialogOnlineIdList));
	// 			SceGameCustomDataDialogResult	dialogResult;
	// 			memset( &dialogResult, 0x0, sizeof(SceGameCustomDataDialogResult) );
	// 			dialogResult.sentOnlineIds = &sentOnlineIdList;
	//
	// 			int32_t	ret = sceGameCustomDataDialogGetResult( &dialogResult );
	//
	// 			if( SCE_OK != ret )
	// 			{
	// 				app.DebugPrintf( "***** sceGameCustomDataDialogGetResult error:0x%x\n", ret);
	// 			}
	// 			sceGameCustomDataDialogClose();
	// 			sceGameCustomDataDialogTerminate();
	// 			b_inviteRecvGUIRunning = false;
	// 		}
	// 	}
}

// Get the data for an invite into a statically allocated array of invites, and pass a pointer of this back up to the game. Elements in the array are used in a circular fashion, to save any issues with handling freeing of this invite data as the
// qnet equivalent of this seems to just assume that the data persists forever.
void SQRNetworkManager_Vita::GetInviteDataAndProcess(sce::Toolkit::NP::MessageAttachment* pInvite)
{

	app.DebugPrintf("GameCustomData attachment size : %d\n", pInvite->getAttachmentSize());
	if(pInvite->getAttachmentSize() == sizeof(m_gameBootInvite_data))
	{
		memcpy(&m_gameBootInvite_data,  pInvite->getAttachmentData(), sizeof(m_gameBootInvite_data));
		m_gameBootInvite = &m_gameBootInvite_data;
	}
}

void SQRNetworkManager_Vita::GetJoinablePresenceDataAndProcess(SceAppUtilNpBasicJoinablePresenceParam* pJoinablePresenceData)
{
	memcpy(&m_joinablePresenceParam, pJoinablePresenceData, sizeof(SceAppUtilNpBasicJoinablePresenceParam));
	if(s_safeToRespondToGameBootInvite && ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()))
	{
		ProcessJoinablePresenceData();
	}
	else
	{
		m_bJoinablePresenceWaitingForOnline = true;
	}
}

void SQRNetworkManager_Vita::ProcessJoinablePresenceData()
{
	static SceNpBasicGamePresence presenceDetails;
	int ret = sceNpBasicGetGamePresenceOfFriend(&m_joinablePresenceParam.npId, &presenceDetails);
	if( ret == 0 )
	{
		PresenceSyncInfo *pso = (PresenceSyncInfo *)presenceDetails.inGamePresence.data;
		memcpy(&m_gameBootInvite_data,  pso, sizeof(m_gameBootInvite_data));
		m_gameBootInvite = &m_gameBootInvite_data;
	}
	m_bJoinablePresenceWaitingForOnline = false;
}




// This case happens when we were in the main menus when we got an invite, and weren't signed in... now can proceed with the normal flow of code for this situation
// The pair of methods MustSignInReturned_1 & PSNSignInReturned_1 handle this
int	MustSignInReturnedPresenceInvite(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	if(result==C4JStorage::EMessage_ResultAccept)
	{
		SQRNetworkManager_Vita::AttemptPSNSignIn(&SQRNetworkManager_Vita::PSNSignInReturnedPresenceInvite, pParam,true);
	}
	return 0;
}

int SQRNetworkManager_Vita::PSNSignInReturnedPresenceInvite(void* pParam, bool bContinue, int iPad)
{
	INVITE_INFO *inviteInfo = (INVITE_INFO *)pParam;

	// If the invite data isn't set up yet (indicated by it being all zeroes, easiest detected via the net version), then try and get it again... this can happen if we got
	// the invite whilst signed out

	if( bContinue )
	{
		m_bJoinablePresenceWaitingForOnline = true;
	}
	return 0;
}



void SQRNetworkManager_Vita::TickJoinablePresenceData()
{
	if(s_safeToRespondToGameBootInvite && m_bJoinablePresenceWaitingForOnline)
	{
		if(ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()))
			ProcessJoinablePresenceData();
		else
		{
			m_bJoinablePresenceWaitingForOnline = false; // will be set to true again if we sign in succesfully
			// Determine why they're not "signed in live"
			// MGH - we need to add a new message at some point for connecting when already signed in
			// 		if (ProfileManager.IsSignedInPSN(ProfileManager.GetPrimaryPad()))
			// 		{
			// 			// Signed in to PSN but not connected (no internet access)
			// 			UINT uiIDA[1];
			// 			uiIDA[0] = IDS_OK;
			// 			ui.RequestMessageBox( IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, uiIDA, 1, ProfileManager.GetPrimaryPad(), nullptr, nullptr, app.GetStringTable());
			// 		}
			// 		else
			{
				// Not signed in to PSN
				UINT uiIDA[1];
				uiIDA[0] = IDS_PRO_NOTONLINE_ACCEPT;
				ui.RequestAlertMessage( IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 1, ProfileManager.GetPrimaryPad(), &MustSignInReturnedPresenceInvite, nullptr);
			}

		}

	}
}

bool SQRNetworkManager_Vita::UpdateInviteData(SQRNetworkManager_Vita::PresenceSyncInfo *invite)
{
	PSVITA_STUBBED;
	return false;

	// 	size_t dataSize = sizeof(SQRNetworkManager_Vita::PresenceSyncInfo);
	// 	int ret = sceNpBasicRecvMessageAttachmentLoad(s_lastInviteIdToRetry, invite, &dataSize);
	// 	return (ret == 0);
}

// This method is a helper used in MapRoomSlotPlayers - tries to find a player that matches:
// (1) the playerType
// (2) if playerType is remote, memberId
// (3) localPlayerIdx
// The reason we don't care about memberid when the player isn't remote is that it doesn't matter (since we know the player is either on this machine, or it is the host and there's only one of those),
// and there's a period when starting up the host game where it doesn't accurately know the memberId for its own local players
void SQRNetworkManager_Vita::FindOrCreateNonNetworkPlayer(int slot, int playerType, SceNpMatching2RoomMemberId memberId, int localPlayerIdx, int smallId)
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
	PlayerUID *pUID = nullptr;
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
void	SQRNetworkManager_Vita::LocalDataSend(SQRNetworkPlayer *playerFrom, SQRNetworkPlayer *playerTo, const void *data, unsigned int dataSize)
{
	assert(m_isHosting);
	if(m_listener)
	{
		m_listener->HandleDataReceived( playerFrom, playerTo, (unsigned char *)data, dataSize );
	}
}

int SQRNetworkManager_Vita::GetSessionIndex(SQRNetworkPlayer *player)
{
	int roomSlotPlayerCount = m_roomSyncData.getPlayerCount();
	for( int i = 0; i < roomSlotPlayerCount; i++  )
	{
		if( m_aRoomSlotPlayers[i] == player ) return i;
	}
	return 0;
}

// Updates m_aRoomSlotPlayers, based on what is in m_roomSyncData. This needs to be updated when room members join & leave, and when any SQRNetworkPlayer is created externally that this should be mapping to
void SQRNetworkManager_Vita::MapRoomSlotPlayers(int roomSlotPlayerCount/*=-1*/)
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
					m_aRoomSlotPlayers[i] = nullptr;
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
					m_aRoomSlotPlayers[i] = nullptr;
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
void SQRNetworkManager_Vita::UpdateRoomSyncUIDsFromPlayers()
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
void SQRNetworkManager_Vita::UpdatePlayersFromRoomSyncUIDs()
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
bool SQRNetworkManager_Vita::AddRemotePlayersAndSync( SceNpMatching2RoomMemberId memberId, int playerMask, bool *isFull/*==nullptr*/ )
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

	return true;
}

// Host only - remove all remote players belonging to the supplied memberId, and in the supplied mask, and synchronise this with other room members
void SQRNetworkManager_Vita::RemoveRemotePlayersAndSync( SceNpMatching2RoomMemberId memberId, int mask )
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
				int err = sceRudpTerminate( ctx );
				assert(err == SCE_OK);
				if( m_listener )
				{
					m_listener->HandlePlayerLeaving(player);
				}
				// Delete the player itself and the mapping from context to player map as this context is no longer valid
				delete player;
				m_RudpCtxToPlayerMap.erase(ctx);

				removePlayerFromVoiceChat(player);
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
			m_aRoomSlotPlayers[m_roomSyncData.getPlayerCount()] = nullptr;
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

	// 	if(GetOnlinePlayerCount() == 0)
	// 		SonyVoiceChat::shutdown();
}

// Client only - remove all network players matching the supplied mask
void SQRNetworkManager_Vita::RemoveNetworkPlayers( int mask )
{
	assert( !m_isHosting );

    for (auto it = m_RudpCtxToPlayerMap.begin(); it != m_RudpCtxToPlayerMap.end();)
    {
		SQRNetworkPlayer *player = it->second;
		if( (player->m_roomMemberId == m_localMemberId ) && ( ( 1 << player->m_localPlayerIdx ) & mask ) )
		{
			// Get Rudp context for this player, close that context down ( which will in turn close the socket if required)
			int ctx = it->first;
			int err = sceRudpTerminate( ctx );
			assert(err == SCE_OK);
			if( m_listener )
			{
				m_listener->HandlePlayerLeaving(player);
			}
			// Delete any reference to this player from the player mappings
			for( int i = 0; i < MAX_ONLINE_PLAYER_COUNT; i++ )
			{
				if( m_aRoomSlotPlayers[i] == player )
				{
					m_aRoomSlotPlayers[i] = nullptr;
				}
			}
			// And delete the reference from the ctx->player map
			it = m_RudpCtxToPlayerMap.erase(it);

			removePlayerFromVoiceChat(player);

			// Delete the player itself and the mapping from context to player map as this context is no longer valid
			delete player;
		}
		else
		{
			it++;
		}
	}
	assert(m_RudpCtxToPlayerMap.size() == 0);
}

// Host only - update the memberId of the local players, and synchronise with other room members
void SQRNetworkManager_Vita::SetLocalPlayersAndSync()
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
void SQRNetworkManager_Vita::SyncRoomData()
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
	sceNpMatching2SetRoomDataInternal ( m_matchingContext, &reqParam, nullptr, &m_setRoomDataRequestId );
}

// Check if the matching context is valid, and if not attempt to create one. If to do this requires starting an asynchronous process, then sets the internal state to the state passed in
// before doing this.
// Returns true on success.
bool SQRNetworkManager_Vita::GetMatchingContext(eSQRNetworkManagerInternalState asyncState)
{
	if( m_matchingContextValid ) return true;

	int ret = 0;
	if( !m_matching2initialised)
	{
		app.DebugPrintf("sceNpMatching2Init\n");
		ret = sceNpMatching2Init(0, 0, SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT, 0);
	}
	if( ret < 0 )
	{
		app.DebugPrintf("SQRNetworkManager::GetMatchingContext - sceNpMatching2Init2 failed with code 0x%08x\n", ret);
		return false;
	}
	m_matching2initialised = true;

	// Get NP ID of the signed-in user
	SceNpId npId;
	app.DebugPrintf("GetSceNpId\n");

	/*ret = */ProfileManager.GetSceNpId(ProfileManager.GetPrimaryPad(), &npId);


	// Create context
	app.DebugPrintf("sceNpMatching2CreateContext\n");
	ret = sceNpMatching2CreateContext(&npId, &s_npCommunicationId, &s_npCommunicationPassphrase, &m_matchingContext/*, option*/);
	//ret = sceNpMatching2CreateContext(&npId, nullptr,nullptr, &m_matchingContext/*, option*/);
	if( ret < 0 )
	{
		app.DebugPrintf("SQRNetworkManager::GetMatchingContext - sceNpMatching2CreateContext failed with code 0x%08x\n", ret);
		return false;
	}
	if( ret < 0 ) return false;

	app.DebugPrintf("RegisterCallbacks\n");
	if( !RegisterCallbacks() )
	{
		app.DebugPrintf("SQRNetworkManager::GetMatchingContext - RegisterCallbacks failed\n");
		return false;
	}

	// Set internal state & kick off async process that will actually start the context.
	SetState(asyncState);
	app.DebugPrintf("sceNpMatching2ContextStart\n");
	ret = sceNpMatching2ContextStart(m_matchingContext, (10*1000*1000));
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
bool SQRNetworkManager_Vita::GetServerContext()
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
bool SQRNetworkManager_Vita::GetServerContext2()
{

	m_aServerId = (SceNpMatching2ServerId *)realloc( m_aServerId, sizeof(SceNpMatching2ServerId) * 1 );
	SceNpMatching2Server server;
	app.DebugPrintf("sceNpMatching2GetServerLocal\n");
	int err = sceNpMatching2GetServerLocal(m_matchingContext, &server);
	assert(server.status == SCE_NP_MATCHING2_SERVER_STATUS_AVAILABLE);
	*m_aServerId = server.serverId;
	if(err != SCE_OK)
	{
		m_serverCount = 0;
		assert(0);
	}
	m_serverCount = 1;

	SetState(SNM_INT_STATE_HOSTING_SEARCHING_FOR_SERVER);
	return SelectRandomServer();
}

// Overloaded method for (as before) obtaining a server context. This version is so that can also get a server context for a specific server rather than a random one,
// using mainly the same code by making a single element list. This is used when joining an existing room.
bool SQRNetworkManager_Vita::GetServerContext(SceNpMatching2ServerId serverId)
{
	if(m_state == SNM_INT_STATE_STARTING_CONTEXT)
	{
		// MGH - added for devtrack 5936 : race between the context starting after going online, and trying to start it here, so skip this one if we're already starting.
		m_serverCount = 1;
		m_totalServerCount = m_serverCount;
		m_aServerId = (SceNpMatching2ServerId *)realloc(m_aServerId, sizeof(SceNpMatching2ServerId) * m_serverCount );
		m_aServerId[0] = serverId;
		SetState(SNM_INT_STATE_JOINING_STARTING_MATCHING_CONTEXT);
		return true;
	}
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
	//	if( m_state != SNM_INT_STATE_JOINING_STARTING_MATCHING_CONTEXT )
	//	{
	// 		// Get list of server IDs of servers allocated to the application. We don't actually need to do this, but it is as good a way as any to try a matching2 service and check that
	// 		// the context *really* is valid.
	// 		int serverCount = sceNpMatching2GetServerIdListLocal( m_matchingContext, nullptr, 0 );
	// 		// If an error is returned here, we need to destroy and recerate our server - if this goes ok we should come back through this path again
	// 		if( ( serverCount == SCE_NP_MATCHING2_ERROR_CONTEXT_UNAVAILABLE ) ||		// This error has been seen (occasionally) in a normal working environment
	// 			( serverCount == SCE_NP_MATCHING2_ERROR_CONTEXT_NOT_STARTED ) )				// Also checking for this as a means of simulating the previous error
	// 		{
	// 			sceNpMatching2DestroyContext(m_matchingContext);
	// 			m_matchingContextValid = false;
	// 			if( !GetMatchingContext(SNM_INT_STATE_JOINING_STARTING_MATCHING_CONTEXT) ) return false;
	// 		}
	//	}
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
void SQRNetworkManager_Vita::ServerContextTick()
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
		m_serverContextValid = true;
		ServerContextValid_CreateRoom();
		break;

	case SNM_INT_STATE_JOINING_SERVER_FOUND:
		m_serverContextValid = true;
		ServerContextValid_JoinRoom();
		break;
	default:
		break;
	}
}

// Tick the process of creating a room.
void SQRNetworkManager_Vita::RoomCreateTick()
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

			optSignalingParam.type = SCE_NP_MATCHING2_SIGNALING_TYPE_MESH;
			optSignalingParam.hubMemberId = 0;	// Room owner is the hub of the star
			SetState(SNM_INT_STATE_HOSTING_CREATE_ROOM_CREATING_ROOM);
			app.DebugPrintf(CMinecraftApp::USER_RR,">> Creating room start\n");
			s_roomStartTime = System::currentTimeMillis();
			app.DebugPrintf("sceNpMatching2CreateJoinRoom\n");
			int ret = sceNpMatching2CreateJoinRoom( m_matchingContext, &reqParam, nullptr, &m_createRoomRequestId );
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
void SQRNetworkManager_Vita::NetworkPlayerConnectionComplete(SQRNetworkPlayer *player)
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
void SQRNetworkManager_Vita::NetworkPlayerSmallIdAllocated(SQRNetworkPlayer *player, unsigned char smallId)
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
void SQRNetworkManager_Vita::NetworkPlayerInitialDataReceived(SQRNetworkPlayer *player, void *data)
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
void SQRNetworkManager_Vita::NonNetworkPlayerComplete(SQRNetworkPlayer *player, unsigned char smallId)
{
	player->ConnectionComplete();
	player->SmallIdAllocated(smallId);
}

void SQRNetworkManager_Vita::HandlePlayerJoined(SQRNetworkPlayer *player)
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
bool SQRNetworkManager_Vita::SelectRandomServer()
{
	app.DebugPrintf("SQRNetworkManager_Vita::SelectRandomServer\n");

	assert( (m_state == SNM_INT_STATE_HOSTING_SEARCHING_FOR_SERVER) || (m_state == SNM_INT_STATE_JOINING_SEARCHING_FOR_SERVER) );

	if( m_serverCount == 0 )
	{
		SetState((m_state == SNM_INT_STATE_HOSTING_SEARCHING_FOR_SERVER) ? SNM_INT_STATE_HOSTING_SERVER_SEARCH_FAILED : SNM_INT_STATE_JOINING_SERVER_SEARCH_FAILED);
		app.DebugPrintf("SQRNetworkManager::SelectRandomServer - Server count is 0\n");
		return false;
	}

	// not really selecting a random server, as we've already been allocated one, but calling this to match PS3
	int serverIdx;
	serverIdx = 0;
	m_serverCount--;
	m_aServerId[serverIdx] = m_aServerId[m_serverCount];

	// This server is available
	SetState((m_state == SNM_INT_STATE_HOSTING_SEARCHING_FOR_SERVER) ? SNM_INT_STATE_HOSTING_SERVER_FOUND : SNM_INT_STATE_JOINING_SERVER_FOUND);
	m_serverId = m_aServerId[serverIdx];

	return true;
}

// Delete the current server context. Should be called when finished with the current host or client game session.
void SQRNetworkManager_Vita::DeleteServerContext()
{
	// No server context on PS4, so we just set the state, and then we'll check all the UDP connections have shutdown before setting to idle
	if( m_serverContextValid )
	{
		m_serverContextValid = false;
		SetState(SNM_INT_STATE_SERVER_DELETING_CONTEXT);
	}
}

// Creates a set of Rudp connections by the "active open" method. This requires that both ends of the connection call cellRudpInitiate to fully create a connection. We
// create one connection per local play on any remote machine.
//
// peerMemberId is the room member Id of the remote end of the connection
// playersMemberId is the room member Id that the players belong to
// ie for the host (when matching incoming connections), these will be the same thing... and for the client, peerMemberId will be the host, whereas playersMemberId will be itself


static std::string getIPAddressString(SceNetInAddr add)
{
	char str[32];
	unsigned char *vals = (unsigned char*)&add.s_addr;
	sprintf(str, "%d.%d.%d.%d", (int)vals[0], (int)vals[1], (int)vals[2], (int)vals[3]);
	return std::string(str);
}

bool SQRNetworkManager_Vita::CreateSocket()
{
	// First get details of the UDPP2P connection that has been established
	// 	int connStatus;
	SceNetSockaddrIn sinp2pLocal;//, sinp2pPeer;
	SceNpMatching2SignalingNetInfo netInfo;

	// Local end first...
	memset(&sinp2pLocal, 0, sizeof(sinp2pLocal));
	memset(&netInfo, 0 , sizeof(netInfo));
	netInfo.size = sizeof(netInfo);
	int ret = sceNpMatching2SignalingGetLocalNetInfo(&netInfo);
	if( ret < 0 ) return false;
	sinp2pLocal.sin_len =  sizeof(sinp2pLocal);
	sinp2pLocal.sin_family = SCE_NET_AF_INET;
	sinp2pLocal.sin_port = sceNetHtons(SCE_NP_PORT);
	sinp2pLocal.sin_addr = netInfo.localAddr;


	// Set vport for both ends of connection
	sinp2pLocal.sin_vport = sceNetHtons(1);

	// Create socket & bind
	ret = sceNetSocket("rupdSocket", SCE_NET_AF_INET, SCE_NET_SOCK_DGRAM_P2P, 0);
	assert(ret >= 0);
	m_soc = ret;
	int optval = 1;
	ret = sceNetSetsockopt(m_soc, SCE_NET_SOL_SOCKET, SCE_NET_SO_USECRYPTO, &optval, sizeof(optval));
	if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_SETSOCKOPT_0) ) return false;
	ret = sceNetSetsockopt(m_soc, SCE_NET_SOL_SOCKET, SCE_NET_SO_USESIGNATURE, &optval, sizeof(optval));
	if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_SETSOCKOPT_1) ) return false;
	ret = sceNetSetsockopt(m_soc, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &optval,	sizeof(optval));
	if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_SETSOCKOPT_2) ) return false;

	ret = sceNetBind(m_soc, &sinp2pLocal, sizeof(sinp2pLocal));
	if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_SOCK_BIND) ) return false;
	return true;

}


bool SQRNetworkManager_Vita::CreateVoiceRudpConnections(SceNpMatching2RoomId roomId, SceNpMatching2RoomMemberId peerMemberId, int playerMask)
{
	SceNetSockaddrIn sinp2pPeer;
	SceNpMatching2SignalingNetInfo netInfo;
	int connStatus;

	memset(&sinp2pPeer, 0, sizeof(sinp2pPeer));
	sinp2pPeer.sin_len =  sizeof(sinp2pPeer);
	sinp2pPeer.sin_family = SCE_NET_AF_INET;
	int ret = sceNpMatching2SignalingGetConnectionStatus(m_matchingContext, roomId, peerMemberId, &connStatus, &sinp2pPeer.sin_addr, &sinp2pPeer.sin_port);
	sinp2pPeer.sin_vport = sceNetHtons(1);


	ret = 0;
	// Create socket & bind, if we don't already have one
	if( m_soc == -1 )
	{
		if(CreateSocket() == false)
			return false;
	}

	// create this connection if we don't have it already
	SQRVoiceConnection* pConnection = SonyVoiceChat_Vita::getVoiceConnectionFromRoomMemberID(peerMemberId);
	if(pConnection == nullptr)
	{

		// Create an Rudp context for the voice connection, this will happen regardless of whether the peer is client or host
		int rudpCtx;
		ret = sceRudpCreateContext( RudpContextCallback, this, &rudpCtx );
		if(ret < 0){ app.DebugPrintf("sceRudpCreateContext failed : 0x%08x\n", ret); assert(0); }
		if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_CREATE_RUDP_CONTEXT) ) return false;

		// Bind the context to the socket we've just created, and initiate. The initiation needs to happen on both client & host sides of the connection to complete.
		ret = sceRudpBind( rudpCtx, m_soc , 5, SCE_RUDP_MUXMODE_P2P );
		if(ret < 0){ app.DebugPrintf("sceRudpBind failed : 0x%08x\n", ret); assert(0); }
		if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_RUDP_BIND) ) return false;

		ret = sceRudpInitiate( rudpCtx, (SceNetSockaddr*)&sinp2pPeer, sizeof(sinp2pPeer), 0);
		if(ret < 0){ app.DebugPrintf("sceRudpInitiate failed : 0x%08x\n", ret); assert(0); }
		if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_RUDP_INIT2) ) return false;

		app.DebugPrintf("-----------------------------\n");
		app.DebugPrintf("Voice rudp context created %d connected to %s\n", rudpCtx, getIPAddressString(sinp2pPeer.sin_addr).c_str());
		app.DebugPrintf("-----------------------------\n");

		pConnection = SonyVoiceChat_Vita::addRemoteConnection(rudpCtx, peerMemberId);
	}

	for( int i = 0; i < MAX_LOCAL_PLAYER_COUNT; i++ )
	{
		bool bMaskVal = ( playerMask & ( 1 << i ) );

		if(bMaskVal || GetLocalPlayerByUserIndex(i))
			SonyVoiceChat_Vita::connectPlayer(pConnection, i);
	}
	return true;
}



bool SQRNetworkManager_Vita::CreateRudpConnections(SceNpMatching2RoomId roomId, SceNpMatching2RoomMemberId peerMemberId, int playerMask, SceNpMatching2RoomMemberId playersMemberId)
{
	// First get details of the UDPP2P connection that has been established
	int connStatus;
	SceNetSockaddrIn sinp2pPeer;

	// get the peer
	memset(&sinp2pPeer, 0, sizeof(sinp2pPeer));
	sinp2pPeer.sin_len =  sizeof(sinp2pPeer);
	sinp2pPeer.sin_family = SCE_NET_AF_INET;

	int ret = sceNpMatching2SignalingGetConnectionStatus(m_matchingContext, roomId, peerMemberId, &connStatus, &sinp2pPeer.sin_addr, &sinp2pPeer.sin_port);
	app.DebugPrintf(CMinecraftApp::USER_RR,"sceNpMatching2SignalingGetConnectionStatus returned 0x%x, connStatus %d peer add:%s peer port:0x%x\n",ret, connStatus,getIPAddressString(sinp2pPeer.sin_addr).c_str(),sinp2pPeer.sin_port);

	// Set vport
	sinp2pPeer.sin_vport = sceNetHtons(1);

	// Create socket & bind, if we don't already have one
	if( m_soc == -1 )
	{
		if(CreateSocket() == false)
			return false;
	}

	// Create an Rudp context for each local player that is required. These can be used as individual virtual connections between room members (ie consoles), which are multiplexed
	// over the socket we have just made
	for( int i = 0; i < MAX_LOCAL_PLAYER_COUNT; i++ )
	{
		if( ( playerMask & ( 1 << i ) ) == 0 ) continue;

		int rudpCtx;

		// Socket for the local network node created, now can create an Rupd context.
		ret = sceRudpCreateContext( RudpContextCallback, this, &rudpCtx );
		if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_CREATE_RUDP_CONTEXT) ) return false;
		if( m_isHosting )
		{
			m_RudpCtxToPlayerMap[ rudpCtx ] = new SQRNetworkPlayer( this, SQRNetworkPlayer::SNP_TYPE_REMOTE, true, playersMemberId, i, rudpCtx, nullptr );
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
		ret = sceRudpBind( rudpCtx, m_soc , 1 + i, SCE_RUDP_MUXMODE_P2P );
		if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_RUDP_BIND) ) return false;

		ret = sceRudpInitiate( rudpCtx, &sinp2pPeer, sizeof(sinp2pPeer), 0);
		if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_RUDP_INIT2) ) return false;
	}
	return true;
}


SQRNetworkPlayer *SQRNetworkManager_Vita::GetPlayerFromRudpCtx(int rudpCtx)
{
    auto it = m_RudpCtxToPlayerMap.find(rudpCtx);
    if( it != m_RudpCtxToPlayerMap.end() )
	{
		return it->second;
	}
	return nullptr;
}



SQRNetworkPlayer *SQRNetworkManager_Vita::GetPlayerFromRoomMemberAndLocalIdx(int roomMember, int localIdx)
{
    for (auto it = m_RudpCtxToPlayerMap.begin(); it != m_RudpCtxToPlayerMap.end(); it++)
    {
		if( (it->second->m_roomMemberId == roomMember ) && ( it->second->m_localPlayerIdx == localIdx ) )
		{
			return it->second;
		}
	}
	return nullptr;
}


// This is called as part of the general initialisation of the network manager, to register any callbacks that the sony libraries require.
// Returns true if all were registered successfully.
bool SQRNetworkManager_Vita::RegisterCallbacks()
{
	// Register RUDP event handler
	app.DebugPrintf("sceRudpSetEventHandler\n");
	int ret = sceRudpSetEventHandler(RudpEventCallback, this);
	if (ret < 0)
	{
		app.DebugPrintf("SQRNetworkManager::RegisterCallbacks - cellRudpSetEventHandler failed with code 0x%08x\n", ret);
		return false;
	}

	// Register the context callback function
	app.DebugPrintf("sceNpMatching2RegisterContextCallback\n");
	ret = sceNpMatching2RegisterContextCallback(ContextCallback, this);
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

	app.DebugPrintf("sceNpMatching2SetDefaultRequestOptParam\n");
	ret = sceNpMatching2SetDefaultRequestOptParam(m_matchingContext, &optParam);
	if (ret < 0)
	{
		app.DebugPrintf("SQRNetworkManager::RegisterCallbacks - sceNpMatching2SetDefaultRequestOptParam failed with code 0x%08x\n", ret);
		return false;
	}

	// Register signalling callback
	app.DebugPrintf("sceNpMatching2RegisterSignalingCallback\n");
	ret = sceNpMatching2RegisterSignalingCallback(m_matchingContext, SignallingCallback, this);
	if (ret < 0)
	{
		return false;
	}

	// Register room event callback
	app.DebugPrintf("sceNpMatching2RegisterRoomEventCallback\n");
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
void SQRNetworkManager_Vita::ContextCallback(SceNpMatching2ContextId  id, SceNpMatching2Event event, SceNpMatching2EventCause eventCause, int errorCode, void *arg)
{
	if(CGameNetworkManager::usingAdhocMode())  // MGH - added to fix #5772
		return;


	int ret;
	SQRNetworkManager_Vita *manager = (SQRNetworkManager_Vita *)arg;
	EnterCriticalSection(&manager->m_csMatching);
	if (id != manager->m_matchingContext)
	{
		LeaveCriticalSection(&manager->m_csMatching);
		return;
	}

	switch( event )
	{
	case SCE_NP_MATCHING2_CONTEXT_EVENT_STARTED:
		app.DebugPrintf("SCE_NP_MATCHING2_CONTEXT_EVENT_STARTED\n");
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
			manager->GetExtDataForRoom(0, nullptr, nullptr, nullptr);
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
		if ( manager->m_state == SNM_INT_STATE_HOSTING_CREATE_ROOM_RESTART_MATCHING_CONTEXT )
		{
			manager->ServerContextValid_CreateRoom();
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
			// 					s_SignInCompleteCallbackFn = nullptr;
			// 				}


			// Check to see if we were booted from an invite. Only do this once, the first time we have all our networking stuff set up on boot-up
			if( manager->m_doBootInviteCheck )
			{
				//					ORBIS_STUBBED;
				// 					unsigned int type, attributes;
				// 					CellGameContentSize gameSize;`
				// 					char dirName[CELL_GAME_DIRNAME_SIZE];
				//
				// 					if( g_bBootedFromInvite )
				// 					{
				// 						manager->GetInviteDataAndProcess(SCE_NP_BASIC_SELECTED_INVITATION_DATA);
				// 						manager->m_doBootInviteCheck  = false;
				// 					}
			}
		}
		break;
	case SCE_NP_MATCHING2_CONTEXT_EVENT_STOPPED:
		app.DebugPrintf("SCE_NP_MATCHING2_CONTEXT_EVENT_STOPPED\n");
		// Can happen when we stop the PSN to switch to adhoc mode
		//assert(false);
		if( manager->m_state == SNM_INT_STATE_HOSTING_CREATE_ROOM_RESTART_MATCHING_CONTEXT )
		{
			sceNpMatching2DestroyContext(manager->m_matchingContext);
			manager->m_matchingContextValid = false;
			if(!manager->GetMatchingContext(SNM_INT_STATE_HOSTING_CREATE_ROOM_RESTART_MATCHING_CONTEXT))
			{
				manager->m_offlineSQR = true;
				manager->SetState(SNM_INT_STATE_INITIALISE_FAILED);
			}
		}
		break;
	case SCE_NP_MATCHING2_CONTEXT_EVENT_START_OVER:

		app.DebugPrintf("SCE_NP_MATCHING2_CONTEXT_EVENT_START_OVER\n");
		app.DebugPrintf("SCE_NP_MATCHING2_CONTEXT_EVENT_START_OVER\n");
		app.DebugPrintf("eventCause=%u, errorCode=0x%08x\n", eventCause, errorCode);
		app.DebugPrintf("sceNpMatching2DestroyContext\n");
		sceNpMatching2DestroyContext(manager->m_matchingContext);
		if(manager->m_state == SNM_INT_STATE_IDLE_RECREATING_MATCHING_CONTEXT)		// MGH - added this to catch when the context start fails when getting GetExtDataForRoom
		{
			if(manager->m_FriendSessionUpdatedFn)
				manager->m_FriendSessionUpdatedFn(false, manager->m_pParamFriendSessionUpdated);
		}
		manager->m_matchingContextValid = false;
		manager->m_offlineSQR = true;
		manager->SetState(SNM_INT_STATE_INITIALISE_FAILED);
		break;
	}

	LeaveCriticalSection(&manager->m_csMatching);
}

// This is an implementation of SceNpMatching2RequestCallback. This callback is used by default for any matching 2 request functions.
void SQRNetworkManager_Vita::DefaultRequestCallback(SceNpMatching2ContextId id, SceNpMatching2RequestId reqId, SceNpMatching2Event event, int errorCode, const void *data, void *arg)
{
	SQRNetworkManager_Vita *manager = (SQRNetworkManager_Vita *)arg;
	EnterCriticalSection(&manager->m_csMatching);
	// 	int ret;
	if( id != manager->m_matchingContext )
	{
		LeaveCriticalSection(&manager->m_csMatching);
		return;
	}


	switch( event )
	{
		// This is the response to sceNpMatching2GetWorldInfoList, which is called as part of the process to create a room (which needs a world to be created in). We aren't anticipating
		// using worlds in a meaningful way so just getting the first world we find on the server here, and then advancing the state so that the tick can get on with the rest of the process.
	case SCE_NP_MATCHING2_REQUEST_EVENT_GET_WORLD_INFO_LIST:
		{
			app.DebugPrintf("SCE_NP_MATCHING2_REQUEST_EVENT_GET_WORLD_INFO_LIST\n");
			SceNpServiceState serviceState;
			int retVal = sceNpGetServiceState(&serviceState);
			assert(retVal == 0);
			assert(serviceState == SCE_NP_SERVICE_STATE_ONLINE);

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
			// We get this error when starting a new game after a disconnect occurred in a previous game with at least one remote player. Fix by stopping/starting the matching context.
			// We stop the context here, which is picked up in the callback, and started again. Then the start event is picked up and reattempts the sceNpMatching2GetWorldInfoList.
			if( errorCode == SCE_NET_ERROR_EAGAIN  || errorCode == SCE_NET_ERROR_RESOLVER_ETIMEDOUT || errorCode == SCE_NET_CTL_ERROR_WIFI_DISABLED )
			{
				sceNpMatching2ContextStop(manager->m_matchingContext);
				manager->SetState(SNM_INT_STATE_HOSTING_CREATE_ROOM_RESTART_MATCHING_CONTEXT);
				break;
			}
			app.DebugPrintf("SCE_NP_MATCHING2_REQUEST_EVENT_GET_WORLD_INFO_LIST failed, errorCode 0x%x, data %d\n", errorCode, data);

			manager->SetState(SNM_INT_STATE_HOSTING_CREATE_ROOM_FAILED);
		}
		break;
		// This is the response to sceNpMatching2CreateJoinRoom, which if successful means that we are just about ready to move to an online state as host of a game. The final
		// transition actually occurs in the create room tick, on detecting that the state has transitioned to SNM_INT_STATE_HOSTING_CREATE_ROOM_SUCCESS here.
	case SCE_NP_MATCHING2_REQUEST_EVENT_CREATE_JOIN_ROOM:
		app.DebugPrintf("SCE_NP_MATCHING2_REQUEST_EVENT_CREATE_JOIN_ROOM\n");
		if( errorCode == SCE_NP_MATCHING2_ERROR_NP_SIGNED_OUT )
		{
			// If we've already signed out, then we should have detected this already elsewhere and so can silently ignore any errors coming in for pending requests here
			break;
		}

		if(ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) == false)
		{
			// MGH - added to catch a case where the user signed out of PSN, but this still called back with no error
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
		// This is the response to sceNpMatching2JoinRoom, which is called as the final stage of the process started when calling the JoinRoom method. If this is successful, then
		// the state can change to SNM_INT_STATE_JOINING_WAITING_FOR_LOCAL_PLAYERS. We can transition out of that state once we have told the application that all the local players
		// have joined.
	case SCE_NP_MATCHING2_REQUEST_EVENT_JOIN_ROOM:
		app.DebugPrintf("SCE_NP_MATCHING2_REQUEST_EVENT_JOIN_ROOM\n");
		assert( manager->m_state == SNM_INT_STATE_JOINING_JOIN_ROOM);
		if( errorCode == 0 )
		{
			if( data != 0 )
			{
				SceNpMatching2JoinRoomResponse *roomData = (SceNpMatching2JoinRoomResponse *)data;

				manager->m_localMemberId = roomData->roomDataInternal->memberList.me->memberId;
				manager->m_room = roomData->roomDataInternal->roomId;
				// 					SonyVoiceChat::init(manager);
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
			app.DebugPrintf("setting DisconnectPacket::eDisconnect_ServerFull\n");
			Minecraft::GetInstance()->connectionDisconnected(ProfileManager.GetPrimaryPad(), DisconnectPacket::eDisconnect_ServerFull);
			app.SetDisconnectReason(DisconnectPacket::eDisconnect_ServerFull);		// MGH - added to fix when joining from an invite
		}
		break;
		// This is the response to sceNpMatching2GetRoomMemberDataInternal.This only happens on the host, as a response to an incoming connection being established, when we
		// kick off the request for room member internal data so that we can determine what local players that remote machine is intending to bring into the game. At this point we can
		// activate the host end of each of the Rupd connection that this machine requires. We can also update our player slot data (which gets syncronised back out to other room members) at this point.
	case SCE_NP_MATCHING2_REQUEST_EVENT_GET_ROOM_MEMBER_DATA_INTERNAL:
		app.DebugPrintf("SCE_NP_MATCHING2_REQUEST_EVENT_GET_ROOM_MEMBER_DATA_INTERNAL\n");
		if(	manager->m_state == SNM_INT_STATE_INITIALISE_FAILED ||
			manager->m_state == SNM_INT_STATE_IDLE ||
			manager->m_state == SNM_INT_STATE_LEAVING ||
			manager->m_state == SNM_INT_STATE_ENDING )
		{
			// MGH -  I've caught this being triggered after join has already failed, and then UDP connections are created that aren't killed off
			// so just break out here if we're not in the expected state
			// fixes  devtrack #5807
			app.DebugPrintf("SCE_NP_MATCHING2_REQUEST_EVENT_GET_ROOM_MEMBER_DATA_INTERNAL - manager->GetState() : %d\n", manager->GetState() );
			break;
		}
		if( errorCode == 0 )
		{

			if( data != 0 )
			{
				SceNpMatching2GetRoomMemberDataInternalResponse *pRoomMemberData = (SceNpMatching2GetRoomMemberDataInternalResponse *)data;
				assert( pRoomMemberData->roomMemberDataInternal->roomMemberBinAttrInternalNum == 1 );

				if( manager->m_isHosting )
				{
					int playerMask = *((int *)(pRoomMemberData->roomMemberDataInternal->roomMemberBinAttrInternal->data.ptr));

					bool isFull = false;
					bool success1 = manager->AddRemotePlayersAndSync( pRoomMemberData->roomMemberDataInternal->memberId, playerMask, &isFull );
					bool success2;
					if( success1 )
					{
						success2 = manager->CreateRudpConnections(manager->m_room, pRoomMemberData->roomMemberDataInternal->memberId, playerMask, pRoomMemberData->roomMemberDataInternal->memberId);
						if( success2 )
						{
							bool ret = manager->CreateVoiceRudpConnections( manager->m_room, pRoomMemberData->roomMemberDataInternal->memberId, 0);
							assert(ret == true);
							break;
						}
					}
					// Something has gone wrong adding these players to the room - kick out the player
					SceNpMatching2KickoutRoomMemberRequest reqParam;
					// 					SceNpMatching2PresenceOptionData optParam;
					memset(&reqParam,0,sizeof(reqParam));
					reqParam.roomId = manager->m_room;
					reqParam.target = pRoomMemberData->roomMemberDataInternal->memberId;
					// Set flag to indicate whether we were kicked for being out of room or not
					reqParam.optData.data[0] = isFull ? 1 : 0;
					reqParam.optData.len = 1;
					int ret = sceNpMatching2KickoutRoomMember(manager->m_matchingContext, &reqParam, nullptr, &manager->m_kickRequestId);
					app.DebugPrintf(CMinecraftApp::USER_RR,"sceNpMatching2KickoutRoomMember returns error 0x%x\n",ret);
				}
				else
				{
					if(pRoomMemberData->roomMemberDataInternal->roomMemberBinAttrInternal->data.ptr == nullptr)
					{
						// the host doesn't send out data, so this must be the host we're connecting to

						// If we are the client, then we locally know what Rupd connections we need (from m_localPlayerJoinMask) and can kick this off.
						manager->m_hostMemberId = pRoomMemberData->roomMemberDataInternal->memberId;
						bool ret = manager->CreateRudpConnections( manager->m_room, pRoomMemberData->roomMemberDataInternal->memberId, manager->m_localPlayerJoinMask, manager->m_localMemberId);
						if( ret == false )
						{
							manager->DeleteServerContext();
						}
						else
						{
							bool ret = manager->CreateVoiceRudpConnections( manager->m_room, pRoomMemberData->roomMemberDataInternal->memberId, manager->m_localPlayerJoinMask);
							assert(ret == true);
						}
					}
					else
					{
						// client <-> client
						bool ret = manager->CreateVoiceRudpConnections( manager->m_room, pRoomMemberData->roomMemberDataInternal->memberId, manager->m_localPlayerJoinMask);
						assert(ret == true);
					}
				}

			}
		}
		break;
	case SCE_NP_MATCHING2_REQUEST_EVENT_LEAVE_ROOM:
		app.DebugPrintf("SCE_NP_MATCHING2_REQUEST_EVENT_LEAVE_ROOM\n");
		// This is the response to sceNpMatching2LeaveRoom - from the Sony docs, this doesn't ever fail so no need to do error checking here
		// 			SonyVoiceChat::signalDisconnected();
		assert(manager->m_state == SNM_INT_STATE_LEAVING );
		manager->DeleteServerContext();
		break;
		// This is the response to SceNpMatching2GetRoomDataExternalListRequest, which happens when we request the full details of a room we are interested in joining
	case SCE_NP_MATCHING2_REQUEST_EVENT_GET_ROOM_DATA_EXTERNAL_LIST:
		app.DebugPrintf("SCE_NP_MATCHING2_REQUEST_EVENT_GET_ROOM_DATA_EXTERNAL_LIST\n");
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
					manager->m_FriendSessionUpdatedFn(false, manager->m_pParamFriendSessionUpdated);
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
	case SCE_NP_MATCHING2_REQUEST_EVENT_SET_ROOM_DATA_EXTERNAL:
		app.DebugPrintf("SCE_NP_MATCHING2_REQUEST_EVENT_SET_ROOM_DATA_EXTERNAL\n");
		if( ( errorCode != 0 ) || manager->ForceErrorPoint(SNM_FORCE_ERROR_SET_ROOM_DATA_CALLBACK) )
		{
			app.DebugPrintf(CMinecraftApp::USER_RR,"Error updating external data 0x%x (from SCE_NP_MATCHING2_REQUEST_EVENT_SetRoomDataExternal event in callback)\n",errorCode);
			// If we ever fail to send the external room data, we start a countdown so that we attempt to resend. Not sure how likely it is that updating this will fail without the whole network being broken,
			// but if in particular we don't update the flag to say that the session is joinable, then nobody is ever going to see this session.
			manager->m_resendExternalRoomDataCountdown = 60;
		}
		break;
	};

	LeaveCriticalSection(&manager->m_csMatching);
}

void SQRNetworkManager_Vita::RoomEventCallback(SceNpMatching2ContextId id, SceNpMatching2RoomId roomId, SceNpMatching2Event event, const void *data, void *arg)
{
	SQRNetworkManager_Vita *manager = (SQRNetworkManager_Vita *)arg;

	// 	bool gotEventData = false;
	switch( event )
	{
	case SCE_NP_MATCHING2_ROOM_EVENT_MEMBER_JOINED:
		break;
	case SCE_NP_MATCHING2_ROOM_EVENT_MEMBER_LEFT:
		break;
	case SCE_NP_MATCHING2_ROOM_EVENT_KICKEDOUT:
		{
			// 				SonyVoiceChat::signalRoomKickedOut();
			// We've been kicked out. This server has rejected our attempt to join, most likely because there wasn't enough space in the server to have us. There's a flag set
			// so we can determine which thing has happened
			// 				assert ( dataSize <= SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomUpdateInfo );
			// 				int ret = sceNpMatching2GetEventData( manager->m_matchingContext, eventKey, manager->cRoomDataUpdateInfo, SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomUpdateInfo);
			//				app.DebugPrintf(CMinecraftApp::USER_RR,"SCE_NP_MATCHING2_ROOM_EVENT_Kickedout, sceNpMatching2GetEventData returning 0x%x\n",ret);

			bool bIsFull = false;
			if( ( data ) && !manager->ForceErrorPoint(SNM_FORCE_ERROR_UPDATED_ROOM_DATA) )
			{
				// 				gotEventData = true;
				SceNpMatching2RoomUpdateInfo *pUpdateInfo = (SceNpMatching2RoomUpdateInfo *)(data);
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
	case SCE_NP_MATCHING2_ROOM_EVENT_ROOM_DESTROYED:
		// 			SonyVoiceChat::signalRoomDestroyed();

		{
			SceNpMatching2RoomUpdateInfo *pUpdateInfo = (SceNpMatching2RoomUpdateInfo *)data;
			app.DebugPrintf("SCE_NP_MATCHING2_ROOM_EVENT_RoomDestroyed\n");
			if( pUpdateInfo )
			{
				app.DebugPrintf("Further info: Error 0x%x, cause %d\n",pUpdateInfo->errorCode,pUpdateInfo->eventCause);
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

				// MGH - added a check for the PSN sign in state, we don't seem to get the signed out matching2 error here
				bool bSignedInPSN = ProfileManager.IsSignedInPSN(ProfileManager.GetPrimaryPad());
				bool bSignedOutError = pUpdateInfo && (pUpdateInfo->eventCause==SCE_NP_MATCHING2_EVENT_CAUSE_NP_SIGNED_OUT);

				if(bSignedOutError || (bSignedInPSN == false) )
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
	case SCE_NP_MATCHING2_ROOM_EVENT_ROOM_OWNER_CHANGED:
		break;
	case SCE_NP_MATCHING2_ROOM_EVENT_UPDATED_ROOM_DATA_INTERNAL:
		// We are using the room internal data to synchronise the player data stored in m_roomSyncData from the host to clients.
		// The host is the thing creating the internal room data, so it doesn't need to update itself.
		if( !manager->m_isHosting )
		{
			// 				assert ( dataSize <= SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomDataInternalUpdateInfo );
			// 				int ret = sceNpMatching2GetEventData( manager->m_matchingContext, eventKey, manager->cRoomDataInternal, SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomDataInternalUpdateInfo);
			if( ( data) && !manager->ForceErrorPoint(SNM_FORCE_ERROR_UPDATED_ROOM_DATA) )
			{
				// 				gotEventData = true;
				SceNpMatching2RoomDataInternalUpdateInfo *pRoomData = (SceNpMatching2RoomDataInternalUpdateInfo *)(data);
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
	case SCE_NP_MATCHING2_ROOM_EVENT_UPDATED_ROOM_MEMBER_DATA_INTERNAL:
		if( /*( errorCode == 0 )  && */(!manager->ForceErrorPoint(SNM_FORCE_ERROR_UPDATED_ROOM_MEMBER_DATA_INTERNAL1) ) )
		{
			// We'll get this sync'd round all the connected clients, but we only care about it on the host where we can use it to work out if any RUDP connections need to be made or released
			if( manager->m_isHosting )
			{
				// 					assert( dataSize <= SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomMemberDataInternalUpdateInfo );
				// 					int ret = sceNpMatching2GetEventData(manager->m_matchingContext, eventKey, (void *)(manager->cRoomMemberDataInternalUpdate), SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomMemberDataInternalUpdateInfo);
				if( ( data ) && (!manager->ForceErrorPoint(SNM_FORCE_ERROR_UPDATED_ROOM_MEMBER_DATA_INTERNAL2) ) )
				{
					// 					gotEventData = true;
					SceNpMatching2RoomMemberDataInternalUpdateInfo *pRoomMemberData = (SceNpMatching2RoomMemberDataInternalUpdateInfo *)(data);
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

							int ret = sceNpMatching2SetRoomMemberDataInternal( manager->m_matchingContext, &reqParam, nullptr, &manager->m_setRoomMemberInternalDataRequestId );
						}
						else
						{
							success = manager->CreateVoiceRudpConnections( manager->m_room, pRoomMemberData->newRoomMemberDataInternal->memberId, 0);
							assert(success);
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
				// 					assert( dataSize <= SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomMemberDataInternalUpdateInfo );
				// 					int ret = sceNpMatching2GetEventData(manager->m_matchingContext, eventKey, (void *)(manager->cRoomMemberDataInternalUpdate), SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomMemberDataInternalUpdateInfo);
				if( ( data ) && (!manager->ForceErrorPoint(SNM_FORCE_ERROR_UPDATED_ROOM_MEMBER_DATA_INTERNAL4) ) )
				{
					// 					gotEventData = true;
					SceNpMatching2RoomMemberDataInternalUpdateInfo *pRoomMemberData = (SceNpMatching2RoomMemberDataInternalUpdateInfo *)(data);
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
	case SCE_NP_MATCHING2_ROOM_EVENT_UPDATED_SIGNALING_OPT_PARAM:
		break;
	};

	// 	// If we didn't get the event data, then we need to clear it, or the system even queue will overflow
	// 	if( !gotEventData )
	// 	{
	// 		sceNpMatching2ClearEventData(manager->m_matchingContext, eventKey);
	// 	}
}

// This is an implementation of SceNpMatching2SignalingCallback. We configure our too automatically create a star network of connections with the host at the hub, and can respond here to
// the connections being set up to layer sockets and Rudp on top.
void SQRNetworkManager_Vita::SignallingCallback(SceNpMatching2ContextId ctxId, SceNpMatching2RoomId roomId, SceNpMatching2RoomMemberId peerMemberId, SceNpMatching2Event event, int error_code, void *arg)
{
	SQRNetworkManager_Vita *manager = (SQRNetworkManager_Vita *)arg;

	switch( event )
	{
	case SCE_NP_MATCHING2_SIGNALING_EVENT_DEAD:
		{
			if( manager->m_isHosting )
			{
				// Remove any players associated with this peer
				manager->RemoveRemotePlayersAndSync( peerMemberId, 15 );
			}
			else
			{
				SQRVoiceConnection* pVoice = SonyVoiceChat_Vita::getVoiceConnectionFromRoomMemberID(peerMemberId);
				if(pVoice)
				{
					SonyVoiceChat_Vita::disconnectRemoteConnection(pVoice);
				}
				if(peerMemberId == manager->m_hostMemberId || pVoice == nullptr) //  MGH - added check for voice, as we sometime get here before m_hostMemberId has been filled in
				{
					// Host has left the game... so its all over for this client too. Finish everything up now, including deleting the server context which belongs to this gaming session
					// This also might be a response to a request to leave the game from our end too so don't need to do anything in that case
					if( manager->m_state != SNM_INT_STATE_LEAVING )
					{
						manager->DeleteServerContext();
						manager->ResetToIdle();
					}
				}
			}
		}
	case SCE_NP_MATCHING2_SIGNALING_EVENT_ESTABLISHED:
		{

			// MGH - changed this to always get the data now, as we need to know if the connecting peer is the host or not
			// If we're the host, then we need to get the data associated with the connecting peer to know what connections we should be trying to match. So
			// the actual creation of connections happens when the response for this request is processed.

			SceNpMatching2GetRoomMemberDataInternalRequest  reqParam;
			memset( &reqParam, 0, sizeof(reqParam));
			reqParam.roomId = roomId;
			reqParam.memberId = peerMemberId;
			SceNpMatching2AttributeId attrs[1] = {SCE_NP_MATCHING2_ROOMMEMBER_BIN_ATTR_INTERNAL_1_ID};
			reqParam.attrId = attrs;
			reqParam.attrIdNum = 1;

			sceNpMatching2GetRoomMemberDataInternal( manager->m_matchingContext, &reqParam, nullptr, &manager->m_roomMemberDataRequestId);
		}
		break;
	}
}


// Implementation of SceNpBasicEventHandler
int SQRNetworkManager_Vita::BasicEventCallback(int event, int retCode, uint32_t reqId, void *arg)
{
	PSVITA_STUBBED;
	// 	SQRNetworkManager_Vita *manager = (SQRNetworkManager_Vita *)arg;
	// 	// We aren't allowed to actually get the event directly from this callback, so send our own internal event to a thread dedicated to doing this
	// 	sceKernelTriggerUserEvent(m_basicEventQueue, sc_UserEventHandle, nullptr);

	return 0;
}

// Implementation of SceNpManagerCallback
void SQRNetworkManager_Vita::OnlineCheck()
{
	static bool s_bFullVersion = ProfileManager.IsFullVersion();
	if(s_bFullVersion != ProfileManager.IsFullVersion())
	{
		s_bFullVersion = ProfileManager.IsFullVersion();
		// we've switched from trial to full version here, if we're already online, call InitialiseAfterOnline, as this is now returns immediately in trial mode (devtrack #5921)
		if(GetOnlineStatus() == true)
		{
			InitialiseAfterOnline();
		}
	}

	bool bSignedIn = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad());
	if(GetOnlineStatus() == false)
	{
		if(bSignedIn)
		{
			SceNpServiceState serviceState;
			int retVal = sceNpGetServiceState(&serviceState);
			assert(retVal == 0);
			assert(serviceState == SCE_NP_SERVICE_STATE_ONLINE);
			InitialiseAfterOnline();
		}
	}
	else
	{
		if(bSignedIn == false)
			m_listener->HandleDisconnect(false);
	}
	UpdateOnlineStatus(bSignedIn);
}

// Implementation of CellSysutilCallback
void SQRNetworkManager_Vita::SysUtilCallback(uint64_t status, uint64_t param, void *userdata)
{
	// 	SQRNetworkManager_Vita *manager = (SQRNetworkManager_Vita *)userdata;
	//     struct CellNetCtlNetStartDialogResult netstart_result;
	//     int ret = 0;
	//     netstart_result.size = sizeof(netstart_result);
	//     switch(status)
	// 	{
	// 		case CELL_SYSUTIL_NET_CTL_NETSTART_FINISHED:
	// 			ret = cellNetCtlNetStartDialogUnloadAsync(&netstart_result);
	// 			if(ret < 0)
	// 			{
	// 				manager->SetState(SNM_INT_STATE_INITIALISE_FAILED);
	// 				if( s_SignInCompleteCallbackFn )
	// 				{
	// 					if( s_signInCompleteCallbackIfFailed )
	// 					{
	// 						s_SignInCompleteCallbackFn(s_SignInCompleteParam,false,0);
	// 					}
	// 					s_SignInCompleteCallbackFn  = nullptr;
	// 				}
	// 				return;
	// 			}
	//
	// 			if( netstart_result.result != 0 )
	// 			{
	// 				// Failed, or user may have decided not to sign in - maybe need to differentiate here
	// 				manager->SetState(SNM_INT_STATE_INITIALISE_FAILED);
	// 				if( s_SignInCompleteCallbackFn )
	// 				{
	// 					if( s_signInCompleteCallbackIfFailed )
	// 					{
	// 						s_SignInCompleteCallbackFn(s_SignInCompleteParam,false,0);
	// 					}
	// 					s_SignInCompleteCallbackFn  = nullptr;
	// 				}
	// 			}
	//
	// 			break;
	// 		case CELL_SYSUTIL_NET_CTL_NETSTART_UNLOADED:
	// 			break;
	// 		case CELL_SYSUTIL_NP_INVITATION_SELECTED:
	// 			manager->GetInviteDataAndProcess(SCE_NP_BASIC_SELECTED_INVITATION_DATA);
	// 			break;
	// 		default:
	// 			break;
	//     }
}


void SQRNetworkManager_Vita::updateNetCheckDialog()
{
	if(ProfileManager.IsSystemUIDisplayed())
	{
		if( sceNetCheckDialogGetStatus() == SCE_COMMON_DIALOG_STATUS_FINISHED )
		{
			//Check for errors
			SceNetCheckDialogResult netCheckResult;
			int ret = sceNetCheckDialogGetResult(&netCheckResult);
			app.DebugPrintf("NetCheckDialogResult = 0x%x\n", netCheckResult.result);
			ret = sceNetCheckDialogTerm();
			app.DebugPrintf("NetCheckDialogTerm ret = 0x%x\n", ret);
			ProfileManager.SetSysUIShowing( false );

			bool bConnectedOK = (netCheckResult.result == SCE_COMMON_DIALOG_RESULT_OK);
			if(bConnectedOK)
			{
				SceNetCtlInfo info;
				sceNetCtlInetGetInfo(SCE_NET_CTL_INFO_DEVICE, &info);
				if(info.device == SCE_NET_CTL_DEVICE_PHONE) // 3G connection, we're not going to allow this
				{
 					app.DebugPrintf("Online with 3G connection!!\n");
					ProfileManager.DisplaySystemMessage( SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_WIFI_REQUIRED_OPERATION, 0 );
					bConnectedOK = false;
				}
			}
			app.DebugPrintf("------------>>>>>>>>   sceNetCheckDialog finished\n");

			if( bConnectedOK )
			{
				if( s_SignInCompleteCallbackFn )
				{
					s_SignInCompleteCallbackFn(s_SignInCompleteParam,true,0);
					s_SignInCompleteCallbackFn  = nullptr;
				}
			}
			else
			{
				//		SCE_COMMON_DIALOG_RESULT_USER_CANCELED
				//		SCE_COMMON_DIALOG_RESULT_ABORTED

				// Failed, or user may have decided not to sign in - maybe need to differentiate here
				SetState(SNM_INT_STATE_INITIALISE_FAILED);
				if( s_SignInCompleteCallbackFn )
				{
					if( s_signInCompleteCallbackIfFailed )
					{
						s_SignInCompleteCallbackFn(s_SignInCompleteParam,false,0);
					}
					s_SignInCompleteCallbackFn  = nullptr;
				}
			}
		}
	}
}

// Implementation of CellRudpContextEventHandler. This is associate with an Rudp context every time one is created, and can be used to determine the status of each
// Rudp connection. We create one context/connection per local player on the non-hosting consoles.
void SQRNetworkManager_Vita::RudpContextCallback(int ctx_id, int event_id, int error_code, void *arg)
{
	SQRNetworkManager_Vita *manager = (SQRNetworkManager_Vita *)arg;
	switch(event_id)
	{
	case SCE_RUDP_CONTEXT_EVENT_CLOSED:
		{
			SQRVoiceConnection* pVoice = SonyVoiceChat_Vita::GetVoiceConnectionFromRudpCtx(ctx_id);
			if(pVoice)
			{
				pVoice->m_bConnected = false;
			}
			else
			{
				app.DebugPrintf(CMinecraftApp::USER_RR,"RUDP closed - event error 0x%x\n",error_code);
				if( !manager->m_isHosting )
				{
					if( manager->m_state == SNM_INT_STATE_JOINING_WAITING_FOR_LOCAL_PLAYERS )
					{
						manager->LeaveRoom(true);
					}
				}
			}
		}
		break;
	case SCE_RUDP_CONTEXT_EVENT_ESTABLISHED:
		{
			SQRNetworkPlayer *player = manager->GetPlayerFromRudpCtx(ctx_id);
			if( player )
			{
				// Flag connection stage as being completed for this player
				manager->NetworkPlayerConnectionComplete(player);
			}
			else
			{
				SonyVoiceChat_Vita::setConnected(ctx_id);
			}
		}
		break;
	case SCE_RUDP_CONTEXT_EVENT_ERROR:
		break;
	case SCE_RUDP_CONTEXT_EVENT_WRITABLE:
		{
			SQRNetworkPlayer *player = manager->GetPlayerFromRudpCtx(ctx_id);
			// This event signifies that room has opened up in the write buffer, so attempt to send something
			if( player )
			{
				player->SendMoreInternal();
			}
			else
			{
				SQRVoiceConnection* pVoice = SonyVoiceChat_Vita::GetVoiceConnectionFromRudpCtx(ctx_id);
				assert(pVoice);
			}
		}
		break;
	case SCE_RUDP_CONTEXT_EVENT_READABLE:
		if( manager->m_listener )
		{
			SQRVoiceConnection* pVoice = SonyVoiceChat_Vita::GetVoiceConnectionFromRudpCtx(ctx_id);
			if(pVoice)
			{
				pVoice->readRemoteData();
			}
			else
			{
				SQRNetworkPlayer *playerIncomingData = manager->GetPlayerFromRudpCtx( ctx_id );
				unsigned int dataSize = playerIncomingData->GetPacketDataSize();
				// If we're the host, and this player hasn't yet had its small id confirmed, then the first byte sent to us should be this id
				if( manager->m_isHosting )
				{
					SQRNetworkPlayer *playerFrom = manager->GetPlayerFromRudpCtx( ctx_id );
					if( playerFrom && !playerFrom->HasSmallIdConfirmed() )
					{
						if( dataSize >= sizeof(SQRNetworkPlayer::InitSendData) )
						{
							SQRNetworkPlayer::InitSendData ISD;
							int bytesRead = playerFrom->ReadDataPacket( &ISD, sizeof(SQRNetworkPlayer::InitSendData));
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
					int bytesRead = playerIncomingData->ReadDataPacket( data, dataSize );
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
						if( ( playerFrom != nullptr ) && ( playerTo != nullptr ) )
						{
							manager->m_listener->HandleDataReceived( playerFrom, playerTo, data, bytesRead );
						}
					}
					delete [] data;
				}
			}
		}
		break;
	case SCE_RUDP_CONTEXT_EVENT_FLUSHED:
		break;
	}
}

// Implementation of CellRudpEventHandler
#ifdef __PS3__
int SQRNetworkManager_Vita::RudpEventCallback(int event_id, int soc, uint8_t const *data, size_t datalen, struct sockaddr const *addr, socklen_t addrlen, void *arg)
#else
int SQRNetworkManager_Vita::RudpEventCallback(int event_id, int soc, uint8_t const *data, size_t datalen, struct SceNetSockaddr  const *addr, SceNetSocklen_t addrlen, void *arg)
#endif
{
	SQRNetworkManager_Vita *manager = (SQRNetworkManager_Vita *)arg;
	if( event_id == SCE_RUDP_EVENT_SOCKET_RELEASED )
	{
		assert( soc == manager->m_soc );
		sceNetSocketClose(soc);
		manager->m_soc = -1;
	}
	return 0;
}

void SQRNetworkManager_Vita::NetCtlCallback(int eventType, void *arg)
{
	SQRNetworkManager_Vita *manager = (SQRNetworkManager_Vita *)arg;
	// Oddly, the disconnect event comes in with a new state of "CELL_NET_CTL_STATE_Connecting"... looks like the event is more important than the state to
	// determine what has just happened
	if( eventType == SCE_NET_CTL_EVENT_TYPE_DISCONNECTED)// CELL_NET_CTL_EVENT_LINK_DISCONNECTED )
	{
		manager->m_bLinkDisconnected = true;
		manager->m_listener->HandleDisconnect(false);
	}
	else //if( event == CELL_NET_CTL_EVENT_ESTABLISH )
	{
		manager->m_bLinkDisconnected = false;
	}

}

// Called when the context has been created, and we are intending to create a room.
void SQRNetworkManager_Vita::ServerContextValid_CreateRoom()
{
	// First find a world
	SetState(SNM_INT_STATE_HOSTING_CREATE_ROOM_SEARCHING_FOR_WORLD);

	SceNpMatching2GetWorldInfoListRequest reqParam;

	//  Request parameters
	memset(&reqParam, 0, sizeof(reqParam));
	reqParam.serverId = m_serverId;

	int ret = -1;
	if( !ForceErrorPoint(SNM_FORCE_ERROR_GET_WORLD_INFO_LIST) )
	{
		app.DebugPrintf("sceNpMatching2GetWorldInfoList\n");
		m_getWorldRequestId=0;
		ret = sceNpMatching2GetWorldInfoList( m_matchingContext, &reqParam, nullptr, &m_getWorldRequestId);
	}
	if (ret < 0)
	{
		SetState(SNM_INT_STATE_HOSTING_CREATE_ROOM_FAILED);
		return;
	}
}

// Called when the context has been created, and we are intending to join a pre-existing room.
void SQRNetworkManager_Vita::ServerContextValid_JoinRoom()
{
	// 	assert( m_state == SNM_INT_STATE_JOINING_SERVER_SEARCH_CREATING_CONTEXT );

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

	int ret = sceNpMatching2JoinRoom( m_matchingContext, &reqParam, nullptr, &m_joinRoomRequestId );
	if ( (ret < 0) || ForceErrorPoint(SNM_FORCE_ERROR_JOIN_ROOM) )
	{
		if( ret == SCE_NP_MATCHING2_SERVER_ERROR_NAT_TYPE_MISMATCH)
		{
			app.SetDisconnectReason( DisconnectPacket::eDisconnect_NATMismatch );
		}
		SetState(SNM_INT_STATE_JOINING_JOIN_ROOM_FAILED);
	}
}

const SceNpCommunicationId* SQRNetworkManager_Vita::GetSceNpCommsId()
{
	return &s_npCommunicationId;
}

const SceNpCommunicationSignature* SQRNetworkManager_Vita::GetSceNpCommsSig()
{
	return &s_npCommunicationSignature;
}

const SceNpTitleId* SQRNetworkManager_Vita::GetSceNpTitleId()
{
	PSVITA_STUBBED;
	return nullptr;
	// 	return &s_npTitleId;
}

const SceNpTitleSecret* SQRNetworkManager_Vita::GetSceNpTitleSecret()
{
	PSVITA_STUBBED;
	return nullptr;
	//	return &s_npTitleSecret;
}

int	SQRNetworkManager_Vita::GetOldMask(SceNpMatching2RoomMemberId memberId)
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

int	SQRNetworkManager_Vita::GetAddedMask(int newMask, int oldMask)
{
	return newMask & ~oldMask;
}

int	SQRNetworkManager_Vita::GetRemovedMask(int newMask, int oldMask)
{
	return oldMask & ~newMask;
}


void SQRNetworkManager_Vita::GetExtDataForRoom( SceNpMatching2RoomId roomId, void *extData, void (* FriendSessionUpdatedFn)(bool success, void *pParam), void *pParam )
{
	static SceNpMatching2GetRoomDataExternalListRequest reqParam;
	static SceNpMatching2RoomId aRoomId[1];
	static SceNpMatching2AttributeId attr[1];

	// All parameters will be nullptr if this is being called a second time, after creating a new matching context via one of the paths below (using GetMatchingContext).
	// nullptr parameters therefore basically represents an attempt to retry the last sceNpMatching2GetRoomDataExternalList
	if( extData != nullptr )
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

	// Kicked off an asynchronous thing that will create a matching context, and then call this method back again (with nullptr params) once done, so we can reattempt. Don't do anything more now.
	if( m_state == SNM_INT_STATE_IDLE_RECREATING_MATCHING_CONTEXT )
	{
		app.DebugPrintf("Having to recreate matching context, setting state to SNM_INT_STATE_IDLE_RECREATING_MATCHING_CONTEXT\n");
		return;
	}

	int ret = sceNpMatching2GetRoomDataExternalList( m_matchingContext, &reqParam, nullptr, &m_roomDataExternalListRequestId );

	// If we hadn't properly detected that a matching context was unvailable, we might still get an error indicating that it is from the previous call. Handle similarly, but we need
	// to destroy the context first.
	if( ret == SCE_NP_MATCHING2_ERROR_CONTEXT_NOT_STARTED )			// Also checking for this as a means of simulating the previous error
	{
		sceNpMatching2DestroyContext(m_matchingContext);
		m_matchingContextValid = false;
		if( !GetMatchingContext(SNM_INT_STATE_IDLE_RECREATING_MATCHING_CONTEXT) )
		{
			// No matching context, and failed to try and make one. We're really broken here.
			m_FriendSessionUpdatedFn(false, m_pParamFriendSessionUpdated);
			return;
		};
		// Kicked off an asynchronous thing that will create a matching context, and then call this method back again (with nullptr params) once done, so we can reattempt. Don't do anything more now.
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
bool SQRNetworkManager_Vita::ForceErrorPoint(eSQRForceError error)
{
	return false;
}
#else
bool SQRNetworkManager_Vita::aForceError[SNM_FORCE_ERROR_COUNT] =
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

bool SQRNetworkManager_Vita::ForceErrorPoint(eSQRForceError err)
{
	return aForceError[err];
}
#endif

void SQRNetworkManager_Vita::AttemptPSNSignIn(int (*SignInCompleteCallbackFn)(void *pParam, bool bContinue, int pad), void *pParam, bool callIfFailed/*=false*/)
{
	if(CGameNetworkManager::usingAdhocMode())
	{
		SQRNetworkManager_AdHoc_Vita::AttemptPSNSignIn(SignInCompleteCallbackFn, pParam, callIfFailed);
		return;
	}
	s_SignInCompleteCallbackFn = SignInCompleteCallbackFn;
	s_signInCompleteCallbackIfFailed = callIfFailed;
	s_SignInCompleteParam = pParam;

	SceNetCheckDialogParam param;
	memset(&param, 0x00, sizeof(param));
	sceNetCheckDialogParamInit(&param);
	param.mode = SCE_NETCHECK_DIALOG_MODE_PSN_ONLINE;
	param.defaultAgeRestriction = ProfileManager.GetMinimumAge();

	// -------------------------------------------------------------
	// MGH -  this code is duplicated in the adhoc manager now too, so any changes will have to be made there too
	// -------------------------------------------------------------
	//CD - Only add if EU sku, not SCEA or SCEJ
	if( app.GetProductSKU() == e_sku_SCEE )
	{
		//CD - Added Country age restrictions
		SceNetCheckDialogAgeRestriction restrictions[5];
		memset( restrictions, 0x0, sizeof(SceNetCheckDialogAgeRestriction) * 5 );
		//Germany
		restrictions[0].age = ProfileManager.GetGermanyMinimumAge();
		memcpy(	restrictions[0].countryCode, "de", 2 );
		//Russia
		restrictions[1].age = ProfileManager.GetRussiaMinimumAge();
		memcpy(	restrictions[1].countryCode, "ru", 2 );
		//Australia
		restrictions[2].age = ProfileManager.GetAustraliaMinimumAge();
		memcpy(	restrictions[2].countryCode, "au", 2 );
		//Japan
		restrictions[3].age = ProfileManager.GetJapanMinimumAge();
		memcpy(	restrictions[3].countryCode, "jp", 2 );
		//Korea
		restrictions[4].age = ProfileManager.GetKoreaMinimumAge();
		memcpy(	restrictions[4].countryCode, "kr", 2 );
		//Set
		param.ageRestriction = restrictions;
		param.ageRestrictionCount = 5;
	}

	memcpy(&param.npCommunicationId.data, &s_npCommunicationId, sizeof(s_npCommunicationId));
	param.npCommunicationId.term = '\0';
	param.npCommunicationId.num = 0;

	int ret = sceNetCheckDialogInit(&param);

	ProfileManager.SetSysUIShowing( true );
	app.DebugPrintf("------------>>>>>>>>   sceNetCheckDialogInit : PSN Mode\n");

	if( ret < 0 )
	{
		if(s_SignInCompleteCallbackFn) // MGH - added after crash on PS4
		{
			if( s_signInCompleteCallbackIfFailed )
			{
				s_SignInCompleteCallbackFn(s_SignInCompleteParam,false,0);
			}
			s_SignInCompleteCallbackFn  = nullptr;
		}
	}
}

int SQRNetworkManager_Vita::SetRichPresence(const void *data)
{
	const sce::Toolkit::NP::PresenceDetails *newPresenceInfo = (const sce::Toolkit::NP::PresenceDetails *)data;

	s_lastPresenceInfo.status = newPresenceInfo->status;
	// 	s_lastPresenceInfo.userInfo = newPresenceInfo->userInfo;
	s_lastPresenceInfo.presenceType = SCE_NP_BASIC_IN_GAME_PRESENCE_TYPE_DEFAULT;

	s_presenceStatusDirty = true;
	if(s_resendPresenceCountdown == 0)
	{
		s_resendPresenceCountdown = 5; // wait a few ticks before setting the rich presence value, so if there's a few being set at one time (like on game startup) we can send them all in a single call
	}

	// Return as if no error happened no matter what, as we'll be resending ourselves if we need to and don't want the calling system to retry
	return 0;
}

void SQRNetworkManager_Vita::UpdateRichPresenceCustomData(void *data, unsigned int dataBytes)
{
	assert(dataBytes <= SCE_NP_BASIC_IN_GAME_PRESENCE_DATA_SIZE_MAX );
	memcpy(s_lastPresenceInfo.data, data, dataBytes);
	s_lastPresenceInfo.size = dataBytes;

	s_presenceStatusDirty = true;
	if(s_resendPresenceCountdown == 0)
	{
		s_resendPresenceCountdown = 5; // wait a few ticks before setting the rich presence value, so if there's a few being set at one time (like on game startup) we can send them all in a single call
	}
}

void SQRNetworkManager_Vita::TickRichPresence()
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

void SQRNetworkManager_Vita::SendLastPresenceInfo()
{
	// Don't attempt to send if we are already waiting to resend
	if( s_resendPresenceCountdown ) return;

	// MGH - On Vita, change this to use SCE_NP_BASIC_IN_GAME_PRESENCE_TYPE_GAME_JOINING  at some point

	// On PS4 we can't set the status and the data at the same time
	if( s_presenceStatusDirty == false)
	{
		return; // nothing to be done.
	}

	int err = 0;
	// check if we're connected to the PSN first
	if(ProfileManager.IsSignedInLive(0))//ProfileManager.getQuadrant(s_lastPresenceInfo.userInfo.userId)))
	{
		s_lastPresenceInfo.presenceType = SCE_NP_BASIC_IN_GAME_PRESENCE_TYPE_DEFAULT;
		// MGH - if we have data being sent, that means we're in an online game, so let others join from our presence info on the XMB
		if(s_lastPresenceInfo.size > 0 )
		{
			// make sure it's not an invite only game
			PresenceSyncInfo *pso = (PresenceSyncInfo *)s_lastPresenceInfo.data;
			if(!pso->inviteOnly)
			{
				s_lastPresenceInfo.presenceType = SCE_NP_BASIC_IN_GAME_PRESENCE_TYPE_GAME_JOINING;
			}
		}
		err = sce::Toolkit::NP::Presence::Interface::setPresence(&s_lastPresenceInfo);
	}

	if( err != SCE_TOOLKIT_NP_SUCCESS )
	{
		s_resendPresenceCountdown = (20 * 65);  // Bit over a minute before attempting to resend, we should get a new token every minute
	}
	else
	{
		s_presenceStatusDirty = false;
	}
}

void SQRNetworkManager_Vita::SetPresenceFailedCallback()
{
	s_presenceStatusDirty = true;
	s_resendPresenceCountdown = (20 * 65);  // Bit over a minute before attempting to resend, we should get a new token every minute
}

void SQRNetworkManager_Vita::SetPresenceDataStartHostingGame()
{
	if( m_offlineGame )
	{
		SQRNetworkManager_Vita::UpdateRichPresenceCustomData(&c_presenceSyncInfoNULL, sizeof(SQRNetworkManager_Vita::PresenceSyncInfo) );
	}
	else
	{
		SQRNetworkManager_Vita::PresenceSyncInfo presenceInfo;
		CPlatformNetworkManagerSony::SetSQRPresenceInfoFromExtData( &presenceInfo, m_joinExtData, m_room, m_serverId );
		SQRNetworkManager_Vita::UpdateRichPresenceCustomData(&presenceInfo, sizeof(SQRNetworkManager_Vita::PresenceSyncInfo) );
		//		OrbisNPToolkit::createNPSession();
	}
}

int SQRNetworkManager_Vita::GetJoiningReadyPercentage()
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

void SQRNetworkManager_Vita::removePlayerFromVoiceChat( SQRNetworkPlayer* pPlayer )
{
	if(pPlayer->IsLocal())
	{
		SonyVoiceChat_Vita::disconnectLocalPlayer(pPlayer->GetLocalPlayerIndex());
	}
	else
	{
		int numRemotePlayersLeft = 0;
		for( int i = 0; i < MAX_ONLINE_PLAYER_COUNT; i++ )
		{
			if( m_aRoomSlotPlayers[i] )
			{
				if( m_aRoomSlotPlayers[i] != pPlayer )
				{
					if(m_aRoomSlotPlayers[i]->m_roomMemberId == pPlayer->m_roomMemberId)
						numRemotePlayersLeft++;
				}
			}
		}
		if(numRemotePlayersLeft == 0)
		{
			// no players left on the remote machine once we remove this one
			SQRVoiceConnection* pVoice = SonyVoiceChat_Vita::getVoiceConnectionFromRoomMemberID(pPlayer->m_roomMemberId);
			if(pVoice)
				SonyVoiceChat_Vita::disconnectRemoteConnection(pVoice);
		}
	}
}


SQRNetworkPlayer *SQRNetworkManager_Vita::GetPlayerByXuid(PlayerUID xuid)
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
	return nullptr;
}

