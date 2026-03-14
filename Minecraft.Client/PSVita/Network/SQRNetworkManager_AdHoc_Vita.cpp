#include "stdafx.h"
#include "SQRNetworkManager_AdHoc_Vita.h"
#include "SonyVoiceChat_Vita.h"
#include "Common/Network/Sony/PlatformNetworkManagerSony.h"

#include <np.h>
#include <rudp.h>
#include <np/np_basic.h>
#include <libnetctl.h>
#include <netcheck_dialog.h>
#include <pspnet_adhoc.h>
#include <adhoc_matching.h>

#include "PSVita/PSVitaExtras/Conf.h"
#include "Common/Network/Sony/SonyHttp.h"
#include "../../../Minecraft.World/C4JThread.h"


#define MATCHING_PORT						(1)
#define MATCHING_RXBUFLEN					(2048)
#define HELLO_INTERVAL						(1 * 1000 * 1000)

#define KEEPALIVE_INTERVAL					(1000 * 1000)
#define INIT_COUNT							(10)		// seconds before timeout
#define REXMT_INTERVAL						(1000 * 1000)
#define MATCHING_EVENT_HANDLER_STACK_SIZE	((4 * 1024) + SCE_NET_ADHOC_MATCHING_POOLSIZE_DEFAULT)

#define ADHOC_VPORT 4649
class HelloSyncInfo
{
public:
	SQRNetworkManager::PresenceSyncInfo				m_presenceSyncInfo;
	GameSessionData									m_gameSessionData;
	SQRNetworkManager::RoomSyncData					m_roomSyncData;
};


static const bool sc_voiceChatEnabled = false; //  don't think we'll need voice chat in ad-hoc, will need some work to get the mesh connections if we do.


bool g_bNetworkAdHocMode = false;

int										SQRNetworkManager_AdHoc_Vita::m_adhocStatus = false;


static unsigned char	s_Matching2Pool[SCE_NET_ADHOC_MATCHING_POOLSIZE_DEFAULT];

int (* SQRNetworkManager_AdHoc_Vita::s_SignInCompleteCallbackFn)(void *pParam, bool bContinue, int pad) = nullptr;
void * SQRNetworkManager_AdHoc_Vita::s_SignInCompleteParam = nullptr;
sce::Toolkit::NP::PresenceDetails SQRNetworkManager_AdHoc_Vita::s_lastPresenceInfo;
int SQRNetworkManager_AdHoc_Vita::s_resendPresenceCountdown = 0;
bool SQRNetworkManager_AdHoc_Vita::s_presenceStatusDirty = false;
bool SQRNetworkManager_AdHoc_Vita::s_presenceDataDirty = false;
bool SQRNetworkManager_AdHoc_Vita::s_signInCompleteCallbackIfFailed = false;
HelloSyncInfo SQRNetworkManager_AdHoc_Vita::s_lastPresenceSyncInfo = { 0 };
HelloSyncInfo SQRNetworkManager_AdHoc_Vita::c_presenceSyncInfoNULL = { 0 };
//SceNpBasicAttachmentDataId SQRNetworkManager_AdHoc_Vita::s_lastInviteIdToRetry = SCE_NP_BASIC_INVALID_ATTACHMENT_DATA_ID;
long long SQRNetworkManager_AdHoc_Vita::s_roomStartTime = 0;
bool SQRNetworkManager_AdHoc_Vita::b_inviteRecvGUIRunning = false;
// HelloSyncInfo* SQRNetworkManager_AdHoc_Vita::m_gameBootInvite;
// HelloSyncInfo SQRNetworkManager_AdHoc_Vita::m_gameBootInvite_data;

// static const int sc_UserEventHandle = 0;

//unsigned int SQRNetworkManager_AdHoc_Vita::RoomSyncData::playerCount = 0;

SQRNetworkManager_AdHoc_Vita* s_pAdhocVitaManager;//  have to use a static var for this as the callback function doesn't take an arg
static bool s_attemptSignInAdhoc = true; // false if we're trying to sign in to the PSN while in adhoc mode, so we can ignore the error if it fails

// This maps internal to extern states, and needs to match element-by-element the eSQRNetworkManagerInternalState enumerated type
const SQRNetworkManager_AdHoc_Vita::eSQRNetworkManagerState SQRNetworkManager_AdHoc_Vita::m_INTtoEXTStateMappings[SQRNetworkManager_AdHoc_Vita::SNM_INT_STATE_COUNT] =
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

SQRNetworkManager_AdHoc_Vita::SQRNetworkManager_AdHoc_Vita(ISQRNetworkManagerListener *listener)
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
	m_resendExternalRoomDataCountdown = 0;
	m_matching2initialised = false;
	m_matchingContextClientValid = false;
	m_matchingContextServerValid = false;
	m_soc = -1;
// 	m_inviteIndex = 0;
// 	m_doBootInviteCheck = true;
	m_isInSession = false;
	m_offlineGame = false;
	m_offlineSQR = false;
	m_aServerId = nullptr;
// 	m_gameBootInvite = nullptr;
	m_adhocStatus = false;
	m_bLinkDisconnected = false;
	m_bIsInitialised=false;

	InitializeCriticalSection(&m_csRoomSyncData);
	InitializeCriticalSection(&m_csPlayerState);
	InitializeCriticalSection(&m_csStateChangeQueue);
	InitializeCriticalSection(&m_csAckQueue);

	memset( &m_roomSyncData,0,sizeof(m_roomSyncData));		// MGH -  added to fix problem when joining a full room, and the sync data wasn't populated

// 	int ret = sceKernelCreateEqueue(&m_basicEventQueue, "SQRNetworkManager_AdHoc_Vita EQ");
// 	assert(ret == SCE_OK);
// 	ret = sceKernelAddUserEvent(m_basicEventQueue, sc_UserEventHandle);
// 	assert(ret == SCE_OK);
//
// 	m_basicEventThread = new C4JThread(&BasicEventThreadProc,this,"Basic Event Handler");
// 	m_basicEventThread->Run();
}

static std::string getIPAddressString(SceNetInAddr add)
{
	char str[32];
	unsigned char *vals = (unsigned char*)&add.s_addr;
	sprintf(str, "%d.%d.%d.%d", (int)vals[0], (int)vals[1], (int)vals[2], (int)vals[3]);
	return std::string(str);
}


// First stage of initialisation. This initialises a few things that don't require the user to be signed in, and then kicks of the network start dialog utility.
// Initialisation continues in InitialiseAfterOnline once this completes.
void SQRNetworkManager_AdHoc_Vita::Initialise()
{
#define NP_IN_GAME_MESSAGE_POOL_SIZE ( 16 * 1024 )

	int32_t ret = 0;
// 	int32_t libCtxId = 0;
// 	ret = sceNpInGameMessageInitialize(NP_IN_GAME_MESSAGE_POOL_SIZE, nullptr);
// 	assert (ret >= 0);
// 	libCtxId = ret;

	assert( m_state == SNM_INT_STATE_UNINITIALISED );


	//Initialize libnetctl
//  	ret = sceNetCtlInit();
// 	if( ( ret < 0 && ret != SCE_NET_CTL_ERROR_NOT_TERMINATED ) || ForceErrorPoint( SNM_FORCE_ERROR_NET_CTL_INIT ) )
// 	{
// 		SetState(SNM_INT_STATE_INITIALISE_FAILED);
// 		return;
// 	}
	m_hid=0;
	ret = sceNetCtlAdhocRegisterCallback(NetCtlCallback,this,&m_hid);
	assert(ret == SCE_OK);

	// Initialise RUDP
	const int RUDP_POOL_SIZE = (500 * 1024);		// TODO - find out what we need, this size is copied from library reference
	uint8_t *rudp_pool = (uint8_t *)malloc(RUDP_POOL_SIZE);
	ret = sceRudpInit(rudp_pool, RUDP_POOL_SIZE);
	if( ( ret < 0 ) || ForceErrorPoint( SNM_FORCE_ERROR_RUDP_INIT ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return;
	}

	SetState(SNM_INT_STATE_SIGNING_IN);


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
//
 	ret = sceRudpEnableInternalIOThread(RUDP_THREAD_STACK_SIZE, SCE_KERNEL_DEFAULT_PRIORITY);
 	if(ret < 0)
 	{
 		app.DebugPrintf("sceRudpEnableInternalIOThread failed with error code 0x%08x\n", ret);
 		assert(0);
 	}

	/* initialize pspnet adhoc */
	ret = sceNetAdhocInit();
	if(ret < 0)
	{
		app.DebugPrintf("sceNetAdhocInit() failed. ret = 0x%x\n", ret);
		//sceNetCtlTerm();
		//sceNetTerm();
		assert(0);
		return;
	}

	/* initialize pspnet adhoc ctrl */
	SceNetAdhocctlAdhocId adhocId;
	memset(&adhocId, 0x00, sizeof(SceNetAdhocctlAdhocId));
	adhocId.type = SCE_NET_ADHOCCTL_ADHOCTYPE_RESERVED;
	memcpy(&adhocId.data[0], s_npCommunicationId.data, SCE_NET_ADHOCCTL_ADHOCID_LEN);
	ret = sceNetAdhocctlInit(&adhocId);
	if(ret < 0)
	{
		app.DebugPrintf("sceNetAdhocctlInit() failed. ret = 0x%x\n", ret);
		assert(0);
	}

	OnlineCheck();
	// Already online? the callback won't catch this, so carry on initialising now
	if(GetAdhocStatus())
	{
		InitialiseAfterOnline();
	}
// 	if(sc_voiceChatEnabled)
// 	{
//  		SonyVoiceChat_Vita::init();
// 	}

	m_bIsInitialised=true;
}

bool SQRNetworkManager_AdHoc_Vita::IsInitialised()
{
	return m_bIsInitialised;
}

void SQRNetworkManager_AdHoc_Vita::UnInitialise()
{
	int ret;
	StopMatchingContext();
	// These can fail if we've not initialised after online
	ret = sceNetAdhocMatchingTerm();
	ret = sceRudpEnd();
	ret = sceNpMatching2Term();
	///////////////////////////////////////////////////////

	ret = sceNetCtlAdhocUnregisterCallback(m_hid);

	//sceNetCtlTerm();

	ret = sceNetAdhocTerm();

//	ret = sceNetTerm();
	ret = sceNetAdhocctlTerm();

	SetState(SNM_INT_STATE_UNINITIALISED);

	m_bIsInitialised=false;
}

void SQRNetworkManager_AdHoc_Vita::Terminate()
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

	int ret = sceRudpEnd();
	ret = sceNpMatching2Term();
	// Terminate event thread by sending it a non-zero value for data
// 	sceKernelTriggerUserEvent(m_basicEventQueue, sc_UserEventHandle, (void*)1);


// 	do
// 	{
// 		Sleep(10);
// 	} while( m_basicEventThread->isRunning() );
}


SceNpMatching2RoomMemberId getRoomMemberID(SceNetInAddr* addr)
{

	// adhoc IP address are of the format 169.254.***.***
	// the last 2 digits of the IP address should be unique, so we're using them as a room member ID value
	return addr->s_addr >> 16;
}

void SQRNetworkManager_AdHoc_Vita::StopMatchingContext()
{
	if(m_matchingContextServerValid || m_matchingContextClientValid)
	{
		int err = sceNetAdhocMatchingStop(m_matchingContext);
		assert(err == SCE_OK);
		err =  sceNetAdhocMatchingDelete(m_matchingContext);
		assert(err == SCE_OK);
		m_matchingContextServerValid = false;
		m_matchingContextClientValid = false;
		m_matchingContext = -1;
	}
}


bool SQRNetworkManager_AdHoc_Vita::CreateMatchingContext(bool bServer /*= false*/)
{
	int matchingMode;
	if(bServer)
	{
		if(m_matchingContextServerValid)
		{
			SetState(SNM_INT_STATE_IDLE);
			return true;
		}
		matchingMode = SCE_NET_ADHOC_MATCHING_MODE_PARENT;
	}
	else
	{
		if(m_matchingContextClientValid)
		{
			SetState(SNM_INT_STATE_IDLE);
			return true;
		}
		matchingMode = SCE_NET_ADHOC_MATCHING_MODE_CHILD;
	}
	StopMatchingContext();

	int ret = sceNetAdhocMatchingCreate(matchingMode,			// create this as a client (child) context at first so we can search for other servers (parents)
		MAX_ONLINE_PLAYER_COUNT, MATCHING_PORT, MATCHING_RXBUFLEN,
		HELLO_INTERVAL, KEEPALIVE_INTERVAL, INIT_COUNT,
		REXMT_INTERVAL, MatchingEventHandler);

	s_pAdhocVitaManager = this;

	if( ( ret < 0 ) || ForceErrorPoint( SNM_FORCE_ERROR_CREATE_MATCHING_CONTEXT ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return false;
	}

	bool bRet = RegisterCallbacks();
	if( ( !bRet ) || ForceErrorPoint( SNM_FORCE_ERROR_REGISTER_CALLBACKS ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return false;
	}
	m_matchingContext = ret;



	// Free up any external data that we received from the previous search
	for( size_t i = 0; i < m_aFriendSearchResults.size(); i++ )
	{
		if(m_aFriendSearchResults[i].m_RoomExtDataReceived)
			free(m_aFriendSearchResults[i].m_RoomExtDataReceived);
		m_aFriendSearchResults[i].m_RoomExtDataReceived = nullptr;
		if(m_aFriendSearchResults[i].m_gameSessionData)
			free(m_aFriendSearchResults[i].m_gameSessionData);
		m_aFriendSearchResults[i].m_gameSessionData = nullptr;
	}
	m_friendCount = 0;
	m_aFriendSearchResults.clear();


	// Start the context
	// Set time-out time to 10 seconds
	ret = sceNetAdhocMatchingStart(m_matchingContext,
		SCE_KERNEL_DEFAULT_PRIORITY_USER, MATCHING_EVENT_HANDLER_STACK_SIZE,
		SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT,
		0, nullptr);//sizeof(g_myInfo.name), &g_myInfo.name);

	if( ( ret < 0 ) || ForceErrorPoint( SNM_FORCE_ERROR_CONTEXT_START_ASYNC ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return false;
	}



	if(bServer)
		m_matchingContextServerValid = true;
	else
		m_matchingContextClientValid = true;

	HandleMatchingContextStart();

	return true;


}



// Second stage of initialisation, that requires NP Manager to be online & the player to be signed in. This kicks of the creation of a context
// for Np Matching 2. Initialisation is finally complete when we get a callback to ContextCallback. The SQRNetworkManager_AdHoc_Vita is then finally moved
// into SNM_INT_STATE_IDLE at this stage.
void SQRNetworkManager_AdHoc_Vita::InitialiseAfterOnline()
{
// 	SceNpId npId;
// 	int option = 0;

	// We should only be doing this if we have come in from an initialisation stage (SQRNetworkManager_AdHoc_Vita::Initialise) or we've had a network disconnect and are coming in from an offline state.
	// Don't do anything otherwise - this mainly to catch a bit of a corner case in the initialisation phase where potentially we could register for the callback that would call this with sceNpManagerRegisterCallback.
	// and could then really quickly go online so that the there becomes two paths (via the callback or SQRNetworkManager_AdHoc_Vita::Initialise) by which this could be called
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
	int ret = sceNetAdhocMatchingInit(SCE_NET_ADHOC_MATCHING_POOLSIZE_DEFAULT,	s_Matching2Pool);

	if( ( ret < 0 ) || ForceErrorPoint( SNM_FORCE_ERROR_MATCHING2_INIT ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return;
	}
	app.DebugPrintf("SQRNetworkManager::InitialiseAfterOnline - matching context is now valid\n");
	m_matching2initialised = true;

	bool bRet = RegisterCallbacks();
	if( ( !bRet ) || ForceErrorPoint( SNM_FORCE_ERROR_REGISTER_CALLBACKS ) )
	{
		SetState(SNM_INT_STATE_INITIALISE_FAILED);
		return;
	}

	// State should be starting context until the callback that this has been created happens
	SetState(SNM_INT_STATE_STARTING_CONTEXT);
	if(CreateMatchingContext())
	{
		return;
	}

	SetState(SNM_INT_STATE_IDLE);
}


// General tick function to be called from main game loop - any internal tick functions should be called from here.
void SQRNetworkManager_AdHoc_Vita::Tick()
{
	TickWriteAcks();
	OnlineCheck();
	int ret;
	if((ret = sceNetCtlCheckCallback()) < 0 )
	{
		app.DebugPrintf("sceNetCtlCheckCallback error[%d]",ret);
	}
	updateNetCheckDialog(); //CD - Added this to match the SQRNetworkManager_Vita class
	RoomCreateTick();
	FriendSearchTick();
	TickRichPresence();
// 	TickInviteGUI();  //  TODO

	// to fix the crash when spamming the x button on signing in to PSN, don't bring up all the disconnect stuff till the pause menu disappears
 	if(!ui.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad()))
 	{
 		if(!m_offlineGame && m_bLinkDisconnected)
 		{
 			m_bLinkDisconnected = false;
 			m_listener->HandleDisconnect(false);
 		}
 	}


// 	if( ( m_gameBootInvite m) && ( s_safeToRespondToGameBootInvite ) )
// 	{
// 		m_listener->HandleInviteReceived( ProfileManager.GetPrimaryPad(), m_gameBootInvite );
// 		m_gameBootInvite = nullptr;
// 	}

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
			UpdateRichPresenceCustomData(&s_lastPresenceSyncInfo, sizeof(HelloSyncInfo));
			SetState( SNM_INT_STATE_STARTING);
			SetState( SNM_INT_STATE_PLAYING );
		}
	}

	if( m_state == SNM_INT_STATE_SERVER_DELETING_CONTEXT )
	{
		// make sure we've removed all the remote players and killed the udp connections before we bail out
// 		if(m_RudpCtxToPlayerMap.size() == 0)
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
}

// Detect any states which reflect internal error states, do anything required, and transition away again
void SQRNetworkManager_AdHoc_Vita::ErrorHandlingTick()
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
		if( m_isInSession && m_offlineSQR )
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
void SQRNetworkManager_AdHoc_Vita::CreateAndJoinRoom(int hostIndex, int localPlayerMask, void *extData, int extDataSize, bool offline)
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
		SetState(SNM_INT_STATE_HOSTING_STARTING_MATCHING_CONTEXT);
		// Kick off the sequence of events required for an online game, starting with getting the server context
		if(CreateMatchingContext(true))
		{
			m_isInSession = true;
			SetState(SNM_INT_STATE_HOSTING_CREATE_ROOM_SUCCESS);
		}
	}
}

// Updates the externally visible data that was associated with the room when it was created with CreateAndJoinRoom.
void SQRNetworkManager_AdHoc_Vita::UpdateExternalRoomData()
{
	// Update the hello message here
	if( m_isHosting )
	{
		HelloSyncInfo presenceInfo;
		CPlatformNetworkManagerSony::SetSQRPresenceInfoFromExtData( &presenceInfo.m_presenceSyncInfo, m_joinExtData, m_room, m_serverId );
		assert(m_joinExtDataSize == sizeof(GameSessionData));
		memcpy(&presenceInfo.m_gameSessionData, m_joinExtData, sizeof(GameSessionData));
		memcpy(&presenceInfo.m_roomSyncData, &m_roomSyncData, sizeof(RoomSyncData));
		SQRNetworkManager_AdHoc_Vita::UpdateRichPresenceCustomData(&presenceInfo, sizeof(HelloSyncInfo) );
		//		OrbisNPToolkit::createNPSession();
	}
}

// Determine if the friend room manager is busy. If it isn't busy, then other operations (searching for a friend, reading the found friend's room lists) may safely be performed
bool SQRNetworkManager_AdHoc_Vita::FriendRoomManagerIsBusy()
{
	return (m_friendSearchState != SNM_FRIEND_SEARCH_STATE_IDLE);
}

// Initiate a search for rooms that the signed in user's friends are in. This is an asynchronous operation, this function returns after it kicks off a search across all game servers
// for any of the player's friends.
bool SQRNetworkManager_AdHoc_Vita::FriendRoomManagerSearch()
{
	if( m_state != SNM_INT_STATE_IDLE ) return false;

	// Don't start another search if we're already searching...
	if( m_friendSearchState != SNM_FRIEND_SEARCH_STATE_IDLE )
	{
		return false;
	}


	m_friendSearchState = SNM_FRIEND_SEARCH_STATE_GETTING_FRIEND_COUNT;

	// Get friend list - doing this in another thread as it can lock up for a few seconds
// 	m_getFriendCountThread = new C4JThread(&GetFriendsThreadProc,this,"GetFriendsThreadProc");
// 	m_getFriendCountThread->Run();

	return true;
}

bool SQRNetworkManager_AdHoc_Vita::FriendRoomManagerSearch2()
{
	m_friendSearchState = SNM_FRIEND_SEARCH_STATE_IDLE;
// 	if( m_friendCount == 0 )
// 	{
// 		m_friendSearchState = SNM_FRIEND_SEARCH_STATE_IDLE;
// 		return false;
// 	}
//
// 	if( m_aFriendSearchResults.size() > 0 )
// 	{
// 		// If we have some results, then we also want to make sure that we don't have any duplicate rooms here if more than one friend is playing in the same room.
// 		unordered_set<SceNpMatching2RoomId> uniqueRooms;
// 		for( unsigned int i = 0; i < m_aFriendSearchResults.size(); i++ )
// 		{
// 			if(m_aFriendSearchResults[i].m_RoomFound)
// 			{
// 				uniqueRooms.insert( m_aFriendSearchResults[i].m_RoomId );
// 			}
// 		}
//
// 		// Tidy the results up further based on this
// 		for( unsigned int i = 0; i < m_aFriendSearchResults.size(); )
// 		{
// 			if( uniqueRooms.find(m_aFriendSearchResults[i].m_RoomId) == uniqueRooms.end() )
// 			{
// 				free(m_aFriendSearchResults[i].m_RoomExtDataReceived);
// 				m_aFriendSearchResults[i] = m_aFriendSearchResults.back();
// 				m_aFriendSearchResults.pop_back();
// 			}
// 			else
// 			{
// 				uniqueRooms.erase(m_aFriendSearchResults[i].m_RoomId);
// 				i++;
// 			}
// 		}
// 	}
// 	m_friendSearchState = SNM_FRIEND_SEARCH_STATE_IDLE;
	return true;
}

void SQRNetworkManager_AdHoc_Vita::FriendSearchTick()
{
	// Move onto next state if we're done getting our friend count
	if( m_friendSearchState == SNM_FRIEND_SEARCH_STATE_GETTING_FRIEND_COUNT )
	{
// 		if( !m_getFriendCountThread->isRunning() )
// 		{
// 			m_friendSearchState = SNM_FRIEND_SEARCH_STATE_GETTING_FRIEND_INFO;
// 			delete m_getFriendCountThread;
// 			m_getFriendCountThread = nullptr;
			FriendRoomManagerSearch2();
// 		}
	}
}

// The handler for basic events can't actually get the events themselves, this has to be done on another thread. Instead, we send a sys_event_t to a queue on This thread,
// which has a single data item used which we can use to determine whether to terminate this thread or get a basic event & handle that.
int SQRNetworkManager_AdHoc_Vita::BasicEventThreadProc( void *lpParameter )
{
	PSVITA_STUBBED;
	return 0;
// 	SQRNetworkManager_AdHoc_Vita *manager = (SQRNetworkManager_AdHoc_Vita *)lpParameter;
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

// int SQRNetworkManager_AdHoc_Vita::GetFriendsThreadProc( void* lpParameter )
// {
// 	SQRNetworkManager_AdHoc_Vita *manager = (SQRNetworkManager_AdHoc_Vita *)lpParameter;
//
// 	int ret = 0;
// 	manager->m_aFriendSearchResults.clear();
// 	manager->m_friendCount = 0;
// 	if(!ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()))
// 	{
// 		app.DebugPrintf("getFriendslist failed, not signed into Live! \n");
// 		return 0;
// 	}
//
//
// 	ret = sceNpBasicGetFriendListEntryCount(&manager->m_friendCount);
// 	if( ( ret < 0 ) || manager->ForceErrorPoint( SNM_FORCE_ERROR_GET_FRIEND_LIST_ENTRY_COUNT ) )
// 	{
// 		// This is likely when friend list hasn't been received from the server yet - will be returning SCE_NP_BASIC_ERROR_BUSY in this case
// 		manager->m_friendCount = 0;
// 	}
//
//
// 	// There shouldn't ever be more than 100 friends returned but limit here just in case
// 	if( manager->m_friendCount > 100 ) manager->m_friendCount = 100;
//
// 	SceNpId* friendIDs = nullptr;
// 	if(manager->m_friendCount > 0)
// 	{
// 		// grab all the friend IDs first
// 		friendIDs = new SceNpId[manager->m_friendCount];
// 		SceSize numRecieved;
// 		ret = sceNpBasicGetFriendListEntries(0, friendIDs, manager->m_friendCount, &numRecieved);
// 		if (ret < 0)
// 		{
// 			app.DebugPrintf("sceNpBasicGetFriendListEntries() failed: ret = 0x%x\n", ret);
// 			manager->m_friendCount = 0;
// 		}
// 		else
// 		{
// 			assert(numRecieved == manager->m_friendCount);
// 		}
// 	}
//
//
// 	// It is possible that the size of the friend list might vary from what we just received, so only add in friends that we successfully get an entry for
// 	for( unsigned int i = 0; i < manager->m_friendCount; i++ )
// 	{
// 		static SceNpBasicGamePresence presenceDetails;
// 		static SceNpBasicFriendContextState contextState;
// 		int ret = sceNpBasicGetFriendContextState(&friendIDs[i], &contextState);
// 		if (ret < 0)
// 		{
// 			app.DebugPrintf("sceNpBasicGetFriendContextState() failed: ret = 0x%x\n", ret);
// 			contextState = SCE_NP_BASIC_FRIEND_CONTEXT_STATE_UNKNOWN;
// 		}
// 		if(contextState == SCE_NP_BASIC_FRIEND_CONTEXT_STATE_IN_CONTEXT) // using the same SceNpCommunicationId, so playing Minecraft
// 		{
// 			ret = sceNpBasicGetGamePresenceOfFriend(&friendIDs[i], &presenceDetails);
// 			if( ( ret == 0 ) && ( !manager->ForceErrorPoint( SNM_FORCE_ERROR_GET_FRIEND_LIST_ENTRY ) ) )
// 			{
// 				FriendSearchResult result;
// 				memcpy(&result.m_NpId, &friendIDs[i], sizeof(SceNpId));
// 				result.m_RoomFound = false;
//
// 				// Only include the friend's game if its the same network id ( this also filters out generally Zeroed HelloSyncInfo, which we do when we aren't in an active game session)
// // 				if( presenceDetails.size == sizeof(HelloSyncInfo) )
// 				{
// 					HelloSyncInfo *pso = (HelloSyncInfo *)presenceDetails.inGamePresence.data;
// 					if( pso->netVersion == MINECRAFT_NET_VERSION )
// 					{
// 						if( !pso->inviteOnly )
// 						{
// 							result.m_RoomFound = true;
// 							result.m_RoomId = pso->m_RoomId;
// 							result.m_ServerId = pso->m_ServerId;
//
// 							CPlatformNetworkManagerSony::MallocAndSetExtDataFromSQRPresenceInfo(&result.m_RoomExtDataReceived, pso);
// 							manager->m_aFriendSearchResults.push_back(result);
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}
//
// 	if(friendIDs)
// 		delete friendIDs;
// 	return 0;
// }

// Get count of rooms that friends are playing in. Only valid when FriendRoomManagerIsBusy() returns false
int	SQRNetworkManager_AdHoc_Vita::FriendRoomManagerGetCount()
{
	assert( m_friendSearchState == SNM_FRIEND_SEARCH_STATE_IDLE );
	return m_aFriendSearchResults.size();
}

// Get details of a found session that a friend is playing in. 0 < idx < FriendRoomManagerGetCount(). Only valid when FriendRoomManagerIsBusy() returns false
void SQRNetworkManager_AdHoc_Vita::FriendRoomManagerGetRoomInfo(int idx, SQRNetworkManager_AdHoc_Vita::SessionSearchResult *searchResult)
{
	assert( idx < m_aFriendSearchResults.size() );
	assert( m_friendSearchState == SNM_FRIEND_SEARCH_STATE_IDLE );

	searchResult->m_NpId					= m_aFriendSearchResults[idx].m_NpId;

	ZeroMemory(&searchResult->m_sessionId, sizeof(SQRNetworkManager::SessionID));
 	searchResult->m_sessionId.m_RoomId		= m_aFriendSearchResults[idx].m_netAddr.s_addr;
 	//searchResult->m_sessionId.m_ServerId	= m_aFriendSearchResults[idx].m_ServerId;
	searchResult->m_netAddr					= m_aFriendSearchResults[idx].m_netAddr;
	searchResult->m_extData					= m_aFriendSearchResults[idx].m_RoomExtDataReceived;
}

// Get overall state of the network manager.
SQRNetworkManager_AdHoc_Vita::eSQRNetworkManagerState SQRNetworkManager_AdHoc_Vita::GetState()
{
	return m_stateExternal;;
}

bool SQRNetworkManager_AdHoc_Vita::IsHost()
{
	return m_isHosting;
}

bool SQRNetworkManager_AdHoc_Vita::IsReadyToPlayOrIdle()
{
	return (( m_state == SNM_INT_STATE_HOSTING_WAITING_TO_PLAY ) || ( m_state == SNM_INT_STATE_PLAYING ) || ( m_state == SNM_INT_STATE_IDLE ) );
}


// Consider as "in session" from the moment that a game is created or joined, until the point where the game itself has been told via state change that we are now idle. The
// game code requires IsInSession to return true as soon as it has asked to do one of these things (even if the state P hasn't really caught up with this request yet), and
// it also requires that it is informed of the state changes leading up to not being in the session, before this should report false.
bool SQRNetworkManager_AdHoc_Vita::IsInSession()
{
	return m_isInSession;
}

// Get count of players currently in the session
int	SQRNetworkManager_AdHoc_Vita::GetPlayerCount()
{
	return m_roomSyncData.getPlayerCount();
}

// Get count of players who are in the session, but not local to this machine
int SQRNetworkManager_AdHoc_Vita::GetOnlinePlayerCount()
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

SQRNetworkPlayer *SQRNetworkManager_AdHoc_Vita::GetPlayerByIndex(int idx)
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

SQRNetworkPlayer *SQRNetworkManager_AdHoc_Vita::GetPlayerBySmallId(int idx)
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

SQRNetworkPlayer *SQRNetworkManager_AdHoc_Vita::GetLocalPlayerByUserIndex(int idx)
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

SQRNetworkPlayer *SQRNetworkManager_AdHoc_Vita::GetHostPlayer()
{
	EnterCriticalSection(&m_csRoomSyncData);
	SQRNetworkPlayer *player = GetPlayerIfReady(m_aRoomSlotPlayers[0]);
	LeaveCriticalSection(&m_csRoomSyncData);
	return player;
}

SQRNetworkPlayer *SQRNetworkManager_AdHoc_Vita::GetPlayerIfReady(SQRNetworkPlayer *player)
{
	if( player == nullptr ) return nullptr;

	if( player->IsReady() ) return player;

	return nullptr;
}

// Update state internally
void SQRNetworkManager_AdHoc_Vita::SetState(SQRNetworkManager_AdHoc_Vita::eSQRNetworkManagerInternalState state)
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

void SQRNetworkManager_AdHoc_Vita::ResetToIdle()
{
	app.DebugPrintf("------------------ResetToIdle--------------------\n");
	// If we're the client, remove any networked players properly ( this will destory their rupd context etc.)
	if( !m_isHosting )
	{
		RemoveNetworkPlayers((1 << MAX_LOCAL_PLAYER_COUNT)-1);
	}
	else
	{
		// we don't get signalling back from the matching libs to destroy connections, so we need to kill everything here
		std::vector<SceNpMatching2RoomMemberId> memberIDs;
		for(int i = 0; i < m_roomSyncData.getPlayerCount(); i++ )
		{
			memberIDs.push_back(m_aRoomSlotPlayers[i]->m_roomMemberId);
		}
		for(size_t i=0;i<memberIDs.size();i++)
		{
			if(memberIDs[i] != m_hostMemberId)
				RemoveRemotePlayersAndSync(memberIDs[i], 15);
		}
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
	m_hostMemberId = 0;
	LeaveCriticalSection(&m_csRoomSyncData);
	SetState(SNM_INT_STATE_IDLE);
	if(sc_voiceChatEnabled)
 	{
		SonyVoiceChat_Vita::checkFinished();
	}
	StopMatchingContext();
}

// Join a room that was found with FriendRoomManagerSearch. 0 < idx < FriendRoomManagerGetCount(). Only valid when FriendRoomManagerIsBusy() returns false
bool SQRNetworkManager_AdHoc_Vita::JoinRoom(SQRNetworkManager_AdHoc_Vita::SessionSearchResult *searchResult, int localPlayerMask)
{
	// Set up the presence info we would like to synchronise out when we have fully joined the game
	// 	CPlatformNetworkManagerSony::SetSQRPresenceInfoFromExtData(&s_lastPresenceSyncInfo, searchResult->m_extData, searchResult->m_sessionId.m_RoomId, searchResult->m_sessionId.m_ServerId);
	return JoinRoom(searchResult->m_netAddr, localPlayerMask, nullptr);
}

bool SQRNetworkManager_AdHoc_Vita::JoinRoom(SceNpMatching2RoomId roomId, SceNpMatching2ServerId serverId, int localPlayerMask, const PresenceSyncInfo *presence)
{
	assert(0);//  only here to match the parent class interface
	return false;
}


// Join room with a specified roomId. This is used when joining from an invite, as well as by the previous method
bool SQRNetworkManager_AdHoc_Vita::JoinRoom(SceNetInAddr netAddr, int localPlayerMask, const HelloSyncInfo *presence)
{
	// The presence info will be directly passed in if we are joining from an invite, otherwise it has already been set up. This is synchronised out when we have fully joined the game.
	if( presence )
	{
		memcpy( &s_lastPresenceSyncInfo, presence, sizeof(HelloSyncInfo) );
	}
	else
	{
		for(size_t i=0;i<m_aFriendSearchResults.size();i++)
		{
			if(m_aFriendSearchResults[i].m_netAddr.s_addr == netAddr.s_addr)
			{
				memcpy(&m_roomSyncData, &m_aFriendSearchResults[i].m_roomSyncData, sizeof(m_roomSyncData));
			}
		}
	}

	m_isInSession = true;

	m_isHosting = false;
	m_offlineGame = false;
// 	m_roomToJoin = roomId;
	m_localPlayerJoinMask = localPlayerMask;
	m_localPlayerCount = 0;
	m_localPlayerJoined = 0;

	for( int i = 0; i < MAX_LOCAL_PLAYER_COUNT; i++ )
	{
		if( m_localPlayerJoinMask & ( 1 << i ) ) m_localPlayerCount++;
	}
	if(!CreateMatchingContext())
		return false;

	int err = sceNetAdhocMatchingSelectTarget(m_matchingContext, &netAddr, 0, nullptr);
	m_hostMemberId = getRoomMemberID(&netAddr);
	m_hostIPAddr = netAddr;


	assert(err == SCE_OK);
	return (err == SCE_OK); //GetServerContext( serverId );
}

void SQRNetworkManager_AdHoc_Vita::StartGame()
{
	assert( ( m_state == SNM_INT_STATE_HOSTING_WAITING_TO_PLAY ) || (( m_state == SNM_INT_STATE_IDLE ) && m_offlineSQR) );

	SetState( SNM_INT_STATE_STARTING);
	SetState( SNM_INT_STATE_PLAYING);
}

void SQRNetworkManager_AdHoc_Vita::LeaveRoom(bool bActuallyLeaveRoom)
{
	if( m_offlineGame )
	{
		if( m_state != SNM_INT_STATE_PLAYING ) return;

		SetState(SNM_INT_STATE_LEAVING);
		SetState(SNM_INT_STATE_ENDING);
		ResetToIdle();
		return;
	}

	UpdateRichPresenceCustomData(& c_presenceSyncInfoNULL, sizeof(HelloSyncInfo) );

	// 	SonyVoiceChat::shutdown();

	// Attempt to leave the room if we are in any of the states we could be in if we have successfully created it
	if( bActuallyLeaveRoom )
	{
		if( ( m_state == SNM_INT_STATE_HOSTING_CREATE_ROOM_SUCCESS ) ||
			( m_state == SNM_INT_STATE_HOSTING_WAITING_TO_PLAY ) ||
			( m_state == SNM_INT_STATE_JOINING_WAITING_FOR_LOCAL_PLAYERS ) ||
			( m_state == SNM_INT_STATE_PLAYING ) )
		{
// 			SceNpMatching2LeaveRoomRequest reqParam;
// 			memset( &reqParam, 0, sizeof(reqParam) );
// 			reqParam.roomId = m_room;
			if(!m_isHosting)
			{
				int ret = sceNetAdhocMatchingCancelTarget(m_matchingContext, &m_hostIPAddr);
				if (ret < 0)
				{
					app.DebugPrintf("sceNetAdhocMatchingCancelTarget error :[%d] [%x]\n",ret,ret) ;
					assert(0);
				}
			}


			SetState(SNM_INT_STATE_LEAVING);

// 			if( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_LEAVE_ROOM) )
// 			{
// 				SetState(SNM_INT_STATE_LEAVING_FAILED);
// 			}
// 			else
			{
				DeleteServerContext();
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

void SQRNetworkManager_AdHoc_Vita::EndGame()
{
}

bool SQRNetworkManager_AdHoc_Vita::SessionHasSpace(int spaceRequired)
{
	return( ( m_roomSyncData.getPlayerCount() + spaceRequired ) <= MAX_ONLINE_PLAYER_COUNT );
}

bool SQRNetworkManager_AdHoc_Vita::AddLocalPlayerByUserIndex(int idx)
{
	assert(0); // we should never be adding a local player on Vita, as there is only ever 1 local player
	return false;
}

bool SQRNetworkManager_AdHoc_Vita::RemoveLocalPlayerByUserIndex(int idx)
{
	assert(0);// we should never be removing a local player on Vita, as there is only ever 1 local player
	return false;
}


extern uint8_t *mallocAndCreateUTF8ArrayFromString(int iID);

// Bring up a Gui to send an invite so a player that the user can select. This invite will contain the room Id so that
void SQRNetworkManager_AdHoc_Vita::SendInviteGUI()
{
	PSVITA_STUBBED;
}


void SQRNetworkManager_AdHoc_Vita::RecvInviteGUI()
{
	PSVITA_STUBBED;
}


void SQRNetworkManager_AdHoc_Vita::TickInviteGUI()
{
	PSVITA_STUBBED;
}

// Get the data for an invite into a statically allocated array of invites, and pass a pointer of this back up to the game. Elements in the array are used in a circular fashion, to save any issues with handling freeing of this invite data as the
// qnet equivalent of this seems to just assume that the data persists forever.
void SQRNetworkManager_AdHoc_Vita::GetInviteDataAndProcess(sce::Toolkit::NP::MessageAttachment* pInvite)
{
	PSVITA_STUBBED;
}

bool SQRNetworkManager_AdHoc_Vita::UpdateInviteData(HelloSyncInfo *invite)
{
	PSVITA_STUBBED;
	return false;
}

// This method is a helper used in MapRoomSlotPlayers - tries to find a player that matches:
// (1) the playerType
// (2) if playerType is remote, memberId
// (3) localPlayerIdx
// The reason we don't care about memberid when the player isn't remote is that it doesn't matter (since we know the player is either on this machine, or it is the host and there's only one of those),
// and there's a period when starting up the host game where it doesn't accurately know the memberId for its own local players
void SQRNetworkManager_AdHoc_Vita::FindOrCreateNonNetworkPlayer(int slot, int playerType, SceNpMatching2RoomMemberId memberId, int localPlayerIdx, int smallId)
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
void	SQRNetworkManager_AdHoc_Vita::LocalDataSend(SQRNetworkPlayer *playerFrom, SQRNetworkPlayer *playerTo, const void *data, unsigned int dataSize)
{
	assert(m_isHosting);
	if(m_listener)
	{
		m_listener->HandleDataReceived( playerFrom, playerTo, (unsigned char *)data, dataSize );
	}
}

int SQRNetworkManager_AdHoc_Vita::GetSessionIndex(SQRNetworkPlayer *player)
{
	int roomSlotPlayerCount = m_roomSyncData.getPlayerCount();
	for( int i = 0; i < roomSlotPlayerCount; i++  )
	{
		if( m_aRoomSlotPlayers[i] == player ) return i;
	}
	return 0;
}

// Updates m_aRoomSlotPlayers, based on what is in m_roomSyncData. This needs to be updated when room members join & leave, and when any SQRNetworkPlayer is created externally that this should be mapping to
void SQRNetworkManager_AdHoc_Vita::MapRoomSlotPlayers(int roomSlotPlayerCount/*=-1*/)
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
void SQRNetworkManager_AdHoc_Vita::UpdateRoomSyncUIDsFromPlayers()
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
void SQRNetworkManager_AdHoc_Vita::UpdatePlayersFromRoomSyncUIDs()
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
bool SQRNetworkManager_AdHoc_Vita::AddRemotePlayersAndSync( SceNpMatching2RoomMemberId memberId, int playerMask, bool *isFull/*==nullptr*/ )
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
void SQRNetworkManager_AdHoc_Vita::RemoveRemotePlayersAndSync( SceNpMatching2RoomMemberId memberId, int mask )
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
void SQRNetworkManager_AdHoc_Vita::RemoveNetworkPlayers( int mask )
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

}

// Host only - update the memberId of the local players, and synchronise with other room members
void SQRNetworkManager_AdHoc_Vita::SetLocalPlayersAndSync()
{
	assert( m_isHosting );

	// Update local IP address
 	UpdateLocalIPAddress();

	m_localMemberId = getRoomMemberID(&m_localIPAddr);
	if(IsHost())
		m_hostMemberId = m_localMemberId;


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
void SQRNetworkManager_AdHoc_Vita::SyncRoomData()
{
	if( m_offlineGame ) return;

	UpdateRoomSyncUIDsFromPlayers();

	// send the room data packet out to all the clients
    for (auto iter = m_RudpCtxToPlayerMap.begin(); iter != m_RudpCtxToPlayerMap.end(); ++iter)
    {
		int ctx =  iter->first;
		SQRNetworkPlayer* pPlayer = GetPlayerFromRudpCtx(ctx);
		assert(pPlayer);
		sendDataPacket(*GetIPAddrFromRudpCtx(ctx), e_dataTag_RoomSync, &m_roomSyncData, sizeof(m_roomSyncData));
	}
}



void SQRNetworkManager_AdHoc_Vita::MatchingEventHandler(int id, int event, SceNetInAddr*	peer, int optlen, void *opt)
{
	SQRNetworkManager_AdHoc_Vita* manager = s_pAdhocVitaManager;

	app.DebugPrintf("MatchingEventHandler_Server : ev : %d, ip addr : %s\n", event, getIPAddressString(*peer).c_str());
	int ret;

	switch (event)
	{
	case SCE_NET_ADHOC_MATCHING_EVENT_HELLO:
		app.DebugPrintf("P2P SCE_NET_ADHOC_MATCHING_EVENT_HELLO Received!!\n");

		if(manager->m_isHosting)
		{
			assert(0); // the host should never see the hello message
		}
		else if(optlen > 0)
		{
			if(optlen == sizeof(HelloSyncInfo))
			{
				FriendSearchResult result;
	// 			memcpy(&result.m_NpId, &friendIDs[i], sizeof(SceNpId));
				result.m_RoomFound = false;

				HelloSyncInfo *pso = (HelloSyncInfo *)opt;
				if( pso->m_presenceSyncInfo.netVersion == MINECRAFT_NET_VERSION )
				{
					if( !pso->m_presenceSyncInfo.inviteOnly )
					{
						result.m_netAddr = *peer;
						result.m_RoomFound = true;
						memcpy(result.m_NpId.handle.data, pso->m_presenceSyncInfo.hostPlayerUID.getOnlineID(), SCE_NP_ONLINEID_MAX_LENGTH);
	// 					result.m_RoomId = pso->m_RoomId;
	// 					result.m_ServerId = pso->m_ServerId;

						CPlatformNetworkManagerSony::MallocAndSetExtDataFromSQRPresenceInfo(&result.m_RoomExtDataReceived, &pso->m_presenceSyncInfo);
						result.m_gameSessionData = malloc(sizeof(GameSessionData));
						memcpy(result.m_gameSessionData, &pso->m_gameSessionData, sizeof(GameSessionData));
						memcpy(&result.m_roomSyncData, &pso->m_roomSyncData, sizeof(RoomSyncData));
						// check we don't have this already
						int currIndex = -1;
						bool bChanged = false;
						for(size_t i=0; i<manager->m_aFriendSearchResults.size(); i++)
						{
							if(manager->m_aFriendSearchResults[i].m_netAddr.s_addr == peer->s_addr)
							{
								currIndex = i;
								if(memcmp(result.m_gameSessionData, manager->m_aFriendSearchResults[i].m_gameSessionData, sizeof(GameSessionData)) != 0)
									bChanged = true;
								if(memcmp(&result.m_roomSyncData, &manager->m_aFriendSearchResults[i].m_roomSyncData, sizeof(RoomSyncData)) != 0)
									bChanged = true;
								if(memcmp(&result.m_roomSyncData, &manager->m_aFriendSearchResults[i].m_roomSyncData, sizeof(RoomSyncData)) != 0)
									bChanged = true;
								break;
							}
						}
						if(currIndex>=0 && bChanged)
							manager->m_aFriendSearchResults.erase(manager->m_aFriendSearchResults.begin() + currIndex);
						if(currIndex<0 || bChanged)
							manager->m_aFriendSearchResults.push_back(result);
						app.DebugPrintf("m_aFriendSearchResults playerCount : %d\n", result.m_roomSyncData.players[0].m_playerCount);
					}
				}
				else
				{
					app.DebugPrintf("mismatching net version, expected %d, but got %d !!\n", MINECRAFT_NET_VERSION, pso->m_presenceSyncInfo.netVersion);
				}
			}
			else
			{
				app.DebugPrintf("Wrong size for HelloSyncInfo, should be %d bytes, but was %d bytes!!\n", sizeof(HelloSyncInfo), optlen);
				assert(0);
			}
		}
		break;

	case SCE_NET_ADHOC_MATCHING_EVENT_REQUEST:		// A join request was received
		app.DebugPrintf("P2P SCE_NET_ADHOC_MATCHING_EVENT_REQUEST Received!!\n");
		if (optlen > 0 && opt != nullptr)
		{
			ret = SCE_OK;// parentRequestAdd(opt);
			if (ret != SCE_OK)
			{
				ret = sceNetAdhocMatchingCancelTarget(manager->m_matchingContext, peer);
				if (ret < 0)
				{
					app.DebugPrintf("sceNetAdhocMatchingCancelTarget error :[%d] [%x]\n",ret,ret) ;
					assert(0);
					break;
				}
			}
		}
		// accept the join request
		sceNetAdhocMatchingSelectTarget(manager->m_matchingContext, peer, sizeof(manager->m_roomSyncData), &manager->m_roomSyncData);
		break;


	case SCE_NET_ADHOC_MATCHING_EVENT_ACCEPT:		// The join request was accepted
		app.DebugPrintf("P2P SCE_NET_ADHOC_MATCHING_EVENT_ACCEPT Received!!\n");
		if( manager->m_isHosting == false )
		{
			assert(opt && optlen == sizeof(m_roomSyncData));
			memcpy(&manager->m_roomSyncData, opt, sizeof(manager->m_roomSyncData));
		}
		break;

	case SCE_NET_ADHOC_MATCHING_EVENT_ESTABLISHED:		// A participation agreement was established
		{
			app.DebugPrintf("P2P SCE_NET_ADHOC_MATCHING_EVENT_ESTABLISHED Received!!\n");


			SceNetInAddr localAddr;
			int ret = sceNetCtlAdhocGetInAddr(&localAddr);
			assert(ret == SCE_OK);
			manager->m_localMemberId = getRoomMemberID(&localAddr);

			if( manager->m_isHosting )
			{
				manager->m_hostMemberId = manager->m_localMemberId;

				bool isFull = false;
				bool success = manager->AddRemotePlayersAndSync( getRoomMemberID(peer), 1, &isFull );
				if( success )
				{
					bool success2 =	manager->CreateRudpConnections(*peer);
					if(success2)
					{
						manager->SyncRoomData(); // have to syn the room data again now we have the peer in our list
						break;
					}
				}
				// Something has gone wrong adding these players to the room - kick out the player
				assert(0);
			}
			else
			{
				// Local players can establish their UID at this point
				PlayerUID localUID;
				ProfileManager.GetXUID(0,&localUID,true);
				localUID.setForAdhoc();

				bool success =	manager->CreateRudpConnections(*peer);
				manager->SetState(SNM_INT_STATE_JOINING_WAITING_FOR_LOCAL_PLAYERS);
			}

	// 		// If we've created a player, then we want to try and patch up any connections that we should have to it
	// 		manager->MapRoomSlotPlayers();
	//
	// 		SQRNetworkPlayer *player = manager->GetPlayerFromRudpCtx(peer->s_addr);
	// 		if( player )
	// 		{
	// 			// Flag connection stage as being completed for this player
	// 			manager->NetworkPlayerConnectionComplete(player);
	// 		}
		}

		break;


	case SCE_NET_ADHOC_MATCHING_EVENT_DENY:
	case SCE_NET_ADHOC_MATCHING_EVENT_LEAVE:		// The participation agreement was canceled by the target player
	case SCE_NET_ADHOC_MATCHING_EVENT_CANCEL:		// The join request was canceled by the client
	case SCE_NET_ADHOC_MATCHING_EVENT_ERROR:		//	A protocol error occurred
	case SCE_NET_ADHOC_MATCHING_EVENT_TIMEOUT:		// The participation agreement was canceled because of a Keep Alive timeout
	case SCE_NET_ADHOC_MATCHING_EVENT_DATA_TIMEOUT:
		if(event == SCE_NET_ADHOC_MATCHING_EVENT_DENY)
			app.SetDisconnectReason(DisconnectPacket::eDisconnect_ServerFull);
		else
			app.SetDisconnectReason(DisconnectPacket::eDisconnect_TimeOut);
		ret = sceNetAdhocMatchingCancelTarget(manager->m_matchingContext, peer);
		if ( ret < 0 )
		{
			app.DebugPrintf("sceNetAdhocMatchingCancelTarget error :[%d] [%x]\n",ret,ret) ;
		}
		// ->
		// no break, carry on through to the BYE case ->
		// ->
	case SCE_NET_ADHOC_MATCHING_EVENT_BYE:	// Target participating player has stopped matching
		{
			app.DebugPrintf("P2P SCE_NET_ADHOC_MATCHING_EVENT_BYE Received!!\n");

			if(event == SCE_NET_ADHOC_MATCHING_EVENT_BYE && ! manager->IsInSession()) //
			{
				// the BYE event comes through like the HELLO event, so even even if we're not connected
				// so make sure we're actually in session
				break;
			}
			SceNpMatching2RoomMemberId peerMemberId = getRoomMemberID(peer);
			if( manager->m_isHosting )
			{
				// Remove any players associated with this peer
				manager->RemoveRemotePlayersAndSync( peerMemberId, 15 );
			}
			else if(peerMemberId == manager->m_hostMemberId)
			{
				// Host has left the game... so its all over for this client too. Finish everything up now, including deleting the server context which belongs to this gaming session
				// This also might be a response to a request to leave the game from our end too so don't need to do anything in that case
				if( manager->m_state != SNM_INT_STATE_LEAVING )
				{
					manager->DeleteServerContext();
				}
			}
			else
			{
				if(sc_voiceChatEnabled)
				{
					// we've lost connection to another client (voice only) so kill the voice connection
					// no players left on the remote machine once we remove this one
					SQRVoiceConnection* pVoice = SonyVoiceChat_Vita::getVoiceConnectionFromRoomMemberID(peerMemberId);
					assert(pVoice);
					if(pVoice)
						SonyVoiceChat_Vita::disconnectRemoteConnection(pVoice);
				}
			}
		}

		break;

	case SCE_NET_ADHOC_MATCHING_EVENT_DATA:
		{
			app.DebugPrintf("P2P SCE_NET_ADHOC_MATCHING_EVENT_DATA Received!!\n");

			if (optlen <= 0 || opt == nullptr)
			{
				assert(0);
				break;
			}
			AdhocDataPacket* pPacket = (AdhocDataPacket*)opt;
			unsigned int dataSize = optlen - 4;

			if(pPacket->m_tag == e_dataTag_RoomSync)
			{
				// room sync data, copy it over
				EnterCriticalSection(&manager->m_csRoomSyncData);
				assert(dataSize == sizeof(manager->m_roomSyncData));
				memcpy(&manager->m_roomSyncData, pPacket->m_pData, dataSize);
				LeaveCriticalSection(&manager->m_csRoomSyncData);
				manager->MapRoomSlotPlayers();
			}
			else
			{
				assert(0);
			}
		}
		break ;

	case SCE_NET_ADHOC_MATCHING_EVENT_DATA_ACK:
		{
			SQRNetworkPlayer *player = manager->GetPlayerFromRudpCtx(peer->s_addr);
			// This event signifies that the previous packet has been sent, so we can try to send some more
			if( player )
			{
				player->SendMoreInternal();
			}
		}
		break;


	default:
		break;
	}
}


// Tick the process of creating a room.
void SQRNetworkManager_AdHoc_Vita::RoomCreateTick()
{
	switch( m_state )
	{
	case SNM_INT_STATE_HOSTING_CREATE_ROOM_SEARCHING_FOR_WORLD:
		break;
	case SNM_INT_STATE_HOSTING_CREATE_ROOM_WORLD_FOUND:
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
void SQRNetworkManager_AdHoc_Vita::NetworkPlayerConnectionComplete(SQRNetworkPlayer *player)
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
void SQRNetworkManager_AdHoc_Vita::NetworkPlayerSmallIdAllocated(SQRNetworkPlayer *player, unsigned char smallId)
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
void SQRNetworkManager_AdHoc_Vita::NetworkPlayerInitialDataReceived(SQRNetworkPlayer *player, void *data)
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
void SQRNetworkManager_AdHoc_Vita::NonNetworkPlayerComplete(SQRNetworkPlayer *player, unsigned char smallId)
{
	player->ConnectionComplete();
	player->SmallIdAllocated(smallId);
}

void SQRNetworkManager_AdHoc_Vita::HandlePlayerJoined(SQRNetworkPlayer *player)
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
bool SQRNetworkManager_AdHoc_Vita::SelectRandomServer()
{
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
void SQRNetworkManager_AdHoc_Vita::DeleteServerContext()
{
	// No server context on PS4, so we just set the state, and then we'll check all the UDP connections have shutdown before setting to idle
	if(m_serverContextValid)
	{
		m_serverContextValid = false;
	}
	SetState(SNM_INT_STATE_SERVER_DELETING_CONTEXT);
}

// Creates a set of Rudp connections by the "active open" method. This requires that both ends of the connection call cellRudpInitiate to fully create a connection. We
// create one connection per local play on any remote machine.
//
// peerMemberId is the room member Id of the remote end of the connection
// playersMemberId is the room member Id that the players belong to
// ie for the host (when matching incoming connections), these will be the same thing... and for the client, peerMemberId will be the host, whereas playersMemberId will be itself




bool SQRNetworkManager_AdHoc_Vita::CreateVoiceRudpConnections(SceNpMatching2RoomId roomId, SceNpMatching2RoomMemberId peerMemberId, int playerMask)
{
	PSVITA_STUBBED;
	return true;
}

bool SQRNetworkManager_AdHoc_Vita::CreateSocket()
{
	int ret;
	// First get details of the UDPP2P connection that has been established
	// 	int connStatus;
	SceNetSockaddrIn sinp2pLocal;//, sinp2pPeer;

	// Local end first...
	memset(&sinp2pLocal, 0, sizeof(sinp2pLocal));

	// Update local IP address
 	UpdateLocalIPAddress();

	sinp2pLocal.sin_len =  sizeof(sinp2pLocal);
	sinp2pLocal.sin_family = SCE_NET_AF_INET;
	sinp2pLocal.sin_port = sceNetHtons(SCE_NP_PORT);
	sinp2pLocal.sin_vport  = sceNetHtons(ADHOC_VPORT) ;
 	sinp2pLocal.sin_addr = m_localIPAddr;

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


bool SQRNetworkManager_AdHoc_Vita::CreateRudpConnections(SceNetInAddr peer)
{
	// First get details of the UDPP2P connection that has been established
	SceNetSockaddrIn sinp2pPeer;

	// get the peer
	memset(&sinp2pPeer, 0, sizeof(sinp2pPeer));
	sinp2pPeer.sin_len =  sizeof(sinp2pPeer);
	sinp2pPeer.sin_family = SCE_NET_AF_INET;
	sinp2pPeer.sin_addr	 = peer;
	sinp2pPeer.sin_port = sceNetHtons(SCE_NP_PORT);

	// Set vport
	sinp2pPeer.sin_vport = sceNetHtons(ADHOC_VPORT);

	// Create socket & bind, if we don't already have one
	if( m_soc == -1 )
	{
		if(CreateSocket() == false)
			return false;
	}

	// Create an Rudp context for each local player that is required. These can be used as individual virtual connections between room members (ie consoles), which are multiplexed
	// over the socket we have just made

	int rudpCtx;

	// Socket for the local network node created, now can create an Rupd context.
	int ret = sceRudpCreateContext( RudpContextCallback, this, &rudpCtx );
	if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_CREATE_RUDP_CONTEXT) ) return false;
	if( m_isHosting )
	{
		m_RudpCtxToPlayerMap[ rudpCtx ] = new SQRNetworkPlayer( this, SQRNetworkPlayer::SNP_TYPE_REMOTE, true, getRoomMemberID((&peer)), 0, rudpCtx, nullptr );
		m_RudpCtxToIPAddrMap[ rudpCtx ] = peer;
	}
	else
	{
		// Local players can establish their UID at this point
		PlayerUID localUID;
		ProfileManager.GetXUID(0,&localUID,true);
		localUID.setForAdhoc();

		m_RudpCtxToPlayerMap[ rudpCtx ] = new SQRNetworkPlayer( this, SQRNetworkPlayer::SNP_TYPE_LOCAL, false, m_localMemberId, 0, rudpCtx, &localUID );
		m_RudpCtxToIPAddrMap[ rudpCtx ] = m_localIPAddr;
}

	// If we've created a player, then we want to try and patch up any connections that we should have to it
	MapRoomSlotPlayers();

	// TODO - set any non-default options for the context. By default, the context is set to have delivery critical and order critical both on

	// Bind the context to the socket we've just created, and initiate. The initiation needs to happen on both client & host sides of the connection to complete.
	ret = sceRudpBind( rudpCtx, m_soc , 1 + 0, SCE_RUDP_MUXMODE_P2P );
	if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_RUDP_BIND) ) return false;
	if(ret < 0)
		app.DebugPrintf(" sceRudpBind failed with error 0x%08x\n");

	ret = sceRudpInitiate( rudpCtx, &sinp2pPeer, sizeof(sinp2pPeer), 0);
	if(ret < 0)
		app.DebugPrintf(" sceRudpInitiate failed with error 0x%08x\n");
	if ( ( ret < 0 ) || ForceErrorPoint(SNM_FORCE_ERROR_RUDP_INIT2) ) return false;

	return true;
}


SQRNetworkPlayer *SQRNetworkManager_AdHoc_Vita::GetPlayerFromRudpCtx(int rudpCtx)
{
    auto it = m_RudpCtxToPlayerMap.find(rudpCtx);
    if( it != m_RudpCtxToPlayerMap.end() )
	{
		return it->second;
	}
	return nullptr;
}


SceNetInAddr* SQRNetworkManager_AdHoc_Vita::GetIPAddrFromRudpCtx(int rudpCtx)
{
    auto it = m_RudpCtxToIPAddrMap.find(rudpCtx);
    if( it != m_RudpCtxToIPAddrMap.end() )
	{
		return &it->second;
	}
	return nullptr;
}



SQRNetworkPlayer *SQRNetworkManager_AdHoc_Vita::GetPlayerFromRoomMemberAndLocalIdx(int roomMember, int localIdx)
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
bool SQRNetworkManager_AdHoc_Vita::RegisterCallbacks()
{
	// Register RUDP event handler
	int ret = sceRudpSetEventHandler(RudpEventCallback, this);
	if (ret < 0)
	{
		app.DebugPrintf("SQRNetworkManager::RegisterCallbacks - cellRudpSetEventHandler failed with code 0x%08x\n", ret);
		return false;
	}

	// Register the context callback function
// 	ret = sceNpMatching2RegisterContextCallback(ContextCallback, this);
// 	if (ret < 0)
// 	{
// 		app.DebugPrintf("SQRNetworkManager::RegisterCallbacks - sceNpMatching2RegisterContextCallback failed with code 0x%08x\n", ret);
// 		return false;
// 	}
//
// 	// Register the default request callback & parameters
// 	SceNpMatching2RequestOptParam optParam;
//
// 	memset(&optParam, 0, sizeof(optParam));
// 	optParam.cbFunc = DefaultRequestCallback;
// 	optParam.cbFuncArg = this;
// 	optParam.timeout = (30 * 1000 * 1000);
// 	optParam.appReqId = 0;
//
// 	ret = sceNpMatching2SetDefaultRequestOptParam(m_matchingContext, &optParam);
// 	if (ret < 0)
// 	{
// 		app.DebugPrintf("SQRNetworkManager::RegisterCallbacks - sceNpMatching2SetDefaultRequestOptParam failed with code 0x%08x\n", ret);
// 		return false;
// 	}
//
// 	// Register signalling callback
// 	ret = sceNpMatching2RegisterSignalingCallback(m_matchingContext, SignallingCallback, this);
// 	if (ret < 0)
// 	{
// 		return false;
// 	}
//
// 	// Register room event callback
// 	ret = sceNpMatching2RegisterRoomEventCallback(m_matchingContext, RoomEventCallback, this);
// 	if (ret < 0)
// 	{
// 		app.DebugPrintf("SQRNetworkManager::RegisterCallbacks - sceNpMatching2RegisterRoomEventCallback failed with code 0x%08x\n", ret);
// 		return false;
// 	}
//
	return true;
}

extern bool g_bBootedFromInvite;



void SQRNetworkManager_AdHoc_Vita::HandleMatchingContextStart()
{
	// on the standard networking model this happens during a callback signalled from the context starting
	// Some special cases to detect when this event is coming in, in case we had to start the matching context because there wasn't a valid context when we went to get a server context. These two
	// responses here complete what should then happen to get the server context in each case (for hosting or joining a game)
	if( m_state == SNM_INT_STATE_IDLE_RECREATING_MATCHING_CONTEXT )
	{
		SetState( SNM_INT_STATE_IDLE );
		GetExtDataForRoom(0, nullptr, nullptr, nullptr);
	}
	else if( m_state == SNM_INT_STATE_HOSTING_STARTING_MATCHING_CONTEXT )
	{
		// no world matching stuff to setup here, we can just signal that we're ready
		SetState(SNM_INT_STATE_HOSTING_WAITING_TO_PLAY);
		// Now we know the local member id we can update our local players
		SetLocalPlayersAndSync();
//		GetServerContext2();
	}
	else if( m_state == SNM_INT_STATE_JOINING_STARTING_MATCHING_CONTEXT )
	{
		SetState(SNM_INT_STATE_JOINING_SEARCHING_FOR_SERVER);
		SelectRandomServer();
	}
	else
	{
		// Normal handling of context starting, from standard initialisation procedure
		assert( m_state == SNM_INT_STATE_STARTING_CONTEXT );
		m_offlineSQR = false;
		SetState(SNM_INT_STATE_IDLE);

		if(s_SignInCompleteCallbackFn)
		{
			s_SignInCompleteCallbackFn(s_SignInCompleteParam, true, 0);
			s_SignInCompleteCallbackFn = nullptr;
		}
	}
}





// Implementation of SceNpBasicEventHandler
int SQRNetworkManager_AdHoc_Vita::BasicEventCallback(int event, int retCode, uint32_t reqId, void *arg)
{
	PSVITA_STUBBED;
	// 	SQRNetworkManager_AdHoc_Vita *manager = (SQRNetworkManager_AdHoc_Vita *)arg;
	// 	// We aren't allowed to actually get the event directly from this callback, so send our own internal event to a thread dedicated to doing this
	// 	sceKernelTriggerUserEvent(m_basicEventQueue, sc_UserEventHandle, nullptr);

	return 0;
}

// Implementation of SceNpManagerCallback
void SQRNetworkManager_AdHoc_Vita::OnlineCheck()
{
	int state;
	int ret = sceNetCtlAdhocGetState(&state);


	bool bConnected = (state == SCE_NET_CTL_STATE_IPOBTAINED); //ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad());
	bool oldAdhocStatus = GetAdhocStatus();
	UpdateAdhocStatus(bConnected);
 	if(oldAdhocStatus == false)
 	{
 		if(bConnected)
		{
 			InitialiseAfterOnline();
		}
 	}
}

// Implementation of CellSysutilCallback
void SQRNetworkManager_AdHoc_Vita::SysUtilCallback(uint64_t status, uint64_t param, void *userdata)
{
	// 	SQRNetworkManager_AdHoc_Vita *manager = (SQRNetworkManager_AdHoc_Vita *)userdata;
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

//CD - Added this to match the SQRNetworkManager_Vita class
void SQRNetworkManager_AdHoc_Vita::updateNetCheckDialog()
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

			app.DebugPrintf("------------>>>>>>>>   sceNetCheckDialog finished\n");

			if( netCheckResult.result == SCE_COMMON_DIALOG_RESULT_OK )
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
				if(s_attemptSignInAdhoc)		// don't fail if it was an attempted PSN signin
				{
					SetState(SNM_INT_STATE_INITIALISE_FAILED);
				}
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
void SQRNetworkManager_AdHoc_Vita::RudpContextCallback(int ctx_id, int event_id, int error_code, void *arg)
{
	SQRNetworkManager_AdHoc_Vita *manager = (SQRNetworkManager_AdHoc_Vita *)arg;
	switch(event_id)
	{
	case SCE_RUDP_CONTEXT_EVENT_CLOSED:
		{
			SQRVoiceConnection* pVoice = nullptr;
			if(sc_voiceChatEnabled)
				SonyVoiceChat_Vita::GetVoiceConnectionFromRudpCtx(ctx_id);

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
				if(sc_voiceChatEnabled)
				{
					SonyVoiceChat_Vita::setConnected(ctx_id);
				}
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
				if(sc_voiceChatEnabled)
				{
					SQRVoiceConnection* pVoice = SonyVoiceChat_Vita::GetVoiceConnectionFromRudpCtx(ctx_id);
					assert(pVoice);
				}
			}
		}
		break;
	case SCE_RUDP_CONTEXT_EVENT_READABLE:
		if( manager->m_listener )
		{
			SQRVoiceConnection* pVoice = nullptr;
			if(sc_voiceChatEnabled)
			{
				SonyVoiceChat_Vita::GetVoiceConnectionFromRudpCtx(ctx_id);
			}

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

int SQRNetworkManager_AdHoc_Vita::RudpEventCallback(int event_id, int soc, uint8_t const *data, size_t datalen, struct SceNetSockaddr  const *addr, SceNetSocklen_t addrlen, void *arg)
{
	SQRNetworkManager_AdHoc_Vita *manager = (SQRNetworkManager_AdHoc_Vita *)arg;
	if( event_id == SCE_RUDP_EVENT_SOCKET_RELEASED )
	{
		assert( soc == manager->m_soc );
		sceNetSocketClose(soc);
		manager->m_soc = -1;
	}
	return 0;
}

void SQRNetworkManager_AdHoc_Vita::NetCtlCallback(int eventType, void *arg)
{
	SQRNetworkManager_AdHoc_Vita *manager = (SQRNetworkManager_AdHoc_Vita *)arg;
	// Oddly, the disconnect event comes in with a new state of "CELL_NET_CTL_STATE_Connecting"... looks like the event is more important than the state to
	// determine what has just happened
	switch(eventType)
	{
	case SCE_NET_CTL_EVENT_TYPE_DISCONNECTED:
	case SCE_NET_CTL_EVENT_TYPE_DISCONNECT_REQ_FINISHED:
		manager->m_bLinkDisconnected = true;
//		manager->m_listener->HandleDisconnect(true, true);
		break;
	case SCE_NET_CTL_EVENT_TYPE_IPOBTAINED:
		manager->m_bLinkDisconnected = false;
		break;
	default:
		assert(0);
		break;
	}
}

// Called when the context has been created, and we are intending to create a room.
void SQRNetworkManager_AdHoc_Vita::ServerContextValid_CreateRoom()
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
		ret = sceNpMatching2GetWorldInfoList( m_matchingContext, &reqParam, nullptr, &m_getWorldRequestId);
	}
	if (ret < 0)
	{
		SetState(SNM_INT_STATE_HOSTING_CREATE_ROOM_FAILED);
		return;
	}
}

// Called when the context has been created, and we are intending to join a pre-existing room.
void SQRNetworkManager_AdHoc_Vita::ServerContextValid_JoinRoom()
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

const SceNpCommunicationId* SQRNetworkManager_AdHoc_Vita::GetSceNpCommsId()
{
	return &s_npCommunicationId;
}

const SceNpCommunicationSignature* SQRNetworkManager_AdHoc_Vita::GetSceNpCommsSig()
{
	return &s_npCommunicationSignature;
}

const SceNpTitleId* SQRNetworkManager_AdHoc_Vita::GetSceNpTitleId()
{
	PSVITA_STUBBED;
	return nullptr;
// 	return &s_npTitleId;
}

const SceNpTitleSecret* SQRNetworkManager_AdHoc_Vita::GetSceNpTitleSecret()
{
	PSVITA_STUBBED;
	return nullptr;
//	return &s_npTitleSecret;
}

int	SQRNetworkManager_AdHoc_Vita::GetOldMask(SceNpMatching2RoomMemberId memberId)
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

int	SQRNetworkManager_AdHoc_Vita::GetAddedMask(int newMask, int oldMask)
{
	return newMask & ~oldMask;
}

int	SQRNetworkManager_AdHoc_Vita::GetRemovedMask(int newMask, int oldMask)
{
	return oldMask & ~newMask;
}


void SQRNetworkManager_AdHoc_Vita::GetExtDataForRoom( SceNpMatching2RoomId roomId, void *extData, void (* FriendSessionUpdatedFn)(bool success, void *pParam), void *pParam )
{

	for(size_t i=0;i<m_aFriendSearchResults.size();i++)
	{
		if(m_aFriendSearchResults[i].m_netAddr.s_addr == roomId)
		{

			memcpy(extData, m_aFriendSearchResults[i].m_gameSessionData, sizeof(GameSessionData));
		}
	}
	m_FriendSessionUpdatedFn = FriendSessionUpdatedFn;
	m_pParamFriendSessionUpdated = pParam;

	m_FriendSessionUpdatedFn(true, m_pParamFriendSessionUpdated);
}


#ifdef _CONTENT_PACKAGE
bool SQRNetworkManager_AdHoc_Vita::ForceErrorPoint(eSQRForceError error)
{
	return false;
}
#else
bool SQRNetworkManager_AdHoc_Vita::aForceError[SNM_FORCE_ERROR_COUNT] =
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

bool SQRNetworkManager_AdHoc_Vita::ForceErrorPoint(eSQRForceError err)
{
	return aForceError[err];
}
#endif

void SQRNetworkManager_AdHoc_Vita::AttemptAdhocSignIn(int (*SignInCompleteCallbackFn)(void *pParam, bool bContinue, int pad), void *pParam, bool callIfFailed/*=false*/)
{
	s_SignInCompleteCallbackFn = SignInCompleteCallbackFn;
	s_signInCompleteCallbackIfFailed = callIfFailed;
	s_SignInCompleteParam = pParam;
	app.DebugPrintf("s_SignInCompleteCallbackFn - 0x%08x : s_SignInCompleteParam - 0x%08x\n", (unsigned int)s_SignInCompleteCallbackFn, (unsigned int)s_SignInCompleteParam);
	SceNetCheckDialogParam param;
	memset(&param, 0x00, sizeof(param));
	sceNetCheckDialogParamInit(&param);

	s_attemptSignInAdhoc = true;		// so we know which sign in we're trying to make in the netCheckUpdate

	SceNetAdhocctlGroupName groupName;

	memset(groupName.data, 0x00, SCE_NET_ADHOCCTL_GROUPNAME_LEN);
	param.groupName = &groupName;
	param.mode = SCE_NETCHECK_DIALOG_MODE_ADHOC_CONN;

	memcpy(&param.npCommunicationId.data, &s_npCommunicationId, sizeof(s_npCommunicationId));
	param.npCommunicationId.term = '\0';
	param.npCommunicationId.num = 0;

	int ret = sceNetCheckDialogInit(&param);

 	ProfileManager.SetSysUIShowing( true );
	app.DebugPrintf("------------>>>>>>>>   sceNetCheckDialogInit : Adhoc Mode\n");

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



void SQRNetworkManager_AdHoc_Vita::AttemptPSNSignIn(int (*SignInCompleteCallbackFn)(void *pParam, bool bContinue, int pad), void *pParam, bool callIfFailed/*=false*/)
{
	s_SignInCompleteCallbackFn = SignInCompleteCallbackFn;
	s_signInCompleteCallbackIfFailed = callIfFailed;
	s_SignInCompleteParam = pParam;
	app.DebugPrintf("s_SignInCompleteCallbackFn - 0x%08x : s_SignInCompleteParam - 0x%08x\n", (unsigned int)s_SignInCompleteCallbackFn, (unsigned int)s_SignInCompleteParam);

	if(SQRNetworkManager_AdHoc_Vita::GetAdhocStatus())
	{
		// if the adhoc connection is running, kill it here
		sceNetCtlAdhocDisconnect();
	}

	SceNetCheckDialogParam param;
	memset(&param, 0x00, sizeof(param));
	sceNetCheckDialogParamInit(&param);
	param.mode = SCE_NETCHECK_DIALOG_MODE_PSN_ONLINE;
	param.defaultAgeRestriction = ProfileManager.GetMinimumAge();

	s_attemptSignInAdhoc = false;	// so we know which sign in we're trying to make in the netCheckUpdate


	// -------------------------------------------------------------
	// MGH -  this code is duplicated in the PSN network manager now too, so any changes will have to be made there too
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


int SQRNetworkManager_AdHoc_Vita::SetRichPresence(const void *data)
{
	const sce::Toolkit::NP::PresenceDetails *newPresenceInfo = (const sce::Toolkit::NP::PresenceDetails *)data;

	s_lastPresenceInfo.status = newPresenceInfo->status;
// 	s_lastPresenceInfo.userInfo = newPresenceInfo->userInfo;
	s_lastPresenceInfo.presenceType = SCE_NP_BASIC_IN_GAME_PRESENCE_TYPE_DEFAULT;

	s_presenceStatusDirty = true;
	SendLastPresenceInfo();

	// Return as if no error happened no matter what, as we'll be resending ourselves if we need to and don't want the calling system to retry
	return 0;
}

void SQRNetworkManager_AdHoc_Vita::UpdateRichPresenceCustomData(void *data, unsigned int dataBytes)
{
	if(m_isHosting == false)
		return;
	if(m_matchingContextServerValid)	// if the wifi connection has dropped (say after the console has woken from sleep) this can be invalid
	{
		int err = sceNetAdhocMatchingSetHelloOpt(m_matchingContext, dataBytes, data);
		assert(err == SCE_OK);
	}
	else
	{
		app.DebugPrintf("UpdateRichPresenceCustomData failed, m_matchingContextServerValid == false\n");
	}

// C
//
// 	assert(dataBytes <= SCE_NP_BASIC_IN_GAME_PRESENCE_DATA_SIZE_MAX );
// 	memcpy(s_lastPresenceInfo.data, data, dataBytes);
// 	s_lastPresenceInfo.size = dataBytes;
//
// 	s_presenceDataDirty = true;
// 	SendLastPresenceInfo();
}

void SQRNetworkManager_AdHoc_Vita::TickRichPresence()
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

void SQRNetworkManager_AdHoc_Vita::SendLastPresenceInfo()
{
	// Don't attempt to send if we are already waiting to resend
	if( s_resendPresenceCountdown ) return;

	// MGH - On Vita, change this to use SCE_NP_BASIC_IN_GAME_PRESENCE_TYPE_GAME_JOINING  at some point

	// On PS4 we can't set the status and the data at the same time
// 	if( s_presenceDataDirty )
// 	{
// 		s_presenceDataDirty = false;
// 		s_lastPresenceInfo.presenceType = SCE_TOOLKIT_NP_PRESENCE_DATA;
// 	}
// 	else if( s_presenceStatusDirty )
// 	{
// 		s_presenceStatusDirty = false;
// 		s_lastPresenceInfo.presenceType = SCE_TOOLKIT_NP_PRESENCE_STATUS;
// 	}

	int err = 0;
	// check if we're connected to the PSN first
	if(ProfileManager.IsSignedInLive(0))//ProfileManager.getQuadrant(s_lastPresenceInfo.userInfo.userId)))
	{
		err = sce::Toolkit::NP::Presence::Interface::setPresence(&s_lastPresenceInfo);
	}

	if( err != SCE_TOOLKIT_NP_SUCCESS )
	{
		assert(0); // this should only happen for bad data
	}
}

void SQRNetworkManager_AdHoc_Vita::SetPresenceDataStartHostingGame()
{
	if( m_offlineGame )
	{
		SQRNetworkManager_AdHoc_Vita::UpdateRichPresenceCustomData(&c_presenceSyncInfoNULL, sizeof(HelloSyncInfo) );
	}
	else
	{
		HelloSyncInfo presenceInfo;
		CPlatformNetworkManagerSony::SetSQRPresenceInfoFromExtData( &presenceInfo.m_presenceSyncInfo, m_joinExtData, m_room, m_serverId );
		assert(m_joinExtDataSize == sizeof(GameSessionData));
		memcpy(&presenceInfo.m_gameSessionData, m_joinExtData, sizeof(GameSessionData));
		memcpy(&presenceInfo.m_roomSyncData, &m_roomSyncData, sizeof(RoomSyncData));
		SQRNetworkManager_AdHoc_Vita::UpdateRichPresenceCustomData(&presenceInfo, sizeof(HelloSyncInfo) );
		//		OrbisNPToolkit::createNPSession();
	}
}

int SQRNetworkManager_AdHoc_Vita::GetJoiningReadyPercentage()
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

void SQRNetworkManager_AdHoc_Vita::removePlayerFromVoiceChat( SQRNetworkPlayer* pPlayer )
{
	if(!sc_voiceChatEnabled)
		return;


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
 			assert(pVoice);
 			if(pVoice)
 				SonyVoiceChat_Vita::disconnectRemoteConnection(pVoice);
		}
	}
}

int SQRNetworkManager_AdHoc_Vita::sendDataPacket( SceNetInAddr addr, EAdhocDataTag tag, void* data, int dataSize )
{
	static unsigned char s_dataBuffer[SCE_NET_ADHOC_MATCHING_MAXDATALEN];
	uint32_t* buf = (uint32_t*)s_dataBuffer;
	buf[0] = tag;
	memcpy(&buf[1], data, dataSize);
	int ret = sceNetAdhocMatchingSendData(m_matchingContext, &addr, dataSize+4, s_dataBuffer);
// 	assert(ret == SCE_OK);
	return ret;
}

int SQRNetworkManager_AdHoc_Vita::sendDataPacket( SceNetInAddr addr, void* data, int dataSize )
{
	int ret = sceNetAdhocMatchingSendData(m_matchingContext, &addr, dataSize, data);
	// 	assert(ret == SCE_OK);
	return ret;
}

void SQRNetworkManager_AdHoc_Vita::startMatching()
{
	SetState(SNM_INT_STATE_STARTING_CONTEXT);
	CreateMatchingContext();
}

SQRNetworkPlayer *SQRNetworkManager_AdHoc_Vita::GetPlayerByXuid(PlayerUID xuid)
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

void SQRNetworkManager_AdHoc_Vita::UpdateLocalIPAddress()
{
	SceNetInAddr localIPAddr;
	int ret = sceNetCtlAdhocGetInAddr(&localIPAddr);

	// If we got the IP address, update our cached value
	if (ret == SCE_OK)
	{
		m_localIPAddr = localIPAddr;
	}
}

