#include "stdafx.h"

#include "DQRNetworkManager.h"
#include "PartyController.h"
#include <collection.h>
#include <ppltasks.h>
#include <ws2tcpip.h>
#include "..\Minecraft.World\StringHelpers.h"
#include "base64.h"

#ifdef _DURANGO
#include "..\Minecraft.World\DurangoStats.h"
#endif

#include "ChatIntegrationLayer.h"


using namespace Concurrency;
using namespace Windows::Foundation::Collections;

DQRNetworkManager::ePartyProcessType	DQRNetworkManager::m_partyProcess = DQRNetworkManager::DNM_PARTY_PROCESS_NONE;

bool DQRNetworkManager::m_inviteReceived = false;
int DQRNetworkManager::m_bootUserIndex;
wstring	DQRNetworkManager::m_bootSessionName;
wstring	DQRNetworkManager::m_bootServiceConfig;
wstring	DQRNetworkManager::m_bootSessionTemplate;
DQRNetworkManager * DQRNetworkManager::s_pDQRManager = NULL;

//using namespace Windows::Xbox::Networking;

DQRNetworkManager::SessionInfo::SessionInfo(wstring& sessionName, wstring& serviceConfig, wstring& sessionTemplate)
{
	m_detailsValid = true;
	m_sessionName = sessionName;
	m_serviceConfig = serviceConfig;
	m_sessionTemplate = sessionTemplate;
}

DQRNetworkManager::SessionInfo::SessionInfo()
{
	m_detailsValid = false;
}

// This maps internal to extern states, and needs to match element-by-element the eSQRNetworkManagerInternalState enumerated type
const DQRNetworkManager::eDQRNetworkManagerState DQRNetworkManager::m_INTtoEXTStateMappings[DQRNetworkManager::DNM_INT_STATE_COUNT] = 
{
	DNM_STATE_INITIALISING,			// DNM_INT_STATE_INITIALISING
	DNM_STATE_INITIALISE_FAILED,	// DNM_INT_STATE_INITIALISE_FAILED
	DNM_STATE_IDLE,					// DNM_INT_STATE_IDLE
	DNM_STATE_HOSTING,				// DNM_INT_STATE_HOSTING
	DNM_STATE_HOSTING,				// DNM_INT_STATE_HOSTING_WAITING_TO_PLAY
	DNM_STATE_HOSTING,				// DNM_INT_STATE_HOSTING_FAILED
	DNM_STATE_JOINING,				// DNM_INT_STATE_JOINING
	DNM_STATE_JOINING,				// DNM_INT_STATE_JOINING_WAITING_FOR_RESERVATIONS
	DNM_STATE_JOINING,				// DNM_INT_STATE_JOINING_GET_SDA
	DNM_STATE_JOINING,				// DNM_INT_STATE_JOINING_WAITING_FOR_SDA
	DNM_STATE_JOINING,				// DNM_INT_STATE_JOINING_CREATE_SESSION
	DNM_STATE_JOINING,				// DNM_INT_STATE_JOINING_WAITING_FOR_ACTIVE_SESSION
	DNM_STATE_JOINING,				// DNM_INT_STATE_JOINING_SENDING_UNRELIABLE
	DNM_STATE_JOINING,				// DNM_INT_STATE_JOINING_FAILED_TIDY_UP
	DNM_STATE_JOINING,				// DNM_INT_STATE_JOINING_FAILED
	DNM_STATE_STARTING,				// DNM_INT_STATE_STARTING
	DNM_STATE_PLAYING,				// DNM_INT_STATE_PLAYING
	DNM_STATE_LEAVING,				// DNM_INT_STATE_LEAVING
	DNM_STATE_LEAVING,				// DNM_INT_STATE_LEAVING_FAILED
	DNM_STATE_ENDING,				// DNM_INT_STATE_ENDING		
};

DQRNetworkManager::DQRNetworkManager(IDQRNetworkManagerListener *listener)
{
	s_pDQRManager			= this;
	m_listener				= listener;
	m_eventHandlers			= ref new DQRNetworkManagerEventHandlers(this);
	m_XRNS_Session			= nullptr;
	m_multiplayerSession	= nullptr;
	m_sda					= nullptr;
	m_currentSmallId		= 0;
	m_hostSmallId			= 0;
	m_isHosting				= false;
	m_isInSession			= false;
	m_partyController		= new PartyController(this);
	m_partyController->RegisterEventHandlers();
	memset(m_sessionAddressFromSmallId,0,sizeof(m_sessionAddressFromSmallId));
	memset(m_channelFromSmallId,0,sizeof(m_channelFromSmallId));

	memset(&m_roomSyncData, 0, sizeof(m_roomSyncData));
	memset(m_players, 0, sizeof(m_players));

	m_CreateSessionThread	= NULL;
	m_GetFriendPartyThread  = NULL;
	m_UpdateCustomSessionDataThread = NULL;

	m_CheckPartyInviteThread = NULL;
	m_notifyForFullParty	 = false;

	m_customDataDirtyUpdateTicks = 0;
	m_sessionResultCount	= 0;
	m_sessionSearchResults	= NULL;
	m_joinSessionUserMask	= 0;
	m_cancelJoinFromSearchResult = false;

	InitializeCriticalSection(&m_csStateChangeQueue);
	InitializeCriticalSection(&m_csHostGamertagResolveResults);
	InitializeCriticalSection(&m_csRTSMessageQueueIncoming);
	InitializeCriticalSection(&m_csRTSMessageQueueOutgoing);
	InitializeCriticalSection(&m_csSendBytes);
	InitializeCriticalSection(&m_csVecChatPlayers);
	InitializeCriticalSection(&m_csRoomSyncData);
	InitializeCriticalSection(&m_csPartyViewVector);

	m_joinSessionXUIDs = ref new Platform::Array<Platform::String ^>(4);
	m_addLocalPlayerState		= DNM_ADD_PLAYER_STATE_IDLE;
	m_removeLocalPlayerState	= DNM_REMOVE_PLAYER_STATE_IDLE;

	m_playersLeftParty = 0;

	m_handleForcedSignOut = false;

	m_RTS_Stat_totalBytes = 0;
	m_RTS_Stat_totalSends = 0;

	m_RTS_DoWorkThread = new C4JThread(DQRNetworkManager::_RTSDoWorkThread, this, "Realtimesession processing");
	m_RTS_DoWorkThread->SetProcessor(CPU_CORE_DQR_REALTIMESESSION);
	m_RTS_DoWorkThread->SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);
	m_RTS_DoWorkThread->Run();
}

void DQRNetworkManager::Initialise()
{
	m_associationTemplate = WXN::SecureDeviceAssociationTemplate::GetTemplateByName( L"MultiplayerUdp" );

	m_state			= DNM_INT_STATE_IDLE;
	m_stateExternal = DNM_STATE_IDLE;

	m_chat = GetChatIntegrationLayer();
	m_chat->InitializeChatManager(true, true, false, false, this);
}

// This method can be called on any xbox live context, to enable tracing of the service calls that go on internally when anything is done using that context
void DQRNetworkManager::EnableDebugXBLContext(MXS::XboxLiveContext^ XBLContext)
{
#ifndef _CONTENT_PACKAGE
	// Turn on debug logging to Output debug window for Xbox Services
	XBLContext->Settings->DiagnosticsTraceLevel = MXS::XboxServicesDiagnosticsTraceLevel::Verbose;

	// Show service calls from Xbox Services on the UI for easy debugging
	XBLContext->Settings->EnableServiceCallRoutedEvents = true;
	XBLContext->Settings->ServiceCallRouted += ref new Windows::Foundation::EventHandler<Microsoft::Xbox::Services::XboxServiceCallRoutedEventArgs^>( 
		[=]( Platform::Object^, Microsoft::Xbox::Services::XboxServiceCallRoutedEventArgs^ args )
	{
		//if( args->HttpStatus != 200 )
		{
			LogComment(L"[URL]: " + args->HttpMethod + " " + args->Url->AbsoluteUri);
			if( !args->RequestBody->IsEmpty() )
			{
				LogComment(L"[RequestBody]: " + args->RequestBody );
			}
			LogComment(L"");
			LogComment(L"[Response]: " + args->HttpStatus.ToString() + " " + args->ResponseBody);
			LogComment(L"");
		}
	});
#endif
}

// This is the top level method called when starting to host a network game. Most of the functionality is asynchronously run in a separate thread kicked off here - see ::HostGameThreadProc
void DQRNetworkManager::CreateAndJoinSession(int usersMask, unsigned char *customSessionData, unsigned int customSessionDataSize, bool offline)
{
	m_isHosting = true;
	m_isInSession = true;
	m_isOfflineGame = offline;
	m_currentUserMask = usersMask;
	SetState(DNM_INT_STATE_HOSTING);
	m_customSessionData = customSessionData;
	m_customSessionDataSize = customSessionDataSize;

	m_CreateSessionThread = new C4JThread(&DQRNetworkManager::_HostGameThreadProc, this, "Create session");
	m_CreateSessionThread->Run();
}

// Flag that the custom session data has been updated - this isn't actually updated here since updating is an asynchronous process and we may already be in the middle of doing an
// update. Instead the custom data is flagged flagged as dirty here, and it will be considered for updated when next appropriate during a tick.
void DQRNetworkManager::UpdateCustomSessionData()
{
	if( !m_isHosting) return;
	if( m_isOfflineGame ) return;

	// Update data on next tick
	m_customDataDirtyUpdateTicks = 1;
}

// This is the main method for finishing joining a game session itself, by any method.
// By the point this is called, we should already have a reserved slot in the game session, by virtue
// of adding our local players to the party, having this noticed by the host, and the host add reserved slots for us in the game session.
// At this point we need to:
// (1) Set out players state in the session to active, so that they won't timeout & be removed
// (2) Get the network details of the  host that we need to connect to
// (3) Set state up so that in the next tick we'll attempt to set up the network communications for this endpoint
// Note that the reason that the final setting up of the network endpoint isn't just directly in this method itself, is that we have seen it fail in
// the past and so we need to be able to retry it, which is simpler if it is part of our general state machine to be able to repeat this operation.
void DQRNetworkManager::JoinSession(int playerMask)
{
	// Establish a primary user & xbox live context for this user. We can use these for operations which aren't particular to any specific user on the local console
	m_primaryUser = ProfileManager.GetUser(0);
	if( m_primaryUser == nullptr )
	{
		app.DebugPrintf("DNM_INT_STATE_JOINING_FAILED getting primary user\n");
		SetState(DNM_INT_STATE_JOINING_FAILED);
		return;
	}

	m_primaryUserXboxLiveContext = ref new MXS::XboxLiveContext(m_primaryUser);
	if( m_primaryUserXboxLiveContext == nullptr )
	{
		app.DebugPrintf("DNM_INT_STATE_JOINING_FAILED getting primary context\n");
		SetState(DNM_INT_STATE_JOINING_FAILED);
		return;
	}
	EnableDebugXBLContext(m_primaryUserXboxLiveContext);

	SetState(DNM_INT_STATE_JOINING);

	m_partyController->RefreshPartyView();
	m_isInSession = true;
	m_isOfflineGame = false;

	for( int i = 0; i < MAX_LOCAL_PLAYER_COUNT; i++ )
	{
		// Get the game session associated with our party. We need to get this once for every person joining to set them individually to be active
		if( playerMask & ( 1 << i ) )
		{
			MXSM::MultiplayerSession^ session = nullptr;

			// Get user & xbox live context for this specific local user that we are attempting to join
			WXS::User^ joiningUser = ProfileManager.GetUser(i);
			if( joiningUser == nullptr )
			{
				app.DebugPrintf("DNM_INT_STATE_JOINING_FAILED getting joining user\n");
				SetState(DNM_INT_STATE_JOINING_FAILED);
				return;
			}

			MXS::XboxLiveContext^ joiningUserXBLContext = ref new MXS::XboxLiveContext(joiningUser);
			if( joiningUserXBLContext == nullptr )
			{
				app.DebugPrintf("DNM_INT_STATE_JOINING_FAILED getting joining context\n");
				SetState(DNM_INT_STATE_JOINING_FAILED);
				return;
			}

			if( m_partyController->GetPartyView() == nullptr )
			{
				app.DebugPrintf("DNM_INT_STATE_JOINING_FAILED getting party view\n");
				SetState(DNM_INT_STATE_JOINING_FAILED);
				return;
			}

			// Get a copy of the session document, for this user
			auto multiplayerSessionAsync = joiningUserXBLContext->MultiplayerService->GetCurrentSessionAsync( ConvertToMicrosoftXboxServicesMultiplayerSessionReference(m_partyController->GetPartyView()->GameSession));
			create_task(multiplayerSessionAsync).then([&session,this](task<MXSM::MultiplayerSession^> t)
			{
				try
				{
					session = t.get(); // if t.get() didn't throw, it succeeded
				}
				catch (Platform::COMException^ ex)
				{
					LogCommentWithError( L"MultiplayerSession failed", ex->HResult );
				}
			})
			.wait();

			// If we found the session, then set the status of this member to be active (should be reserved). This will stop our slot timing out and us being dropped out of the session.
			if( session != nullptr )
			{
				if(!IsPlayerInSession(joiningUser->XboxUserId, session, NULL) )
				{
					app.DebugPrintf("DNM_INT_STATE_JOINING_FAILED didn't find required player in session\n");
					SetState(DNM_INT_STATE_JOINING_FAILED);
					return;
				}
				session->SetCurrentUserStatus(MXSM::MultiplayerSessionMemberStatus::Active);
				HRESULT hr = S_OK;
				session = WriteSessionHelper( joiningUserXBLContext, session, MXSM::MultiplayerSessionWriteMode::UpdateExisting, hr );
				HandleSessionChange(session);
			}
			else
			{
				app.DebugPrintf("DNM_INT_STATE_JOINING_FAILED didn't find session\n");
				SetState(DNM_INT_STATE_JOINING_FAILED);
				return;
			}
		}
	}

	MXSM::MultiplayerSession^ session = m_multiplayerSession;

	if( session != nullptr )
	{
		// Get the secure device address for the host player, and then attempt to create a association with it
		int hostSessionIndex = GetSessionIndexAndSmallIdForHost(&m_hostSmallId);

		MXSM::MultiplayerSessionMember^ member = m_multiplayerSession->Members->GetAt(hostSessionIndex);

		m_secureDeviceAddressBase64 = member->SecureDeviceAddressBase64;

		m_isHosting = false;

		sockaddr_in6 localSocketAddressStorage;
    
		ZeroMemory(&localSocketAddressStorage, sizeof(localSocketAddressStorage));
    
		localSocketAddressStorage.sin6_family = AF_INET6;
		localSocketAddressStorage.sin6_port = htons(m_associationTemplate->AcceptorSocketDescription->BoundPortRangeLower);
    
		memcpy(&localSocketAddressStorage.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    
		m_localSocketAddress = Platform::ArrayReference<BYTE>(reinterpret_cast<BYTE*>(&localSocketAddressStorage), sizeof(localSocketAddressStorage));

		m_joinCreateSessionAttempts = 0;

		m_joinSmallIdMask = playerMask;

		SetState(DNM_INT_STATE_JOINING_GET_SDA);
	}
	else
	{
		app.DebugPrintf("DNM_INT_STATE_JOINING_FAILED getting session\n");
		SetState(DNM_INT_STATE_JOINING_FAILED);
	}
}

void DQRNetworkManager::JoinSessionFromInviteInfo(int playerMask)
{
	// Gather set of XUIDs for the players that we are joining with
	for( int i = 0; i < MAX_LOCAL_PLAYER_COUNT; i++ )
	{
		if( playerMask & ( 1 << i ) )
		{
			WXS::User^ user = ProfileManager.GetUser(i);
			if( user == nullptr )
			{
				return;
			}
			m_joinSessionXUIDs[i] = user->XboxUserId;
		}
		else
		{
			m_joinSessionXUIDs[i] = nullptr;
		}
	}

	// It is possible that in addition to the player that has been invited (and will already have a slot in the session) that we added more local player(s) from the quadrant sign in.
	// In this case, then we need to attempt to add these to the party at this stage, and then wait for another gameSession ready event(s) before trying to progress to getting the whole
	// set of local players into the game

	bool playerAdded = m_partyController->AddLocalUsersToParty( playerMask, ProfileManager.GetUser(0) );

	if( playerAdded )
	{
		app.DebugPrintf("Joining from invite, but extra non-party user(s) found so waiting for reservations\n");
		// Wait until we get notification via game session ready that our newly added party members have slots, then proceed to join session
		m_isInSession = true;
		m_startedWaitingForReservationsTime = System::currentTimeMillis();
		m_joinSessionUserMask = playerMask;
		m_currentUserMask = 0;
		m_isOfflineGame = false;

		SetState(DNM_INT_STATE_JOINING_WAITING_FOR_RESERVATIONS);
	}
	else
	{
		app.DebugPrintf("Joining from invite, no extra non-party users required\n");
		// No further players added - continue directly on with joining
		JoinSession(playerMask);
	}
}


// Add one or more local users (specified by bits set in playerMask) to the session. We use this as a client in a network game. At the stage this
// is called, the players being added should already have reserved slots in the session - ie we've already put the plyers in the party, this has
// been detected by the host, and it has added the reserved slots in the sesion that we require.
//
// At this stage we need to:
// (1) Set the player's state in the session to active
// (2) Send the small Id of the player to the host - we've already got reliable network communications to the host at this point. This lets the
//     host know that there is an active player on this communication channel (we are multiplexing 4 channels, one for each local player)

bool DQRNetworkManager::AddUsersToSession(int playerMask, MXSM::MultiplayerSessionReference^ sessionRef )
{
	if( m_isHosting )
	{
		return false;
	}

	bool bSuccess = true;
	for( int i = 0; i < MAX_LOCAL_PLAYER_COUNT; i++ )
	{
		if( playerMask & ( 1 << i ) )
		{
			// We need to get a MultiplayerSession for each player that is joining

			MXSM::MultiplayerSession^ session = nullptr;
			
			WXS::User^ newUser = ProfileManager.GetUser(i);
			if( newUser == nullptr )
			{
				bSuccess = false;
				continue;
			}
			MXS::XboxLiveContext^ newUserXBLContext = ref new MXS::XboxLiveContext(newUser);

			auto multiplayerSessionAsync = newUserXBLContext->MultiplayerService->GetCurrentSessionAsync( sessionRef );
			create_task(multiplayerSessionAsync).then([&session,this](task<MXSM::MultiplayerSession^> t)
			{
				try
				{
					session = t.get(); // if t.get() didn't throw, it succeeded
				}
				catch (Platform::COMException^ ex)
				{
					LogCommentWithError( L"MultiplayerSession failed", ex->HResult );
				}
			})
			.wait();

			// If we found the session, then set the status of this member to be active (should be reserved). This will stop our slot timing out and us being dropped out of the session.
			if( session != nullptr )
			{
				int smallId = -1;
				if(!IsPlayerInSession(newUser->XboxUserId, session, &smallId) )
				{
					bSuccess = false;
					continue;
				}
				session->SetCurrentUserStatus(MXSM::MultiplayerSessionMemberStatus::Active);
				HRESULT hr = S_OK;
				session = WriteSessionHelper( newUserXBLContext, session, MXSM::MultiplayerSessionWriteMode::UpdateExisting, hr );
				HandleSessionChange(session);

				SendSmallId(true, 1 << i);
			}
		}
	}
	return bSuccess;
}

bool DQRNetworkManager::AddLocalPlayerByUserIndex(int userIndex)
{
	// We need to handle this differently for the host and other machines. As the procedure for adding a reserved slot for a local player whilst on the host doesn't seem to work
	//
	// On the host machine, we:
	// 
	// (1) Get a MPSD for the player that is being added
	// (2) Call the join method
	// (3) Write the MPSD
	// (4) Update the player sync data, and broadcast out to all clients
	
	// On remote machines, we:
	//
	// (1) join the party
	// (2) the host should (if a slot is available) put a reserved slot in the session and attempt to pull reserved players
	// (3) the client will respond to the gamesessionready event that is then received to set the added player to be active

	// If we're already in some of the asynchronous processing for adding as player, then we can't add another one - just fail straight away
	if( m_addLocalPlayerState != DNM_ADD_PLAYER_STATE_IDLE ) return false;

	if( m_isHosting )
	{
		WXS::User^ newUser = ProfileManager.GetUser(userIndex);
		if( newUser == nullptr )
		{
			return false;
		}
		
		if( !m_isOfflineGame )
		{
			// This is going to involve some async processing 

			MXS::XboxLiveContext^ newUserXBLContext = ref new MXS::XboxLiveContext(newUser);
			if( newUserXBLContext == nullptr )
			{
				return false;
			}

			EnableDebugXBLContext( newUserXBLContext);

			m_addLocalPlayerState = DNM_ADD_PLAYER_STATE_PROCESSING;

			auto asyncOp = newUserXBLContext->MultiplayerService->GetCurrentSessionAsync( m_multiplayerSession->SessionReference );
			create_task(asyncOp)
				.then([this,newUserXBLContext,userIndex] (task<Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^> t)
			{
				try
				{
					Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^ currentSession = t.get();

					// Don't attempt to join a player if we've no slots left in the session (this will include reserved slots)
					if( currentSession->Members->Size < currentSession->SessionConstants->MaxMembersInSession )
					{
						int smallId = m_currentSmallId;
						MXSM::MultiplayerSessionMember ^member = currentSession->Join(GetNextSmallIdAsJsonString(), false);
						currentSession->SetCurrentUserStatus( MXSM::MultiplayerSessionMemberStatus::Active );
						m_currentUserMask |= (1 << userIndex );

						// Get device ID for current user & set in the session
						Platform::String^ secureDeviceAddress =  WXN::SecureDeviceAddress::GetLocal()->GetBase64String();
						currentSession->SetCurrentUserSecureDeviceAddressBase64( secureDeviceAddress );

						HRESULT result;
						WriteSessionHelper(newUserXBLContext, currentSession, MXSM::MultiplayerSessionWriteMode::UpdateExisting, result);    // ************ WAITING **************

						DQRNetworkPlayer* pPlayer = new DQRNetworkPlayer(this, DQRNetworkPlayer::DNP_TYPE_LOCAL, true, userIndex, m_XRNS_Session->LocalSessionAddress);
						pPlayer->SetSmallId(smallId);
						pPlayer->SetName(ProfileManager.GetUser(userIndex)->DisplayInfo->Gamertag->Data());
						pPlayer->SetDisplayName(ProfileManager.GetDisplayName(userIndex));
						pPlayer->SetUID(PlayerUID(ProfileManager.GetUser(userIndex)->XboxUserId->Data()));

						// Also add to the party so that our friends can find us. The host will get notified of this additional player in the party, but we should ignore since we're already in the session
						m_partyController->AddLocalUsersToParty(1 << userIndex, ProfileManager.GetUser(0));

						m_addLocalPlayerSuccessPlayer = pPlayer;
						m_addLocalPlayerSuccessIndex = userIndex;
						m_addLocalPlayerState = DNM_ADD_PLAYER_STATE_COMPLETE_SUCCESS;
					}
					else
					{
						m_addLocalPlayerFailedIndex = userIndex;
						m_addLocalPlayerState = DNM_ADD_PLAYER_STATE_COMPLETE_FAIL_FULL;
					}
				}
				catch ( Platform::COMException^ ex )
				{
					m_addLocalPlayerFailedIndex = userIndex;
					m_addLocalPlayerState = DNM_ADD_PLAYER_STATE_COMPLETE_FAIL;
				}
				catch ( Platform::Exception ^ex )
				{
					m_addLocalPlayerFailedIndex = userIndex;
					m_addLocalPlayerState = DNM_ADD_PLAYER_STATE_COMPLETE_FAIL;
				}
			});

			return true;
		}
		else
		{
			DQRNetworkPlayer* pPlayer = new DQRNetworkPlayer(this, DQRNetworkPlayer::DNP_TYPE_LOCAL, true, userIndex, 0);
			pPlayer->SetSmallId(m_currentSmallId++);
			pPlayer->SetName(ProfileManager.GetUser(userIndex)->DisplayInfo->Gamertag->Data());
			pPlayer->SetDisplayName(ProfileManager.GetDisplayName(userIndex));
			pPlayer->SetUID(PlayerUID(ProfileManager.GetUser(userIndex)->XboxUserId->Data()));

			// TODO - could this add fail?
			if(AddRoomSyncPlayer( pPlayer, 0, userIndex))
			{
				SendRoomSyncInfo();
				m_listener->HandlePlayerJoined(pPlayer);		// This is for notifying of local players joining in an offline game
			}
			else 
			{
				// Can fail (notably if m_roomSyncData contains players who've left)
				assert(0);
				return false;
			}
		}
		return true;
	}
	else
	{
		// Check if there's any available slots before attempting to add the player to the party. We can still fail joining later if
		// the host can't add a reserved slot for us for some reason but better checking on the client side before even attempting.
		
		WXS::User^ newUser = ProfileManager.GetUser(userIndex);

		MXS::XboxLiveContext^ newUserXBLContext = ref new MXS::XboxLiveContext(newUser);
		if( newUserXBLContext == nullptr )
		{
			return false;
		}

		m_addLocalPlayerState = DNM_ADD_PLAYER_STATE_PROCESSING;
		auto asyncOp = newUserXBLContext->MultiplayerService->GetCurrentSessionAsync( m_multiplayerSession->SessionReference );
		create_task(asyncOp)
			.then([this,newUserXBLContext,userIndex] (task<Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^> t)
		{
			try
			{
				Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^ currentSession = t.get();

				if( currentSession->Members->Size == currentSession->SessionConstants->MaxMembersInSession )
				{
					m_addLocalPlayerFailedIndex = userIndex;
					m_addLocalPlayerState = DNM_ADD_PLAYER_STATE_COMPLETE_FAIL_FULL;
				}
				else
				{
					m_joinSessionUserMask |= (1 << userIndex);
					m_joinSessionXUIDs[userIndex] = ProfileManager.GetUser(userIndex)->XboxUserId;
					m_partyController->AddLocalUsersToParty(1 << userIndex, ProfileManager.GetUser(0));

					m_addLocalPlayerSuccessPlayer = NULL;
					m_addLocalPlayerState = DNM_ADD_PLAYER_STATE_COMPLETE_SUCCESS;
				}
			}
			catch( Platform::COMException^ ex )
			{
				m_addLocalPlayerFailedIndex = userIndex;
				m_addLocalPlayerState = DNM_ADD_PLAYER_STATE_COMPLETE_FAIL;
			}
			catch ( Platform::Exception ^ex )
			{
				m_addLocalPlayerFailedIndex = userIndex;
				m_addLocalPlayerState = DNM_ADD_PLAYER_STATE_COMPLETE_FAIL;
			}
		});

		return true;
	}
}

bool DQRNetworkManager::RemoveLocalPlayerByUserIndex(int userIndex)
{
	// We need to handle this differently for the host and other machines. 
	//
	// On the host machine, we:
	// 
	// (1) Get a MPSD for the player that is being removed
	// (2) Call the leave method
	// (3) Write the MPSD
	// (4) Leave the party
	// (5) Update the player sync data, and broadcast out to all clients
	
	// On remote machines, we:
	//
	// (1) Get a MPSD for the player that is being removed
	// (2) Call the leave method
	// (3) Write the MPSD
	// (4) Leave the party
	// (5) send message to the host that this player has left
	// (6) host should respond to this message by removing the player from the player sync data, and notifying all clients of updated players
	// (7) the client should respond to the player leaving that will happen and this will actually notify the game that the player has left

	// TODO - this should be rearranged so that the async stuff isn't waited on here, and so that the callbacks don't get called from the task's thread

	if( m_removeLocalPlayerState != DNM_REMOVE_PLAYER_STATE_IDLE ) return false;

	WXS::User^ leavingUser = ProfileManager.GetUser(userIndex, true);
	if( leavingUser == nullptr )
	{
		return false;
	}
	
	if( !m_isOfflineGame )
	{
		if( m_chat )
		{
			m_chat->RemoveLocalUser(leavingUser);
		}
		MXS::XboxLiveContext^ leavingUserXBLContext = ref new MXS::XboxLiveContext(leavingUser);
		if( leavingUserXBLContext == nullptr )
		{
			return false;
		}
		EnableDebugXBLContext( leavingUserXBLContext);
		m_removeLocalPlayerState = DNM_REMOVE_PLAYER_STATE_PROCESSING;
		auto asyncOp = leavingUserXBLContext->MultiplayerService->GetCurrentSessionAsync( m_multiplayerSession->SessionReference );
		create_task(asyncOp)
			.then([this,leavingUserXBLContext,userIndex,leavingUser] (task<Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^> t)
		{
			try
			{
				Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^ currentSession = t.get();

				// Remove from the party
				LogComment(L"Removing user from party");
				m_partyController->RemoveLocalUserFromParty(leavingUser);
				LogComment(L"Removed user from party, now leaving session");

				// Then leave & update the session
				currentSession->Leave();
				HRESULT result;
				WriteSessionHelper(leavingUserXBLContext, currentSession, MXSM::MultiplayerSessionWriteMode::UpdateExisting, result);
				m_currentUserMask &= ~(1<<userIndex);
				LogComment(L"Finished leaving session");

				// Complete any deferred sign outs
				ProfileManager.CompleteDeferredSignouts();

				m_removeLocalPlayerIndex = userIndex;
				m_removeLocalPlayerState = DNM_REMOVE_PLAYER_STATE_COMPLETE_SUCCESS;
			}
			catch ( Platform::COMException^ ex )
			{
				app.DebugPrintf("DQRNetworkManager::RemoveLocalPlayerByUserIndex: Failed to remove local player %i (0x%x)\n", userIndex, ex->HResult);

				m_removeLocalPlayerState = DNM_REMOVE_PLAYER_STATE_COMPLETE_FAIL;
				m_removeLocalPlayerIndex = userIndex;
			}
		});
	}
	else
	{
		DQRNetworkPlayer* pPlayer = GetLocalPlayerByUserIndex( userIndex );
		RemoveRoomSyncPlayer( pPlayer );
	}
	return true;
}

bool DQRNetworkManager::IsHost()
{
	return m_isHosting;
}

// Consider as "in session" from the moment that a game is created or joined, until the point where the game itself has been told via state change that we are now idle. The
// game code requires IsInSession to return true as soon as it has asked to do one of these things (even if the state system hasn't really caught up with this request yet), and 
// it also requires that it is informed of the state changes leading up to not being in the session, before this should report false.
bool DQRNetworkManager::IsInSession()
{
	return m_isInSession;
}

// Get count of players currently in the session
int	DQRNetworkManager::GetPlayerCount()
{
	return m_roomSyncData.playerCount;
}

// Get count of players who are in the session, but not local to this machine
int DQRNetworkManager::GetOnlinePlayerCount()
{
	int count = 0;
	for( int i = 0; i < MAX_ONLINE_PLAYER_COUNT; i++ )
	{
		if( m_players[i] )
		{
			if( !m_players[i]->IsLocal() )
			{
				count++;
			}
		}
	}
	return count;
}


DQRNetworkPlayer *DQRNetworkManager::GetPlayerByIndex(int idx)
{
	return m_players[idx];

}

DQRNetworkPlayer *DQRNetworkManager::GetPlayerBySmallId(int idx)
{
	for( int i = 0; i < MAX_ONLINE_PLAYER_COUNT; i++ )
	{
		if( m_players[i] )
		{
			if( m_players[i]->GetSmallId() == idx)
			{
				return m_players[i];
			}
		}
	}
	return NULL;
}

DQRNetworkPlayer *DQRNetworkManager::GetPlayerByXuid(PlayerUID xuid)
{
	for( int i = 0; i < MAX_ONLINE_PLAYER_COUNT; i++ )
	{
		if( m_players[i] )
		{
			if( m_players[i]->GetUID() == xuid)
			{
				return m_players[i];
			}
		}
	}
	return NULL;
}

// Retrieve player display name by gamertag
wstring DQRNetworkManager::GetDisplayNameByGamertag(wstring gamertag)
{
	if (m_displayNames.find(gamertag) != m_displayNames.end())
	{
		return m_displayNames[gamertag];
	}
	else 
	{
		return gamertag;
	}
}

DQRNetworkPlayer *DQRNetworkManager::GetLocalPlayerByUserIndex(int idx)
{
	for( int i = 0; i < MAX_ONLINE_PLAYER_COUNT; i++ )
	{
		if( m_players[i] )
		{
			if( m_players[i]->IsLocal() )
			{
				if( m_players[i]->GetLocalPlayerIndex() == idx )
				{
					return m_players[i];
				}
			}
		}
	}
	return NULL;
}

DQRNetworkPlayer *DQRNetworkManager::GetHostPlayer()
{
	return GetPlayerBySmallId(m_hostSmallId);
}


int DQRNetworkManager::GetSessionIndex(DQRNetworkPlayer *player)
{
	for( int i = 0; i < MAX_ONLINE_PLAYER_COUNT; i++ )
	{
		if( m_players[i] == player )
		{
			return i;
		}
	}
	return 0;
}

void DQRNetworkManager::SetState(DQRNetworkManager::eDQRNetworkManagerInternalState state)
{
	eDQRNetworkManagerState oldState = m_INTtoEXTStateMappings[m_state];
	eDQRNetworkManagerState newState = m_INTtoEXTStateMappings[state];
	m_state = state;

	// Queue any important (ie externally relevant) state changes - we will do a call back for these in our main tick. Don't do it directly here
	// as we could be coming from any thread at this stage, with any stack size etc. and so we don't generally want to expect the game to be able to handle itself in such circumstances.
	if( newState != oldState )
	{
		EnterCriticalSection(&m_csStateChangeQueue);
		m_stateChangeQueue.push(StateChangeInfo(oldState,newState));
		LeaveCriticalSection(&m_csStateChangeQueue);
	}
}

DQRNetworkManager::eDQRNetworkManagerState DQRNetworkManager::GetState()
{
	return m_stateExternal;;
}

void DQRNetworkManager::Tick()
{
	Tick_XRNS();
	Tick_VoiceChat();
	Tick_Party();
	Tick_CustomSessionData();
	Tick_AddAndRemoveLocalPlayers();
	Tick_ResolveGamertags();
	Tick_PartyProcess();
	Tick_StateMachine();
	Tick_CheckInviteParty();
}

void DQRNetworkManager::Tick_XRNS()
{
	ProcessRTSMessagesIncoming();
}

void DQRNetworkManager::Tick_VoiceChat()
{
#if 0
	static int chatDumpCount = 0;
	chatDumpCount++;
	if( ( chatDumpCount % 40 ) == 0 )
	{
		if( m_chat )
		{
			LogCommentFormat(L"ChatManager: hasFocus:%d\n",m_chat->HasMicFocus());
			IVectorView<Microsoft::Xbox::GameChat::ChatUser^>^ chatUsers = m_chat->GetChatUsers();
			for( int i = 0; i < chatUsers->Size; i++ )
			{
				Microsoft::Xbox::GameChat::ChatUser^ chatUser = chatUsers->GetAt(i);
				LogCommentFormat(L"local: %d muted: %d type:%s restriction:%s mode:%s volume:%f [xuid:%s]\n",
					chatUser->IsLocal,chatUser->IsMuted,chatUser->ParticipantType.ToString()->Data(),chatUser->RestrictionMode.ToString()->Data(), chatUser->TalkingMode.ToString()->Data(),chatUser->Volume,
					chatUser->XboxUserId->Data());
			}
		}
	}
#endif
	// If we have to inform the chat integration layer of any players that have joined, do that now
	EnterCriticalSection(&m_csVecChatPlayers);
	for( int i = 0; i < m_vecChatPlayersJoined.size(); i++ )
	{
		int idx = m_vecChatPlayersJoined[i];
		if( m_chat )
		{
			WXS::User^ user = ProfileManager.GetUser(idx);
			if( user != nullptr )
			{
				m_chat->AddLocalUser(user);
			}
		}		
	}
	m_vecChatPlayersJoined.clear();
	LeaveCriticalSection(&m_csVecChatPlayers);
}

void DQRNetworkManager::Tick_Party()
{	
	// If the primary player has been flagged as having left the party, then we don't respond immediately as it is possible we are just transitioning from one party to another, and it would be much
	// nicer to handle this kind of transition directly. If we do get a new party within this time period, then we'll handle by asking the user if they want to leave the game they are currently in etc.
	if( m_playersLeftParty )
	{
		if( ( System::currentTimeMillis() - m_playersLeftPartyTime ) > PRIMARY_PLAYER_LEAVING_PARTY_WAIT_MS )
		{
			// We've waited long enough. User must (hopefully) have just left the party
			// Previously we'd switch to offline but that causes a world of pain with forced sign-outs
			if( m_playersLeftParty & 1 )
			{		
				// Before we switch to an offline game, check to see if there is currently a new party. If this is the case and
				// we're here, then its because we were added to a party, but didn't receive a gamesessionready event. So if we have
				// a party here that we've joined, and the number of players in the party (including us) is more than MAX_PLAYERS_IN_TEMPLATE,
				// then it seems reasonable to assume that the reason we're not in the game is due to lack of space, and we can inform the
				// user of this when converting to an offline game

				m_partyController->RefreshPartyView();
				WXM::PartyView^ partyView = m_partyController->GetPartyView();
				if( partyView )
				{
					int partySize = partyView->Members->Size;
					if( partySize > MAX_PLAYERS_IN_TEMPLATE )
					{
						g_NetworkManager.SetFullSessionMessageOnNextSessionChange();
					}
				}

				DQRNetworkManager::LogComment(L"Primary player on this system has left the party, switching to offline\n");
				app.SetAction(0, eAppAction_EthernetDisconnected);
			}
			else
			{
				// Secondary player(s) leaving, just remove as if they had chosen to exit themselves from the game
				for( int i = 0; i < MAX_LOCAL_PLAYERS; i++ )
				{
					if( m_playersLeftParty & ( 1 << i ) )
					{
						RemoveLocalPlayerByUserIndex(i);
					}
				}
			}

			m_playersLeftParty = 0;
		}
	}
	
	// Forced sign out
	if (m_handleForcedSignOut)
	{
		HandleForcedSignOut();
		m_handleForcedSignOut = false;
	}
}

void DQRNetworkManager::Tick_CustomSessionData()
{
	// If there was a thread updaing our custom session data, then clear it up if it is done
	if( m_UpdateCustomSessionDataThread != NULL )
	{
		if( !m_UpdateCustomSessionDataThread->isRunning() )
		{
			delete m_UpdateCustomSessionDataThread;
			m_UpdateCustomSessionDataThread = NULL;
		}
	}

	// If our custom data is dirty, and we aren't currently updating, then kick off a thread to update it
	if( m_isHosting && ( !m_isOfflineGame ) )
	{
		if( m_UpdateCustomSessionDataThread == NULL )
		{
			if( m_customDataDirtyUpdateTicks )
			{
				m_customDataDirtyUpdateTicks--;
				if( m_customDataDirtyUpdateTicks == 0 )
				{
					m_UpdateCustomSessionDataThread = new C4JThread(&DQRNetworkManager::_UpdateCustomSessionDataThreadProc, this, "Updating custom data");
					m_UpdateCustomSessionDataThread->Run();
				}
			}
		}
	}
	else
	{
		m_customDataDirtyUpdateTicks = 0;
	}
}

void DQRNetworkManager::Tick_AddAndRemoveLocalPlayers()
{
	// A lot of handling adding local players is handled asynchronously. Trying to avoid having the callbacks that may result from this being called from the task threads, so handling this aspect of it here in the tick
	if( m_addLocalPlayerState == DNM_ADD_PLAYER_STATE_COMPLETE_SUCCESS )
	{
		// If we've completed, and we're the host, then we should have the new player to create stored here in m_localPlayerSuccessCreated. For clients, this will just be NULL as the actual
		// adding of the player happens as part of a longer process of the host creating us a reserved slot etc. etc.
		if( m_addLocalPlayerSuccessPlayer )
		{
			if( AddRoomSyncPlayer( m_addLocalPlayerSuccessPlayer, m_XRNS_Session->LocalSessionAddress, m_addLocalPlayerSuccessIndex) )
			{
				SendRoomSyncInfo();
				m_listener->HandlePlayerJoined(m_addLocalPlayerSuccessPlayer);		// This is notifying local players joining, when online (host only)
			}
		}
		m_addLocalPlayerState = DNM_ADD_PLAYER_STATE_IDLE;
	}
	else if( m_addLocalPlayerState == DNM_ADD_PLAYER_STATE_COMPLETE_FAIL )
	{
		m_listener->HandleAddLocalPlayerFailed(m_addLocalPlayerFailedIndex, false);
		m_addLocalPlayerState = DNM_ADD_PLAYER_STATE_IDLE;
	}
	else if( m_addLocalPlayerState == DNM_ADD_PLAYER_STATE_COMPLETE_FAIL_FULL )
	{
		m_listener->HandleAddLocalPlayerFailed(m_addLocalPlayerFailedIndex, true);
		m_addLocalPlayerState = DNM_ADD_PLAYER_STATE_IDLE;
	}

	// Similarly for removing local players - avoiding having callbacks from the async task threads, so this aspect of the process is handled here

	if( m_removeLocalPlayerState == DNM_REMOVE_PLAYER_STATE_COMPLETE_SUCCESS || m_removeLocalPlayerState == DNM_REMOVE_PLAYER_STATE_COMPLETE_FAIL )
	{
		// Note: we now remove the player from the room sync data even if remove from session/party failed,
		// either way we need to clean up

		// On host, directly remove from the player sync data. On client, send a message to the host which will do this
		if( m_isHosting)
		{
			DQRNetworkPlayer* pPlayer = GetLocalPlayerByUserIndex( m_removeLocalPlayerIndex );
			RemoveRoomSyncPlayer( pPlayer );
			SendRoomSyncInfo();
		}
		else
		{
			// Check if this player actually exists yet on this machine. If it is, then we need to send a message to the host to unassign it which
			// ultimately will end up with this player being removed when the host syncs back with us. If it hasn't then there isn't anything to
			// unassign with the host
			DQRNetworkPlayer* pPlayer = GetLocalPlayerByUserIndex( m_removeLocalPlayerIndex );
			if( pPlayer )
			{
				SendUnassignSmallId(m_removeLocalPlayerIndex);
			}
		}
		m_removeLocalPlayerState = DNM_REMOVE_PLAYER_STATE_IDLE;
	}
}

void DQRNetworkManager::Tick_ResolveGamertags()
{
	// Host only - if there are any player display names which have been resolved (or failed to resolve), then this is the last stage in the player becoming active on the host's side and so do a few things here
	EnterCriticalSection(&m_csHostGamertagResolveResults);
	while( !m_hostGamertagResolveResults.empty() )
	{
		HostGamertagResolveDetails *details = m_hostGamertagResolveResults.front();

		details->m_pPlayer->SetName(details->m_name.c_str());
		
		LogComment("Adding a player");
		if( AddRoomSyncPlayer(details->m_pPlayer, details->m_sessionAddress, details->m_channel ) )
		{
			LogComment("Adding a player - success");
			m_listener->HandlePlayerJoined(details->m_pPlayer);		// This is for notifying of remote players joining, when online (when we are the host), as we have resolved their names
			// The last name to be resolve in any one atomic set (ie that comes in from a single DQR_INTERNAL_ASSIGN_SMALL_IDS message) will have this flag set, so we know this is the point
			// to synchronise out to the clients
			if( details->m_sync )
			{
				LogComment("Synchronising players with clients");
				SendRoomSyncInfo();
			}
		}
		else
		{
			LogComment("Adding a player - failed");
			delete details->m_pPlayer;

			// TODO - what to do if adding a player failed here?
			assert(false);
		}

		delete details;
		m_hostGamertagResolveResults.pop();
	}
	LeaveCriticalSection(&m_csHostGamertagResolveResults);
}

void DQRNetworkManager::Tick_PartyProcess()
{
	// On starting up the game, there's 3 options of what we need to do...
	// (1) Attempt to join a game session that was passed in on activation (this will have happened if we were started from a game ready notification)
	// (2) Attempt to join whatever game the party is associated with (this will happen if we were started in response to a party invite)
	switch( m_partyProcess )
	{
		case DNM_PARTY_PROCESS_NONE:
			break;
		case DNM_PARTY_PROCESS_JOIN_PARTY:
			if( GetBestPartyUserIndex() )
			{
				m_listener->HandleInviteReceived(0, new SessionInfo());
			}
			break;
		case DNM_PARTY_PROCESS_JOIN_SPECIFIED:
			m_listener->HandleInviteReceived(m_bootUserIndex, new SessionInfo(m_bootSessionName, m_bootServiceConfig, m_bootSessionTemplate));
			break;
	}
	m_partyProcess = DNM_PARTY_PROCESS_NONE;
}

void DQRNetworkManager::Tick_StateMachine()
{
	switch( m_state )
	{
		case DNM_INT_STATE_JOINING_GET_SDA:
			{
				SetState(DNM_INT_STATE_JOINING_WAITING_FOR_SDA);
				auto asyncOp = m_associationTemplate->CreateAssociationAsync(WXN::SecureDeviceAddress::FromBase64String(m_secureDeviceAddressBase64), WXN::CreateSecureDeviceAssociationBehavior::Reevaluate);
				create_task(asyncOp).then([this](task<WXN::SecureDeviceAssociation^> t)
				{
					m_sda = nullptr;
					try
					{
						m_sda = t.get();
					}
					catch (Platform::COMException^ ex)
					{
						LogCommentWithError( L"CreateAssociationAsync failed", ex->HResult );
					}
					// If this succeeded, then make a store of all the things we'll need to initiate the network communication endpoint for this machine (our local address, remove address, secure device association etc.)
					if( m_sda)
					{
						m_remoteSocketAddress = ref new Platform::Array<BYTE>(sizeof(SOCKADDR_STORAGE));
						m_sda->GetRemoteSocketAddressBytes(m_remoteSocketAddress);
						SetState(DNM_INT_STATE_JOINING_CREATE_SESSION);
					}
					else
					{
						SetState(DNM_INT_STATE_JOINING_FAILED);
					}
				});
			}
			break;
		case DNM_INT_STATE_JOINING_CREATE_SESSION:
			RTS_StartCient();
			SetState(DNM_INT_STATE_JOINING_WAITING_FOR_ACTIVE_SESSION);
			break;
		case DNM_INT_STATE_JOINING_SENDING_UNRELIABLE:
			{
				__int64 timeNow = System::currentTimeMillis();
				// m_firstUnreliableSendTime of 0 indicates that we haven't tried sending an unreliable packet yet so need to send one and initialise things
				if( m_firstUnreliableSendTime == 0 )
				{
					m_firstUnreliableSendTime = timeNow;
					m_lastUnreliableSendTime = timeNow;

					SendSmallId(false, m_joinSmallIdMask);
				}
				else
				{
					// Timeout if we've exceeded the threshold for this
					if( (timeNow - m_firstUnreliableSendTime) > JOIN_PACKET_RESEND_TIMEOUT_MS )
					{
						app.DebugPrintf("DNM_INT_STATE_JOINING_FAILED unreliable resend timeout\n");
						SetState(DNM_INT_STATE_JOINING_FAILED);
					}
					else
					{
						// Possibly send another packet if it has been long enough
						if( (timeNow - m_lastUnreliableSendTime ) > JOIN_PACKET_RESEND_DELAY_MS )
						{
							LogComment("Resending unreliable packet\n");
							m_lastUnreliableSendTime = timeNow;
							SendSmallId(false, m_joinSmallIdMask);
						}
					}
				}
			}
			break;
		case DNM_INT_STATE_JOINING_WAITING_FOR_RESERVATIONS:
			{
				// Timeout if we've been waiting for reserved slots for our joining players for too long. This is most likely because the host doesn't have room for all the slots we wanted, and we weren't able to determine this
				// when we went to join the game (ie someone else was joining at the same time). At this point we need to remove any local players that did already join, from both the session and the party.
				__int64 timeNow = System::currentTimeMillis();
				if( ( timeNow - m_startedWaitingForReservationsTime ) > JOIN_RESERVATION_WAIT_TIME )
				{
					SetState(DNM_INT_STATE_JOINING_FAILED_TIDY_UP);
					TidyUpFailedJoin();
				}
			}
			break;
		case DNM_INT_STATE_ENDING:
			SetState(DNM_INT_STATE_IDLE);
			break;
		case DNM_INT_STATE_HOSTING_WAITING_TO_PLAY:
			delete m_CreateSessionThread;
			m_CreateSessionThread = NULL;
			// If the game is offline we can transition straight to playing
			if (m_isOfflineGame) StartGame();
			break;
		case DNM_INT_STATE_JOINING_FAILED:
			SetState(DNM_INT_STATE_JOINING_FAILED_TIDY_UP);
			TidyUpFailedJoin();
			break;
		case DNM_INT_STATE_HOSTING_FAILED:
			m_multiplayerSession = nullptr;
			LogComment("Error DNM_INT_STATE_HOSTING_FAILED\n");
			SetState(DNM_INT_STATE_IDLE);
			break;
		case DNM_INT_STATE_LEAVING_FAILED:
			m_multiplayerSession = nullptr;
			LogComment("Error DNM_INT_STATE_LEAVING_FAILED\n");
			SetState(DNM_INT_STATE_IDLE);
			break;
	}

	EnterCriticalSection(&m_csStateChangeQueue);
	while(m_stateChangeQueue.size() > 0 )
	{
		if( m_listener )
		{
			m_listener->HandleStateChange(m_stateChangeQueue.front().m_oldState, m_stateChangeQueue.front().m_newState);
			if( m_stateChangeQueue.front().m_newState == DNM_STATE_IDLE )
			{
				m_isInSession = false;
			}
		}
		m_stateExternal = m_stateChangeQueue.front().m_newState;
		m_stateChangeQueue.pop();
	}
	LeaveCriticalSection(&m_csStateChangeQueue);
}

void DQRNetworkManager::Tick_CheckInviteParty()
{
	if( m_inviteReceived )
	{
		if( m_CheckPartyInviteThread )
		{
			if( !m_CheckPartyInviteThread->isRunning() )
			{
				delete m_CheckPartyInviteThread;
				m_CheckPartyInviteThread = NULL;
			}
		}
		if( m_CheckPartyInviteThread == NULL )
		{
			m_inviteReceived = false;
			m_CheckPartyInviteThread = new C4JThread(&DQRNetworkManager::_CheckInviteThreadProc, this, "Check invite thread");
			m_CheckPartyInviteThread->Run();
		}
	}
}

bool DQRNetworkManager::ShouldMessageForFullSession()
{
	bool retval = m_notifyForFullParty;
	m_notifyForFullParty = false;
	return retval;
}

void DQRNetworkManager::FlagInvitedToFullSession()
{
	m_notifyForFullParty = true;
}

void DQRNetworkManager::UnflagInvitedToFullSession()
{
	m_notifyForFullParty = false;
}

void	DQRNetworkManager::AddPlayerFailed(Platform::String ^xuid)
{
	// A request to add a player (via the party) has been rejected by the host. If this is a player that we were waiting to join, then we can now:
	// (1) stop waiting
	// (2) remove from the party
	// (3) inform the game of the failure
	LogCommentFormat(L"AddPlayerFailed received, for XUID %s", xuid->Data());
	for( int i = 0; i < MAX_LOCAL_PLAYER_COUNT; i++ )
	{
		if( m_joinSessionUserMask & ( 1 << i ) )
		{
			if( m_joinSessionXUIDs[i] == xuid )
			{
				LogCommentFormat(L"AddPlayerFailed received, XUID matched with joining player so handling (index %d)",i);
				m_joinSessionUserMask &= ~( 1 << i );
				m_joinSessionXUIDs[i] = nullptr;
				m_partyController->RemoveLocalUsersFromParty(m_primaryUser, 1 << i, m_multiplayerSession->SessionReference );
				m_listener->HandleAddLocalPlayerFailed(i, true);
			}
		}
	}
}

// Utility method to remove the braces at the start and end of a GUID and return the remaining string
Platform::String^ DQRNetworkManager::RemoveBracesFromGuidString(__in Platform::String^ guid )
{
    std::wstring strGuid = guid->ToString()->Data();

    if(strGuid.length() > 0 && strGuid[0] == L'{')
    {
        // Remove the {
        strGuid.erase(0, 1);
    }

    if(strGuid.length() > 0 && strGuid[strGuid.length() - 1] == L'}')
    {
        // Remove the }
        strGuid.erase(strGuid.end() - 1, strGuid.end());
    }

    return ref new Platform::String(strGuid.c_str());
}

void DQRNetworkManager::HandleSessionChange(MXSM::MultiplayerSession^ multiplayerSession)
{
	// 4J-JEV: Fix for Durango #152208 - [CRASH] Code: Gameplay: Title crashes during loading screen after signing in when prompted.
	if (multiplayerSession != nullptr)
	{
		// 4J-JEV: This id is needed to link stats together.
		// I thought setting the value from here would be less intrusive than adding an accessor.
		((DurangoStats*)GenericStats::getInstance())->setMultiplayerCorrelationId(multiplayerSession->MultiplayerCorrelationId);
	}
	else
	{
		((DurangoStats*)GenericStats::getInstance())->setMultiplayerCorrelationId( nullptr );
	}
	
	m_multiplayerSession = multiplayerSession;
}

// Utility method to update a session on the server, synchronously.
MXSM::MultiplayerSession^ DQRNetworkManager::WriteSessionHelper( MXS::XboxLiveContext^ xboxLiveContext, MXSM::MultiplayerSession^ multiplayerSession, MXSM::MultiplayerSessionWriteMode writeMode, HRESULT& hr )
{
    if (multiplayerSession == nullptr)
    {
        return nullptr;
    }

    auto asyncOpWriteSessionAsync = xboxLiveContext->MultiplayerService->WriteSessionAsync( multiplayerSession, writeMode );

    MXSM::MultiplayerSession^ outputMultiplayerSession = nullptr;

    create_task(asyncOpWriteSessionAsync)
    .then([&outputMultiplayerSession, &hr](task<MXSM::MultiplayerSession^> t)
    {
        try
        {
            outputMultiplayerSession = t.get(); // if t.get() didn't throw, it succeeded
        }
        catch ( Platform::COMException^ ex )
        {
            hr = ex->HResult;
        }
    })
    .wait();

    if( outputMultiplayerSession != nullptr && 
        outputMultiplayerSession->SessionReference != nullptr )
    {
		app.DebugPrintf( "Session written OK\n" );
    }

    return outputMultiplayerSession;
}

MXSM::MultiplayerSessionMember^ DQRNetworkManager::GetUserMemberInSession( Windows::Xbox::System::User^ user, MXSM::MultiplayerSession^ session)
{
    for each (MXSM::MultiplayerSessionMember^ member in session->Members)
    {
		if( _wcsicmp(member->XboxUserId->Data(), user->XboxUserId->Data()) == 0)
        {
            return member;
        }
    }

    return nullptr;
}

bool DQRNetworkManager::IsPlayerInSession( Platform::String^ xboxUserId, MXSM::MultiplayerSession^ session, int *smallId )
{
    if( session == nullptr )
    {
        LogComment(L"IsPlayerInSession: invalid session.");
        return false;
    }

    for each (MXSM::MultiplayerSessionMember^ member in session->Members)
    {
        if( _wcsicmp(xboxUserId->Data(), member->XboxUserId->Data() ) == 0 )
        {
			if( smallId )
			{
				// Get small Id for this member
				try
				{
					Windows::Data::Json::JsonObject^ customConstant = Windows::Data::Json::JsonObject::Parse(member->MemberCustomConstantsJson);
					Windows::Data::Json::JsonValue^ customValue = customConstant->GetNamedValue(L"smallId");
					*smallId = (int)(customValue->GetNumber()) & 255;
				}
				catch (Platform::COMException^ ex)
				{
					LogCommentWithError( L"Custom constant Parse/GetNamedValue failed", ex->HResult );
				}
			}

            return true;
        }
    }

    return false;
}

WXM::MultiplayerSessionReference^ DQRNetworkManager::ConvertToWindowsXboxMultiplayerSessionReference(MXSM::MultiplayerSessionReference^ sessionRef )
{
    return ref new WXM::MultiplayerSessionReference(
        sessionRef->SessionName,
        sessionRef->ServiceConfigurationId, 
        sessionRef->SessionTemplateName
        );
}

MXSM::MultiplayerSessionReference^ DQRNetworkManager::ConvertToMicrosoftXboxServicesMultiplayerSessionReference(WXM::MultiplayerSessionReference^ sessionRef )
{
    return ref new MXSM::MultiplayerSessionReference(
        sessionRef->ServiceConfigurationId,
        sessionRef->SessionTemplateName,
        sessionRef->SessionName
        );
}

// This is called on the client, when new room sync data is received. By comparing this with the existing room sync data,
// this method is able to work out who has been added or removed from the game session, and notify the game of these events.
void DQRNetworkManager::UpdateRoomSyncPlayers(RoomSyncData *pNewSyncData)
{
	vector<DQRNetworkPlayer *> tempPlayers;
	vector<DQRNetworkPlayer *> newPlayers;

	EnterCriticalSection(&m_csRoomSyncData);

	// Make temporary vector with all the current players in
	for( int j = 0; j < m_roomSyncData.playerCount; j++ )
	{
		tempPlayers.push_back(m_players[j]);
		m_players[j] = NULL;
	}

	// For each new player, it's either:
	// (1) In the temp player array already, so we already knew about it.
	// (2) Not in the temp player array already, so its a new player. Need to inform the game.
	// And when we are done, anything left in the temporary vector must be a player that left
	for( int i = 0; i < pNewSyncData->playerCount; i++ )
	{
		PlayerSyncData *pNewPlayer = &pNewSyncData->players[i];		
		bool bAlreadyExisted = false;
		for (auto it = tempPlayers.begin(); it != tempPlayers.end(); it++)
		{
			if( pNewPlayer->m_smallId == (*it)->GetSmallId() )
			{
				m_players[i] = (*it);
				it = tempPlayers.erase(it);
				bAlreadyExisted = true;
				break;
			}
		}
		if( !bAlreadyExisted )
		{
			// Create the new player, and tell the game etc. about it - the game is now free to send data via this player since it is active
			if( i == 0 )
			{
				// Player 0 is always the host
				m_players[i] = new DQRNetworkPlayer(this, DQRNetworkPlayer::DNP_TYPE_HOST, false, pNewPlayer->m_channel, pNewPlayer->m_sessionAddress);
			}
			else
			{
				if( pNewPlayer->m_sessionAddress == m_XRNS_LocalAddress )
				{
					m_players[i] = new DQRNetworkPlayer(this, DQRNetworkPlayer::DNP_TYPE_LOCAL, false, pNewPlayer->m_channel, pNewPlayer->m_sessionAddress);
				}
				else
				{
					m_players[i] = new DQRNetworkPlayer(this, DQRNetworkPlayer::DNP_TYPE_REMOTE, false, pNewPlayer->m_channel, pNewPlayer->m_sessionAddress);
				}
			}

			LogCommentFormat(L"Adding new player, index %d - type %d, small Id %d, name %s, xuid %s\n",i,m_players[i]->m_type,pNewPlayer->m_smallId,pNewPlayer->m_name,pNewPlayer->m_XUID);
			
			m_players[i]->SetSmallId(pNewPlayer->m_smallId);
			m_players[i]->SetName(pNewPlayer->m_name);
			m_players[i]->SetUID(PlayerUID(pNewPlayer->m_XUID));
			if (m_players[i]->IsLocal())
			{
				m_players[i]->SetDisplayName(ProfileManager.GetDisplayName(i));
			}

			newPlayers.push_back( m_players[i] );
		}
	}
	for( int i = 0; i < m_roomSyncData.playerCount; i++ )
	{
		delete [] m_roomSyncData.players[i].m_XUID;
	}
	memcpy(&m_roomSyncData, pNewSyncData, sizeof(m_roomSyncData));

	for( int i = 0; i < tempPlayers.size(); i++ )
	{
		m_listener->HandlePlayerLeaving(tempPlayers[i]);
		delete tempPlayers[i];
	}
	for( int i = 0; i < newPlayers.size(); i++ )
	{
		m_listener->HandlePlayerJoined(newPlayers[i]);	// For clients, this is where we get notified of local and remote players joining
	}
	LeaveCriticalSection(&m_csRoomSyncData);
}

// This is called from the host, to add a new player into the room sync data that is then sent out to the clients.
bool DQRNetworkManager::AddRoomSyncPlayer(DQRNetworkPlayer *pPlayer, unsigned int sessionAddress, int channel)
{
	if( m_roomSyncData.playerCount == MAX_ONLINE_PLAYER_COUNT ) return false;
	
	EnterCriticalSection(&m_csRoomSyncData);
	// Find the first entry that isn't us, to decide what to sync before. Don't consider entry #0 as this is reserved to indicate the host.
	int insertAtIdx = m_roomSyncData.playerCount;
	for( int i = 1; i < m_roomSyncData.playerCount; i++ )
	{
		if( m_roomSyncData.players[i].m_sessionAddress != sessionAddress )
		{
			insertAtIdx = i;
			break;
		}
		else
		{
			// Don't add the same local index twice
			if( m_roomSyncData.players[i].m_channel == channel )
			{
				LeaveCriticalSection(&m_csRoomSyncData);
				return false;
			}
		}
	}

	// Make room for a new entry...
	for( int i = m_roomSyncData.playerCount; i > insertAtIdx; i-- )
	{
		m_roomSyncData.players[i] = m_roomSyncData.players[i-1];
		m_players[i] = m_players[i - 1];
	} 
	m_roomSyncData.players[insertAtIdx].m_channel = channel;
	m_roomSyncData.players[insertAtIdx].m_sessionAddress = sessionAddress;
	int xuidLength = pPlayer->GetUID().toString().length() + 1; // +1 for terminator
	m_roomSyncData.players[insertAtIdx].m_XUID = new wchar_t [xuidLength]; 
	wcsncpy(m_roomSyncData.players[insertAtIdx].m_XUID, pPlayer->GetUID().toString().c_str(), xuidLength);
	m_roomSyncData.players[insertAtIdx].m_smallId = pPlayer->GetSmallId();
	wcscpy_s(m_roomSyncData.players[insertAtIdx].m_name, pPlayer->GetName());
	m_players[insertAtIdx] = pPlayer;


	m_roomSyncData.playerCount++;

	LeaveCriticalSection(&m_csRoomSyncData);
	return true;
}

// This is called from the host to remove players from the room sync data that is sent out to the clients.
// This method removes any players sharing a session address, and is used when one machine leaves the network game.
void DQRNetworkManager::RemoveRoomSyncPlayersWithSessionAddress(unsigned int sessionAddress)
{
	EnterCriticalSection(&m_csRoomSyncData);
	vector<DQRNetworkPlayer *> removedPlayers;
	int iWriteIdx = 0;
	for( int i = 0; i < m_roomSyncData.playerCount; i++ )
	{
		if( m_roomSyncData.players[i].m_sessionAddress == sessionAddress )
		{
			removedPlayers.push_back(m_players[i]);
			delete [] m_roomSyncData.players[i].m_XUID;
		}
		else
		{
			m_roomSyncData.players[iWriteIdx] = m_roomSyncData.players[i];
			m_players[iWriteIdx]			  = m_players[i];
			iWriteIdx++;			
		}
	}
	m_roomSyncData.playerCount = iWriteIdx;

	for( int i = 0; i < removedPlayers.size(); i++ )
	{
		m_listener->HandlePlayerLeaving(removedPlayers[i]);
		delete removedPlayers[i];
		memset(&m_roomSyncData.players[m_roomSyncData.playerCount + i], 0, sizeof(PlayerSyncData));
		m_players[m_roomSyncData.playerCount + i] = NULL;
	}
	LeaveCriticalSection(&m_csRoomSyncData);
}

// This is called from the host a remove player from the room sync data that is sent out to the clients.
void DQRNetworkManager::RemoveRoomSyncPlayer(DQRNetworkPlayer *pPlayer)
{
	vector<DQRNetworkPlayer *> removedPlayers;
	int iWriteIdx = 0;
	for( int i = 0; i < m_roomSyncData.playerCount; i++ )
	{
		if( m_players[i] == pPlayer )
		{
			removedPlayers.push_back(m_players[i]);
			delete [] m_roomSyncData.players[i].m_XUID;
		}
		else
		{
			m_roomSyncData.players[iWriteIdx] = m_roomSyncData.players[i];
			m_players[iWriteIdx]			  = m_players[i];
			iWriteIdx++;			
		}
	}
	m_roomSyncData.playerCount = iWriteIdx;

	for( int i = 0; i < removedPlayers.size(); i++ )
	{
		m_listener->HandlePlayerLeaving(removedPlayers[i]);
		delete removedPlayers[i];
		memset(&m_roomSyncData.players[m_roomSyncData.playerCount + i], 0, sizeof(PlayerSyncData));
		m_players[m_roomSyncData.playerCount + i] = NULL;
	}
}

// Broadcast RoomSyncData to all clients (host only)
void DQRNetworkManager::SendRoomSyncInfo()
{
	if( m_isOfflineGame ) return;

	EnterCriticalSection(&m_csRoomSyncData);
	// Calculate the size of data we are going to be sending. This is composed of:
	// (1) 2 byte general header
	// (2) A single byte internal data tag
	// (3) An unsigned int encoding the size of the combined size of all the strings in stage (5)
	// (4) The RoomSyncData structure itself
	// (5) A wchar NULL terminated string for every active player to encode the XUID
	unsigned int xuidBytes = 0;
	for( int i = 0 ; i < m_roomSyncData.playerCount; i++ )
	{
		LogCommentFormat(L"Sending room sync info for player with XUID %s",m_roomSyncData.players[i].m_XUID);
		xuidBytes += ( wcslen(m_roomSyncData.players[i].m_XUID) + 1 ) * sizeof(wchar_t);		// 2 bytes per character, including 0 terminator
	}

	unsigned int internalBytes = 1 + 4 + sizeof(RoomSyncData) + xuidBytes;
	unsigned int totalBytes = 2 + internalBytes;

	unsigned char *data = new unsigned char[totalBytes];

	uint32_t sizeHigh = internalBytes >> 8;
	uint32_t sizeLow = internalBytes & 0xff;

	data[0] = 0x80 | sizeHigh;			// Header - flag as internal data (0x80), sending 
	data[1] = sizeLow;					// Data following has the a single byte to say what it is, followed by the room sync data itself
	data[2] = DQR_INTERNAL_PLAYER_TABLE;

	memcpy(data + 3, &xuidBytes, 4);
	memcpy(data + 7, &m_roomSyncData, sizeof(RoomSyncData));
	unsigned char *pucCurr = data + 7 + sizeof(RoomSyncData); 

	for( int i = 0 ; i < m_roomSyncData.playerCount; i++ )
	{
		unsigned int thisBytes = ( wcslen(m_roomSyncData.players[i].m_XUID) + 1 ) * sizeof(wchar_t);
		memcpy(pucCurr, m_roomSyncData.players[i].m_XUID, thisBytes);
		pucCurr += thisBytes;
	}

	SendBytesRaw(-1, data, totalBytes, true);

	delete [] data;
	LeaveCriticalSection(&m_csRoomSyncData);
}

// Broadcast the fact that joining a particular XUID has failed (host only)
void DQRNetworkManager::SendAddPlayerFailed(Platform::String^ xuid)
{
	if( m_isOfflineGame ) return;

	// Calculate the size of data we are going to be sending. This is composed of:
	// (1) 2 byte general header
	// (2) A single byte internal data tag
	// (3) An unsigned int encoding the size size of the string
	// (5) A wchar NULL terminated string storing the xuid of the player which has failed to join

	unsigned int xuidBytes = sizeof(wchar_t) * ( xuid->Length() + 1 );

	unsigned int internalBytes = 1 + 4 + xuidBytes;
	unsigned int totalBytes = 2 + internalBytes;

	unsigned char *data = new unsigned char[totalBytes];

	uint32_t sizeHigh = internalBytes >> 8;
	uint32_t sizeLow = internalBytes & 0xff;

	data[0] = 0x80 | sizeHigh;			// Header - flag as internal data (0x80), sending 
	data[1] = sizeLow;					// Data following has the a single byte to say what it is, followed by the room sync data itself
	data[2] = DQR_INTERNAL_ADD_PLAYER_FAILED;

	memcpy(data + 3, &xuidBytes, 4);
	memcpy(data + 7, xuid->Data(), xuidBytes); 

	SendBytesRaw(-1, data, totalBytes, true);

	delete [] data;
}

// This method is used by the client to send a small Id to the host. When the host receives this on a particular communication channel,
// it then knows the association between communication channel & small Id, and that a player is actitve.
void DQRNetworkManager::SendSmallId(bool reliableAndSequential, int playerMask)
{
	// Send data with small Id setting mode - see full comment in DQRNetworkManagerEventHandlers::DataReceivedHandler
	BYTE data[8];
	data[0] = 0x80;													// Send 6 bytes, internal mode. Send on channel 0 but this is ignored.
	data[1] = 6;
	data[2] = DQR_INTERNAL_ASSIGN_SMALL_IDS;						// Internal data type
	data[3] = playerMask;											// Actual id
	data[4] = 0;
	data[5] = 0;
	data[6] = 0;
	data[7] = 0;

	bool bError = false;
	for( int j = 0; j < MAX_LOCAL_PLAYER_COUNT; j++ )
	{
		if( playerMask & ( 1 << j ) )
		{
			bool bFound = false;
			for( unsigned int i = 0; i < m_multiplayerSession->Members->Size; i++ )
			{
				MXSM::MultiplayerSessionMember^ member = m_multiplayerSession->Members->GetAt(i);

				if( member->XboxUserId == m_joinSessionXUIDs[j] )
				{
					BYTE smallId = 0;
					try
					{
						Windows::Data::Json::JsonObject^ customConstant = Windows::Data::Json::JsonObject::Parse(member->MemberCustomConstantsJson);
						Windows::Data::Json::JsonValue^ customValue = customConstant->GetNamedValue(L"smallId");
						smallId = (BYTE)(customValue->GetNumber());
						bFound = true;
					}
					catch (Platform::COMException^ ex)
					{
						bError = true;
						LogCommentWithError( L"Custom constant Parse/GetNamedValue failed", ex->HResult );
					}

					m_channelFromSmallId[smallId] = j;

					data[ 4 + j ] = smallId;
					m_connectionInfoClient.m_smallId[ j ] = smallId;
					LogCommentFormat( L"SendSmallId, channel %d, id %d\n", m_channelFromSmallId[smallId], smallId);
				}
			}
			if( !bFound )
			{
				bError = true;
			}
		}
	}

	assert(bError == false );

	SendBytesRaw(0, data, 8, reliableAndSequential);
}

// This is sent by the client to the host, acting to undo a previous SendSmallId call
void DQRNetworkManager::SendUnassignSmallId(int playerIndex)
{
	LogCommentFormat( L"SendUnassignSmallId, channel %d\n", playerIndex);
	// Send data with small Id setting mode - see full comment in DQRNetworkManagerEventHandlers::DataReceivedHandler
	BYTE data[4];
	data[0] = 0x80 | ( playerIndex << 5 );						// Send 1 byte, internal mode
	data[1] = 1;
	data[2] = DQR_INTERNAL_UNASSIGN_SMALL_ID;					// Internal data type

	SendBytesRaw(0, data, 3, true);
}

// This method gets the player index within the session document for a particular small Id.
int DQRNetworkManager::GetSessionIndexForSmallId(unsigned char smallId)
{
	for( unsigned int i = 0; i < m_multiplayerSession->Members->Size; i++ )
	{
		MXSM::MultiplayerSessionMember^ member = m_multiplayerSession->Members->GetAt(i);
		BYTE smallIdMember = 0;
		try
		{
			Windows::Data::Json::JsonObject^ customConstant = Windows::Data::Json::JsonObject::Parse(member->MemberCustomConstantsJson);
			Windows::Data::Json::JsonValue^ customValue = customConstant->GetNamedValue(L"smallId");
			smallIdMember = (BYTE)(customValue->GetNumber());
		}
		catch (Platform::COMException^ ex)
		{
			LogCommentWithError( L"Custom constant Parse/GetNamedValue failed", ex->HResult );
		}
		if( smallIdMember == smallId )
		{
			return i;
		}
	}
	return -1;
}

// This method gets the player index and small id for the host, which is sent with a small id that has 256 added to it
int DQRNetworkManager::GetSessionIndexAndSmallIdForHost(unsigned char *smallId)
{
	for( unsigned int i = 0; i < m_multiplayerSession->Members->Size; i++ )
	{
		MXSM::MultiplayerSessionMember^ member = m_multiplayerSession->Members->GetAt(i);
		int smallIdMember = 0;
		try
		{
			Windows::Data::Json::JsonObject^ customConstant = Windows::Data::Json::JsonObject::Parse(member->MemberCustomConstantsJson);
			Windows::Data::Json::JsonValue^ customValue = customConstant->GetNamedValue(L"smallId");
			smallIdMember = customValue->GetNumber();
		}
		catch (Platform::COMException^ ex)
		{
			LogCommentWithError( L"Custom constant Parse/GetNamedValue failed", ex->HResult );
		}
		if( smallIdMember > 255 )
		{
			*smallId = (BYTE)(smallIdMember);
			return i;
		}
	}
	return -1;
}

// Connection info is used to store the current state of a communcation endpoint, on both host & client
DQRConnectionInfo::DQRConnectionInfo()
{
	Reset();
}

void DQRConnectionInfo::Reset()
{
	m_currentChannel = 0;
	m_internalFlag = false;
	m_bytesRemaining = 0;
	m_internalDataState = ConnectionState_InternalHeaderByte;
	for( int i = 0; i < 4; i++ )
	{
		m_smallId[i] = 0;
		m_channelActive[i] = false;
	}
	m_state = ConnectionState_HeaderByte0;
	m_initialPacketReceived = false;
}

// This method allocates the next available small id, returning it as a json formatted named value so that it can be inserted as custom data in the session document
Platform::String^ DQRNetworkManager::GetNextSmallIdAsJsonString()
{
	Windows::Data::Json::JsonObject^ customConstant = ref new Windows::Data::Json::JsonObject();

	// The host sends it small Id with 256 added to it, so we can determine which player is the host easily at the client side
	int smallIdToSend = m_currentSmallId;
	if( smallIdToSend == m_hostSmallId )
	{
		smallIdToSend += 256;
	}
	customConstant->Insert(L"smallId",  Windows::Data::Json::JsonValue::CreateNumberValue( smallIdToSend ));
	m_sessionAddressFromSmallId[m_currentSmallId++] = 0;

	return customConstant->Stringify();
}

int DQRNetworkManager::_HostGameThreadProc( void* lpParameter )
{
	DQRNetworkManager *pDQR = (DQRNetworkManager *)lpParameter;
	return pDQR->HostGameThreadProc();
}

// This is the main asynchronous method that is called when hosting a game (initiated by calling ::createAndJoinSession). This is called for
// both online & offline games, and it must:
// (1) Create a new multiplayer session document, with active players for all local players starting the game
// (2) Create a new game party, with matching players in it (possibly clearing any existing party)
// (3) Get a device token for the host & assign to the session
// (4) Initialise the room sync data, and inform the game of all local players joining at this stage

int DQRNetworkManager::HostGameThreadProc()
{
	Platform::String^ sessionName;

	// Get primary user that we are going to create the session with, and create an xbox live context from it

	WXS::User^ primaryUser = ProfileManager.GetUser(0);
	m_primaryUser = primaryUser;
	m_primaryUserXboxLiveContext = nullptr;
	if( primaryUser == nullptr )
	{
		SetState(DNM_INT_STATE_HOSTING_FAILED);
		return 0;
	}

	int localSessionAddress = 0;

	if( !m_isOfflineGame )
	{
		MXS::XboxLiveContext^ primaryUserXBLContext = ref new MXS::XboxLiveContext(primaryUser);
		m_primaryUserXboxLiveContext = primaryUserXBLContext;
		if( primaryUserXBLContext == nullptr )
		{
			SetState(DNM_INT_STATE_HOSTING_FAILED);
			return 0;
		}

		EnableDebugXBLContext(m_primaryUserXboxLiveContext);

		// Get a globally unique identifier to use as our session name that we are going to create
		GUID sessionNameGUID;
		CoCreateGuid( &sessionNameGUID );
		Platform::Guid sessionGuidName = Platform::Guid( sessionNameGUID );
		sessionName = RemoveBracesFromGuidString( sessionGuidName.ToString() );

		MXSM::MultiplayerSession^ session = nullptr;
		// Actually create the session (locally), using the primary player's context
		try
		{
			session = ref new MXSM::MultiplayerSession( primaryUserXBLContext, 
														ref new MXSM::MultiplayerSessionReference( SERVICE_CONFIG_ID, MATCH_SESSION_TEMPLATE_NAME, sessionName ),
														0, // this means that it will use the maxMembers specified in the session template.
														false,
														MXSM::MultiplayerSessionVisibility::Open,
														nullptr,
														nullptr
														);
		}
		catch (Platform::COMException^ ex)
		{
			SetState(DNM_INT_STATE_HOSTING_FAILED);
			return 0;
		}

		// Set custom property with the game session data
		session->SetSessionCustomPropertyJson( L"GameSessionData", Windows::Data::Json::JsonValue::CreateStringValue( base64_encode(m_customSessionData, m_customSessionDataSize ) )->Stringify() );

		// More custom data, for the XUIDs of the players to match our room sync data. This isn't itself set up at t:his point but we know what is going in it.
		Windows::Data::Json::JsonArray ^currentXuidArray = ref new Windows::Data::Json::JsonArray();
		for( int i = 0 ; i < MAX_LOCAL_PLAYER_COUNT; i++ )
		{
			if( m_currentUserMask & ( 1 << i ) )
			{
				WXS::User^ newUser = ProfileManager.GetUser(i);
				if( newUser != nullptr )
				{
					currentXuidArray->Append( Windows::Data::Json::JsonValue::CreateStringValue( newUser->XboxUserId ) );
				}
				else
				{
					SetState(DNM_INT_STATE_HOSTING_FAILED);
					return 0;
				}
			}
		}
		session->SetSessionCustomPropertyJson( L"RoomSyncData", currentXuidArray->Stringify() );

		// Join session with the primary user for whom the session was created (via their xbox live context)
		// user to store the small Id
		m_hostSmallId = m_currentSmallId;
		m_sessionAddressFromSmallId[m_hostSmallId] = 0;

		session->Join( GetNextSmallIdAsJsonString(), true );
		session->SetCurrentUserStatus( MXSM::MultiplayerSessionMemberStatus::Active );
    
		// Get device ID for current user & set in the session
		Platform::String^ secureDeviceAddress =  WXN::SecureDeviceAddress::GetLocal()->GetBase64String();
		session->SetCurrentUserSecureDeviceAddressBase64( secureDeviceAddress );
		LogComment(L"Setting host secure device: " + secureDeviceAddress + L"\n");

		// If there is a party currently, then remove all our local players from it so that we can start our own one
		m_partyController->RefreshPartyView();
		WXM::PartyView^ partyView = m_partyController->GetPartyView();

		if( partyView != nullptr )
		{
			m_partyController->RemoveLocalUsersFromParty(primaryUser);
		}

		m_partyController->AddLocalUsersToParty(m_currentUserMask, primaryUser);
		partyView = m_partyController->GetPartyView();

		// If there is no party by this stage, then we can't proceed.
		if( partyView == nullptr )
		{
			SetState(DNM_INT_STATE_HOSTING_FAILED);
			return 0;
		}
		m_partyController->SetJoinability(m_listener->IsSessionJoinable());
	
		// Add reservations for anyone in the party, who isn't the primary player. Just adding local players for now, but perhaps this should add
		// other party members at this stage?
		for ( WXM::PartyMember^ member : partyView->Members )
		{
			if( member->IsLocal )
			{
				if ( _wcsicmp(primaryUser->XboxUserId->Data(), member->XboxUserId->Data()) != 0)
				{
					session->AddMemberReservation( member->XboxUserId, GetNextSmallIdAsJsonString(), true );
					LogCommentFormat( L"Added %s to session\n", member->XboxUserId->Data());
				}
			}
		}

		// This is everything now set up locally as we want to start the session. Now attempt to write the session data to the server - will return a valid new session object if we succeeeded.
		HRESULT hr = S_OK;
		session = WriteSessionHelper( primaryUserXBLContext, session, MXSM::MultiplayerSessionWriteMode::UpdateOrCreateNew, hr );

		// It is important to set the session as soon as we have written it, so that if we get any incoming events (such as the game session ready one) then we will be aware that we are actually already in the session
		HandleSessionChange(session);

		// If this was successful, then do further set up of the session
		if( session != nullptr )
		{
			// Get reference to the current user within the session
			MXSM::MultiplayerSessionMember^ hostMember = GetUserMemberInSession(primaryUser, session);

			// Set the device token of the host from this user (since we're hosting)
			session->SetHostDeviceToken( hostMember->DeviceToken );

			m_partyController->RegisterGamePlayersChangedEventHandler();
	
			// Update session on the server    
			HRESULT hr = S_OK;
			session = WriteSessionHelper( primaryUserXBLContext, session, MXSM::MultiplayerSessionWriteMode::UpdateExisting, hr );

			if ( FAILED(hr) )
			{
				app.DebugPrintf("Failed setting host device token");

				SetState(DNM_INT_STATE_HOSTING_FAILED);
				return 0;
			}

			// Convert the session reference (in Microsoft::Xbox::Services::Multiplayer namespace) to Windows::Xbox::Multiplayer names space so we can use in the party system
			WXM::MultiplayerSessionReference^ winSessionRef = ConvertToWindowsXboxMultiplayerSessionReference(session->SessionReference);

			// Register this multiplayer session as the current session for the party
			auto registerSessionAsync = WXM::Party::RegisterGameSessionAsync(primaryUser, winSessionRef);
			create_task(registerSessionAsync).then([this](task<void> t)
			{
				try
				{
					t.get(); // if t.get() didn't throw, it succeeded
				}
				catch (Platform::COMException^ ex)
				{
					LogCommentWithError( L"RegisterGameSessionAsync failed", ex->HResult );

					SetState(DNM_INT_STATE_HOSTING_FAILED);
				}
			})
			.wait();
			if( m_state == DNM_INT_STATE_HOSTING_FAILED) return 0;

			m_partyController->RefreshPartyView();

			// We've now created the session with our local player in it, and reserved slots for everyone else in the party. We've also set the party to say that
			// the current game the party are playing is this session. We can now request that the reserved player's be pulled into the game. For people not currently running the
			// game, this will ask them if they want to start playing, and for people already in the title it will send an even to say that the game is ready.
			auto pullPlayersAsync = WXM::Party::PullReservedPlayersAsync(primaryUser, winSessionRef);
			create_task(pullPlayersAsync).then([this](task<void> t)
			{
				try
				{
					t.get(); // if t.get() didn't throw, it succeeded
				}
				catch (Platform::COMException^ ex)
				{
					LogCommentWithError( L"PullReservedPlayersAsync failed", ex->HResult );

	//				SetState(DNM_INT_STATE_HOSTING_FAILED);			// This does seem to fail (harmlessly?) with a code of 0x87cc0008 sometimes
				}
			})
			.wait();
			if( m_state == DNM_INT_STATE_HOSTING_FAILED) return 0;

			sockaddr_in6 localSocketAddressStorage;
    
			ZeroMemory(&localSocketAddressStorage, sizeof(localSocketAddressStorage));
    
			localSocketAddressStorage.sin6_family = AF_INET6;
			localSocketAddressStorage.sin6_port = htons(m_associationTemplate->AcceptorSocketDescription->BoundPortRangeLower);
    
			memcpy(&localSocketAddressStorage.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    
			m_localSocketAddress = Platform::ArrayReference<BYTE>(reinterpret_cast<BYTE*>(&localSocketAddressStorage), sizeof(localSocketAddressStorage));

			// This shouldn't ever happen, but seems worth checking that we don't have a pre-existing session in case there's any way to get here with one already running
			if( m_XRNS_Session )
			{
				RTS_Terminate();
				do
				{
					Sleep(20);
				} while ( m_XRNS_Session != nullptr );
			}

			// Start a new session (this actually happens in the RTS processing thread), and wait until it exists
			RTS_StartHost();
			do
			{
				Sleep(20);
			} while ( m_XRNS_Session == nullptr );

			// Set any other local players that we added as reserved in the session, to be active. This must be done by getting and writing the multiplayer session
			// document from each player's xbox live context in turn

			for( int i = 1; i < 4; i++ )
			{
				if( m_currentUserMask & ( 1 << i ) )
				{
					WXS::User^ extraUser = ProfileManager.GetUser(i);
					if( extraUser == nullptr )
					{
						SetState(DNM_INT_STATE_HOSTING_FAILED);
						return 0;
					}
					MXS::XboxLiveContext^ extraUserXBLContext = ref new MXS::XboxLiveContext(extraUser);
					if( extraUserXBLContext == nullptr )
					{
						SetState(DNM_INT_STATE_HOSTING_FAILED);
						return 0;
					}

					auto asyncOp = extraUserXBLContext->MultiplayerService->GetCurrentSessionAsync( session->SessionReference );
					create_task(asyncOp).then([this, extraUserXBLContext, &session] (task<MXSM::MultiplayerSession^> t)
					{
						try
						{
							MXSM::MultiplayerSession^ currentSession = t.get();

							currentSession->SetCurrentUserStatus(MXSM::MultiplayerSessionMemberStatus::Active);
							HRESULT hr = S_OK;
							session =  WriteSessionHelper(extraUserXBLContext, currentSession, MXSM::MultiplayerSessionWriteMode::UpdateExisting, hr);
						}
						catch ( Platform::COMException^ ex )
						{
						}
					}).wait();
				}
			}

			HandleSessionChange(session);
		}
		else
		{
			app.DebugPrintf("Error creating session");
			SetState(DNM_INT_STATE_HOSTING_FAILED);
			return 0;
		}
		m_XRNS_LocalAddress = m_XRNS_Session->LocalSessionAddress;
		m_XRNS_OldestAddress = m_XRNS_Session->OldestSessionAddress;
		localSessionAddress = m_XRNS_Session->LocalSessionAddress;
	}
	else
	{
		m_hostSmallId = m_currentSmallId;
		// Offline game - get small id incremented to the same value that it would have ended up with
		for( int i = 0; i < 4; i++ )
		{
			if( m_currentUserMask & ( 1 << i ) )
			{
				m_currentSmallId++;
			}
		}
	}

	// At this stage, set the local players up. We know we'll have created these with smallIds from m_hostSmallId (for the host) to a max of m_hostSmallId+3 for the other local players
	int smallId = m_hostSmallId;
	for( int i = 0; i < 4; i++ )
	{
		// If user is present in mask and currently signed in
		if( m_currentUserMask & ( 1 << i ) && ProfileManager.IsSignedIn(i))
		{
			auto user = ProfileManager.GetUser(i);
			wstring displayName = ProfileManager.GetDisplayName(i); 

			DQRNetworkPlayer* pPlayer = new DQRNetworkPlayer(this, ( ( smallId == m_hostSmallId ) ? DQRNetworkPlayer::DNP_TYPE_HOST : DQRNetworkPlayer::DNP_TYPE_LOCAL ), true, i, localSessionAddress);
			pPlayer->SetSmallId(smallId);
			pPlayer->SetName(user->DisplayInfo->Gamertag->Data());
			pPlayer->SetDisplayName(displayName);
			pPlayer->SetUID(PlayerUID(user->XboxUserId->Data()));

			AddRoomSyncPlayer( pPlayer, localSessionAddress, i);

			m_listener->HandlePlayerJoined(pPlayer);

			smallId++;
		}
	}

	SetState(DNM_INT_STATE_HOSTING_WAITING_TO_PLAY);

	return 0;
}

int DQRNetworkManager::_LeaveRoomThreadProc( void* lpParameter )
{

	DQRNetworkManager *pDQR = (DQRNetworkManager *)lpParameter;
	return pDQR->LeaveRoomThreadProc();
}

int DQRNetworkManager::LeaveRoomThreadProc()
{
	WXS::User^ primaryUser = ProfileManager.GetUser(0, true);

	LogComment(L"LeaveRoomThreadProc");

	if( !m_isOfflineGame && m_multiplayerSession != nullptr )
	{
		// If we are hosting, then we need to disassociate the gaming session from the party. We also need to make sure that we don't subscribe to gameplayer events anymore
		// or else if we subsequently become a client in another game, things will get confused
		if( m_isHosting )
		{
			m_partyController->DisassociateSessionFromParty( m_multiplayerSession->SessionReference );

			m_partyController->UnregisterGamePlayersEventHandler();
		}

		// Remove local players from the party
		m_partyController->RemoveLocalUsersFromParty(primaryUser, m_currentUserMask, m_multiplayerSession->SessionReference);

		// Request RTS to be terminated
		RTS_Terminate();
	
		// Now leave the game session. We need to do this for each player in turn, writing each time
		bool bError = false;
		for( int i = 0; i < 4; i++ )
		{
			if( m_currentUserMask & ( 1 << i ) )
			{
				LogCommentFormat(L"DQRNetworkManager::LeaveRoomThreadProc: Attempting to remove player %d from session",i);
				WXS::User^ leavingUser = ProfileManager.GetUser(i);
				if( leavingUser == nullptr )
				{
					bError = true;
					continue;
				}
				if( m_chat )
				{
					m_chat->RemoveLocalUser(leavingUser);
				}

				MXS::XboxLiveContext^ leavingUserXBLContext = ref new MXS::XboxLiveContext(leavingUser);
				if( leavingUserXBLContext == nullptr )
				{
					bError = true;
					continue;
				}

				auto asyncOp = leavingUserXBLContext->MultiplayerService->GetCurrentSessionAsync( m_multiplayerSession->SessionReference );
				create_task(asyncOp).then([this, leavingUserXBLContext,&bError] (task<MXSM::MultiplayerSession^> t)
				{
					try
					{
						MXSM::MultiplayerSession^ currentSession = t.get();

						if (currentSession != nullptr)
						{
							currentSession->Leave();
							HRESULT hr = S_OK;
							WriteSessionHelper(leavingUserXBLContext, currentSession, MXSM::MultiplayerSessionWriteMode::UpdateExisting, hr);
						}
						else
						{
							// Specific error case where session is gone, this generally happens if all players have left (e.g. party of one and player signs out)
							app.DebugPrintf("DQRNetworkManager::LeaveRoomThreadProc: Error removing a player from the session, session was null (user probably signed out)");
						}
					}
					catch ( Platform::COMException^ ex )
					{
						bError = true;
					}
				}).wait();
			}
		}

		if ( bError )
		{
			app.DebugPrintf("DQRNetworkManager::LeaveRoomThreadProc: Error removing a player from the session");
			assert(true);
		}
	}

	ProfileManager.CompleteDeferredSignouts();
	app.DebugPrintf("DQRNetworkManager::LeaveRoomThreadProc: Completing deferred sign out");
	ProfileManager.SetDeferredSignoutEnabled(false);

	m_multiplayerSession = nullptr;
	m_currentUserMask = 0;
	m_joinSessionUserMask = 0;
	UnflagInvitedToFullSession();

	SetState(DNM_INT_STATE_ENDING);

	return 0;
}

int DQRNetworkManager::_TidyUpJoinThreadProc( void* lpParameter )
{

	DQRNetworkManager *pDQR = (DQRNetworkManager *)lpParameter;
	return pDQR->TidyUpJoinThreadProc();
}

// This method is called when joining the game fails in some situations, ie when we attempted to join with a
// set of players and not all managed to get into the session. Tidies things up by removing any players that
// Did get into the session, and removing any players we added to the party.
int DQRNetworkManager::TidyUpJoinThreadProc()
{
	WXS::User^ primaryUser = ProfileManager.GetUser(0);

	LogComment(L"TidyUpJoinThreadProc");

	if( primaryUser != nullptr )
	{
		// Remove any local users at all who are in the party - but first get a session reference from the party whilst we still have it
		m_partyController->RefreshPartyView();
		MXSM::MultiplayerSessionReference ^sessionRef = m_partyController->GetGamePartySessionReference();
		m_partyController->RemoveLocalUsersFromParty(primaryUser);

		if( sessionRef != nullptr )
		{
			// Now leave the game session. We need to do this for each player in turn, writing each time. Consider that any of the joining
			// members *may* have a slot (reserved or active) depending on how far progressed the joining got.
			bool bError = false;

			// We can fail to join at various points, and in at least one case (if it is down to RUDP unreliable packets timing out) then we don't have m_joinSessionUserMask bits set any more,
			// but we Do have m_currentUserMask set. Any of these should be considered users we should be attempting to remove from the session.
			int removeSessionMask = m_joinSessionUserMask | m_currentUserMask;		
			for( int i = 0; i < 4; i++ )
			{
				if( removeSessionMask & ( 1 << i ) )
				{
					LogCommentFormat(L"Attempting to remove player %d from session",i);
					WXS::User^ leavingUser = ProfileManager.GetUser(i);
					if( leavingUser == nullptr )
					{
						bError = true;
						continue;
					}
					MXS::XboxLiveContext^ leavingUserXBLContext = ref new MXS::XboxLiveContext(leavingUser);
					if( leavingUserXBLContext == nullptr )
					{
						bError = true;
						continue;
					}

					EnableDebugXBLContext(leavingUserXBLContext);

					auto asyncOp = leavingUserXBLContext->MultiplayerService->GetCurrentSessionAsync( sessionRef );
					create_task(asyncOp).then([this, leavingUser, leavingUserXBLContext,&bError] (task<MXSM::MultiplayerSession^> t)
					{
						try
						{
							MXSM::MultiplayerSession^ currentSession = t.get();

							if( currentSession )
							{
								bool bFound = false;
								for( int i = 0; i < currentSession->Members->Size; i++ )
								{
									if( currentSession->Members->GetAt(i)->XboxUserId == leavingUser->XboxUserId )
									{
										bFound = true;
										break;
									}
								}

								if( bFound )
								{
									currentSession->Leave();
									HRESULT hr = S_OK;
									WriteSessionHelper(leavingUserXBLContext, currentSession, MXSM::MultiplayerSessionWriteMode::UpdateExisting, hr);
								}
							}
						}
						catch ( Platform::COMException^ ex )
						{
							bError = true;
						}
					}).wait();
				}
			}

			if ( bError )
			{
				app.DebugPrintf("Error removing a player from the session");
				assert(true);
			}
		}
	}

	m_multiplayerSession = nullptr;
	m_currentUserMask = 0;
	m_joinSessionUserMask = 0;

	// Fixing up things from joining needs to go straight from joining -> idle to be properly detected as a failed join by the higher-level networking systems
	SetState(DNM_INT_STATE_IDLE);

	return 0;
}

int DQRNetworkManager::_UpdateCustomSessionDataThreadProc( void* lpParameter )
{

	DQRNetworkManager *pDQR = (DQRNetworkManager *)lpParameter;
	return pDQR->UpdateCustomSessionDataThreadProc();
}

// This method is called to updat the custom session data associated with the multiplayer session. This is done only on the host. The custom data is specified
// when we create the game session (when calling CreateAndJoinSession) as a region of memory so that this layer doesn't have to be concerned with what it
// represents, however we use it to synchronise a GameSessionData structure containing details of the current game session. This data is base 64 encoded
// and then put in the session document as a custom value as a json string name/value pair.
int DQRNetworkManager::UpdateCustomSessionDataThreadProc()
{
	LogComment(L"Starting thread to update custom data");
	WXS::User^ primaryUser = ProfileManager.GetUser(0);
	
	if( primaryUser == nullptr )
	{
		return 0;
	}
	MXS::XboxLiveContext^ primaryUserXBLContext = ref new MXS::XboxLiveContext(primaryUser);
	if( primaryUserXBLContext == nullptr )
	{
		return 0;
	}

	MXSM::MultiplayerSession^ session = nullptr;

	auto multiplayerSessionAsync = primaryUserXBLContext->MultiplayerService->GetCurrentSessionAsync( m_multiplayerSession->SessionReference );
	create_task(multiplayerSessionAsync).then([&session,this](task<MXSM::MultiplayerSession^> t)
	{
		try
		{
			session = t.get(); // if t.get() didn't throw, it succeeded
		}
		catch (Platform::COMException^ ex)
		{
			LogCommentWithError( L"MultiplayerSession failed", ex->HResult );
		}
	})
	.wait();

	if( session != nullptr )
	{
		// Set custom property with the game session data
		session->SetSessionCustomPropertyJson( L"GameSessionData", Windows::Data::Json::JsonValue::CreateStringValue( base64_encode(m_customSessionData, m_customSessionDataSize ) )->Stringify() );

		// Update XUIDs from room sync data which we also have as custom data
		EnterCriticalSection(&m_csRoomSyncData);
		Windows::Data::Json::JsonArray ^currentXuidArray = ref new Windows::Data::Json::JsonArray();
		for( int i = 0 ; i < m_roomSyncData.playerCount; i++ )
		{
			currentXuidArray->Append( Windows::Data::Json::JsonValue::CreateStringValue( ref new Platform::String(m_roomSyncData.players[i].m_XUID ) ) );
		}
		session->SetSessionCustomPropertyJson( L"RoomSyncData", currentXuidArray->Stringify() );
		LeaveCriticalSection(&m_csRoomSyncData);

		HRESULT hr = S_OK;
		WriteSessionHelper( primaryUserXBLContext, session, MXSM::MultiplayerSessionWriteMode::UpdateExisting, hr );

		// If we didn't succeed, then retry later
		if( hr != S_OK )
		{
			if( m_customDataDirtyUpdateTicks == 0 )
			{
				LogCommentFormat(L"Error updating custom data 0x%x, will retry", hr);
				m_customDataDirtyUpdateTicks = 20;
			}
		}
	}

	LogComment(L"Ending thread to update custom data");

	return 0;
}

int DQRNetworkManager::_CheckInviteThreadProc(void* lpParameter)
{
	DQRNetworkManager *pDQR = (DQRNetworkManager *)lpParameter;
	return pDQR->CheckInviteThreadProc();
}

int DQRNetworkManager::CheckInviteThreadProc()
{
	for( int i = 4; i < 12; i++ )
	{
		WXS::User^ anyUser = InputManager.GetUserForGamepad(i);
		if( anyUser )
		{
			m_partyController->CheckPartySessionFull(anyUser);
			return 0;
		}
	}
	return 0;
}

// This method is called by the the party controller if a new party is found for a local player. This will happen after the party system
// has called HandlePlayerRemovedFromParty, if the player is being removed from one party only to be placed in another. If Only
// HandlePlayerRemovedFromParty is called (with no following HandleNewPartyFoundForPlayer), then we must assume that the player was
// just removed from a party, and Isn't moving to another party. We try to differentiate between these two events by allowing a window of time
// to pass after a user is removed from a party before processing it.
void DQRNetworkManager::HandleNewPartyFoundForPlayer()
{
	LogCommentFormat(L"HandleNewPartyFoundForPlayer called after %d ms\n", System::currentTimeMillis() - m_playersLeftPartyTime);
	m_playersLeftParty = 0;
}

void DQRNetworkManager::HandlePlayerRemovedFromParty(int playerMask)
{
	if( m_state == DNM_INT_STATE_PLAYING )
	{
		if( m_isHosting )
		{
			// We should check that they're still here, they might already have left
			for( int i = 0; i < MAX_LOCAL_PLAYER_COUNT; i++ )
			{
				if( playerMask & ( 1 << i ) )
				{
					if (!ProfileManager.IsSignedIn(i))
					{
						// Player is already gone so remove them from the mask
						playerMask &= ~(1 << i);
					}
				}
			}

			LogCommentFormat(L"HandlePlayerRemovedFromParty called mask %d\n", playerMask);
			m_playersLeftParty |= playerMask;
			m_playersLeftPartyTime = System::currentTimeMillis();

			// Check for forced sign out
			CheckForcedSignOut();
		}
		else
		{
			// As a client, we don't have any messy changing to offline game or saving etc. to do, so we can respond immediately to leaving the party
			if( playerMask & 1 )
			{			
				DQRNetworkManager::LogComment(L"Primary player on this system has left the party - leaving game\n");
				app.SetDisconnectReason(DisconnectPacket::eDisconnect_ExitedGame);
				LeaveRoom();
			}
			else
			{
				// Secondary player(s) leaving, just remove as if they had chosen to exit themselves from the game
				for( int i = 0; i < MAX_LOCAL_PLAYERS; i++ )
				{
					if( playerMask & ( 1 << i ) )
					{
						RemoveLocalPlayerByUserIndex(i);
					}
				}
			}
		}
	}
	else
	{
		LogComment(L"HandlePlayerRemovedFromParty called, ignoring as not in state DNM_INT_STATE_PLAYING\n");
	}
}

// Method called by the DQRNetworkManager when we need to inform the chat system that a new player has been added to the game. We don't do anything
// directly here, as this ends up getting called whilst we are in a handler for receiving network data, and telling the chat system that a player
// has joined causes it to direct attempt to send data on the network, causing us to lock up. This is processed in the tick instead.
void DQRNetworkManager::ChatPlayerJoined(int idx)
{
	EnterCriticalSection(&m_csVecChatPlayers);
	m_vecChatPlayersJoined.push_back(idx);
	LeaveCriticalSection(&m_csVecChatPlayers);
}

bool DQRNetworkManager::IsReadyToPlayOrIdle()
{
	return (( m_state == DNM_INT_STATE_HOSTING_WAITING_TO_PLAY ) || ( m_state == DNM_INT_STATE_PLAYING ) || ( m_state == DNM_INT_STATE_IDLE ) );
}

void DQRNetworkManager::StartGame()
{
	SetState( DNM_INT_STATE_STARTING);
	SetState( DNM_INT_STATE_PLAYING);
}

// Removes all local players from a network game (aysnchronously) and leaves the game
void DQRNetworkManager::LeaveRoom()
{
	m_playersLeftParty = 0;
	if( ( m_state == DNM_INT_STATE_HOSTING_WAITING_TO_PLAY ) ||
		( m_state == DNM_INT_STATE_STARTING ) ||
		( m_state == DNM_INT_STATE_PLAYING ) )
	{
		SetState(DNM_INT_STATE_LEAVING);

		// Empty out the room of players & notify the game (needs separate loops for each as HandlePlayerLeaving checks the players)
		for( int i = 0; i < m_roomSyncData.playerCount; i++ )
		{
			m_listener->HandlePlayerLeaving(m_players[i]);
		}

		for( int i = 0; i < m_roomSyncData.playerCount; i++ )
		{
			delete m_players[i];
			m_players[i] = NULL;
		}
		memset(&m_roomSyncData, 0, sizeof(m_roomSyncData));
		m_displayNames.clear();

		m_LeaveRoomThread = new C4JThread(&DQRNetworkManager::_LeaveRoomThreadProc, this, "Leave room");
		m_LeaveRoomThread->Run();
	}
	else
	{
		SetState(DNM_INT_STATE_IDLE);
	}
}

void DQRNetworkManager::TidyUpFailedJoin()
{
	m_TidyUpJoinThread = new C4JThread(&DQRNetworkManager::_TidyUpJoinThreadProc, this, "Tidying up failed join");
	m_TidyUpJoinThread->Run();
}

// This is called when we get notification that the user has accepted an invite toast - this is a good point to check the session that the party is associated with,
// and see if it is full of people that aren't us
void DQRNetworkManager::SetInviteReceivedFlag()
{
	m_inviteReceived = true;
}

// The "Party process" is picked up in the tick and used to join games from invites etc. This method is static so it can be called from bits of the game that aren't really tied in with the network libs.
void DQRNetworkManager::SetPartyProcessJoinParty()
{
	m_partyProcess = DQRNetworkManager::DNM_PARTY_PROCESS_JOIN_PARTY;
}

// The "Party process" is picked up in the tick and used to join games from invites etc. This method is static so it can be called from bits of the game that aren't really tied in with the network libs.
void DQRNetworkManager::SetPartyProcessJoinSession(int bootUserIndex, Platform::String^ bootSessionName, Platform::String^ bootServiceConfig, Platform::String^ bootSessionTemplate)
{
	if( s_pDQRManager )
	{
		// Don't do anything if we are already in this session
		if( s_pDQRManager->m_multiplayerSession )
		{
			if( s_pDQRManager->m_multiplayerSession->SessionReference->SessionName == bootSessionName )
			{
				return;
			}
		}
	}
	m_bootUserIndex				= bootUserIndex;
	m_bootSessionName			= bootSessionName->Data();
	m_bootServiceConfig			= bootServiceConfig->Data();
	m_bootSessionTemplate		= bootSessionTemplate->Data();
	m_partyProcess				= DQRNetworkManager::DNM_PARTY_PROCESS_JOIN_SPECIFIED;
}

// Brings up the system GUI so that invites can be sent to invite friends to join the current game party
void DQRNetworkManager::SendInviteGUI(int quadrant)
{
	Windows::Xbox::UI::SystemUI::ShowSendInvitesAsync(ProfileManager.GetUser(quadrant));
}

bool DQRNetworkManager::IsAddingPlayer()
{
	return ( m_addLocalPlayerState != DNM_ADD_PLAYER_STATE_IDLE );
}

// Attempt to join a party that we have found, for a given set of local players. This adds the specified users to the
// party, and then we have to wait for the host to detect this change, and (hopefully) reserve slots for us in the game session. When our
// slots are reserved, we will receive a game session ready notification through the party manager & can take things from there.
bool DQRNetworkManager::JoinPartyFromSearchResult(SessionSearchResult *searchResult, int playerMask)
{
	// Assume that primary player is always player 0
	if( ( playerMask & 1 ) == 0 )
	{
		return false;
	}

	// Gather set of XUIDs for the players that we are joining with
	for( int i = 0; i < MAX_LOCAL_PLAYER_COUNT; i++ )
	{
		if( playerMask & ( 1 << i ) )
		{
			WXS::User^ user = ProfileManager.GetUser(i);
			if( user == nullptr )
			{
				return false;
			}
			m_joinSessionXUIDs[i] = user->XboxUserId;
		}
		else
		{
			m_joinSessionXUIDs[i] = nullptr;
		}
	}

	// Set up primary user & context to be used generally by other things once we are in the game
	m_primaryUser = ProfileManager.GetUser(0);
	if( m_primaryUser == nullptr )
	{
		return false;
	}

	m_primaryUserXboxLiveContext = ref new MXS::XboxLiveContext(m_primaryUser);
	if( m_primaryUserXboxLiveContext == nullptr )
	{
		return false;
	}

	m_currentUserMask = 0;
	m_joinSessionUserMask = playerMask;
	m_isInSession = true;
	m_isOfflineGame = false;
	
	m_startedWaitingForReservationsTime = System::currentTimeMillis();
	SetState(DNM_INT_STATE_JOINING_WAITING_FOR_RESERVATIONS);

	// There is a small window that we currently allow cancelling
	m_cancelJoinFromSearchResult = false;

	// Before we join the party, check if any of the joining players are in the session already, and remove. Joining the game cleanly depends
	// on the host detecting us joining the party and then adding us (as reserved) to the session so it can pull us in, which it can't do
	// if we are already in the session

	MXSM::MultiplayerSessionReference ^sessionRef = ref new MXSM::MultiplayerSessionReference(SERVICE_CONFIG_ID, MATCH_SESSION_TEMPLATE_NAME, ref new Platform::String(searchResult->m_sessionName.c_str()));

	bool shownCancelScreen = false;
	if( sessionRef != nullptr )
	{
		// Allow 2 seconds before we let the player cancel
		__int64 allowCancelTime = System::currentTimeMillis() + (1000 * 2);

		// Now leave the game session. We need to do this for each player in turn, writing each time. Consider that any of the joining
		// members *may* have a slot (reserved or active) depending on how far progressed the joining got.
		bool bError = false;
		for( int i = 0; i < 4; i++ )
		{
			if( playerMask & ( 1 << i ) )
			{
				LogCommentFormat(L"Attempting to remove player %d from session",i);
				WXS::User^ leavingUser = ProfileManager.GetUser(i);
				if( leavingUser == nullptr )
				{
					bError = true;
					continue;
				}
				MXS::XboxLiveContext^ leavingUserXBLContext = ref new MXS::XboxLiveContext(leavingUser);
				if( leavingUserXBLContext == nullptr )
				{
					bError = true;
					continue;
				}

				EnableDebugXBLContext(leavingUserXBLContext);

				auto asyncOp = leavingUserXBLContext->MultiplayerService->GetCurrentSessionAsync( sessionRef );
				auto ccTask = create_task(asyncOp).then([this, leavingUser, leavingUserXBLContext,&bError] (task<MXSM::MultiplayerSession^> t)
				{
					try
					{
						MXSM::MultiplayerSession^ currentSession = t.get();

						if( currentSession == nullptr )
						{
							bError = true;
						}
						else
						{
							bool bFound = false;
							for( int i = 0; i < currentSession->Members->Size; i++ )
							{
								if( currentSession->Members->GetAt(i)->XboxUserId == leavingUser->XboxUserId )
								{
									bFound = true;
									break;
								}
							}

							if( bFound )
							{
								currentSession->Leave();
								HRESULT hr = S_OK;
								WriteSessionHelper(leavingUserXBLContext, currentSession, MXSM::MultiplayerSessionWriteMode::UpdateExisting, hr);
							}
						}
					}
					catch ( Platform::COMException^ ex )
					{
						bError = true;
					}
				});

				while(!ccTask.is_done())
				{
					// Check for being disconnected from the network
					if(!ProfileManager.IsSignedInLive(i) || m_cancelJoinFromSearchResult)
					{
						asyncOp->Cancel();
						bError = true;
						break;
					}

					__int64 currentTime = System::currentTimeMillis();
					if( currentTime > allowCancelTime)
					{
						shownCancelScreen = true;

						ConnectionProgressParams *param = new ConnectionProgressParams();
						param->iPad = 0;
						param->stringId = -1;
						param->showTooltips = true;
						param->cancelFunc = &g_NetworkManager.CancelJoinGame;
						param->cancelFuncParam = &g_NetworkManager;

						// Show a progress spinner so that we can cancel the join.
						ui.NavigateToScene(0, eUIScene_ConnectingProgress, param);

						allowCancelTime = LONGLONG_MAX;
					}

					// Tick some simple things
					ProfileManager.Tick();
					StorageManager.Tick();
					InputManager.Tick();
					RenderManager.Tick();
					ui.tick();
					ui.render();
					RenderManager.Present();
				}
				// Check for being disconnected from the network
				if(!ProfileManager.IsSignedInLive(i))
				{
					bError = true;
					break;
				}
			}
		}

		if ( bError && !m_cancelJoinFromSearchResult )
		{
			app.DebugPrintf("Error removing a player from the session");
			assert(true);
		}
	}

	if(shownCancelScreen)
	{
		ui.NavigateToScene(0,eUIScene_Timer);
	}

	if(m_cancelJoinFromSearchResult)
	{
		SetState(DNM_INT_STATE_IDLE);
		m_cancelJoinFromSearchResult= false;

		return false;
	}

	m_cancelJoinFromSearchResult = false;


	// Now we join the party with each of joining players, and then wait to be informed of our reserved slots
	for( int i = 0; i < MAX_LOCAL_PLAYER_COUNT; i++ )
	{
		if( playerMask & ( 1 << i ) )
		{
			WXS::User^ user = ProfileManager.GetUser(i);

			auto joinPartyAsync = WXM::Party::JoinPartyByIdAsync(user, ref new Platform::String(searchResult->m_partyId.c_str()));
			auto ccTask = create_task(joinPartyAsync).then([this](task<void> t)
			{
				try
				{
					t.get(); // if t.get() didn't throw, it succeeded
				}
				catch (Platform::COMException^ ex)
				{
					LogCommentWithError( L"JoinPartyByIdAsync failed", ex->HResult );
					SetState(DNM_INT_STATE_JOINING_FAILED);
				}
			});
			

			while(!ccTask.is_done())
			{
				// Check for being disconnected from the network
				if(!ProfileManager.IsSignedInLive(i))
				{
					joinPartyAsync->Cancel();
					break;
				}

				// Tick some simple things
				ProfileManager.Tick();
				StorageManager.Tick();
				InputManager.Tick();
				RenderManager.Tick();
				ui.tick();
				ui.render();
				RenderManager.Present();
			}
			// Check for being disconnected from the network
			if(!ProfileManager.IsSignedInLive(i))
			{
				break;
			}
		}
	}

	return ( m_state == DNM_INT_STATE_JOINING_WAITING_FOR_RESERVATIONS );
}

void DQRNetworkManager::CancelJoinPartyFromSearchResult()
{
	m_cancelJoinFromSearchResult = true;
}

// This method attempts to find the local player within a party that would be most appropriate to act as our primary player when joining the game.
bool DQRNetworkManager::GetBestPartyUserIndex()
{
	bool playerFound = false;

	// Use context from any user at all for this
	WXS::User ^anyUser = nullptr;
	if( WXS::User::Users->Size > 0 )
	{
		anyUser = WXS::User::Users->GetAt(0);
	}
	if( anyUser == nullptr ) return 0;
	MXS::XboxLiveContext^ xboxLiveContext = ref new MXS::XboxLiveContext(anyUser);

	m_partyController->RefreshPartyView();
	MXSM::MultiplayerSessionReference ^sessionRef = m_partyController->GetGamePartySessionReference();
	if( sessionRef != nullptr )
	{
		auto asyncOp = xboxLiveContext->MultiplayerService->GetCurrentSessionAsync( sessionRef );
		create_task(asyncOp)
			.then([this,&playerFound] (task<Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^> t)
		{
			try
			{
				Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^ session = t.get();

				int playerIdx = -1;
				for( int i = 0; i < session->Members->Size; i++ )									// First pass through to see if we've a signed in user that is in the session we've been added to
				{
					MXSM::MultiplayerSessionMember^ member = session->Members->GetAt(i);
					for( int j = 0; j < MAX_LOCAL_PLAYERS; j++ )
					{
						WXS::User ^user = ProfileManager.GetUser(j);
						if( user != nullptr )
						{
							if( user->XboxUserId == member->XboxUserId )
							{
								DQRNetworkManager::LogCommentFormat(L"Found already signed in player %s to act as invite recipient",member->XboxUserId->Data());
								playerIdx = j;
								playerFound = true;
								break;
							}
						}
					}
				}
				if( playerIdx == -1 )																// If nothing found, second pass through to attempt to find a controller that matches
				{
					for( int i = 0; i < session->Members->Size; i++ )
					{
						MXSM::MultiplayerSessionMember^ member = session->Members->GetAt(i);
						for( int j = 4; j < 12; j++ )
						{
							WXS::User ^user = InputManager.GetUserForGamepad(j);
							if( user != nullptr )
							{
								if( user->XboxUserId == member->XboxUserId )
								{
									DQRNetworkManager::LogCommentFormat(L"Found controller %d for %s (session idx %d) to act as invite recipient (%s)",j,member->XboxUserId->Data(),i, user->XboxUserId->Data());
									playerIdx = ProfileManager.AddGamepadToGame(j);
									if( playerIdx != -1 )
									{
										playerFound = true;
										DQRNetworkManager::LogCommentFormat(L"Assigned controller to user index %d",playerIdx);
										break;
									}
								}
							}
						}
					}
				}
			}
			catch ( Platform::COMException^ ex )
			{
			}
		}).wait();
	}
	return playerFound;
}

// Request the GameDisplayName for this player asynchronously
void DQRNetworkManager::RequestDisplayName(DQRNetworkPlayer *player)
{
	if (player->IsLocal())
	{
		// Player is local so we can just ask profile manager 
		SetDisplayName(player->GetUID(), ProfileManager.GetDisplayName(player->GetLocalPlayerIndex()));
	}
	else
	{
		// Player is remote so we need to do an async request
		PlayerUID xuid = player->GetUID();
		ProfileManager.GetProfile(xuid, &GetProfileCallback, this);
	}
}

void DQRNetworkManager::GetProfileCallback(LPVOID pParam, Microsoft::Xbox::Services::Social::XboxUserProfile^ profile)
{
	DQRNetworkManager *dqnm = (DQRNetworkManager *)pParam;
	dqnm->SetDisplayName(PlayerUID(profile->XboxUserId->Data()), profile->GameDisplayName->Data());
}

// Set player display name
void DQRNetworkManager::SetDisplayName(PlayerUID xuid, wstring displayName)
{
	EnterCriticalSection(&m_csRoomSyncData);
	for (int i = 0; i < m_roomSyncData.playerCount; i++)
	{
		if( m_players[i] )
		{
			if (m_players[i]->GetUID() == xuid)
			{
				// Set player display name
				m_players[i]->SetDisplayName(displayName);
				// Add display name to map
				m_displayNames.insert(std::make_pair(m_players[i]->m_name, m_players[i]->m_displayName));
			}
		}
	}
	LeaveCriticalSection(&m_csRoomSyncData);
}

void DQRNetworkManager::CheckForcedSignOut()
{
	auto asyncOp = m_primaryUserXboxLiveContext->MultiplayerService->GetCurrentSessionAsync(m_multiplayerSession->SessionReference);
	create_task(asyncOp)
		.then([this] (task<Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^> t)
	{
		try
		{
			t.get();
		}
		catch (Platform::COMException^ ex)
		{
			if (ex->HResult == 0x8015DC16)
			{
				m_handleForcedSignOut = true;
			}
		}
	});
}

void DQRNetworkManager::HandleForcedSignOut()
{
	// Bin the session (we can't use it anymore)
	m_multiplayerSession = nullptr;

	// Bin the party
	m_partyController->SetPartyView(nullptr);

	// Forced sign out destroyed the party so time to go
	app.DebugPrintf("DQRNetworkManager::HandleForcedSignOut: Forced sign out destroyed the party, aborting game\n");

	if (IsInSession() && !g_NetworkManager.IsLeavingGame())
	{
		// Exit world
		app.SetAction(0, eAppAction_ExitWorld);
	}
	else
	{
		app.DebugPrintf("DQRNetworkManager::HandleForcedSignOut: Already leaving the game, skipping abort\n");
	}
}
