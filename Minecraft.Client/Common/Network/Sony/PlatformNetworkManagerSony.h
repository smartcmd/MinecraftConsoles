#pragma once
using namespace std;
#include <vector>
#include "..\..\..\Minecraft.World\C4JThread.h"
#include "..\..\Common\Network\NetworkPlayerInterface.h"
#include "..\..\Common\Network\PlatformNetworkManagerInterface.h"
#include "..\..\Common\Network\SessionInfo.h"
#include "SQRNetworkPlayer.h"

// This is how often we allow a search for new games
#define MINECRAFT_PS3ROOM_SEARCH_DELAY_MILLISECONDS 30000

// This is the Sony platform specific implementation of CPlatformNetworkManager. It is implemented using SQRNetworkManager/SQRNetworkPlayer. There shouldn't be any general game code in here,
// this class is for providing a bridge between the common game-side network implementation, and the lowest level platform specific libraries. 

class CPlatformNetworkManagerSony : public CPlatformNetworkManager, ISQRNetworkManagerListener
{
	friend class CGameNetworkManager;
public:
	virtual bool Initialise(CGameNetworkManager *pGameNetworkManager, int flagIndexSize);
	virtual void Terminate();
	virtual int GetJoiningReadyPercentage();
	virtual int CorrectErrorIDS(int IDS);

	virtual void DoWork();
	virtual int GetPlayerCount();
	virtual int GetOnlinePlayerCount();
	virtual int GetLocalPlayerMask(int playerIndex);
	virtual bool AddLocalPlayerByUserIndex( int userIndex );
	virtual bool RemoveLocalPlayerByUserIndex( int userIndex );
	virtual INetworkPlayer *GetLocalPlayerByUserIndex( int userIndex );
	virtual INetworkPlayer *GetPlayerByIndex(int playerIndex);
	virtual INetworkPlayer * GetPlayerByXuid(PlayerUID xuid);
	virtual INetworkPlayer * GetPlayerBySmallId(unsigned char smallId);
	virtual bool ShouldMessageForFullSession();

	virtual INetworkPlayer *GetHostPlayer();
	virtual bool IsHost();
	virtual bool JoinGameFromInviteInfo( int userIndex, int userMask, const INVITE_INFO *pInviteInfo);
	virtual bool LeaveGame(bool bMigrateHost);

	virtual bool IsInSession();
	virtual bool IsInGameplay();
	virtual bool IsReadyToPlayOrIdle();
	virtual bool IsInStatsEnabledSession();
	virtual bool SessionHasSpace(unsigned int spaceRequired = 1);

	virtual void SendInviteGUI(int quadrant);
	virtual bool IsAddingPlayer();

	virtual void HostGame(int localUsersMask, bool bOnlineGame, bool bIsPrivate, unsigned char publicSlots = MINECRAFT_NET_MAX_PLAYERS, unsigned char privateSlots = 0);
	virtual int  JoinGame(FriendSessionInfo *searchResult, int localUsersMask, int primaryUserIndex );
	virtual bool SetLocalGame(bool isLocal);
	virtual bool IsLocalGame();
	virtual void SetPrivateGame(bool isPrivate);
	virtual bool IsPrivateGame();
	virtual bool IsLeavingGame();
	virtual void ResetLeavingGame();

	virtual void RegisterPlayerChangedCallback(int iPad, void (*callback)(void *callbackParam, INetworkPlayer *pPlayer, bool leaving), void *callbackParam);
	virtual void UnRegisterPlayerChangedCallback(int iPad, void (*callback)(void *callbackParam, INetworkPlayer *pPlayer, bool leaving), void *callbackParam);

	virtual void HandleSignInChange();

	virtual bool _RunNetworkGame();

#ifdef __PSVITA__
	bool usingAdhocMode() { return m_bUsingAdhocMode; }
	bool setAdhocMode(bool bAdhoc);
	void startAdhocMatching();
	bool checkValidInviteData(const INVITE_INFO* pInviteInfo);
#endif
	
private:
	bool isSystemPrimaryPlayer(SQRNetworkPlayer *pQNetPlayer);
	virtual bool _LeaveGame(bool bMigrateHost, bool bLeaveRoom);
	virtual void _HostGame(int dwUsersMask, unsigned char publicSlots = MINECRAFT_NET_MAX_PLAYERS, unsigned char privateSlots = 0);
	virtual bool _StartGame();

#ifdef __PSVITA__
	bool							m_bUsingAdhocMode;
	SQRNetworkManager_Vita*         m_pSQRNet_Vita;
	SQRNetworkManager_AdHoc_Vita*   m_pSQRNet_Vita_Adhoc;
#endif
    SQRNetworkManager *             m_pSQRNet;             // pointer to SQRNetworkManager interface

	HANDLE m_notificationListener;

	vector<SQRNetworkPlayer *> m_machineSQRPrimaryPlayers; // collection of players that we deem to be the main one for that system

	bool			m_bLeavingGame;
	bool			m_bLeaveGameOnTick;
	bool			m_migrateHostOnLeave;
	bool			m_bHostChanged;
	bool			m_bLeaveRoomWhenLeavingGame;

	bool			m_bIsOfflineGame;
	bool			m_bIsPrivateGame;
	int				m_flagIndexSize;

	// This is only maintained by the host, and is not valid on client machines
	GameSessionData m_hostGameSessionData;
	CGameNetworkManager *m_pGameNetworkManager;
public:
	virtual void UpdateAndSetGameSessionData(INetworkPlayer *pNetworkPlayerLeaving = NULL);

private:
	// TODO 4J Stu - Do we need to be able to have more than one of these?
	void (*playerChangedCallback[XUSER_MAX_COUNT])(void *callbackParam, INetworkPlayer *pPlayer, bool leaving);
	void *playerChangedCallbackParam[XUSER_MAX_COUNT];

	static int RemovePlayerOnSocketClosedThreadProc( void* lpParam );
	virtual bool RemoveLocalPlayer( INetworkPlayer *pNetworkPlayer );

	// Things for handling per-system flags
	class PlayerFlags
	{
	public:
		INetworkPlayer *m_pNetworkPlayer;
		unsigned char *flags;
		unsigned int count;
		PlayerFlags(INetworkPlayer *pNetworkPlayer, unsigned int count);
		~PlayerFlags();
	};
	vector<PlayerFlags *> m_playerFlags;
	void SystemFlagAddPlayer(INetworkPlayer *pNetworkPlayer);
	void SystemFlagRemovePlayer(INetworkPlayer *pNetworkPlayer);
	void SystemFlagReset();
public:
	virtual void SystemFlagSet(INetworkPlayer *pNetworkPlayer, int index);
	virtual bool SystemFlagGet(INetworkPlayer *pNetworkPlayer, int index);

	// For telemetry
private:
	float m_lastPlayerEventTimeStart;

public:
	wstring GatherStats();
	wstring GatherRTTStats();

private:	
	vector<FriendSessionInfo *> friendsSessions;
	
	int m_lastSearchStartTime;

	// The results that will be filled in with the current search
	int m_searchResultsCount;
	SQRNetworkManager::SessionSearchResult *m_pSearchResults;	

	int m_lastSearchPad;
	bool m_bSearchPending;
	LPVOID m_pSearchParam;
	void (*m_SessionsUpdatedCallback)(LPVOID pParam);

	C4JThread* m_SearchingThread;

	void TickSearch();

	vector<INetworkPlayer *>currentNetworkPlayers;
	INetworkPlayer *addNetworkPlayer(SQRNetworkPlayer *pSQRPlayer);
	void removeNetworkPlayer(SQRNetworkPlayer *pSQRPlayer);
	static INetworkPlayer *getNetworkPlayer(SQRNetworkPlayer *pSQRPlayer);

	virtual void SetSessionTexturePackParentId( int id );
	virtual void SetSessionSubTexturePackId( int id );
	virtual void Notify(int ID, ULONG_PTR Param);

public:
	virtual vector<FriendSessionInfo *> *GetSessionList(int iPad, int localPlayers, bool partyOnly);
	virtual bool GetGameSessionInfo(int iPad, SessionID sessionId,FriendSessionInfo *foundSession);
	virtual void SetSessionsUpdatedCallback( void (*SessionsUpdatedCallback)(LPVOID pParam), LPVOID pSearchParam );
	virtual void GetFullFriendSessionInfo( FriendSessionInfo *foundSession, void (* FriendSessionUpdatedFn)(bool success, void *pParam), void *pParam );
	virtual void ForceFriendsSessionRefresh();

	// ... and the new ones that have been converted to ISQRNetworkManagerListener
	virtual void HandleDataReceived(SQRNetworkPlayer *playerFrom, SQRNetworkPlayer *playerTo, unsigned char *data, unsigned int dataSize);
	virtual void HandlePlayerJoined(SQRNetworkPlayer *player);
	virtual void HandlePlayerLeaving(SQRNetworkPlayer *player);
	virtual void HandleStateChange(SQRNetworkManager::eSQRNetworkManagerState oldState, SQRNetworkManager::eSQRNetworkManagerState newState, bool idleReasonIsSessionFull);
	virtual void HandleResyncPlayerRequest(SQRNetworkPlayer **aPlayers);
	virtual void HandleAddLocalPlayerFailed(int idx);
	virtual void HandleDisconnect(bool bLostRoomOnly,bool bPSNSignOut=false);
	virtual void HandleInviteReceived( int userIndex, const SQRNetworkManager::PresenceSyncInfo *pInviteInfo);

	static void SetSQRPresenceInfoFromExtData(SQRNetworkManager::PresenceSyncInfo *presence, void *pExtData, SceNpMatching2RoomId roomId, SceNpMatching2ServerId serverId);
	static void MallocAndSetExtDataFromSQRPresenceInfo(void **pExtData, SQRNetworkManager::PresenceSyncInfo *presence);
};
