#pragma once
#include <np.h>
#ifdef __PS3__
#include <netex\libnetctl.h>
#include <netex\net.h>
#else
#include <libnetctl.h>
#include <net.h>
#include <np_toolkit.h>
#endif
#include <queue>

#include <unordered_map>
#if defined __PSVITA__
#include "..\..\Minecraft.Client\PSVita\4JLibs\inc\4J_Profile.h"
#endif

class SQRNetworkPlayer;
class ISQRNetworkManagerListener;
class SonyVoiceChat;
class C4JThread;

// This is the lowest level manager for providing network functionality on Sony platforms. This manages various network activities including the players within a gaming session.
// The game shouldn't directly use this class, it is here to provide functionality required by PlatformNetworkManagerSony.

class SQRNetworkManager
{
public:
	static const int MAX_LOCAL_PLAYER_COUNT = XUSER_MAX_COUNT;
	static const int MAX_ONLINE_PLAYER_COUNT = MINECRAFT_NET_MAX_PLAYERS;

	static const int NP_POOL_SIZE = 128 * 1024;
protected:
	friend class SQRNetworkPlayer;
	friend class SonyVoiceChat;
#ifdef __PSVITA__
	friend class HelloSyncInfo;
#endif

	static const int MAX_FRIENDS = 100;
#ifdef __PS3__
	static const int RUDP_THREAD_PRIORITY = 999;
#else // __ORBIS_
	static const int RUDP_THREAD_PRIORITY = 500;
#endif
	static const int RUDP_THREAD_STACK_SIZE = 32878;
	static const int MAX_SIMULTANEOUS_INVITES = 10;
	

	// This class stores everything about a player that must be synchronised between machines. This syncing is carried out
	// by the Matching2 lib by using internal room binary data (ie data that is only visible to current members of a room)
	class PlayerSyncData
	{
	public:
		PlayerUID					m_UID;					// Assigned by the associated player->GetUID()
		SceNpMatching2RoomMemberId	m_roomMemberId;			// Assigned by Matching2 lib, we can use to indicate which machine this player belongs to (note - 16 bits)
		unsigned char				m_smallId;				// Assigned by SQRNetworkManager, to attach a permanent id to this player (until we have to wrap round), to match a similar concept in qnet
		unsigned char				m_localIdx : 4;			// Which local player (by controller index) this represents
		unsigned char				m_playerCount : 4;
	};

	class RoomSyncData
	{
	public:
		PlayerSyncData players[MAX_ONLINE_PLAYER_COUNT];
		void setPlayerCount(int c) { players[0].m_playerCount = c;}
		int getPlayerCount() { return players[0].m_playerCount;}
	};

public:
	class PresenceSyncInfo
	{
	public:
		GameSessionUID									hostPlayerUID;
		SceNpMatching2RoomId							m_RoomId;
		SceNpMatching2ServerId							m_ServerId;
		unsigned int									texturePackParentId;
		unsigned short									netVersion;
		unsigned char									subTexturePackId;
		bool											inviteOnly;
	};


	// Externally exposed state. All internal states are mapped to one of these broader states.
	typedef enum
	{
		SNM_STATE_INITIALISING,
		SNM_STATE_INITIALISE_FAILED,
		SNM_STATE_IDLE,

		SNM_STATE_HOSTING,
		SNM_STATE_JOINING,

		SNM_STATE_STARTING,
		SNM_STATE_PLAYING,		

		SNM_STATE_LEAVING,
		SNM_STATE_ENDING,
	} eSQRNetworkManagerState;

	struct SessionID
	{
		SceNpMatching2RoomId							m_RoomId;
		SceNpMatching2ServerId							m_ServerId;
	};

	struct SessionSearchResult
	{
		SceNpId											m_NpId;
		SessionID										m_sessionId;
		void											*m_extData;
#ifdef __PSVITA__
		SceNetInAddr									m_netAddr;
#endif
	};

protected:

	// On initialisation, state should transition from SNM_INT_STATE_UNINITIALISED -> SNM_INT_STATE_SIGNING_IN -> SNM_INT_STATE_SIGNED_IN -> SNM_INT_STATE_STARTING_CONTEXT -> SNM_INT_STATE_IDLE.
	// Error indicated if we transition at any point to SNM_INT_STATE_INITIALISE_FAILED.

	// NOTE: If anything changes in here, then the mapping from internal -> external state needs to be updated (m_INTtoEXTStateMappings, defined in the cpp file)
	typedef enum
	{
		SNM_INT_STATE_UNINITIALISED,
		SNM_INT_STATE_SIGNING_IN,
		SNM_INT_STATE_STARTING_CONTEXT,
		SNM_INT_STATE_INITIALISE_FAILED,

		SNM_INT_STATE_IDLE,
		SNM_INT_STATE_IDLE_RECREATING_MATCHING_CONTEXT,

		SNM_INT_STATE_HOSTING_STARTING_MATCHING_CONTEXT,
		SNM_INT_STATE_HOSTING_SEARCHING_FOR_SERVER,
		SNM_INT_STATE_HOSTING_SERVER_SEARCH_SERVER_ERROR,
		SNM_INT_STATE_HOSTING_SERVER_FOUND,
		SNM_INT_STATE_HOSTING_SERVER_SEARCH_CREATING_CONTEXT,
		SNM_INT_STATE_HOSTING_SERVER_SEARCH_FAILED,

		SNM_INT_STATE_HOSTING_CREATE_ROOM_SEARCHING_FOR_WORLD,
		SNM_INT_STATE_HOSTING_CREATE_ROOM_WORLD_FOUND,
		SNM_INT_STATE_HOSTING_CREATE_ROOM_CREATING_ROOM,
		SNM_INT_STATE_HOSTING_CREATE_ROOM_SUCCESS,
		SNM_INT_STATE_HOSTING_CREATE_ROOM_FAILED,
		SNM_INT_STATE_HOSTING_CREATE_ROOM_RESTART_MATCHING_CONTEXT,
		SNM_INT_STATE_HOSTING_WAITING_TO_PLAY,

		SNM_INT_STATE_JOINING_STARTING_MATCHING_CONTEXT,
		SNM_INT_STATE_JOINING_SEARCHING_FOR_SERVER,
		SNM_INT_STATE_JOINING_SERVER_SEARCH_SERVER_ERROR,
		SNM_INT_STATE_JOINING_SERVER_FOUND,
		SNM_INT_STATE_JOINING_SERVER_SEARCH_CREATING_CONTEXT,
		SNM_INT_STATE_JOINING_SERVER_SEARCH_FAILED,

		SNM_INT_STATE_JOINING_JOIN_ROOM,
		SNM_INT_STATE_JOINING_JOIN_ROOM_FAILED,

		SNM_INT_STATE_JOINING_WAITING_FOR_LOCAL_PLAYERS,

		SNM_INT_STATE_SERVER_DELETING_CONTEXT,

		SNM_INT_STATE_STARTING,
		SNM_INT_STATE_PLAYING,

		SNM_INT_STATE_LEAVING,
		SNM_INT_STATE_LEAVING_FAILED,

		SNM_INT_STATE_ENDING,

		SNM_INT_STATE_COUNT
		
	} eSQRNetworkManagerInternalState;

	typedef enum
	{
		SNM_FORCE_ERROR_NP2_INIT,
		SNM_FORCE_ERROR_NET_INITIALIZE_NETWORK,
		SNM_FORCE_ERROR_NET_CTL_INIT,
		SNM_FORCE_ERROR_RUDP_INIT,
		SNM_FORCE_ERROR_NET_START_DIALOG,
		SNM_FORCE_ERROR_MATCHING2_INIT,
		SNM_FORCE_ERROR_REGISTER_NP_CALLBACK,
		SNM_FORCE_ERROR_GET_NPID,
		SNM_FORCE_ERROR_CREATE_MATCHING_CONTEXT,
		SNM_FORCE_ERROR_REGISTER_CALLBACKS,
		SNM_FORCE_ERROR_CONTEXT_START_ASYNC,
		SNM_FORCE_ERROR_SET_EXTERNAL_ROOM_DATA,
		SNM_FORCE_ERROR_GET_FRIEND_LIST_ENTRY_COUNT,
		SNM_FORCE_ERROR_GET_FRIEND_LIST_ENTRY,
		SNM_FORCE_ERROR_GET_USER_INFO_LIST,
		SNM_FORCE_ERROR_LEAVE_ROOM,
		SNM_FORCE_ERROR_SET_ROOM_MEMBER_DATA_INTERNAL,
		SNM_FORCE_ERROR_SET_ROOM_MEMBER_DATA_INTERNAL2,
		SNM_FORCE_ERROR_CREATE_SERVER_CONTEXT,
		SNM_FORCE_ERROR_CREATE_JOIN_ROOM,
		SNM_FORCE_ERROR_GET_SERVER_INFO,
		SNM_FORCE_ERROR_DELETE_SERVER_CONTEXT,
		SNM_FORCE_ERROR_SETSOCKOPT_0,
		SNM_FORCE_ERROR_SETSOCKOPT_1,
		SNM_FORCE_ERROR_SETSOCKOPT_2,
		SNM_FORCE_ERROR_SOCK_BIND,
		SNM_FORCE_ERROR_CREATE_RUDP_CONTEXT,
		SNM_FORCE_ERROR_RUDP_BIND,
		SNM_FORCE_ERROR_RUDP_INIT2,
		SNM_FORCE_ERROR_GET_ROOM_EXTERNAL_DATA,
		SNM_FORCE_ERROR_GET_SERVER_INFO_DATA,
		SNM_FORCE_ERROR_GET_WORLD_INFO_DATA,
		SNM_FORCE_ERROR_GET_CREATE_JOIN_ROOM_DATA,
		SNM_FORCE_ERROR_GET_USER_INFO_LIST_DATA,
		SNM_FORCE_ERROR_GET_JOIN_ROOM_DATA,
		SNM_FORCE_ERROR_GET_ROOM_MEMBER_DATA_INTERNAL,
		SNM_FORCE_ERROR_GET_ROOM_EXTERNAL_DATA2,
		SNM_FORCE_ERROR_CREATE_SERVER_CONTEXT_CALLBACK,
		SNM_FORCE_ERROR_SET_ROOM_DATA_CALLBACK,
		SNM_FORCE_ERROR_UPDATED_ROOM_DATA,
		SNM_FORCE_ERROR_UPDATED_ROOM_MEMBER_DATA_INTERNAL1,
		SNM_FORCE_ERROR_UPDATED_ROOM_MEMBER_DATA_INTERNAL2,
		SNM_FORCE_ERROR_UPDATED_ROOM_MEMBER_DATA_INTERNAL3,
		SNM_FORCE_ERROR_UPDATED_ROOM_MEMBER_DATA_INTERNAL4,
		SNM_FORCE_ERROR_GET_WORLD_INFO_LIST,
		SNM_FORCE_ERROR_JOIN_ROOM,

		SNM_FORCE_ERROR_COUNT,
	} eSQRForceError;


	class StateChangeInfo
	{
	public:
		eSQRNetworkManagerState m_oldState;
		eSQRNetworkManagerState m_newState;
		bool m_idleReasonIsSessionFull;
		StateChangeInfo(eSQRNetworkManagerState oldState, eSQRNetworkManagerState newState,bool idleReasonIsSessionFull) : m_oldState(oldState), m_newState(newState), m_idleReasonIsSessionFull(idleReasonIsSessionFull) {}
	};

	std::queue<StateChangeInfo>						m_stateChangeQueue;
	CRITICAL_SECTION								m_csStateChangeQueue;
	CRITICAL_SECTION								m_csMatching;
	CRITICAL_SECTION								m_csAckQueue;
	std::queue<int>									m_queuedAckRequests;

	typedef enum
	{
		SNM_FRIEND_SEARCH_STATE_IDLE,						// Idle - search result will be valid (although it may not have any entries)
		SNM_FRIEND_SEARCH_STATE_GETTING_FRIEND_COUNT,		// Getting count of friends in friend list
		SNM_FRIEND_SEARCH_STATE_GETTING_FRIEND_INFO,		// Getting presence/NpId info for each friend
	} eSQRNetworkManagerFriendSearchState;

	typedef void (*ServerContextValidCallback)(SQRNetworkManager *manager);

	static bool									    s_safeToRespondToGameBootInvite;

public:

	// General 
	virtual void						Tick() = 0;
	virtual void						Initialise() = 0;
#ifdef __PSVITA__
	virtual void						UnInitialise() = 0; // to switch from PSN to Adhoc
	virtual bool						IsInitialised() = 0;
#endif
	virtual void						Terminate() = 0;
	virtual eSQRNetworkManagerState		GetState() = 0;
	virtual bool						IsHost() = 0;
	virtual bool						IsReadyToPlayOrIdle() = 0;
	virtual bool						IsInSession() = 0;

	// Session management
	virtual void						CreateAndJoinRoom(int hostIndex, int localPlayerMask, void *extData, int extDataSize, bool offline) = 0;
	virtual void						UpdateExternalRoomData() = 0;
	virtual bool						FriendRoomManagerIsBusy() = 0;
	virtual bool						FriendRoomManagerSearch() = 0;
	virtual bool						FriendRoomManagerSearch2() = 0;
	virtual int							FriendRoomManagerGetCount() = 0;
	virtual void						FriendRoomManagerGetRoomInfo(int idx, SessionSearchResult *searchResult) = 0;
	virtual bool						JoinRoom(SessionSearchResult *searchResult, int localPlayerMask) = 0;
	virtual bool						JoinRoom(SceNpMatching2RoomId roomId, SceNpMatching2ServerId serverId, int localPlayerMask, const SQRNetworkManager::PresenceSyncInfo *presence) = 0;
	virtual void						StartGame() = 0;
	virtual void						LeaveRoom(bool bActuallyLeaveRoom) = 0;
	virtual void						EndGame() = 0;
	virtual bool						SessionHasSpace(int spaceRequired) = 0;
	virtual bool						AddLocalPlayerByUserIndex(int idx) = 0;
	virtual bool						RemoveLocalPlayerByUserIndex(int idx) = 0;
	virtual void						SendInviteGUI() = 0;

	virtual void						GetExtDataForRoom( SceNpMatching2RoomId roomId, void *extData, void (* FriendSessionUpdatedFn)(bool success, void *pParam), void *pParam ) = 0;

	// Player retrieval
	virtual int							GetPlayerCount() = 0;
	virtual int							GetOnlinePlayerCount() = 0;
	virtual SQRNetworkPlayer			*GetPlayerByIndex(int idx) = 0;
	virtual SQRNetworkPlayer			*GetPlayerBySmallId(int idx) = 0;
	virtual SQRNetworkPlayer			*GetPlayerByXuid(PlayerUID xuid) = 0;
	virtual SQRNetworkPlayer			*GetLocalPlayerByUserIndex(int idx) = 0;
	virtual SQRNetworkPlayer			*GetHostPlayer() = 0;

	virtual void						SetPresenceDataStartHostingGame() = 0;
	virtual int							GetJoiningReadyPercentage() = 0;

	virtual void						LocalDataSend(SQRNetworkPlayer *playerFrom, SQRNetworkPlayer *playerTo, const void *data, unsigned int dataSize) = 0;
	virtual int							GetSessionIndex(SQRNetworkPlayer *player) = 0;

	static void							SafeToRespondToGameBootInvite();

	int									GetOutstandingAckCount(SQRNetworkPlayer *pSonyPlayer);
	int									GetSendQueueSizeBytes();
	int									GetSendQueueSizeMessages();
	void								RequestWriteAck(int smallId);
	void								TickWriteAcks();


};


// Class defining interface to be implemented for class that handles callbacks
class ISQRNetworkManagerListener
{
public:
	virtual void HandleDataReceived(SQRNetworkPlayer *playerFrom, SQRNetworkPlayer *playerTo, unsigned char *data, unsigned int dataSize) = 0;
	virtual void HandlePlayerJoined(SQRNetworkPlayer *player) = 0;
	virtual void HandlePlayerLeaving(SQRNetworkPlayer *player) = 0;
	virtual void HandleStateChange(SQRNetworkManager::eSQRNetworkManagerState oldState, SQRNetworkManager::eSQRNetworkManagerState newState, bool idleReasonIsSessionFull) = 0;
	virtual void HandleResyncPlayerRequest(SQRNetworkPlayer **aPlayers) = 0;
	virtual void HandleAddLocalPlayerFailed(int idx) = 0;
	virtual void HandleDisconnect(bool bLostRoomOnly,bool bPSNSignOut=false) = 0;
	virtual void HandleInviteReceived( int userIndex, const SQRNetworkManager::PresenceSyncInfo *pInviteInfo) = 0;
};
