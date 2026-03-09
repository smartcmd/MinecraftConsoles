#pragma once
#include <np.h>
#ifdef __PS3__
#include <netex\libnetctl.h>
#include <netex\net.h>
#else
#include <libnetctl.h>
#include <net.h>
#endif
#include <queue>

#include "..\..\Common\Network\Sony\SQRNetworkManager.h"

class SQRNetworkPlayer;
class ISQRNetworkManagerListener;
class SonyVoiceChat;
class C4JThread;

// This is the lowest level manager for providing network functionality on Sony platforms. This manages various network activities including the players within a gaming session.
// The game shouldn't directly use this class, it is here to provide functionality required by PlatformNetworkManagerSony.

class SQRNetworkManager_PS3 : public SQRNetworkManager
{
	
	friend class SonyVoiceChat;
	friend class SQRNetworkPlayer;
	static const eSQRNetworkManagerState m_INTtoEXTStateMappings[SNM_INT_STATE_COUNT];



public:
	SQRNetworkManager_PS3(ISQRNetworkManagerListener *listener);

	// General 
	void											Tick();
	void											Initialise();
	void											Terminate();
	eSQRNetworkManagerState							GetState();
	bool											IsHost();
	bool											IsReadyToPlayOrIdle();
	bool											IsInSession();

	// Session management
	void											CreateAndJoinRoom(int hostIndex, int localPlayerMask, void *extData, int extDataSize, bool offline);
	void											UpdateExternalRoomData();
	bool											FriendRoomManagerIsBusy();
	bool											FriendRoomManagerSearch();
	bool											FriendRoomManagerSearch2();
	int												FriendRoomManagerGetCount();
	void											FriendRoomManagerGetRoomInfo(int idx, SessionSearchResult *searchResult);
	bool											JoinRoom(SessionSearchResult *searchResult, int localPlayerMask);
	bool											JoinRoom(SceNpMatching2RoomId roomId, SceNpMatching2ServerId serverId, int localPlayerMask, const SQRNetworkManager_PS3::PresenceSyncInfo *presence);
	void											StartGame();
	void											LeaveRoom(bool bActuallyLeaveRoom);
	void											EndGame();
	bool											SessionHasSpace(int spaceRequired);
	bool											AddLocalPlayerByUserIndex(int idx);
	bool											RemoveLocalPlayerByUserIndex(int idx);
	void											SendInviteGUI();
	void											GetInviteDataAndProcess(SceNpBasicAttachmentDataId id);
	static bool										UpdateInviteData(SQRNetworkManager_PS3::PresenceSyncInfo *invite);
	void											GetExtDataForRoom( SceNpMatching2RoomId roomId, void *extData, void (* FriendSessionUpdatedFn)(bool success, void *pParam), void *pParam );

	// Player retrieval
	int												GetPlayerCount();
	int												GetOnlinePlayerCount();
	SQRNetworkPlayer								*GetPlayerByIndex(int idx);
	SQRNetworkPlayer								*GetPlayerBySmallId(int idx);
	SQRNetworkPlayer								*GetPlayerByXuid(PlayerUID xuid);
	SQRNetworkPlayer								*GetLocalPlayerByUserIndex(int idx);
	SQRNetworkPlayer								*GetHostPlayer();

	// Communication parameter storage
	static const SceNpCommunicationId*				GetSceNpCommsId();
	static const SceNpCommunicationSignature*		GetSceNpCommsSig();

private:
	void											InitialiseAfterOnline();
	void											ErrorHandlingTick();
	void											UpdateOnlineStatus(int status) { m_onlineStatus = status; }
	int												GetOnlineStatus() { return m_onlineStatus; }

	ISQRNetworkManagerListener					    *m_listener;
	SQRNetworkPlayer								*GetPlayerIfReady(SQRNetworkPlayer *player);

	// Internal state
	void											SetState(eSQRNetworkManagerInternalState state);
	void											ResetToIdle();
	eSQRNetworkManagerInternalState					m_state;
	eSQRNetworkManagerState							m_stateExternal;
	bool											m_nextIdleReasonIsFull;
	bool											m_isHosting;
	SceNpMatching2RoomMemberId						m_localMemberId;
	int												m_localPlayerCount;
	int												m_localPlayerJoined;		// Client only, keep a count of how many local players we have confirmed as joined to the application
	SceNpMatching2RoomId							m_room;
	unsigned char									m_currentSmallId;
	int												m_soc;
	bool											m_offlineGame;
	bool											m_offlineSQR;
	int												m_resendExternalRoomDataCountdown;
	bool											m_matching2initialised;
	PresenceSyncInfo								m_inviteReceived[MAX_SIMULTANEOUS_INVITES];	
	int												m_inviteIndex;
	PresenceSyncInfo								*m_gameBootInvite;
	bool											m_doBootInviteCheck;
	bool											m_isInSession;
	static SceNpBasicAttachmentDataId				s_lastInviteIdToRetry;
	int												m_onlineStatus;
	bool											m_bLinkDisconnected;
private:

	CRITICAL_SECTION								m_csRoomSyncData;
	RoomSyncData									m_roomSyncData;
	void											*m_joinExtData;
	int												m_joinExtDataSize;

	std::vector<SQRNetworkPlayer *>					m_vecTempPlayers;
	SQRNetworkPlayer								*m_aRoomSlotPlayers[MAX_ONLINE_PLAYER_COUNT];		// Maps from the players in m_roomSyncData, to SQRNetworkPlayers
	void											FindOrCreateNonNetworkPlayer(int slot, int playerType, SceNpMatching2RoomMemberId memberId, int localPlayerIdx, int smallId);

	void											MapRoomSlotPlayers(int roomSlotPlayerCount =-1);
	void											UpdateRoomSyncUIDsFromPlayers();
	void											UpdatePlayersFromRoomSyncUIDs();
	void											LocalDataSend(SQRNetworkPlayer *playerFrom, SQRNetworkPlayer *playerTo, const void *data, unsigned int dataSize);
	int												GetSessionIndex(SQRNetworkPlayer *player);

	bool											AddRemotePlayersAndSync( SceNpMatching2RoomMemberId memberId, int playerMask, bool *isFull = nullptr );
	void											RemoveRemotePlayersAndSync( SceNpMatching2RoomMemberId memberId, int mask );
	void											RemoveNetworkPlayers( int mask );
	void											SetLocalPlayersAndSync();
	void											SyncRoomData();
	SceNpMatching2RequestId							m_setRoomDataRequestId;
	SceNpMatching2RequestId							m_setRoomIntDataRequestId;
	SceNpMatching2RequestId							m_roomExtDataRequestId;

	// Server context management
	bool											GetMatchingContext(eSQRNetworkManagerInternalState asyncState);
	bool											GetServerContext();
	bool											GetServerContext2();
	bool											GetServerContext(SceNpMatching2ServerId serverId);
	void											DeleteServerContext();
	bool											SelectRandomServer();
	void											ServerContextTick();
	int												m_totalServerCount;
	int												m_serverCount;
	SceNpMatching2ServerId							*m_aServerId;
	SceNpMatching2ServerId							m_serverId;
	bool											m_serverContextValid;
	SceNpMatching2RequestId							m_serverSearchRequestId;
	SceNpMatching2RequestId							m_serverContextRequestId;

	// Room creation management
	SceNpMatching2RequestId							m_getWorldRequestId;
	SceNpMatching2RequestId							m_createRoomRequestId;
	SceNpMatching2WorldId							m_worldId;
	void											RoomCreateTick();

	// Room joining management
	SceNpMatching2RoomId							m_roomToJoin;
	int												m_localPlayerJoinMask;
	SceNpMatching2RequestId							m_joinRoomRequestId;
	SceNpMatching2RequestId							m_kickRequestId;

	// Room leaving management
	SceNpMatching2RequestId							m_leaveRoomRequestId;

	// Adding extra network players management
	SceNpMatching2RequestId							m_setRoomMemberInternalDataRequestId;

	// Player state management
	void											NetworkPlayerConnectionComplete(SQRNetworkPlayer *player);
	void											NetworkPlayerSmallIdAllocated(SQRNetworkPlayer *player, unsigned char smallId);
	void											NetworkPlayerInitialDataReceived(SQRNetworkPlayer *player,void *data);
	void											NonNetworkPlayerComplete(SQRNetworkPlayer *player, unsigned char smallId);
	void											HandlePlayerJoined(SQRNetworkPlayer *player);
	CRITICAL_SECTION								m_csPlayerState;

	// State and thread for managing basic event type messages
	C4JThread										*m_basicEventThread;
	sys_event_port_t								m_basicEventPort;
	sys_event_queue_t								m_basicEventQueue;
	static int										BasicEventThreadProc( void *lpParameter);

	// State and storage for managing search for friends' games
	eSQRNetworkManagerFriendSearchState				m_friendSearchState;
	SceNpMatching2ContextId							m_matchingContext;
	bool											m_matchingContextValid;
	SceNpMatching2RequestId							m_friendSearchRequestId;
	unsigned int									m_friendCount;
	C4JThread										*m_getFriendCountThread;
	static int										GetFriendsThreadProc( void* lpParameter );
	void											FriendSearchTick();
	SceNpMatching2RequestId							m_roomDataExternalListRequestId;
	void											(* m_FriendSessionUpdatedFn)(bool success, void *pParam);
	void											*m_pParamFriendSessionUpdated;
	void											*m_pExtDataToUpdate;

	// Results from searching for rooms that friends are playing in - 5 matched arrays to store their NpIds, rooms, servers, whether a room was found, and whether the external data had been received for the room. Also a count of how many elements are used in this array.
	SceNpId											m_aSearchResultNpId[MAX_FRIENDS];
	SceNpMatching2RoomId							m_aSearchResultRoomId[MAX_FRIENDS];
	SceNpMatching2ServerId							m_aSearchResultServerId[MAX_FRIENDS];
	bool											m_aSearchResultRoomFound[MAX_FRIENDS];
	void											*m_aSearchResultRoomExtDataReceived[MAX_FRIENDS];
	unsigned int									m_searchResultCount;

	// Rudp management and local players
	unordered_map<int,SQRNetworkPlayer	*>			m_RudpCtxToPlayerMap;	
	bool											CreateRudpConnections(SceNpMatching2RoomId roomId, SceNpMatching2RoomMemberId peerMemberId, int playerMask, SceNpMatching2RoomMemberId playersPeerMemberId);
	SQRNetworkPlayer								*GetPlayerFromRudpCtx(int rudpCtx);
	SQRNetworkPlayer								*GetPlayerFromRoomMemberAndLocalIdx(int roomMember, int localIdx);
	SceNpMatching2RequestId							m_roomMemberDataRequestId;

	// Callbacks (for matching)
	bool											RegisterCallbacks();
	static void										ContextCallback(SceNpMatching2ContextId  id, SceNpMatching2Event event, SceNpMatching2EventCause eventCause, int errorCode, void *arg);
#ifdef __PS3__
	static void										DefaultRequestCallback(SceNpMatching2ContextId id, SceNpMatching2RequestId reqId, SceNpMatching2Event event, SceNpMatching2EventKey eventKey, int errorCode, size_t dataSize, void *arg);
	static void										RoomEventCallback(SceNpMatching2ContextId id, SceNpMatching2RoomId roomId, SceNpMatching2Event event, SceNpMatching2EventKey eventKey, int errorCode, size_t dataSize, void *arg);
#else
	static void										DefaultRequestCallback(SceNpMatching2ContextId id, SceNpMatching2RequestId reqId, SceNpMatching2Event event, int errorCode, const void *data, void *arg);
	static void										RoomEventCallback(SceNpMatching2ContextId id, SceNpMatching2RoomId roomId, SceNpMatching2Event event, int errorCode, const void *data, void *arg);
#endif
	static void										SignallingCallback(SceNpMatching2ContextId ctxId, SceNpMatching2RoomId roomId, SceNpMatching2RoomMemberId peerMemberId, SceNpMatching2Event event, int errorCode, void *arg);

	// Callback for NpBasic
	static int										BasicEventCallback(int event, int retCode, uint32_t reqId, void *arg);

	// Callback for NpManager
	static void										ManagerCallback(int event, int result, void *arg);

	// Callback for sys util
	static void										SysUtilCallback(uint64_t status, uint64_t param, void *userdata);

	// Callbacks for rudp
	static void										RudpContextCallback(int ctx_id, int event_id, int error_code, void *arg);
#ifdef __PS3__
	static int										RudpEventCallback(int event_id, int soc, uint8_t const *data, size_t datalen, struct sockaddr const *addr, socklen_t addrlen, void *arg);
#else
	static int										RudpEventCallback(int event_id, int soc, uint8_t const *data, size_t datalen, struct SceNetSockaddr  const *addr, SceNetSocklen_t addrlen, void *arg);
#endif

	// Callback for netctl
	static void										NetCtlCallback(int prev_state, int new_state, int event, int error_code, void *arg);

	// Methods to be called when the server context has been created
	void											ServerContextValid_CreateRoom();
	void											ServerContextValid_JoinRoom();

	// Mask utilities
	int												GetOldMask(SceNpMatching2RoomMemberId memberId);
	int												GetAddedMask(int newMask, int oldMask);
	int												GetRemovedMask(int newMask, int oldMask);

#ifdef __PS3__
	// Response storage
	char											cUserInfoListStorage[SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_GetUserInfoList] __attribute__((__aligned__(4)));
	char											cServerInfoStorage[SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_GetServerInfo] __attribute__((__aligned__(4)));
	char											cWorldListStorage[SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_GetWorldInfoList] __attribute__((__aligned__(4)));
	char											cCreateJoinRoomStorage[SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_CreateJoinRoom] __attribute__((__aligned__(4)));
	char											cJoinRoomStorage[SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_JoinRoom] __attribute__((__aligned__(4)));
	char											cRoomMemberDataInternal[SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_GetRoomMemberDataInternal] __attribute__((__aligned__(4)));
	char											cRoomDataInternal[SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomDataInternalUpdateInfo] __attribute__((__aligned__(4)));
	char											cRoomDataExternalList[SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_GetRoomDataExternalList] __attribute__((__aligned__(4)));
	char											cRoomMemberDataInternalUpdate[SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomMemberDataInternalUpdateInfo] __attribute__((__aligned__(4)));
	char											cRoomDataUpdateInfo[SCE_NP_MATCHING2_EVENT_DATA_MAX_SIZE_RoomUpdateInfo] __attribute__((__aligned__(4)));
#endif

#ifndef _CONTENT_PACKAGE
	static bool										aForceError[SNM_FORCE_ERROR_COUNT];
#endif
	bool											ForceErrorPoint(eSQRForceError err);

public:
	static void										AttemptPSNSignIn(int (*SignInCompleteCallbackFn)(void *pParam, bool bContinue, int pad), void *pParam, bool callIfFailed = false);
	static int										(*s_SignInCompleteCallbackFn)(void *pParam, bool bContinue, int pad);
	static bool										s_signInCompleteCallbackIfFailed;
	static void										*s_SignInCompleteParam;

	static int										SetRichPresence(const void *data, unsigned int options);
	void											SetPresenceDataStartHostingGame();
	int												GetJoiningReadyPercentage();
private:
	static void										UpdateRichPresenceCustomData(void *data, unsigned int dataBytes);
	static void										TickRichPresence();
	static void										SendLastPresenceInfo();

	static SceNpBasicPresenceDetails2				s_lastPresenceInfo;
	static int										s_resendPresenceCountdown;
	static bool										s_presenceStatusDirty;
	static bool										s_presenceDataDirty;
	static PresenceSyncInfo							s_lastPresenceSyncInfo;
	static PresenceSyncInfo							c_presenceSyncInfoNULL;

	// 4J-PB - so we can stop the crash on PS3 when Iggy's LoadMovie is called from the ContextCallback
	static bool										m_bCallPSNSignInCallback;

	// Debug
	static long long								s_roomStartTime;
};
