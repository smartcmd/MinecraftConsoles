//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved
#include "stdafx.h"
#include "PartyController.h"
#include "DQRNetworkManager.h"
#include <collection.h>

/*
#include "time.h"
#include "../GameLogic/Game.h"
#include "../Utils/LogController.h"
#include "../Utils/Utils.h"
#include "UserController.h"
#include "SessionController.h"
*/

using namespace Concurrency;
using namespace Microsoft::Xbox::Services::Multiplayer;
using namespace Microsoft::Xbox::Services::Matchmaking;
using namespace Microsoft::Xbox::Services;
using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage;
using namespace Windows::System;
using namespace Windows::UI::Core;
using namespace Windows::Xbox::Multiplayer;
using namespace Windows::Xbox::Networking;
using namespace Windows::Xbox::System;


PartyController::PartyController(DQRNetworkManager *pDQRNet) :
    m_isGameSessionReadyEventTriggered(false),
    m_isGamePlayerEventRegistered(false),
	m_pDQRNet(pDQRNet)
{
}

void PartyController::DebugPrintPartyView( Windows::Xbox::Multiplayer::PartyView^ partyView )
{
    DQRNetworkManager::LogComment( L"PartyView:" );
    if( partyView == nullptr )
    {
        DQRNetworkManager::LogComment( L"     No party view" );
        return;
    }

    DQRNetworkManager::LogComment( L"      GameSession::SessionName: " + (partyView->GameSession ? partyView->GameSession->SessionName : L"NONE") );
	DQRNetworkManager::LogComment( L"      GameSession::SessionTemplateName " + (partyView->GameSession ? partyView->GameSession->SessionTemplateName : L"NONE") );
	DQRNetworkManager::LogComment( L"      GameSession::ServiceConfigurationId " + (partyView->GameSession ? partyView->GameSession->ServiceConfigurationId : L"NONE") );
    DQRNetworkManager::LogComment( L"      MatchSession: " + (partyView->MatchSession ? partyView->MatchSession->SessionName : L"NONE") );
    DQRNetworkManager::LogComment( L"      IsPartyInAnotherTitle: " + partyView->IsPartyInAnotherTitle.ToString() );

    DQRNetworkManager::LogComment( L"        Members: " + partyView->Members->Size.ToString() );
    for( PartyMember^ member : partyView->Members )
    {
        DQRNetworkManager::LogComment( L"      Member:" );
        DQRNetworkManager::LogComment( L"          XboxUserID: " + member->XboxUserId );
        DQRNetworkManager::LogCommentFormat( L"          IsLocalUser: %s", member->IsLocal ? L"TRUE" : L"FALSE" );
//        DQRNetworkManager::LogComment( L"          JoinTime: " + Utils::DateTimeToString(member->JoinTime) );
    }

    DQRNetworkManager::LogComment( L"      ReservedMembers: " + partyView->ReservedMembers->Size.ToString() );
    for( Platform::String^ memberXuid : partyView->ReservedMembers )
    {
        DQRNetworkManager::LogComment( L"      ReservedMember:" );
        DQRNetworkManager::LogComment( L"          XboxUserID " + memberXuid );
    }

    DQRNetworkManager::LogComment( L"      MembersGroupedByDevice: " + partyView->MembersGroupedByDevice->Size.ToString() );
    for( PartyMemberDeviceGroup^ partyMemberDeviceGroup : partyView->MembersGroupedByDevice )
    {
        DQRNetworkManager::LogComment( L"        Device:" );
        for( PartyMember^ member : partyMemberDeviceGroup->Members )
        {
            DQRNetworkManager::LogComment( L"          Member:" );
            DQRNetworkManager::LogComment( L"            XboxUserID: " + member->XboxUserId );
            DQRNetworkManager::LogCommentFormat( L"            IsLocalUser: %s", member->IsLocal ? L"TRUE" : L"FALSE" );
//            DQRNetworkManager::LogComment( L"            JoinTime: " + Utils::DateTimeToString(member->JoinTime) );
        }
    }
}

void PartyController::RefreshPartyView()
{
//    LogComment( L"RefreshPartyView" );

    PartyView^ partyView;
    try
    {
        IAsyncOperation<PartyView^>^ partyOperation = Party::GetPartyViewAsync();
        create_task(partyOperation)
            .then([this, &partyView] (task<PartyView^> t)
        {
            try
            {
                Concurrency::critical_section::scoped_lock lock(m_lock);
                partyView = t.get();
            }
            catch ( Platform::Exception^ ex )
            {
                if( ex->HResult != static_cast<int>(Windows::Xbox::Multiplayer::PartyErrorStatus::EmptyParty) )
                {
//                    LogCommentWithError( L"GetPartyView failed", ex->HResult  );
                }

            }
        }).wait();
    }
    catch ( Platform::Exception^ ex )
    {
        partyView = nullptr;
//        LogCommentWithError( L"GetPartyView failed", ex->HResult  );
    }

    DebugPrintPartyView(partyView);

    SetPartyView(partyView);
}

// Add any players specified in userMask to the party. This method returns a bool that is true if the a player was added, which wasn't already
// in the game session associated with the party. This is used to determine whether we should be waiting for a slot to be added for it by the host.
bool PartyController::AddLocalUsersToParty(int userMask, Windows::Xbox::System::User^ primaryUser)
{
	bool addedNewPlayerToPartyThatIsntInSession = false;

	PartyView^ partyView = GetPartyView();

	// If there's already a party, then attempt to get a session document as we'll be needing this later
	MultiplayerSession^ session;
	if( partyView )
	{
		MXS::XboxLiveContext^ xblContext = ref new MXS::XboxLiveContext(primaryUser);
		if( xblContext )
		{
			// Get a copy of the session document, for this user
			auto multiplayerSessionAsync = xblContext->MultiplayerService->GetCurrentSessionAsync( m_pDQRNet->ConvertToMicrosoftXboxServicesMultiplayerSessionReference(partyView->GameSession));
			create_task(multiplayerSessionAsync).then([&session,this](task<MXSM::MultiplayerSession^> t)
			{
				try
				{
					session = t.get(); // if t.get() didn't throw, it succeeded
				}
				catch (Platform::COMException^ ex)
				{
					DQRNetworkManager::LogCommentWithError( L"GetCurrentSessionAsync failed", ex->HResult );
				}
			})
			.wait();
		}
	}

    // Adds all local players that are specified in userMask to the party belonging to the primary player
	IVector<Windows::Xbox::System::User^>^ localUsersToAddVector = ref new Platform::Collections::Vector<Windows::Xbox::System::User^>;
	for( int i = 0; i < 4; i++ )
	{
		if( userMask & ( 1 << i ) )
		{
			bool alreadyHere = false;
			if( partyView )
			{
				Windows::Xbox::System::User^ userToAdd = ProfileManager.GetUser(i, true);
				for( int j = 0; j < partyView->Members->Size; j++ )
				{
					if( partyView->Members->GetAt(j)->XboxUserId == userToAdd->XboxUserId )
					{
						primaryUser = userToAdd;	// If there is already a party, then the acting user passed to AddLocalUsersAsync must be a user that is in that party, so make sure this is the case. Doesn't matter Which user.
						alreadyHere = true;
						break;
					}
				}
			}

			if( !alreadyHere )
			{
				localUsersToAddVector->Append(ProfileManager.GetUser(i));
				bool alreadyInSession = false;
				if( session )
				{
					if( m_pDQRNet->IsPlayerInSession( ProfileManager.GetUser(i, true)->XboxUserId, session, nullptr ) )
					{
						alreadyInSession = true;
					}
				}
				if( !alreadyInSession )
				{
					addedNewPlayerToPartyThatIsntInSession = true;
				}
			}
		}
	}

	IVectorView<Windows::Xbox::System::User^>^ localUsersToAddVecView = localUsersToAddVector->GetView();

	try
	{
		IAsyncAction^ addMemberOperation = Party::AddLocalUsersAsync( primaryUser, localUsersToAddVecView );

		create_task(addMemberOperation)
			.then([this] (task<void> t)
		{
			try
			{
				t.get();
				RefreshPartyView();
			}
			catch (Platform::COMException^ ex)
			{
				DQRNetworkManager::LogCommentWithError( L"AddLocalUsersAsync failed", ex->HResult  );
			}
			catch (Platform::OperationCanceledException^ ex)
			{
				DQRNetworkManager::LogCommentWithError( L"AddLocalUsersAsync failed - operation cancelled", ex->HResult  );
			}
		}).wait();
	}
	catch (Platform::COMException^ ex)
	{
		DQRNetworkManager::LogCommentWithError( L"AddLocalUsersAsync failed to create", ex->HResult  );
	}

	return addedNewPlayerToPartyThatIsntInSession;
}

void PartyController::CheckPartySessionFull(Windows::Xbox::System::User^ primaryUser)
{
	RefreshPartyView();

	PartyView^ partyView = GetPartyView();

	// If there's already a party, then attempt to get a session document
	MultiplayerSession^ session;
	if( partyView && partyView->GameSession)
	{
		MXS::XboxLiveContext^ xblContext = ref new MXS::XboxLiveContext(primaryUser);
		if( xblContext )
		{
			// Get a copy of the session document, for this user
			auto multiplayerSessionAsync = xblContext->MultiplayerService->GetCurrentSessionAsync( m_pDQRNet->ConvertToMicrosoftXboxServicesMultiplayerSessionReference(partyView->GameSession));
			create_task(multiplayerSessionAsync).then([&session,this](task<MXSM::MultiplayerSession^> t)
			{
				try
				{
					session = t.get(); // if t.get() didn't throw, it succeeded
				}
				catch (Platform::COMException^ ex)
				{
					DQRNetworkManager::LogCommentWithError( L"GetCurrentSessionAsync failed", ex->HResult );
				}
			})
			.wait();
		}
		if( session )
		{
			int nonLocalPlayerCount = 0;
			for( int i = 0; i < session->Members->Size; i++ )
			{
				MXSM::MultiplayerSessionMember^ member = session->Members->GetAt(i);

				bool isLocalPlayer = false;
				for( int j = 4; j < 12; j++ )
				{
					WXS::User ^user = InputManager.GetUserForGamepad(j);
					if( user != nullptr )
					{
						if( user->XboxUserId == member->XboxUserId )
						{
							isLocalPlayer = true;
						}
					}
				}
				if( !isLocalPlayer )
				{
					nonLocalPlayerCount++;
				}
			}
			app.DebugPrintf(">>>>> Invited to a game with a non-local player count of %d\n", nonLocalPlayerCount);

			if( nonLocalPlayerCount >= DQRNetworkManager::MAX_ONLINE_PLAYER_COUNT )
			{
				m_pDQRNet->FlagInvitedToFullSession();
			}
		}
	}
}

void PartyController::SetJoinability(bool isJoinable)
{
	Party::Joinability = isJoinable ? WXM::SessionJoinability::JoinableByFriends : WXM::SessionJoinability::InviteOnly;
}

void PartyController::DisassociateSessionFromParty( Microsoft::Xbox::Services::Multiplayer::MultiplayerSessionReference^ sessionReference)
{
	RefreshPartyView();

	PartyView^ partyView = GetPartyView();

	if( partyView )
	{
		// We need a user to be the acting user, when disassociating the party. Attempt to find a party member who is local.
		for( int j = 0; j < partyView->Members->Size; j++ )
		{
			Windows::Xbox::Multiplayer::PartyMember^ partyMember = partyView->Members->GetAt(j);
			if( partyMember->IsLocal )
			{
				IVectorView<Windows::Xbox::System::User^>^ localUsers = Windows::Xbox::System::User::Users;

				for( int i = 0; i < localUsers->Size; i++ )
				{
					Windows::Xbox::System::User^ localUser = localUsers->GetAt(i);
					if( localUser->XboxUserId == partyMember->XboxUserId )
					{
						// Convert the session reference (in Microsoft::Xbox::Services::Multiplayer namespace) to Windows::Xbox::Multiplayer names space so we can use in the party system
						WXM::MultiplayerSessionReference^ winSessionRef = m_pDQRNet->ConvertToWindowsXboxMultiplayerSessionReference(sessionReference);

						auto disassociateSessionAsync = WXM::Party::DisassociateGameSessionAsync(localUser, winSessionRef);
						create_task(disassociateSessionAsync).then([this](task<void> t)
						{
							try
							{
								t.get(); // if t.get() didn't throw, it succeeded
							}
							catch (Platform::COMException^ ex)
							{
								m_pDQRNet->LogCommentWithError( L"DisassociateGameSessionAsync failed", ex->HResult );
							}
						});
						//.wait(); // There are situations in which this never completes (?!) so waiting isn't a good idea
						return;
					}
				}
			}
		}
	}


}

void PartyController::RemoveLocalUsersFromParty(Windows::Xbox::System::User^ primaryUser)
{
	PartyView^ partyView = GetPartyView();

	if( partyView == nullptr ) return;

	// Attempt to remove all local users that are currently in the party
	IVectorView<Windows::Xbox::System::User^>^ localUsers = Windows::Xbox::System::User::Users;
	IVector<Windows::Xbox::System::User^>^ localUsersToRemoveVector = ref new Platform::Collections::Vector<Windows::Xbox::System::User^>;
	for( int i = 0; i < localUsers->Size; i++ )
	{
		Windows::Xbox::System::User^ userToRemove = localUsers->GetAt(i);
		for( int j = 0; j < partyView->Members->Size; j++ )
		{
			if( partyView->Members->GetAt(j)->XboxUserId == userToRemove->XboxUserId )
			{
				localUsersToRemoveVector->Append(userToRemove);
				break;
			}
		}
	}
		
	IVectorView<Windows::Xbox::System::User^>^ localUsersToRemoveVecView = localUsersToRemoveVector->GetView();

    IAsyncAction^ removeMemberOperation = Party::RemoveLocalUsersAsync( localUsersToRemoveVecView );

    create_task(removeMemberOperation)
        .then([this] (task<void> t)
    {
        try
        {
            t.get();
            RefreshPartyView();
        }
        catch (Platform::COMException^ ex)
        {
            DQRNetworkManager::LogCommentWithError( L"RemoveLocalUsersAsync failed", ex->HResult  );
        }
    }).wait();
}

void PartyController::RemoveLocalUsersFromParty(Windows::Xbox::System::User^ primaryUser, int playerMask, Microsoft::Xbox::Services::Multiplayer::MultiplayerSessionReference^ sessionReference)
{
	PartyView^ partyView = GetPartyView();

	if( partyView == nullptr ) return;

	// Don't leave the party if it isn't actually the party belonging to the session we are in - this can happen when switching from one game session to another, after accepting an invite to a game whilst already playing
	if( sessionReference != nullptr )
	{
		if( partyView->GameSession != nullptr )
		{
			if( partyView->GameSession->SessionName != sessionReference->SessionName ) return;
		}
	}

	// Attempt to remove all specified local users that are currently in the party. Check that each player is actually in
	// the party, as we'll get an exception for trying to remove players that aren't
	
	IVector<Windows::Xbox::System::User^>^ localUsersToRemoveVector = ref new Platform::Collections::Vector<Windows::Xbox::System::User^>;

	for( int i = 0; i < MAX_LOCAL_PLAYERS; i++ )
	{
		if( playerMask & ( 1 << i ) )
		{
			Windows::Xbox::System::User^ userToRemove = ProfileManager.GetUser(i, true);
			if(userToRemove!=nullptr)
			{			
				for( int j = 0; j < partyView->Members->Size; j++ )
				{
					if( partyView->Members->GetAt(j)->XboxUserId == userToRemove->XboxUserId )
					{
						localUsersToRemoveVector->Append(userToRemove);
						break;
					}
				}
			}
		}
	}
		
	IVectorView<Windows::Xbox::System::User^>^ localUsersToRemoveVecView = localUsersToRemoveVector->GetView();

    IAsyncAction^ removeMemberOperation = Party::RemoveLocalUsersAsync( localUsersToRemoveVecView );

    create_task(removeMemberOperation)
        .then([this] (task<void> t)
    {
        try
        {
            t.get();
            RefreshPartyView();
        }
        catch (Platform::COMException^ ex)
        {
            DQRNetworkManager::LogCommentWithError( L"RemoveLocalUsersAsync failed", ex->HResult  );
        }
    }).wait();
}

void PartyController::RemoveLocalUserFromParty(Windows::Xbox::System::User^ userToRemove)
{
	// Attempt to a specific local user from the party

	RefreshPartyView();

	// Don't need to do anything if there isn't currently a party
	PartyView^ partyView = GetPartyView();

	if( partyView == nullptr ) return;

	// Don't need to do anything if this player isn't in the current party - the remove will raise an exception
	bool inParty = false;
	for( int i = 0; i < partyView->Members->Size; i++ )
	{
		if( partyView->Members->GetAt(i)->XboxUserId == userToRemove->XboxUserId )
		{
			inParty = true;
			break;
		}
	}
	if( !inParty ) return;
	
	IVector<Windows::Xbox::System::User^>^ localUsersToRemoveVector = ref new Platform::Collections::Vector<Windows::Xbox::System::User^>;
	localUsersToRemoveVector->Append(userToRemove);	
	IVectorView<Windows::Xbox::System::User^>^ localUsersToRemoveVecView = localUsersToRemoveVector->GetView();

    IAsyncAction^ removeMemberOperation = Party::RemoveLocalUsersAsync( localUsersToRemoveVecView );

    create_task(removeMemberOperation)
        .then([this] (task<void> t)
    {
        try
        {
            t.get();
            RefreshPartyView();
        }
        catch (Platform::COMException^ ex)
        {
            DQRNetworkManager::LogCommentWithError( L"RemoveLocalUsersAsync failed", ex->HResult  );
        }
    }).wait();
}

void PartyController::RegisterGamePlayersChangedEventHandler()
{
    // Listen to Party::GamePlayersChanged
    // Only the HOST should register for this event to detect if any party members were added that should be pulled into the game.
    EventHandler<GamePlayersChangedEventArgs^>^ partyGamePlayersChanged = ref new EventHandler<GamePlayersChangedEventArgs^>(
        [this] (Platform::Object^, GamePlayersChangedEventArgs^ eventArgs)
    {
        OnGamePlayersChanged( eventArgs );
    });
    m_partyGamePlayersChangedToken = Party::GamePlayersChanged += partyGamePlayersChanged;
    m_isGamePlayerEventRegistered = true;
}

void PartyController::RegisterEventHandlers()
{
    // Listen to Party Roster Changed
    EventHandler<PartyRosterChangedEventArgs^>^ partyRosterChangedEvent = ref new EventHandler<PartyRosterChangedEventArgs^>(
        [this] (Platform::Object^, PartyRosterChangedEventArgs^ eventArgs)
    {
        OnPartyRosterChanged( eventArgs );
    });
    m_partyRosterChangedToken = Party::PartyRosterChanged += partyRosterChangedEvent;

    // Listen to Party State Changed
    EventHandler<PartyStateChangedEventArgs^>^ partyStateChangedEvent = ref new EventHandler<PartyStateChangedEventArgs^>(
        [this] (Platform::Object^, PartyStateChangedEventArgs^ eventArgs)
    {
        OnPartyStateChanged( eventArgs );
    });
    m_partyStateChangedToken = Party::PartyStateChanged += partyStateChangedEvent;
    // Listen to Game Session Ready
    EventHandler<GameSessionReadyEventArgs^>^ gameSessionReadyEvent = ref new EventHandler<GameSessionReadyEventArgs^>(
        [this] (Platform::Object^, GameSessionReadyEventArgs^ eventArgs)
    {
        OnGameSessionReady(eventArgs);
    });
    m_partyGameSessionReadyToken = Party::GameSessionReady += gameSessionReadyEvent;
}

void PartyController::UnregisterEventHandlers()
{
    Party::PartyRosterChanged -= m_partyRosterChangedToken;
    Party::PartyStateChanged -= m_partyStateChangedToken;
    Party::GameSessionReady -= m_partyGameSessionReadyToken;
}

void PartyController::UnregisterGamePlayersEventHandler()
{
    if(m_isGamePlayerEventRegistered)
    {
        Party::GamePlayersChanged -= m_partyGamePlayersChangedToken;
    }
}

// This event will fire when :
// - A new Match Session registered to party, or 
// - A new Game Session is registered to party (via RegisterGame call, or as a result of matchmaking), and 
// - Party Title ID changes (which will trigger change in IsPartyInAnotherTItle bool flag).
void PartyController::OnPartyStateChanged( PartyStateChangedEventArgs^ eventArgs )
{
    DQRNetworkManager::LogComment( L"OnPartyStateChanged");
    RefreshPartyView();
}

void PartyController::OnPartyRosterChanged( PartyRosterChangedEventArgs^ eventArgs )
{
	if( m_pDQRNet->m_multiplayerSession == nullptr) return;

	RefreshPartyView();

	XboxLiveContext^ xboxLiveContext = m_pDQRNet->m_primaryUserXboxLiveContext;
	if( xboxLiveContext == nullptr ) return;
	
	DQRNetworkManager::LogComment( L"OnPartyRosterChanged");

	if( eventArgs->RemovedMembers->Size )
	{
		DQRNetworkManager::LogComment( L"Removed Members:");
	}

	// First, establish whether an active player local to this machine have been removed
	bool activePlayerRemoved = false;
	int playerLeavingMask = 0;

	// If there's no party anymore, then we're all leaving
	if( m_partyView == nullptr )
	{
		playerLeavingMask = m_pDQRNet->m_currentUserMask;
	}
	else
	{
		// Still a party, find out who left
		for( int i = 0; i < eventArgs->RemovedMembers->Size; i++ )
		{
			DQRNetworkPlayer *player = m_pDQRNet->GetPlayerByXuid(static_cast<PlayerUID>(eventArgs->RemovedMembers->GetAt(i)->Data()));
			if( player )
			{
				if( player->IsLocal() )
				{
					playerLeavingMask |= ( 1 << player->GetLocalPlayerIndex() );
				}
			}
			DQRNetworkManager::LogComment(eventArgs->RemovedMembers->GetAt(i));
		}
	}

	// If a local player is leaving the party, we want to handle it generally as if they had selected to exit from within the game, assuming that they have just deliberatly removed themselves from
	// the party via the system interface. However... we may be being removed from this party because we have just accepted a request to join Another party via a "game session ready" sort of prompt.
	// I don't think there's any way to distinguish these two things happening at this stage, so at this point we will signal to the DQR layer what has just happened,
	// and it will only do something about it after some period of time has passed without a new game party becoming ready for the player to join
	if( playerLeavingMask )
	{
		m_pDQRNet->HandlePlayerRemovedFromParty(playerLeavingMask);
	}
}

#define LAST_INVITED_TIME_TIMEOUT                           3 * 60 //secs
void PartyController::AddAvailableGamePlayers(IVectorView<GamePlayer^>^ availablePlayers, int& remainingSlots, MultiplayerSession^ currentSession)
{
    bool bNewMembersAdded = false;
    for each (Windows::Xbox::Multiplayer::GamePlayer^ player in availablePlayers)
    {
        if( remainingSlots <= 0 )
        {
			DQRNetworkManager::LogCommentFormat( L"No more available slots - broadcasting failure of adding player %s",player->XboxUserId->Data());
			m_pDQRNet->SendAddPlayerFailed(player->XboxUserId);
            continue;
        }

		// Not sure what this condition is actually for - removing until I can see a use for it
#if 0
        if( GetTimeBetweenInSeconds(player->LastInvitedTime, GetCurrentTime()) < LAST_INVITED_TIME_TIMEOUT )
        {
            DQRNetworkManager::LogComment( L"Possible user just exited; skipping join request" );
            continue;
        }
#endif

		if( m_pDQRNet->IsPlayerInSession(player->XboxUserId, currentSession, nullptr))
        {
			DQRNetworkManager::LogComment( L"Player is already in session; skipping join request: " + player->XboxUserId->ToString() );
            continue;
        }

		// Have a search through our local players, to see if we have a controller for this player. If so, then we'll want to add them directly to the game (if at all), rather than using
		// the reservation system
		bool bFoundLocal = false;
		for( int i = 4; i < 12; i++ )
		{
			WXS::User^ user = InputManager.GetUserForGamepad(i);
			if( user != nullptr )
			{
				if( user->XboxUserId == player->XboxUserId )
				{
					bFoundLocal = true;
					// Check that they aren't in the game already (don't see how this could be the case anyway)
					if( m_pDQRNet->GetPlayerByXuid( static_cast<PlayerUID>(player->XboxUserId->Data()) ) == nullptr )
					{
						// And check whether they are signed in yet or not
						int userIdx = -1;
						for( int j = 0; j < MAX_LOCAL_PLAYERS; j++ )
						{
							WXS::User^ user2 = ProfileManager.GetUser(j);
							if( user2 != nullptr )
							{
								if(user2->XboxUserId == user->XboxUserId)
								{
									userIdx = j;
									break;
								}
							}
						}

						// If not found, then attempt to add them since we've found a controller
						if( userIdx == -1 )
						{
							// Found the appropriate controller. Attempt to add it - this will return -1 if unsuccessful
							userIdx = ProfileManager.AddGamepadToGame(i);
						}

						// Found a slot, so just need to add the user now
						if( userIdx != -1 )
						{
							m_pDQRNet->AddLocalPlayerByUserIndex(userIdx);
						}
					}
				}
			}
		}
		if(bFoundLocal)
		{
			continue;
		}

        currentSession->AddMemberReservation( player->XboxUserId, m_pDQRNet->GetNextSmallIdAsJsonString(), true );
        DQRNetworkManager::LogComment( L"Member added: " + player->XboxUserId->ToString() );
        bNewMembersAdded = true;
        remainingSlots--;
    }

    if(bNewMembersAdded)
    {
		XboxLiveContext^ xboxLiveContext = m_pDQRNet->m_primaryUserXboxLiveContext;
        DQRNetworkManager::LogComment( L"New members found and added from related parties" );
        HRESULT hr = S_OK;
        Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^ outputSession = m_pDQRNet->WriteSessionHelper(
			xboxLiveContext,
            currentSession,
            Microsoft::Xbox::Services::Multiplayer::MultiplayerSessionWriteMode::UpdateExisting,
            hr
            );

        if(outputSession != nullptr)
        {
            Windows::Xbox::Multiplayer::MultiplayerSessionReference^ convertedSessionRef =
                m_pDQRNet->ConvertToWindowsXboxMultiplayerSessionReference(currentSession->SessionReference);
			Windows::Xbox::System::User^ actingUser = m_pDQRNet->m_primaryUser;
            Party::PullReservedPlayersAsync(actingUser, convertedSessionRef);

			m_pDQRNet->HandleSessionChange( outputSession );
        }
    }
}

void PartyController::OnGamePlayersChanged( GamePlayersChangedEventArgs^ eventArgs )
{
    DQRNetworkManager::LogComment( L"OnGamePlayersChanged");

	RefreshPartyView();

	if( m_pDQRNet->m_primaryUser == nullptr ) return;

	IVectorView<GamePlayer^ >^ availablePlayers = nullptr;
	auto asyncGetAvailablePlayers = Party::GetAvailableGamePlayersAsync(m_pDQRNet->m_primaryUser);
	create_task(asyncGetAvailablePlayers).then([&availablePlayers](task<IVectorView<GamePlayer^ >^> t)
	{
		try
		{
			availablePlayers = t.get();
		}
		catch( Platform::COMException^ ex )
		{
		}
	}).wait();

	if( availablePlayers == nullptr ) return;

    Microsoft::Xbox::Services::Multiplayer::MultiplayerSessionReference^ sessionRef =
		m_pDQRNet->ConvertToMicrosoftXboxServicesMultiplayerSessionReference(eventArgs->GameSession);

    if(sessionRef == nullptr)
    {
//        LogComment(L"OnGamePlayersChanged: invalid sessionRef.");
        return;
    }

	XboxLiveContext^ xboxLiveContext = m_pDQRNet->m_primaryUserXboxLiveContext;
    IAsyncOperation<Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^>^ asyncOp = xboxLiveContext->MultiplayerService->GetCurrentSessionAsync( sessionRef );
    create_task(asyncOp)
        .then([this, availablePlayers, &xboxLiveContext] (task<Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^> t)
    {
        try
        {
            Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^ currentSession = t.get();

            int remainingSlots = currentSession->SessionConstants->MaxMembersInSession - currentSession->Members->Size;

			DQRNetworkManager::LogCommentFormat( L"OnGamePlayersChanged - calling AddAvailableGamePlayers, with %d players available",availablePlayers->Size);

			// This should be called if we have available slots or not, because it also handles broadcasting failed joins to clients already in the game, which
			// the clients need to know that adding a local player has failed
            AddAvailableGamePlayers(availablePlayers, remainingSlots, currentSession);
        }
        catch ( Platform::COMException^ ex )
        {
//            LogCommentWithError( L"OnGameSessionReady failed to retrieve current session", ex->HResult  );
        }
    }).wait();
}

void PartyController::OnGameSessionReady( GameSessionReadyEventArgs^ eventArgs )
{
	 DQRNetworkManager::LogComment( L"OnGameSessionReady");
		
	 // If we are already in this session, then we'll be trying to add a player to the session rather than start up a new game. Set a flag if this is the case.
	 bool bInSession = false;
	 if( m_pDQRNet->m_multiplayerSession )
	 {
		 if( m_pDQRNet->m_multiplayerSession->SessionReference->SessionName == eventArgs->GameSession->SessionName )
		 {
			 bInSession = true;
		 }
	 }

	// Get a current copy of the MPSD, and search for the players that we are trying to join in it
    Microsoft::Xbox::Services::Multiplayer::MultiplayerSessionReference^ sessionRef =
        m_pDQRNet->ConvertToMicrosoftXboxServicesMultiplayerSessionReference(eventArgs->GameSession);

	vector<Platform::String^> localAdhocAdditions;

	// Use context from any user at all for this, since this might happen before we are in a game and won't have anything set up in the network manager itself. We are only
	// using it to read the session so there shouldn't be any requirements to use a particular live context
	WXS::User ^anyUser = nullptr;
	if( WXS::User::Users->Size > 0 )
	{
		anyUser = WXS::User::Users->GetAt(0);
	}
	if( anyUser == nullptr )
	{
		app.DebugPrintf("Abandoning gamesessionready, no user found\n");
		return;
	}
	XboxLiveContext^ xboxLiveContext = ref new MXS::XboxLiveContext(anyUser);

	app.DebugPrintf("Gamesessionready user and xboxlivecontext found\n");

    IAsyncOperation<Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^>^ asyncOp = xboxLiveContext->MultiplayerService->GetCurrentSessionAsync( sessionRef );
    create_task(asyncOp)
		.then([this, eventArgs, bInSession, &localAdhocAdditions,sessionRef] (task<Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^> t)
    {
        try
        {
            Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^ session = t.get();

			// Check if our joining users are in the session
			int userFoundMask = 0;
			m_pDQRNet->LogCommentFormat(L"Found session, size %d\n",session->Members->Size);
			for( int i = 0; i < session->Members->Size; i++ )
			{
				MXSM::MultiplayerSessionMember^ member = session->Members->GetAt(i);
				
				Platform::String^ memberXUID = member->XboxUserId;
				bool isAJoiningXuid = false;
				for( int j = 0; j < MAX_LOCAL_PLAYERS; j++ )
				{
					if( m_pDQRNet->m_joinSessionUserMask & ( 1 << j ) )
					{
						if( m_pDQRNet->m_joinSessionXUIDs[j] == memberXUID )
						{
							userFoundMask |= ( 1 << j );
							isAJoiningXuid = true;
						}
					}
				}
				
				// If:
				//
				// (1) this isn't a player we are actively trying to join
				// (2) it isn't currently in the game (only applicable if we are in the same session we're considering going to)
				// (3) we've got a controller assigned to a user with a matching xuid
				//
				// then we might still be interested in this as this could be someone joining via the system interface
				if( !isAJoiningXuid )	// Isn't someone we are actively trying to join
				{
					if( (!bInSession) ||                 // If not in a game session at all
						( ( m_pDQRNet->GetState() == DQRNetworkManager::DNM_INT_STATE_PLAYING ) && ( m_pDQRNet->GetPlayerByXuid( static_cast<PlayerUID>(memberXUID->Data()) ) == nullptr ) )  // Or we're fully in, and this player isn't in it
						)
					{
						for( int j = 4; j < 12; j++ )		// Final check that we have a gamepad for this xuid so that we might possibly be able to pair someone up
						{
							WXS::User^ user = InputManager.GetUserForGamepad(j);
							if( user != nullptr )
							{
								m_pDQRNet->LogCommentFormat(L"%d %d %s vs %s\n",i,j,user->XboxUserId->Data(), memberXUID->Data());
								if( user->XboxUserId == memberXUID )
								{
									localAdhocAdditions.push_back( memberXUID );
								}
							}
						}
					}
				}
			}

			// If we are in the middle of a game-controlled join, then we only (currently) care about users involved in this turning up in the session
			if( m_pDQRNet->m_joinSessionUserMask != 0 )
			{
				m_pDQRNet->LogComment(L"In game controlled join\n");
				// If all the users we are expecting to join are here, then proceed to either start a new game or just add to the existing session{
				if( userFoundMask == m_pDQRNet->m_joinSessionUserMask ) 
				{
					m_pDQRNet->m_joinSessionUserMask = 0;
					m_pDQRNet->m_currentUserMask |= userFoundMask;

					if( m_pDQRNet->m_state == DQRNetworkManager::DNM_INT_STATE_JOINING_WAITING_FOR_RESERVATIONS )
					{
						// Attempting to join a game via the discovered list of parties that our friends are in
						m_pDQRNet->JoinSession(userFoundMask);
					}
					else
					{
						if( bInSession )
						{
							// Already in a game, and adding a new local player - make them active
							m_pDQRNet->AddUsersToSession( userFoundMask, m_pDQRNet->ConvertToMicrosoftXboxServicesMultiplayerSessionReference( eventArgs->GameSession ));
						}
					}
				}
				if( localAdhocAdditions.size() > 0 )
				{
					// TODO - need to flag up the fact that there were adhoc additions here that we've just ignored, so we can come back and do something with them at a more appropriate time (ie when m_joinSessionUserMask transitions back to 0)
				}
			}
			else if( localAdhocAdditions.size() > 0 )
			{
				m_pDQRNet->LogComment(L"Not in game controlled join, but have other player(s) of possible interest\n");
				// Not trying to do a game controlled join at the moment, and we've got at least one user of interest that we've got a controller for and isn't currently playing

				// If we are in a session, then we might be able to add local players into the game
				if( bInSession )
				{
					m_pDQRNet->LogComment(L"In session, may be able to add local player to game\n");
					int adhocMask = 0;
					for( int i = 0; i < localAdhocAdditions.size(); i++ )
					{
						// First search to see if we already have the player signed in
						int userIdx = -1;
						for( int j = 0; j < MAX_LOCAL_PLAYERS; j++ )
						{
							WXS::User^ user = ProfileManager.GetUser(j);
							if( user != nullptr )
							{
								if(user->XboxUserId == localAdhocAdditions[i])
								{
									userIdx = j;
									break;
								}
							}
						}
						// If not found - see if we have a controller for that person so we can add them
						if( userIdx == -1 )
						{
							for( int j = 4; j < 12; j++ )
							{
								WXS::User^ user = InputManager.GetUserForGamepad(j);
								if( user != nullptr )
								{
									if( user->XboxUserId == localAdhocAdditions[i] )
									{
										// Found the appropriate controller. Attempt to add it - this will return -1 if unsuccessful
										userIdx = ProfileManager.AddGamepadToGame(j);
										break;
									}
								}
							}
						}
						// If we found or were able to add a player to one of the 4 local player slots, accumulate in a mask
						if( userIdx != -1 )
						{
							m_pDQRNet->m_joinSessionXUIDs[userIdx] = localAdhocAdditions[i];
							adhocMask |= ( 1 << userIdx );
						}
					}
					// If we found anyone adhoc to add, join them into the game now
					if( adhocMask )
					{
						m_pDQRNet->AddUsersToSession( adhocMask, m_pDQRNet->ConvertToMicrosoftXboxServicesMultiplayerSessionReference( eventArgs->GameSession ));
					}
				}
				else
				{
					// Not in a game, or not in The game that we've just been notified of. We need to try to do a best-fit to work out who
					// of our signed in players or controllers we know about, should be being considered as being invited to this game
					m_pDQRNet->LogComment(L"Not in a game, considering as possible invite scenario\n");
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
										DQRNetworkManager::LogCommentFormat(L"Found controller %d for %s (session idx %d) to act as invite recipient",j,member->XboxUserId->Data(),i);
										playerIdx = ProfileManager.AddGamepadToGame(j);
										if( playerIdx != -1 )
										{
											DQRNetworkManager::LogCommentFormat(L"Assigned controller to user index %d",playerIdx);
											break;
										}
									}
								}
							}
						}
					}
					if( playerIdx != -1 )
					{
						m_pDQRNet->LogComment(L"Player found, considered as invite scenario\n");
						m_pDQRNet->HandleNewPartyFoundForPlayer();
						// Note - must pass player 0 here as the player that the invite is for, or else the xbox 1 code generally breaks because it sets a non-zero primary player. We're going to
						// be trying to join all current local users to the new game session though. no matter who the invite is for.
						DQRNetworkManager::SetPartyProcessJoinSession(0, sessionRef->SessionName, sessionRef->ServiceConfigurationId, sessionRef->SessionTemplateName);
					}
					else
					{
						app.DebugPrintf("No player found to join party with\n");
					}
				}
			}

        }
        catch ( Platform::COMException^ ex )
        {
           m_pDQRNet->LogCommentWithError( L"OnGameSessionReady failed to retrieve current session", ex->HResult  );
        }
    }).wait();

}

bool PartyController::CanJoinParty()
{
    Concurrency::critical_section::scoped_lock lock(m_lock);
    if( m_partyView == nullptr )
    {
        return false;
    }

    return (m_partyView->GameSession != nullptr && !m_partyView->IsPartyInAnotherTitle);
}

bool PartyController::CanInvitePartyToMyGame(
    Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^ multiplayerSession
    )
{
    if( multiplayerSession == nullptr )
    {
        // If my session doesn't exist then shouldn't invite party
        return false;
    }

    PartyView^ partyView = GetPartyView();
    if( partyView == nullptr )
    {
        // If the party view doesn't have a session, then could invite party
        return true;
    }

    if( partyView->IsPartyInAnotherTitle )
    {
        // If my session doesn't exist then shouldn't invite party
        return true;
    }

    if( partyView->GameSession != nullptr ) 
    {
        // If my session and the party session differs, then could invite party
        if ( _wcsicmp(partyView->GameSession->ServiceConfigurationId->Data(), multiplayerSession->SessionReference->ServiceConfigurationId->Data() ) != 0 )
        {
            return true;
        }
        if ( _wcsicmp(partyView->GameSession->SessionName->Data(), multiplayerSession->SessionReference->SessionName->Data() ) != 0 )
        {
            return true;
        }
        if ( _wcsicmp(partyView->GameSession->SessionTemplateName->Data(), multiplayerSession->SessionReference->SessionTemplateName->Data() ) != 0 )
        {
            return true;
        }
    }
    else
    {
        // If the party doesn't have a session, then I could invite party
        return true;
    }

    // If the party is in my session then return false
    return false;
}

int PartyController::GetActiveAndReservedMemberPartySize()
{
    int partySize = 0;

    PartyView^ partyView = GetPartyView();
    if ( partyView != nullptr )
    {
        partySize = partyView->Members->Size + partyView->ReservedMembers->Size;
    }

    return partySize;
}

bool PartyController::IsPartyInAnotherTitle()
{
    PartyView^ partyView = GetPartyView();
    if( partyView == nullptr )
    {
        return false;
    }

    return partyView->IsPartyInAnotherTitle;
}

bool PartyController::IsGameSessionReadyEventTriggered()
{
    Concurrency::critical_section::scoped_lock lock(m_lock);
    return m_isGameSessionReadyEventTriggered;
}

void PartyController::ClearGameSessionReadyEventTriggered()
{
    Concurrency::critical_section::scoped_lock lock(m_lock);
    m_isGameSessionReadyEventTriggered = false;
}

bool PartyController::DoesPartySessionExist()
{
	Concurrency::critical_section::scoped_lock lock(m_lock);

    if( m_partyView != nullptr && m_partyView->GameSession != nullptr)
    {
        return true;
    }
    return false;
}

Microsoft::Xbox::Services::Multiplayer::MultiplayerSessionReference ^ PartyController::GetGamePartySessionReference()
{
	Concurrency::critical_section::scoped_lock lock(m_lock);

	if( m_partyView != nullptr )
	{
		if( m_partyView->GameSession != nullptr)
		{
			return m_pDQRNet->ConvertToMicrosoftXboxServicesMultiplayerSessionReference(m_partyView->GameSession);
		}
	}
	return nullptr;
}

PartyView^ PartyController::GetPartyView()
{
    Concurrency::critical_section::scoped_lock lock(m_lock);
    return m_partyView;
}

bool PartyController::DoesPartyAndSessionPlayersMatch(
    Microsoft::Xbox::Services::Multiplayer::MultiplayerSession^ session
    )
{
    PartyView^ partyView = GetPartyView();

    // Verify that session size and party size match
    if ( session->Members->Size != partyView->Members->Size )
    {
        return false;
    }

    bool inParty;

    // Verify that session players match current party players
    for ( unsigned int i = 0; i < session->Members->Size; i++ )
    {
        inParty = false;

        MultiplayerSessionMember^ member = session->Members->GetAt( i );

        for ( PartyMember^ partyMember : partyView->Members )
        {
            if ( _wcsicmp(member->XboxUserId->Data(), partyMember->XboxUserId->Data() ) == 0 )
            {
                inParty = inParty || true;
            }
        }

        if ( !inParty )
        {
            return false;
        }
    }

    return true;
}

void PartyController::SetPartyView( PartyView^ partyView )
{
    Concurrency::critical_section::scoped_lock lock(m_lock);
    m_partyView = partyView;
}

Windows::Foundation::DateTime PartyController::GetCurrentTime() 
{
    ULARGE_INTEGER uInt;
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    uInt.LowPart = ft.dwLowDateTime;
    uInt.HighPart = ft.dwHighDateTime;

    Windows::Foundation::DateTime time;
    time.UniversalTime = uInt.QuadPart;
    return time;
}

double PartyController::GetTimeBetweenInSeconds(Windows::Foundation::DateTime dt1, Windows::Foundation::DateTime dt2)
{
    const uint64 tickPerSecond = 10000000i64;
    uint64 deltaTime = dt2.UniversalTime - dt1.UniversalTime;
    return static_cast<double>(deltaTime) / static_cast<double>(tickPerSecond);
}