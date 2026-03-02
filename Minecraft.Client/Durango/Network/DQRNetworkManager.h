#pragma once

#include "DQRNetworkPlayer.h"
#include "..\Minecraft.World\C4JThread.h"
#include <collection.h>

class IDQRNetworkManagerListener;
class PartyController;

class DQRNetworkManager;
class ChatIntegrationLayer;

namespace MXS = Microsoft::Xbox::Services;
namespace MXSM = Microsoft::Xbox::Services::Multiplayer;
namespace MXSS = Microsoft::Xbox::Services::Social;
namespace WXM = Windows::Xbox::Multiplayer;
namespace WXN = Windows::Xbox::Networking;
namespace WXNRs = Windows::Xbox::Networking::RealtimeSession;
namespace WFC = Windows::Foundation::Collections;

#define MATCH_SESSION_TEMPLATE_NAME    L"PeerToHostTemplate"
#define MAX_PLAYERS_IN_TEMPLATE		   8

using namespace std;

ref class DQRNetworkManagerEventHandlers sealed
{
internal:
	DQRNetworkManagerEventHandlers(DQRNetworkManager *pDQRNet);
public:
	void Setup(WXNRs::Session^ session);
	void Pulldown(WXNRs::Session^ session);

    void DataReceivedHandler(Platform::Object^ session, WXNRs::DataReceivedEventArgs^ args);
    void SessionAddressDataChangedHandler(Platform::Object^ session, WXNRs::SessionAddressDataChangedEventArgs^ args);
    void SessionStatusUpdateHandler(Platform::Object^ session, WXNRs::SessionStatusUpdateEventArgs^ args);
    void AddedSessionAddressHandler(Platform::Object^ session, WXNRs::AddedSessionAddressEventArgs^ args);
    void RemovedSessionAddressHandler(Platform::Object^ session, WXNRs::RemovedSessionAddressEventArgs^ args);
    void GlobalSessionDataChangedHandler(Platform::Object^ session, WXNRs::GlobalSessionDataChangedEventArgs^ args);

private:
    Windows::Foundation::EventRegistrationToken	m_dataReceivedToken;
    Windows::Foundation::EventRegistrationToken	m_sessionStatusToken;
    Windows::Foundation::EventRegistrationToken	m_sessionAddressToken;
    Windows::Foundation::EventRegistrationToken	m_addedSessionToken;
    Windows::Foundation::EventRegistrationToken	m_removedSessionToken;
    Windows::Foundation::EventRegistrationToken	m_globalDataToken;

	DQRNetworkManager *m_pDQRNet;
};

typedef enum
{
	DQR_INTERNAL_ASSIGN_SMALL_IDS,
	DQR_INTERNAL_UNASSIGN_SMALL_ID,
	DQR_INTERNAL_PLAYER_TABLE,
	DQR_INTERNAL_ADD_PLAYER_FAILED,
};

class DQRConnectionInfo
{
public:
	typedef enum
	{
		ConnectionState_HeaderByte0,
		ConnectionState_HeaderByte1,
		ConnectionState_ReadBytes
	} eDQRConnectionState;

	typedef enum
	{
		ConnectionState_InternalHeaderByte,
		ConnectionState_InternalAssignSmallIdMask,
		ConnectionState_InternalAssignSmallId0,
		ConnectionState_InternalAssignSmallId1,
		ConnectionState_InternalAssignSmallId2,
		ConnectionState_InternalAssignSmallId3,
		ConnectionState_InternalRoomSyncData,
		ConnectionState_InternalAddPlayerFailedData,
	} eDQRInternalDataState;

	DQRConnectionInfo();
	void		  Reset();

	eDQRConnectionState		m_state;
	eDQRInternalDataState	m_internalDataState;

	int						m_currentChannel;
	bool					m_internalFlag;
	int						m_bytesRemaining;
	int						m_roomSyncDataBytesRead;
	int						m_roomSyncDataBytesToRead;
	unsigned char *			m_pucRoomSyncData;
	int						m_addFailedPlayerDataBytesRead;
	int						m_addFailedPlayerDataBytesToRead;
	unsigned char *			m_pucAddFailedPlayerData;
	unsigned char			m_smallId[4];
	bool					m_channelActive[4];
	unsigned char			m_smallIdReadMask;
	unsigned char			*m_pucsmallIdReadMaskResolved;
	bool					m_initialPacketReceived;
};


class DQRNetworkManager
{
	friend class PartyController;
	friend ref class DQRNetworkManagerEventHandlers;
	friend class DQRNetworkPlayer;
	friend class ChatIntegrationLayer;
public:
	static const int JOIN_PACKET_RESEND_DELAY_MS = 200;
	static const int JOIN_PACKET_RESEND_TIMEOUT_MS = 20000;
	static const int JOIN_RESERVATION_WAIT_TIME = 30000;
	static const int JOIN_CREATE_SESSION_MAX_ATTEMPTS = 5;

	static const int PRIMARY_PLAYER_LEAVING_PARTY_WAIT_MS = 20000;		// Time between primary player leaving and when we should respond to it, to allow time for us to receive a new party for them to be going to if they are just changing party rather than leaving altogether

	class SessionInfo
	{
	public:
		SessionInfo(wstring& sessionName, wstring& serviceConfig, wstring& sessionTemplate);
		SessionInfo();
		bool m_detailsValid;
		wstring m_sessionName;
		wstring m_serviceConfig;
		wstring m_sessionTemplate;
	};

	static const int MAX_LOCAL_PLAYER_COUNT = 4;
	static const int MAX_ONLINE_PLAYER_COUNT = 8;
	static const int MAX_ONLINE_PLAYER_NAME_LENGTH = 21;

	// This class stores everything about a player that must be synchronised between machines. 
	class PlayerSyncData
	{
	public:
		wchar_t						*m_XUID;								// XUID / XboxUserIds are passed round as decimal strings on Xbox 1
		uint32_t					m_sessionAddress;						// XRNS session address for this player, ie can identify which machine this player is on
		uint8_t						m_smallId;								// Assigned by DQRNetworkManager, to attach a permanent id to this player (until we have to wrap round), to match a similar concept in qnet
		uint8_t						m_channel;								// Local index / communication channel within session address, of player on this machine
		wchar_t						m_name[MAX_ONLINE_PLAYER_NAME_LENGTH];
	};

	class RoomSyncData
	{
	public:
		int				playerCount;
		PlayerSyncData	players[MAX_ONLINE_PLAYER_COUNT];
	};

	class HostGamertagResolveDetails
	{
	public:;
		DQRNetworkPlayer*	m_pPlayer;
		wstring				m_name;
		unsigned int		m_sessionAddress;
		int					m_channel;
		bool				m_sync;
	};

	DQRNetworkManager(IDQRNetworkManagerListener *listener);
	void Initialise();
	void EnableDebugXBLContext(MXS::XboxLiveContext^ XBLContext);

	void CreateAndJoinSession(int userMask, unsigned char *customSessionData, unsigned int customSessionDataSize, bool offline);
	void JoinSession(int playerMask);
	void JoinSessionFromInviteInfo(int playerMask);
	bool AddUsersToSession(int playerMask, MXSM::MultiplayerSessionReference^ sessionRef );
	void UpdateCustomSessionData();

	bool AddLocalPlayerByUserIndex(int userIndex);
	bool RemoveLocalPlayerByUserIndex(int userIndex);

	bool IsHost();
	bool IsInSession();

	// Player retrieval
	int					GetPlayerCount();
	int					GetOnlinePlayerCount();
	DQRNetworkPlayer	*GetPlayerByIndex(int idx);
	DQRNetworkPlayer	*GetPlayerBySmallId(int idx);
	DQRNetworkPlayer	*GetPlayerByXuid(PlayerUID xuid);
	wstring				GetDisplayNameByGamertag(wstring gamertag);
	DQRNetworkPlayer	*GetLocalPlayerByUserIndex(int idx);
	DQRNetworkPlayer	*GetHostPlayer();
	int					GetSessionIndex(DQRNetworkPlayer *player);
	void				Tick();
	void				Tick_XRNS();
	void				Tick_VoiceChat();
	void				Tick_Party();
	void				Tick_CustomSessionData();
	void				Tick_AddAndRemoveLocalPlayers();
	void				Tick_ResolveGamertags();
	void				Tick_PartyProcess();
	void				Tick_StateMachine();
	void				Tick_CheckInviteParty();

	bool				ShouldMessageForFullSession();
	void				FlagInvitedToFullSession();
	void				UnflagInvitedToFullSession();
	// Externally exposed state. All internal states are mapped to one of these broader states.
	typedef enum
	{
		DNM_STATE_INITIALISING,
		DNM_STATE_INITIALISE_FAILED,
		DNM_STATE_IDLE,

		DNM_STATE_HOSTING,
		DNM_STATE_JOINING,

		DNM_STATE_STARTING,
		DNM_STATE_PLAYING,		

		DNM_STATE_LEAVING,
		DNM_STATE_ENDING,
	} eDQRNetworkManagerState;

	eDQRNetworkManagerState												GetState();

	class SessionSearchResult
	{
	public:
		SessionSearchResult() { m_extData = NULL; }
		~SessionSearchResult() { free(m_extData); }
		wstring			m_partyId;
		wstring			m_sessionName;

		// These names/xuids reflect the server controlled list of who is actually in the game
		wstring			m_playerNames[MAX_ONLINE_PLAYER_COUNT];
		PlayerUID		m_playerXuids[MAX_ONLINE_PLAYER_COUNT];
		int				m_playerCount;

		// This count & set of xuids reflects the session document list of who is in the game
		wstring			m_sessionXuids[MAX_ONLINE_PLAYER_COUNT];
		int				m_usedSlotCount;

		void			*m_extData;
	};

protected:
	// NOTE: If anything changes in here, then the mapping from internal -> external state needs to be updated (m_INTtoEXTStateMappings, defined in the cpp file)
	typedef enum
	{
		DNM_INT_STATE_INITIALISING,
		DNM_INT_STATE_INITIALISE_FAILED,
		DNM_INT_STATE_IDLE,
		DNM_INT_STATE_HOSTING,
		DNM_INT_STATE_HOSTING_WAITING_TO_PLAY,
		DNM_INT_STATE_HOSTING_FAILED,
		DNM_INT_STATE_JOINING,
		DNM_INT_STATE_JOINING_WAITING_FOR_RESERVATIONS,
		DNM_INT_STATE_JOINING_GET_SDA,
		DNM_INT_STATE_JOINING_WAITING_FOR_SDA,
		DNM_INT_STATE_JOINING_CREATE_SESSION,
		DNM_INT_STATE_JOINING_WAITING_FOR_ACTIVE_SESSION,
		DNM_INT_STATE_JOINING_SENDING_UNRELIABLE,
		DNM_INT_STATE_JOINING_FAILED_TIDY_UP,
		DNM_INT_STATE_JOINING_FAILED,
		DNM_INT_STATE_STARTING,
		DNM_INT_STATE_PLAYING,
		DNM_INT_STATE_LEAVING,
		DNM_INT_STATE_LEAVING_FAILED,
		DNM_INT_STATE_ENDING,
		DNM_INT_STATE_COUNT
	} eDQRNetworkManagerInternalState;

	typedef enum
	{
		DNM_ADD_PLAYER_STATE_IDLE,
		DNM_ADD_PLAYER_STATE_PROCESSING,
		DNM_ADD_PLAYER_STATE_COMPLETE_SUCCESS,
		DNM_ADD_PLAYER_STATE_COMPLETE_FAIL,
		DNM_ADD_PLAYER_STATE_COMPLETE_FAIL_FULL,
	} eDQRAddLocalPlayerState;

	typedef enum
	{
		DNM_REMOVE_PLAYER_STATE_IDLE,
		DNM_REMOVE_PLAYER_STATE_PROCESSING,
		DNM_REMOVE_PLAYER_STATE_COMPLETE_SUCCESS,
		DNM_REMOVE_PLAYER_STATE_COMPLETE_FAIL,
	} eDQRRemoveLocalPlayerState;

	class StateChangeInfo
	{
	public:
		eDQRNetworkManagerState m_oldState;
		eDQRNetworkManagerState m_newState;
		StateChangeInfo(eDQRNetworkManagerState oldState, eDQRNetworkManagerState newState) : m_oldState(oldState), m_newState(newState) {}
	};

	typedef enum
	{
		// Incoming messages
		RTS_MESSAGE_DATA_RECEIVED,
		RTS_MESSAGE_DATA_RECEIVED_CHAT,
		RTS_MESSAGE_ADDED_SESSION_ADDRESS,
		RTS_MESSAGE_REMOVED_SESSION_ADDRESS,
		RTS_MESSAGE_STATUS_ACTIVE,
		RTS_MESSAGE_STATUS_TERMINATED,

		// Outgoing messages
		RTS_MESSAGE_START_CLIENT,
		RTS_MESSAGE_START_HOST,
		RTS_MESSAGE_TERMINATE,
		RTS_MESSAGE_SEND_DATA,
	} eRTSMessageType;

	typedef enum
	{
		RTS_MESSAGE_FLAG_BROADCAST_MODE = 1,
		RTS_MESSAGE_FLAG_RELIABLE = 2,
		RTS_MESSAGE_FLAG_SEQUENTIAL = 4,
		RTS_MESSAGE_FLAG_COALESCE = 8,
		RTS_MESSAGE_FLAG_GAME_CHANNEL = 16,
	} eRTSFlags;

	class RTS_Message
	{
	public:
		eRTSMessageType m_eType;
		unsigned int	m_sessionAddress;
		unsigned char	*m_pucData;
		unsigned int	m_dataSize;
		unsigned int	m_flags;
	};

	std::queue<StateChangeInfo>											m_stateChangeQueue;
	CRITICAL_SECTION													m_csStateChangeQueue;
	CRITICAL_SECTION													m_csSendBytes;
	CRITICAL_SECTION													m_csPartyViewVector;
	std::queue<RTS_Message>												m_RTSMessageQueueIncoming;
	CRITICAL_SECTION													m_csRTSMessageQueueIncoming;
	std::queue<RTS_Message>												m_RTSMessageQueueOutgoing;
	CRITICAL_SECTION													m_csRTSMessageQueueOutgoing;
private:
	void																SetState(DQRNetworkManager::eDQRNetworkManagerInternalState state);
	static const eDQRNetworkManagerState								m_INTtoEXTStateMappings[DNM_INT_STATE_COUNT];
	eDQRNetworkManagerInternalState										m_state;
	eDQRNetworkManagerState												m_stateExternal;
	__int64																m_lastUnreliableSendTime;
	__int64																m_firstUnreliableSendTime;
	__int64																m_startedWaitingForReservationsTime;
	unsigned char														*m_customSessionData;
	unsigned int														m_customSessionDataSize;
	int																	m_customDataDirtyUpdateTicks;
	std::shared_ptr<ChatIntegrationLayer>								m_chat;

	eDQRAddLocalPlayerState												m_addLocalPlayerState;
	DQRNetworkPlayer													*m_addLocalPlayerSuccessPlayer;
	int																	m_addLocalPlayerSuccessIndex;
	int																	m_addLocalPlayerFailedIndex;

	eDQRRemoveLocalPlayerState											m_removeLocalPlayerState;
	int																	m_removeLocalPlayerIndex;

	Windows::Xbox::System::User^										m_primaryUser;
	MXS::XboxLiveContext^												m_primaryUserXboxLiveContext;
	WXN::SecureDeviceAssociationTemplate^								m_associationTemplate;

	CRITICAL_SECTION													m_csRoomSyncData;
	RoomSyncData														m_roomSyncData;
	DQRNetworkPlayer													*m_players[MAX_ONLINE_PLAYER_COUNT];	

	IDQRNetworkManagerListener											*m_listener;
	PartyController														*m_partyController;
	DQRNetworkManagerEventHandlers^										m_eventHandlers;
	WXNRs::Session^														m_XRNS_Session;
	unsigned int														m_XRNS_LocalAddress;
	unsigned int														m_XRNS_OldestAddress;
	MXSM::MultiplayerSession^											m_multiplayerSession;
	WXN::SecureDeviceAssociation^										m_sda;
	Platform::String^													m_secureDeviceAddressBase64;
	unsigned char														m_currentSmallId;
	unsigned char														m_hostSmallId;
	bool																m_isHosting;
	bool																m_isInSession;
	bool																m_isOfflineGame;
public:
	int																	m_currentUserMask;
private:
	int																	m_joinSessionUserMask;
	Platform::Array<Platform::String^>^									m_joinSessionXUIDs;
	int																	m_joinSmallIdMask;

	Platform::Array<BYTE>^												m_remoteSocketAddress;
	Platform::Array<BYTE>^												m_localSocketAddress;
	int																	m_joinCreateSessionAttempts;

	C4JThread															*m_CreateSessionThread;
	C4JThread															*m_LeaveRoomThread;		
	C4JThread															*m_TidyUpJoinThread;
	C4JThread															*m_UpdateCustomSessionDataThread;
	C4JThread															*m_RTS_DoWorkThread;
	C4JThread															*m_CheckPartyInviteThread;

	bool																m_notifyForFullParty;

	unordered_map<unsigned int,DQRConnectionInfo *>						m_sessionAddressToConnectionInfoMapHost;	// For host - there may be more than one remote session attached to this listening session
	unsigned int														m_sessionAddressFromSmallId[256];			// For host - for mapping back from small Id, to session address
	unsigned char														m_channelFromSmallId[256];					// For host and client, for mapping back from small Id, to channel
	DQRConnectionInfo													m_connectionInfoClient;						// For client
	unsigned int														m_hostSessionAddress;						// For client

	CRITICAL_SECTION													m_csHostGamertagResolveResults;
	queue<HostGamertagResolveDetails *>									m_hostGamertagResolveResults;

	void																AddPlayerFailed(Platform::String ^xuid);
	Platform::String^													RemoveBracesFromGuidString(__in Platform::String^ guid );
	void																HandleSessionChange( MXSM::MultiplayerSession^ session );
	MXSM::MultiplayerSession^											WriteSessionHelper(  MXS::XboxLiveContext^ xboxLiveContext,
																							MXSM::MultiplayerSession^ multiplayerSession,
																							MXSM::MultiplayerSessionWriteMode writeMode,
																							HRESULT& hr);
	MXSM::MultiplayerSessionMember^										GetUserMemberInSession( Windows::Xbox::System::User^ user, MXSM::MultiplayerSession^ session);
	bool																IsPlayerInSession( Platform::String^ xboxUserId, MXSM::MultiplayerSession^ session, int *smallId );

	WXM::MultiplayerSessionReference^									ConvertToWindowsXboxMultiplayerSessionReference( MXSM::MultiplayerSessionReference^ sessionRef);
	MXSM::MultiplayerSessionReference^									ConvertToMicrosoftXboxServicesMultiplayerSessionReference( WXM::MultiplayerSessionReference^ sessionRef );

	void																BytesReceived(int smallId, BYTE *bytes, int byteCount);
	void																BytesReceivedInternal(DQRConnectionInfo *connectionInfo, unsigned int sessionAddress, BYTE *bytes, int byteCount);
	void																SendBytes(int smallId, BYTE *bytes, int byteCount);
	int																	GetQueueSizeBytes();
	int																	GetQueueSizeMessages();
	void																SendBytesRaw(int smallId, BYTE *bytes, int byteCount, bool reliableAndSequential);
	void																SendBytesChat(unsigned int address, BYTE *bytes, int byteCount, bool reliable, bool sequential, bool broadcast);
	
	bool																AddRoomSyncPlayer(DQRNetworkPlayer *pPlayer, unsigned int sessionAddress, int channel);
	void																RemoveRoomSyncPlayersWithSessionAddress(unsigned int sessionAddress);
	void																RemoveRoomSyncPlayer(DQRNetworkPlayer *pPlayer);
	void																UpdateRoomSyncPlayers(RoomSyncData *pNewSyncData);
	void																SendRoomSyncInfo();
	void																SendAddPlayerFailed(Platform::String^ xuid);
	void																SendSmallId(bool reliableAndSequential, int playerMask);
	void																SendUnassignSmallId(int playerIndex);
	int																	GetSessionIndexForSmallId(unsigned char smallId);
	int																	GetSessionIndexAndSmallIdForHost(unsigned char *smallId);

	static void															LogComment( Platform::String^ strText );	
	static void															LogCommentFormat( LPCWSTR strMsg, ... );
	static void															LogCommentWithError( Platform::String^ strTest, HRESULT hr );

	static Platform::String^											GetErrorString( HRESULT hr );
	static Platform::String^											FormatString( LPCWSTR strMsg, ... );
	static Platform::String^											ConvertHResultToErrorName( HRESULT hr );

	Platform::String^													GetNextSmallIdAsJsonString();
	static int															_HostGameThreadProc( void* lpParameter );
	int																	HostGameThreadProc();
	static int															_LeaveRoomThreadProc( void* lpParameter );
	int																	LeaveRoomThreadProc();
	static int															_TidyUpJoinThreadProc( void* lpParameter );
	int																	TidyUpJoinThreadProc();
	static int															_UpdateCustomSessionDataThreadProc( void* lpParameter );
	int																	UpdateCustomSessionDataThreadProc();
	static int															_CheckInviteThreadProc(void *lpParameter);
	int																	CheckInviteThreadProc();
	static int															_RTSDoWorkThread(void* lpParameter);
	int																	RTSDoWorkThread();

	CRITICAL_SECTION													m_csVecChatPlayers;
	vector<int>															m_vecChatPlayersJoined;
public:
	void																HandleNewPartyFoundForPlayer();
	void																HandlePlayerRemovedFromParty(int playerMask);

	void																ChatPlayerJoined(int idx);
	bool																IsReadyToPlayOrIdle();
	void																StartGame();
	void																LeaveRoom();
	void																TidyUpFailedJoin();

	static void															SetInviteReceivedFlag();
	static void															SetPartyProcessJoinParty();
	static void															SetPartyProcessJoinSession(int bootUserIndex, Platform::String^ bootSessionName, Platform::String^ bootServiceConfig, Platform::String^ bootSessionTemplate);

	void																SendInviteGUI(int quadrant);
	bool																IsAddingPlayer();

	bool																FriendPartyManagerIsBusy();
	int																	FriendPartyManagerGetCount();
	bool																FriendPartyManagerSearch();
	void																FriendPartyManagerGetSessionInfo(int idx, SessionSearchResult *searchResult);
	WFC::IVectorView<WXM::UserPartyAssociation^>^						FilterPartiesByPermission(MXS::XboxLiveContext ^context, WFC::IVectorView<WXM::UserPartyAssociation^>^ partyResults);

	bool																JoinPartyFromSearchResult(SessionSearchResult *searchResult, int playerMask);
	void																CancelJoinPartyFromSearchResult();
	void																RequestDisplayName(DQRNetworkPlayer *player);
	void																SetDisplayName(PlayerUID xuid, wstring displayName);

private:
	__int64																m_playersLeftPartyTime;
	int																	m_playersLeftParty;

	bool																GetBestPartyUserIndex();
	C4JThread															*m_GetFriendPartyThread;
	static int															_GetFriendsThreadProc( void* lpParameter );
	int																	GetFriendsThreadProc();
	bool																IsSessionFriendsOfFriends(MXSM::MultiplayerSession^ session);
	bool																GetGameSessionData(MXSM::MultiplayerSession^ session, void *gameSessionData);

public:
	static Platform::Collections::Vector<Platform::String^>^			GetFriends(); 

private:
	SessionSearchResult													*m_sessionSearchResults;
	int																	m_sessionResultCount;
	bool																m_cancelJoinFromSearchResult;

	map<wstring, wstring>												m_displayNames; // Player display names by gamertag



	typedef enum
	{
		DNM_PARTY_PROCESS_NONE,
		DNM_PARTY_PROCESS_JOIN_PARTY,
		DNM_PARTY_PROCESS_JOIN_SPECIFIED
	} ePartyProcessType;
	static int															m_bootUserIndex;
	static ePartyProcessType											m_partyProcess;
	static wstring														m_bootSessionName;
	static wstring														m_bootServiceConfig;
	static wstring														m_bootSessionTemplate;
	static bool															m_inviteReceived;

	static DQRNetworkManager											*s_pDQRManager;

	static void															GetProfileCallback(LPVOID pParam, Microsoft::Xbox::Services::Social::XboxUserProfile^ profile);

	// Forced signout
	bool m_handleForcedSignOut;

	void CheckForcedSignOut();
	void HandleForcedSignOut();

	unsigned int														m_RTS_Stat_totalBytes;
	unsigned int														m_RTS_Stat_totalSends;

	void																UpdateRTSStats();

	// Incoming messages - to be called from the main thread, to get incoming messages from the RTS work thread
	void																ProcessRTSMessagesIncoming();
	void																Process_RTS_MESSAGE_DATA_RECEIVED(RTS_Message &message);
	void																Process_RTS_MESSAGE_DATA_RECEIVED_CHAT(RTS_Message &message);
	void																Process_RTS_MESSAGE_ADDED_SESSION_ADDRESS(RTS_Message &message);
	void																Process_RTS_MESSAGE_REMOVED_SESSION_ADDRESS(RTS_Message &message);
	void																Process_RTS_MESSAGE_STATUS_ACTIVE(RTS_Message &message);
	void																Process_RTS_MESSAGE_STATUS_TERMINATED(RTS_Message &message);

	// Outgoing messages - to be called from the RTS work thread, to process requests from the main thread
	
	void																ProcessRTSMessagesOutgoing();
	void																Process_RTS_MESSAGE_START_CLIENT(RTS_Message &message);
	void																Process_RTS_MESSAGE_START_HOST(RTS_Message &message);
	void																Process_RTS_MESSAGE_TERMINATE(RTS_Message &message);
	void																Process_RTS_MESSAGE_SEND_DATA(RTS_Message &message);

	// These methods are called from the main thread, to put an outgoing message onto the queue to be processed by these previous methods
	void																RTS_StartCient();
	void																RTS_StartHost();
	void																RTS_Terminate();
	void																RTS_SendData(unsigned char *pucData, unsigned int dataSize, unsigned int sessionAddress, bool reliable, bool sequential, bool coalesce, bool includeMode, bool gameChannel );

};

// Class defining interface to be implemented for class that handles callbacks
class IDQRNetworkManagerListener
{
public:
	virtual void HandleDataReceived(DQRNetworkPlayer *playerFrom, DQRNetworkPlayer *playerTo, unsigned char *data, unsigned int dataSize) = 0;
	virtual void HandlePlayerJoined(DQRNetworkPlayer *player) = 0;
	virtual void HandlePlayerLeaving(DQRNetworkPlayer *player) = 0;
	virtual void HandleStateChange(DQRNetworkManager::eDQRNetworkManagerState oldState, DQRNetworkManager::eDQRNetworkManagerState newState) = 0;
//	virtual void HandleResyncPlayerRequest(DQRNetworkPlayer **aPlayers) = 0;
	virtual void HandleAddLocalPlayerFailed(int idx, bool serverFull) = 0;
	virtual void HandleDisconnect(bool bLostRoomOnly) = 0;
	virtual void HandleInviteReceived(int playerIndex, DQRNetworkManager::SessionInfo *pInviteInfo) = 0;
	virtual bool IsSessionJoinable() = 0;
};
