#pragma once
using namespace std;
#include <vector>
#include <qnet.h>
#include "..\..\..\Minecraft.World\C4JThread.h"
#include "..\..\Common\Network\NetworkPlayerInterface.h"
#include "..\..\Common\Network\PlatformNetworkManagerInterface.h"
#include "..\..\Common\Network\SessionInfo.h"

// This is how often we allow a search for new games
#define MINECRAFT_XSESSION_SEARCH_DELAY_MILLISECONDS 30000

// This is the xbox 360 specific implementation of CPlatformNetworkManager. It is implemented using QNET. There shouldn't be any general game code in here, this class is
// for providing a bridge between the common game-side network implementation, and the lowest level platform specific libraries (QNET in this case). 

class CPlatformNetworkManagerXbox : public CPlatformNetworkManager, public IQNetCallbacks
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
	virtual bool IsInStatsEnabledSession();
	virtual bool IsReadyToPlayOrIdle();
	virtual bool SessionHasSpace(unsigned int spaceRequired = 1);
	virtual void SendInviteGUI(int quadrant);
	virtual bool IsAddingPlayer();

	virtual void HostGame(int localUsersMask, bool bOnlineGame, bool bIsPrivate, unsigned char publicSlots = MINECRAFT_NET_MAX_PLAYERS, unsigned char privateSlots = 0);
	virtual int  JoinGame(FriendSessionInfo *searchResult, int localUsersMask, int primaryUserIndex );
	virtual bool SetLocalGame(bool isLocal);
	virtual bool IsLocalGame() { return m_bIsOfflineGame; }
	virtual void SetPrivateGame(bool isPrivate);
	virtual bool IsPrivateGame() { return m_bIsPrivateGame; }
	virtual bool IsLeavingGame() { return m_bLeavingGame; }
	virtual void ResetLeavingGame() { m_bLeavingGame = true; }

	virtual void RegisterPlayerChangedCallback(int iPad, void (*callback)(void *callbackParam, INetworkPlayer *pPlayer, bool leaving), void *callbackParam);
	virtual void UnRegisterPlayerChangedCallback(int iPad, void (*callback)(void *callbackParam, INetworkPlayer *pPlayer, bool leaving), void *callbackParam);

	virtual void HandleSignInChange();

	virtual bool _RunNetworkGame();
	
private:
	bool isSystemPrimaryPlayer(IQNetPlayer *pQNetPlayer);
	virtual bool _LeaveGame(bool bMigrateHost, bool bLeaveRoom);
	virtual void _HostGame(int dwUsersMask, unsigned char publicSlots = MINECRAFT_NET_MAX_PLAYERS, unsigned char privateSlots = 0);
	virtual bool _StartGame();

    IQNet *             m_pIQNet;             // pointer to QNet interface

	HANDLE m_notificationListener;

	vector<IQNetPlayer *> m_machineQNetPrimaryPlayers; // collection of players that we deem to be the main one for that system

	bool			m_bLeavingGame;
	bool			m_bLeaveGameOnTick;
	bool			m_migrateHostOnLeave;
	bool			m_bHostChanged;

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
	vector<FriendSessionInfo *> friendsSessions[XUSER_MAX_COUNT];
	int m_searchResultsCount[XUSER_MAX_COUNT];
	int m_lastSearchStartTime[XUSER_MAX_COUNT];

	// The results that will be filled in with the current search
	XSESSION_SEARCHRESULT_HEADER *m_pSearchResults[XUSER_MAX_COUNT];
	XNQOS *m_pQoSResult[XUSER_MAX_COUNT];

	// The results from the previous search, which are currently displayed in the game
	XSESSION_SEARCHRESULT_HEADER *m_pCurrentSearchResults[XUSER_MAX_COUNT];
	XNQOS *m_pCurrentQoSResult[XUSER_MAX_COUNT];
	int m_currentSearchResultsCount[XUSER_MAX_COUNT];
	
	int m_lastSearchPad;
	bool m_bSearchResultsReady;
	bool m_bSearchPending;
	LPVOID m_pSearchParam;
	void (*m_SessionsUpdatedCallback)(LPVOID pParam);

	C4JThread* m_SearchingThread;

	void TickSearch();
	void SearchForGames();
	static int SearchForGamesThreadProc( void* lpParameter );

	void SetSearchResultsReady(int resultCount = 0);

	vector<INetworkPlayer *>currentNetworkPlayers;
	INetworkPlayer *addNetworkPlayer(IQNetPlayer *pQNetPlayer);
	void removeNetworkPlayer(IQNetPlayer *pQNetPlayer);
	static INetworkPlayer *getNetworkPlayer(IQNetPlayer *pQNetPlayer);

	virtual void SetSessionTexturePackParentId( int id );
	virtual void SetSessionSubTexturePackId( int id );
	virtual void Notify(int ID, ULONG_PTR Param);

public:
	virtual vector<FriendSessionInfo *> *GetSessionList(int iPad, int localPlayers, bool partyOnly);
	virtual bool GetGameSessionInfo(int iPad, SessionID sessionId,FriendSessionInfo *foundSession);
	virtual void SetSessionsUpdatedCallback( void (*SessionsUpdatedCallback)(LPVOID pParam), LPVOID pSearchParam );
	virtual void GetFullFriendSessionInfo( FriendSessionInfo *foundSession, void (* FriendSessionUpdatedFn)(bool success, void *pParam), void *pParam );
	virtual void ForceFriendsSessionRefresh();

    // IQNetcallback interface functions
    VOID NotifyStateChanged( __in QNET_STATE OldState, __in QNET_STATE NewState, __in HRESULT hrInfo );
    VOID NotifyPlayerJoined( __in IQNetPlayer *pQNetPlayer );
    VOID NotifyPlayerLeaving(__in IQNetPlayer *pQNetPlayer );
	VOID NotifyNewHost( __in IQNetPlayer * pQNetPlayer);
	VOID NotifyDataReceived( __in IQNetPlayer * pQNetPlayerFrom, __in DWORD dwNumPlayersTo, __in_ecount(dwNumPlayersTo) IQNetPlayer ** apQNetPlayersTo, __in_bcount(dwDataSize) const BYTE * pbData, __in DWORD dwDataSize);
	VOID NotifyWriteStats( __in IQNetPlayer * pQNetPlayer );
	VOID NotifyReadinessChanged(__in IQNetPlayer * pQNetPlayer, __in BOOL bReady);
	VOID NotifyCommSettingsChanged(__in IQNetPlayer * pQNetPlayer);
	VOID NotifyGameSearchComplete(__in IQNetGameSearch * pGameSearch, __in HRESULT hrComplete, __in DWORD dwNumResults);
	VOID NotifyGameInvite( __in DWORD dwUserIndex, __in const INVITE_INFO * pInviteInfo );
	VOID NotifyContextChanged( __in const XUSER_CONTEXT * pContext);
	VOID NotifyPropertyChanged( __in const XUSER_PROPERTY * pProperty);
};