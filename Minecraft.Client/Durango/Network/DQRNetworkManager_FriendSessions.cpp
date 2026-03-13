#include "stdafx.h"

#include "DQRNetworkManager.h"
#include "PartyController.h"
#include <collection.h>
#include <ppltasks.h>
#include <ws2tcpip.h>
#include "../Minecraft.World/StringHelpers.h"
#include "base64.h"

#ifdef _DURANGO
#include "../Minecraft.World/DurangoStats.h"
#endif

#include "ChatIntegrationLayer.h"

using namespace Concurrency;
using namespace Windows::Foundation::Collections;

// Returns true if we are already processing a request to find game parties of friends
bool DQRNetworkManager::FriendPartyManagerIsBusy()
{
	if( m_GetFriendPartyThread )
	{
		if( m_GetFriendPartyThread->isRunning() )
		{
			return true;
		}
	}
	return false;
}

// Returns the total count of game parties that we found for our friends
int	DQRNetworkManager::FriendPartyManagerGetCount()
{
	return m_sessionResultCount;
}

// Initiate the (asynchronous) search for game parties of our friends
bool DQRNetworkManager::FriendPartyManagerSearch()
{
	if( m_GetFriendPartyThread )
	{
		if( m_GetFriendPartyThread->isRunning() )
		{
			return false;
		}
	}

	m_sessionResultCount = 0;
	delete [] m_sessionSearchResults;
	m_sessionSearchResults = nullptr;

	m_GetFriendPartyThread = new C4JThread(&_GetFriendsThreadProc,this,"GetFriendsThreadProc");
	m_GetFriendPartyThread->Run();

	return true;
}

// Get a particular search result for a game party that we have discovered. Index should be from 0 to the value returned by FriendPartyManagerGetCount.
void DQRNetworkManager::FriendPartyManagerGetSessionInfo(int idx, SessionSearchResult *searchResult)
{
	assert( idx < m_sessionResultCount );
	assert( ( m_GetFriendPartyThread == nullptr ) || ( !m_GetFriendPartyThread->isRunning()) );

	// Need to make sure that copied data has independently allocated m_extData, so both copies can be freed
	*searchResult = m_sessionSearchResults[idx];
	searchResult->m_extData = malloc(sizeof(GameSessionData));
	memcpy(searchResult->m_extData, m_sessionSearchResults[idx].m_extData, sizeof(GameSessionData));
}

int DQRNetworkManager::_GetFriendsThreadProc(void* lpParameter)
{
	DQRNetworkManager *pDQR = static_cast<DQRNetworkManager *>(lpParameter);
	return pDQR->GetFriendsThreadProc();
}

// This is the main thread that is kicked off to find game sessions associated with our friends. We have to do this
// by finding parties associated with our friends, and from the parties get the assocated game session.
int DQRNetworkManager::GetFriendsThreadProc()
{
	LogComment(L"Starting GetFriendsThreadProc");
	WXS::User^ primaryUser = ProfileManager.GetUser(0);

	if( primaryUser == nullptr )
	{
		return -1;
	}
	MXS::XboxLiveContext^ primaryUserXboxLiveContext = ref new MXS::XboxLiveContext(primaryUser);
	if( primaryUserXboxLiveContext == nullptr )
	{
		return -1;
	}

	MXSS::XboxSocialRelationshipResult^ socialRelationshipResult = nullptr;

	// First get our friends list (people we follow who may or may not follow us back), note we're requesting all friends
	auto getSocialRelationshipsAsync = primaryUserXboxLiveContext->SocialService->GetSocialRelationshipsAsync(MXSS::SocialRelationship::All, 0, 1100);
	create_task(getSocialRelationshipsAsync).then([this,&socialRelationshipResult](task<MXSS::XboxSocialRelationshipResult^> t)
	{
		try
		{
			socialRelationshipResult = t.get();
		}
		catch (Platform::COMException^ ex)
		{
			LogCommentWithError( L"GetSocialRelationshipsAsync failed", ex->HResult );
		}
	})
	.wait();
	if( socialRelationshipResult == nullptr )
	{
		return -1;
	}

	IVector<Platform::String^>^ friendXUIDs = ref new Platform::Collections::Vector<Platform::String^>;

	// Now construct a vector of these users, that follow us back - these are our "friends"
	for( int i = 0; i < socialRelationshipResult->TotalCount; i++ )
	{
		MXSS::XboxSocialRelationship^ relationship = socialRelationshipResult->Items->GetAt(i);
		if(relationship->IsFollowingCaller)
		{
			friendXUIDs->Append(relationship->XboxUserId);
		}
	}

	// If we don't have any such friends, we're done
	if( friendXUIDs->Size == 0 )
	{
		return 0;
	}

	// Now get party associations for these friends
	auto getPartyAssociationsAsync = WXM::Party::GetUserPartyAssociationsAsync(primaryUser, friendXUIDs->GetView() );

	IVectorView<WXM::UserPartyAssociation^>^ partyResults = nullptr;

	create_task(getPartyAssociationsAsync).then([this,&partyResults](task<IVectorView<WXM::UserPartyAssociation^>^> t)
	{
		try
		{
			partyResults = t.get();
		}
		catch (Platform::COMException^ ex)
		{
			LogCommentWithError( L"getPartyAssociationsAsync failed", ex->HResult );
		}
	})
	.wait();

	if( partyResults == nullptr )
	{
		return -1;
	}

	if( partyResults->Size == 0 )
	{
		return 0;
	}

	// Filter these parties by whether we have permission to see them online
	partyResults = FilterPartiesByPermission(primaryUserXboxLiveContext, partyResults);
	

	// At this point, we have Party Ids for our friends. Now we need to get Party Views for each of these Ids.

	LogComment("Parties found");
	
	// Get party views for each of the user party associations that we have. These seem to be able to (individually) raise errors, so
	// accumulate results into 2 matched vectors declared below so that we can ignore any broken UserPartyAssociations from now
	vector<WXM::PartyView^> partyViewVector;
	vector<WXM::UserPartyAssociation^> partyResultsVector;

	vector<task<void>> taskVector;
    for each(WXM::UserPartyAssociation^ remoteParty in partyResults) 
    {
		auto asyncOp = WXM::Party::GetPartyViewByPartyIdAsync( primaryUser, remoteParty->PartyId );
		task<WXM::PartyView^> asyncTask = create_task(asyncOp);
		
		taskVector.push_back(asyncTask.then([this, &partyViewVector, &partyResultsVector, remoteParty] (task<WXM::PartyView^> t)
		{
			try
			{
				WXM::PartyView^ partyView = t.get();

				if( partyView != nullptr )
				{
					app.DebugPrintf("Got party view\n");
					EnterCriticalSection(&m_csPartyViewVector);
					partyViewVector.push_back(partyView);
					partyResultsVector.push_back(remoteParty);
					LeaveCriticalSection(&m_csPartyViewVector);
				}
			}
			catch ( Platform::COMException^ ex )
			{
				app.DebugPrintf("Getting party view error 0x%x\n",ex->HResult);
			}
		}));
	}
	for( auto it = taskVector.begin(); it != taskVector.end(); it++ )
	{
		it->wait();
	}

	if( partyViewVector.size() == 0 )
	{
		return 0;
	}

	// Filter the party view, and party results vector (partyResultsVector) this is matched to, to remove any that don't have game sessions - or game sessions that aren't this game
	vector<WXM::PartyView^> partyViewVectorFiltered;
	vector<WXM::UserPartyAssociation^> partyResultsFiltered;

	for( int i = 0; i < partyViewVector.size(); i++ )
	{
		WXM::PartyView^ partyView = partyViewVector[i];

		if( partyView->Joinability == WXM::SessionJoinability::JoinableByFriends )
		{
			if( partyView->GameSession )
			{
				if( partyView->GameSession->ServiceConfigurationId == SERVICE_CONFIG_ID )
				{
					partyViewVectorFiltered.push_back( partyView );
					partyResultsFiltered.push_back( partyResultsVector[i] );
				}
			}
		}
	}

	// We now have matched vectors:
	// 
	// partyResultsFiltered
	// partyViewVectorFiltered
	// 
	// and, from the party views, we can now attempt to get game sessions

	vector<MXSM::MultiplayerSession^> sessionVector;
	vector<WXM::PartyView^> partyViewVectorValid;
	vector<WXM::UserPartyAssociation^> partyResultsValid;

  	for( int i = 0; i < partyViewVectorFiltered.size(); i++ )
	{
		WXM::PartyView^ partyView = partyViewVectorFiltered[i];
		Microsoft::Xbox::Services::Multiplayer::MultiplayerSessionReference^ sessionRef = ConvertToMicrosoftXboxServicesMultiplayerSessionReference(partyView->GameSession);

		LogComment(L"Party view vector " + sessionRef->SessionName + L" " + partyResultsFiltered[i]->QueriedXboxUserIds->GetAt(0));

		MXSM::MultiplayerSession^ session = nullptr;
		auto asyncOp = primaryUserXboxLiveContext->MultiplayerService->GetCurrentSessionAsync( sessionRef );
		create_task(asyncOp).then([&session] (task<MXSM::MultiplayerSession^> t)
		{
			try
			{
				session = t.get();
			}
			catch (Platform::COMException^ ex)
			{
			}
		})
		.wait();
		if( session )
		{
			sessionVector.push_back(session);
			partyViewVectorValid.push_back(partyView);
			partyResultsValid.push_back(partyResultsFiltered[i]);
		}
	}

	if( sessionVector.size() == 0 )
	{
		return 0;
	}

	// We now have matched vectors:
	//
	// partyResultsValid
	// partyViewVectorValid
	// sessionVector

	// The next stage is to resolve the display names for the XUIDs of all the players in each of the sessions. It is possible that
	// a session won't have any XUIDs to resolve, which would make GetUserProfilesAsync unhappy, so we'll only be creating a task
	// when there are members. Creating new matching arrays for party results and sessions, to match the results (we don't care about the party view anymore)

	vector<task<IVectorView<MXSS::XboxUserProfile^>^>> nameResolveTaskVector;
	vector<IVectorView<MXSS::XboxUserProfile^>^> nameResolveVector;
	vector<MXSM::MultiplayerSession^>  newSessionVector;
	vector<WXM::UserPartyAssociation^> newPartyVector;

	for( int j = 0; j < sessionVector.size(); j++ )
	{
		MXSM::MultiplayerSession^ session = sessionVector[j];
		IVector<Platform::String^>^ memberXUIDs = ref new Platform::Collections::Vector<Platform::String^>;

		Windows::Data::Json::JsonArray^ roomSyncArray = nullptr;
		try
		{
			Windows::Data::Json::JsonObject^ customJson = Windows::Data::Json::JsonObject::Parse(session->SessionProperties->SessionCustomPropertiesJson);
			Windows::Data::Json::JsonValue^ customValue = customJson->GetNamedValue(L"RoomSyncData");
			roomSyncArray = customValue->GetArray();
			LogComment("Attempting to parse RoomSyncData");
			for( int i = 0; i < roomSyncArray->Size; i++ )
			{
				LogComment(roomSyncArray->GetAt(i)->GetString());
			}
		}
		catch (Platform::COMException^ ex)
		{
			LogCommentWithError( L"Custom RoomSyncData Parse/GetNamedValue failed", ex->HResult );
			continue;
		}

		if( roomSyncArray && ( roomSyncArray->Size > 0 ) )
		{
			// For each session, we want to order these XUIDs so the display name of the first one is what we will name the session by. Prioritise doing this by:
			//
			// (1) If the host player (indicated by having a small id of 0) is our friend, use that
			// (2) Otherwise use anyone who is our friend

			// Default to true
			bool friendsOfFriends = true;

			int hostIndexFound = -1;
			int friendIndexFound = -1;

			friendsOfFriends = IsSessionFriendsOfFriends(session);

			for( int i = 0; i < roomSyncArray->Size; i++ )
			{
				Platform::String^ roomSyncXuid = roomSyncArray->GetAt(i)->GetString();

				// Determine if this player is a friend
				bool isFriend = false;
				for each( Platform::String^ friendXUID in friendXUIDs )
				{
					if( friendXUID == roomSyncXuid )
					{
						isFriend = true;
						break;
					}
				}

				bool isHost = i == 0;

				// Store that what we found at this index if it is a friend, or a friend who is a host
				if( isFriend && ( friendsOfFriends || isHost ) )
				{
					friendIndexFound = i;
					if( isHost ) // Host is always in slot 0
					{
						hostIndexFound = i;
					}
				}
			}

			// Prefer to use index of host who is our friend
			int bestIndex = friendIndexFound;
			if( hostIndexFound != -1 )
			{
				bestIndex = hostIndexFound;
			}

			// Only consider if we have at least found one friend in the list of players
			if( bestIndex != -1 )
			{
				// Compile list of XUIDs to resolve with our specially chosen player as entry 0, then the rest
				memberXUIDs->Append(roomSyncArray->GetAt(bestIndex)->GetString());
				for( int i = 0; i < roomSyncArray->Size; i++ )
				{
					if( i != bestIndex )
					{
						memberXUIDs->Append(roomSyncArray->GetAt(i)->GetString());
					}
				}
				nameResolveTaskVector.push_back( create_task( primaryUserXboxLiveContext->ProfileService->GetUserProfilesAsync( memberXUIDs->GetView() ) ) );
				newSessionVector.push_back(session);
				newPartyVector.push_back(partyResultsValid[j]);
			}
		}
	}

	try
	{
		auto joinTask = when_all(begin(nameResolveTaskVector), end(nameResolveTaskVector) ).then([this, &nameResolveVector](vector<IVectorView<MXSS::XboxUserProfile^>^> results)
		{
			nameResolveVector = results;
		})
		.wait();
	}
	catch(Platform::COMException^ ex)
	{
		return -1;
	}

	// We now have matched vectors:
	//
	// newPartyVector				- contains the party Ids that we'll need should we wish to join
	// nameResolveVector			- contains vectors views of the names of the members of the session each of these parties is in
	// newSessionVector				- contains the session information itself associated with each of the parties

	// Construct the final result vector
	m_sessionResultCount = newSessionVector.size();
	m_sessionSearchResults = new SessionSearchResult[m_sessionResultCount];
	for( int i = 0; i < m_sessionResultCount; i++ )
	{
		m_sessionSearchResults[i].m_partyId = newPartyVector[i]->PartyId->Data();
		m_sessionSearchResults[i].m_sessionName = newSessionVector[i]->SessionReference->SessionName->Data();
		for( int j = 0; j < nameResolveVector[i]->Size; j++ )
		{
			m_sessionSearchResults[i].m_playerNames[j] = nameResolveVector[i]->GetAt(j)->GameDisplayName->Data();
			m_sessionSearchResults[i].m_playerXuids[j] = static_cast<PlayerUID>(nameResolveVector[i]->GetAt(j)->XboxUserId->Data());
		}
		m_sessionSearchResults[i].m_playerCount = nameResolveVector[i]->Size;
		m_sessionSearchResults[i].m_usedSlotCount = newSessionVector[i]->Members->Size;
		if( m_sessionSearchResults[i].m_usedSlotCount > MAX_ONLINE_PLAYER_COUNT )
		{
			// Don't think this could ever happen, but no harm in checking
			m_sessionSearchResults[i].m_usedSlotCount = MAX_ONLINE_PLAYER_COUNT;
		}
		for( int j = 0; j < m_sessionSearchResults[i].m_usedSlotCount; j++ )
		{
			m_sessionSearchResults[i].m_sessionXuids[j] = wstring( newSessionVector[i]->Members->GetAt(j)->XboxUserId->Data() );
		}
		
		m_sessionSearchResults[i].m_extData = malloc( sizeof(GameSessionData) );
		memset( m_sessionSearchResults[i].m_extData, 0, sizeof(GameSessionData) );
		
		GetGameSessionData(newSessionVector[i], m_sessionSearchResults[i].m_extData);
	}

	return 0;
}

// Filters list of parties based on online presence permission (whether the friend is set to invisible or not)
IVectorView<WXM::UserPartyAssociation^>^ DQRNetworkManager::FilterPartiesByPermission(MXS::XboxLiveContext ^context, IVectorView<WXM::UserPartyAssociation^>^ partyResults)
{
	Platform::Collections::Vector<WXM::UserPartyAssociation^>^ filteredPartyResults = ref new Platform::Collections::Vector<WXM::UserPartyAssociation^>();

	// List of permissions we want
	auto permissionIds = ref new Platform::Collections::Vector<Platform::String^>(1, ref new Platform::String(L"ViewTargetPresence"));	

	// List of target users
	auto targetXboxUserIds = ref new Platform::Collections::Vector<Platform::String^>();
	for (int i = 0; i < partyResults->Size; i++)
	{
		assert(partyResults->GetAt(i)->QueriedXboxUserIds->Size > 0);
		targetXboxUserIds->Append( partyResults->GetAt(i)->QueriedXboxUserIds->GetAt(0) );
	}

	// Check
	auto checkPermissionsAsync = context->PrivacyService->CheckMultiplePermissionsWithMultipleTargetUsersAsync(permissionIds->GetView(), targetXboxUserIds->GetView());
	create_task(checkPermissionsAsync).then([&partyResults, &filteredPartyResults](task<IVectorView<MXS::Privacy::MultiplePermissionsCheckResult^>^> t)
	{
		try
		{
			auto results = t.get();

			// For each party, check to see if we have permission for the user
			for (int i = 0; i < partyResults->Size; i++)
			{
				// For each permissions result
				for (int j = 0; j < results->Size; j++)
				{
					auto result = results->GetAt(j);

					// If allowed to see this user AND it's the same user, add the party to the just
					if ((result->Items->GetAt(0)->IsAllowed) && (partyResults->GetAt(i)->QueriedXboxUserIds->GetAt(0) == result->XboxUserId))
					{
						filteredPartyResults->Append(partyResults->GetAt(i));
						break;
					}
				}
			}
		}
		catch (Platform::COMException^ ex)
		{
			LogCommentWithError( L"CheckMultiplePermissionsWithMultipleTargetUsersAsync failed", ex->HResult );
		}
	})
	.wait();

	app.DebugPrintf("DQRNetworkManager::FilterPartiesByPermission: Removed %i parties because of online presence permissions\n", partyResults->Size - filteredPartyResults->Size);

	return filteredPartyResults->GetView();
}

// Get all friends (list of XUIDs) syncronously from the service (slow, may take 300ms+), returns empty list if something goes wrong
Platform::Collections::Vector<Platform::String^>^ DQRNetworkManager::GetFriends()
{	
	auto friends = ref new Platform::Collections::Vector<Platform::String^>;

	auto primaryUser = ProfileManager.GetUser(0);
	if (primaryUser == nullptr)
	{
		// Return empty
		return friends;
	}

	auto xboxLiveContext = ref new MXS::XboxLiveContext(primaryUser);

	// Request ALL friends because there's no other way to check friendships without using the REST API
	auto getSocialRelationshipsAsync = xboxLiveContext->SocialService->GetSocialRelationshipsAsync(MXSS::SocialRelationship::All, 0, 1100);
	MXSS::XboxSocialRelationshipResult^ socialRelationshipResult = nullptr;

	// First get our friends list (people we follow who may or may not follow us back)
	Concurrency::create_task(getSocialRelationshipsAsync).then([&socialRelationshipResult](Concurrency::task<MXSS::XboxSocialRelationshipResult^> t)
	{
		try
		{
			socialRelationshipResult = t.get();
		}
		catch (Platform::COMException^ ex)
		{
			app.DebugPrintf("DQRNetworkManager::GetFriends: GetSocialRelationshipsAsync failed ()\n", ex->HResult);
		}
	})
	.wait();

	if (socialRelationshipResult == nullptr)
	{
		// Return empty
		return friends;
	}

	app.DebugPrintf("DQRNetworkManager::GetFriends: Retrieved %i relationships\n", socialRelationshipResult->TotalCount);

	// Now construct a vector of these users, that follow us back - these are our "friends"
	for( int i = 0; i < socialRelationshipResult->TotalCount; i++ )
	{
		MXSS::XboxSocialRelationship^ relationship = socialRelationshipResult->Items->GetAt(i);
		if(relationship->IsFollowingCaller)
		{
			app.DebugPrintf("DQRNetworkManager::GetFriends: Found friend \"%ls\"\n", relationship->XboxUserId->Data());
			friends->Append(relationship->XboxUserId);
		}
	}

	app.DebugPrintf("DQRNetworkManager::GetFriends: Found %i 2-way friendships\n", friends->Size);

	return friends;
}

// If data for game settings exists returns FriendsOfFriends value, otherwise returns true
bool DQRNetworkManager::IsSessionFriendsOfFriends(MXSM::MultiplayerSession^ session)
{
	// Default to true, don't want to incorrectly prevent joining
	bool friendsOfFriends = true;

	// We retrieve the game session data later too, shouldn't really duplicate this
	void *gameSessionData = malloc( sizeof(GameSessionData));
	memset(gameSessionData, 0, sizeof(GameSessionData));

	bool result = GetGameSessionData(session, gameSessionData);

	if (result)
	{
		friendsOfFriends = app.GetGameHostOption(static_cast<GameSessionData *>(gameSessionData)->m_uiGameHostSettings, eGameHostOption_FriendsOfFriends);
	}

	free(gameSessionData);

	return friendsOfFriends;
}

// Parses custom json data from session and populates game session data param, return true if parse succeeded
bool DQRNetworkManager::GetGameSessionData(MXSM::MultiplayerSession^ session, void *gameSessionData)
{
	Platform::String ^gameSessionDataJson = session->SessionProperties->SessionCustomPropertiesJson;
	if( gameSessionDataJson )
	{
		try
		{
			Windows::Data::Json::JsonObject^ customParam = Windows::Data::Json::JsonObject::Parse(gameSessionDataJson);
			Windows::Data::Json::JsonValue^ customValue = customParam->GetNamedValue(L"GameSessionData");
			Platform::String ^customValueString = customValue->GetString();
			if( customValueString )
			{
				base64_decode( customValueString, static_cast<unsigned char *>(gameSessionData), sizeof(GameSessionData) );
				return true;
			}			
		}
		catch (Platform::COMException^ ex)
		{
			LogCommentWithError( L"Custom GameSessionData parameter Parse/GetNamedValue failed", ex->HResult );
		}
	}

	return false;
}