#pragma once
#include <np.h>
#include <libnetctl.h>
#include <net.h>
#include <np_toolkit.h>

#include <queue>

#include <unordered_map>

#include "..\..\Common\Network\Sony\SQRNetworkManager.h"
// 
class SQRNetworkPlayer;
class ISQRNetworkManagerListener;
class SonyVoiceChat_Vita;
class SQRVoiceConnection;
class C4JThread;


class HelloSyncInfo;

enum EAdhocDataTag
{
	e_dataTag_Normal,
	e_dataTag_RoomSync
};

class AdhocDataPacket
{
public:
	EAdhocDataTag	m_tag;
	uint32_t		m_pData[1];
};

// This is the lowest level manager for providing network functionality on Sony platforms. This manages various network activities including the players within a gaming session.
// The game shouldn't directly use this class, it is here to provide functionality required by PlatformNetworkManagerSony.

class SQRNetworkManager_AdHoc_Vita : public SQRNetworkManager
{
	friend class SonyVoiceChat_Vita;
	friend class SQRNetworkPlayer;

	static const eSQRNetworkManagerState m_INTtoEXTStateMappings[SNM_INT_STATE_COUNT];



public:
	SQRNetworkManager_AdHoc_Vita(ISQRNetworkManagerListener *listener);

	// General 
	void											Tick();
	void											Initialise();
	bool											IsInitialised();
	void											UnInitialise();
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
	bool											JoinRoom(SceNetInAddr netAddr, int localPlayerMask, const HelloSyncInfo *presence);
	bool											JoinRoom(SceNpMatching2RoomId roomId, SceNpMatching2ServerId serverId, int localPlayerMask, const PresenceSyncInfo *presence);
	void											StartGame();
	void											LeaveRoom(bool bActuallyLeaveRoom);
	void											EndGame();
	bool											SessionHasSpace(int spaceRequired);
	bool											AddLocalPlayerByUserIndex(int idx);
	bool											RemoveLocalPlayerByUserIndex(int idx);
	void											SendInviteGUI();
	static void										RecvInviteGUI();
	void											TickInviteGUI();



	// 	void											GetInviteDataAndProcess(SceNpBasicAttachmentDataId id);
	static bool										UpdateInviteData(HelloSyncInfo *invite);
	void											GetExtDataForRoom( SceNpMatching2RoomId roomId, void *extData, void (* FriendSessionUpdatedFn)(bool success, void *pParam), void *pParam );

	// Player retrieval
	int												GetPlayerCount();
	int												GetOnlinePlayerCount();
	SQRNetworkPlayer								*GetPlayerByIndex(int idx);
	SQRNetworkPlayer								*GetPlayerBySmallId(int idx);
	SQRNetworkPlayer								*GetLocalPlayerByUserIndex(int idx);
	SQRNetworkPlayer								*GetPlayerByXuid(PlayerUID xuid);
	SQRNetworkPlayer								*GetHostPlayer();

	void											removePlayerFromVoiceChat(SQRNetworkPlayer* pPlayer);
	// Communication parameter storage
	static const SceNpCommunicationId*				GetSceNpCommsId();
	static const SceNpCommunicationSignature*		GetSceNpCommsSig();
	static const SceNpTitleId*						GetSceNpTitleId();
	static const SceNpTitleSecret*					GetSceNpTitleSecret();

	static void										GetInviteDataAndProcess(sce::Toolkit::NP::MessageAttachment* pInvite);
	static bool										GetAdhocStatus() { return m_adhocStatus; }

	int sendDataPacket(SceNetInAddr addr, EAdhocDataTag tag, void* data, int dataSize);
	int sendDataPacket(SceNetInAddr addr, void* data, int dataSize);

private:
	void											InitialiseAfterOnline();
	void											ErrorHandlingTick();
	void											UpdateAdhocStatus(int status) { m_adhocStatus = status; }
	void											UpdateLocalIPAddress();

	ISQRNetworkManagerListener					    *m_listener;
	SQRNetworkPlayer								*GetPlayerIfReady(SQRNetworkPlayer *player);

	// Internal state
	void											SetState(eSQRNetworkManagerInternalState state);
	void											ResetToIdle();
	eSQRNetworkManagerInternalState					m_state;
	eSQRNetworkManagerState							m_stateExternal;
	bool											m_nextIdleReasonIsFull;
	bool											m_isHosting;
	SceNetInAddr									m_localIPAddr;
	SceNetInAddr									m_hostIPAddr;
	SceNpMatching2RoomMemberId						m_localMemberId;
	SceNpMatching2RoomMemberId						m_hostMemberId; // if we're not the host
	int												m_localPlayerCount;
	int												m_localPlayerJoined;		// Client only, keep a count of how many local players we have confirmed as joined to the application
	SceNpMatching2RoomId							m_room;
	unsigned char									m_currentSmallId;
 	int												m_soc;
	bool											m_offlineGame;
	bool											m_offlineSQR;
	int												m_resendExternalRoomDataCountdown;
	bool											m_matching2initialised;
// 	HelloSyncInfo								m_inviteReceived[MAX_SIMULTANEOUS_INVITES];	
// 	int												m_inviteIndex;
// 	static HelloSyncInfo							*m_gameBootInvite;
// 	static HelloSyncInfo							m_gameBootInvite_data;
// 	bool											m_doBootInviteCheck;
	bool											m_isInSession;
	// 	static SceNpBasicAttachmentDataId				s_lastInviteIdToRetry;
	static int										m_adhocStatus;
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
	bool											GetServerContext_AdHoc();
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
// 	SceKernelEqueue									m_basicEventQueue;
	static int										BasicEventThreadProc( void *lpParameter);

	// State and storage for managing search for friends' games
	eSQRNetworkManagerFriendSearchState				m_friendSearchState;
	SceNpMatching2ContextId							m_matchingContext;
	bool											m_matchingContextServerValid;
	bool											m_matchingContextClientValid;
	SceNpMatching2RequestId							m_friendSearchRequestId;
	unsigned int									m_friendCount;
// 	C4JThread										*m_getFriendCountThread;
// 	static int										GetFriendsThreadProc( void* lpParameter );
	void											FriendSearchTick();
	SceNpMatching2RequestId							m_roomDataExternalListRequestId;
	void											(* m_FriendSessionUpdatedFn)(bool success, void *pParam);
	void											*m_pParamFriendSessionUpdated;
	void											*m_pExtDataToUpdate;

	// Results from searching for rooms that friends are playing in - 5 matched arrays to store their NpIds, rooms, servers, whether a room was found, and whether the external data had been received for the room. Also a count of how many elements are used in this array.
	class FriendSearchResult
	{
	public:
		SceNpId											m_NpId;
		SceNetInAddr									m_netAddr;
// 		SceNpMatching2RoomId							m_RoomId;
// 		SceNpMatching2ServerId							m_ServerId;
		bool											m_RoomFound;
		void											*m_RoomExtDataReceived;
		void*											m_gameSessionData;
		RoomSyncData									m_roomSyncData;
	};
	std::vector<FriendSearchResult>						m_aFriendSearchResults;
	bool												m_bFriendsSearchChanged;

	// Rudp management and local players
	std::unordered_map<int,SQRNetworkPlayer	*>			m_RudpCtxToPlayerMap;	
	std::unordered_map<int,SceNetInAddr>				m_RudpCtxToIPAddrMap;	

	std::unordered_map<SceNetInAddr_t, SQRVoiceConnection*>				m_NetAddrToVoiceConnectionMap;	

	bool											CreateRudpConnections(SceNetInAddr peer);
	bool											CreateVoiceRudpConnections(SceNpMatching2RoomId roomId, SceNpMatching2RoomMemberId peerMemberId, int playerMask);
	bool											CreateSocket();
	SQRNetworkPlayer								*GetPlayerFromRudpCtx(int rudpCtx);
	SceNetInAddr*									GetIPAddrFromRudpCtx(int rudpCtx);
	SQRVoiceConnection*								GetVoiceConnectionFromRudpCtx(int rudpCtx);

	SQRNetworkPlayer								*GetPlayerFromRoomMemberAndLocalIdx(int roomMember, int localIdx);
	SceNpMatching2RequestId							m_roomMemberDataRequestId;

	// Callbacks (for matching)
	bool											RegisterCallbacks();
	void											HandleMatchingContextStart();
	// #ifdef __PS3__
	// 	static void										DefaultRequestCallback(SceNpMatching2ContextId id, SceNpMatching2RequestId reqId, SceNpMatching2Event event, SceNpMatching2EventKey eventKey, int errorCode, size_t dataSize, void *arg);
	// 	static void										RoomEventCallback(SceNpMatching2ContextId id, SceNpMatching2RoomId roomId, SceNpMatching2Event event, SceNpMatching2EventKey eventKey, int errorCode, size_t dataSize, void *arg);
	// #else
// 	static void										DefaultRequestCallback(SceNpMatching2ContextId id, SceNpMatching2RequestId reqId, SceNpMatching2Event event, int errorCode, const void *data, void *arg);
// 	static void										RoomEventCallback(SceNpMatching2ContextId id, SceNpMatching2RoomId roomId, SceNpMatching2Event event, const void *data, void *arg);
	// #endif
// 	static void										SignallingCallback(SceNpMatching2ContextId ctxId, SceNpMatching2RoomId roomId, SceNpMatching2RoomMemberId peerMemberId, SceNpMatching2Event event, int error_code, void *arg);

	// Callback for NpBasic
	static int										BasicEventCallback(int event, int retCode, uint32_t reqId, void *arg);

	// Callback for NpManager
	static void										ManagerCallback(int event, int result, void *arg);

	// Callback for sys util
	static void										SysUtilCallback(uint64_t status, uint64_t param, void *userdata);
	void											updateNetCheckDialog(); // get the status of the dialog and run any callbacks needed [CD - Added to match SQRNetworkManager_Vita]

	// Callbacks for rudp
	static void										RudpContextCallback(int ctx_id, int event_id, int error_code, void *arg);
	static int										RudpEventCallback(int event_id, int soc, uint8_t const *data, size_t datalen, struct SceNetSockaddr  const *addr, SceNetSocklen_t addrlen, void *arg);

	// Callback for netctl
	static void										NetCtlCallback(int eventType, void *arg);

	// Methods to be called when the server context has been created
	void											ServerContextValid_CreateRoom();
	void											ServerContextValid_JoinRoom();

	// Mask utilities
	int												GetOldMask(SceNpMatching2RoomMemberId memberId);
	int												GetAddedMask(int newMask, int oldMask);
	int												GetRemovedMask(int newMask, int oldMask);

#ifndef _CONTENT_PACKAGE
	static bool										aForceError[SNM_FORCE_ERROR_COUNT];
#endif
	bool											ForceErrorPoint(eSQRForceError err);


	static void										MatchingEventHandler(int id, int event, SceNetInAddr* peer, int optlen, void *opt);


public:
	static void										AttemptPSNSignIn(int (*SignInCompleteCallbackFn)(void *pParam, bool bContinue, int pad), void *pParam, bool callIfFailed = false);
	static void										AttemptAdhocSignIn(int (*SignInCompleteCallbackFn)(void *pParam, bool bContinue, int pad), void *pParam, bool callIfFailed = false);
	static int										(*s_SignInCompleteCallbackFn)(void *pParam, bool bContinue, int pad);
	static bool										s_signInCompleteCallbackIfFailed;
	static void										*s_SignInCompleteParam;

	static int										SetRichPresence(const void *data);
	void											SetPresenceDataStartHostingGame();
	int												GetJoiningReadyPercentage();

	void											startMatching();
private:
	void											UpdateRichPresenceCustomData(void *data, unsigned int dataBytes);
	static void										TickRichPresence();
	static void										SendLastPresenceInfo();
	void											OnlineCheck();

	bool											CreateMatchingContext(bool bServer = false);
	void											StopMatchingContext();


	static sce::Toolkit::NP::PresenceDetails		s_lastPresenceInfo;
	static int										s_resendPresenceCountdown;
	static bool										s_presenceStatusDirty;
	static bool										s_presenceDataDirty;
	static HelloSyncInfo							s_lastPresenceSyncInfo;
	static HelloSyncInfo							c_presenceSyncInfoNULL;
	static bool										b_inviteRecvGUIRunning;
	// Debug
	static long long								s_roomStartTime;

	int m_hid;
	bool m_bIsInitialised;

};

