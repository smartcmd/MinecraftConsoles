#pragma once
using namespace std;
#include <vector>
#include <qnet.h>
#include "../../../Minecraft.World/C4JThread.h"
#include "NetworkPlayerInterface.h"
#ifdef _XBOX
#include "../../Xbox/Network/PlatformNetworkManagerXbox.h"
#elif defined __PS3__ || defined __ORBIS__ || defined __PSVITA__
#include "../../Common/Network/Sony/PlatformNetworkManagerSony.h"
#elif defined _DURANGO
#include "../../Durango/Network/PlatformNetworkManagerDurango.h"
#else
#include "PlatformNetworkManagerStub.h"
#endif
#include "SessionInfo.h"

#ifdef __ORBIS__
#include "../../Orbis/Network/PsPlusUpsellWrapper_Orbis.h"
#endif

class ClientConnection;
class Minecraft;

const int NON_QNET_SENDDATA_ACK_REQUIRED = 1;

// This class implements the game-side interface to the networking system. As such, it is platform independent and may contain bits of game-side code where appropriate.
// It shouldn't ever reference any platform specifics of the network implementation (eg QNET), rather it should interface with an implementation of PlatformNetworkManager to
// provide this functionality.

class CGameNetworkManager
{
#ifdef _XBOX
	friend class CPlatformNetworkManagerXbox;
#elif defined __PS3__ || defined __ORBIS__ || defined __PSVITA__
	friend class CPlatformNetworkManagerSony;
#elif defined _DURANGO
	friend class CPlatformNetworkManagerDurango;
#else
	friend class CPlatformNetworkManagerStub;
#endif
public:
	CGameNetworkManager();
	// Misc high level flow

	typedef enum
	{
		JOINGAME_SUCCESS,
		JOINGAME_FAIL_GENERAL,
		JOINGAME_FAIL_SERVER_FULL
	} eJoinGameResult;

	void			Initialise();
	void			Terminate();
	void			DoWork();
	bool			_RunNetworkGame(LPVOID lpParameter);
	bool			StartNetworkGame(Minecraft *minecraft, LPVOID lpParameter);
	int				CorrectErrorIDS(int IDS);

	// Player management

	static int		GetLocalPlayerMask(int playerIndex);
	int				GetPlayerCount();
	int				GetOnlinePlayerCount();
	bool			AddLocalPlayerByUserIndex( int userIndex );
	bool			RemoveLocalPlayerByUserIndex( int userIndex );
	INetworkPlayer	*GetLocalPlayerByUserIndex(int userIndex );
	INetworkPlayer	*GetPlayerByIndex(int playerIndex);
	INetworkPlayer	*GetPlayerByXuid(PlayerUID xuid);
	INetworkPlayer	*GetPlayerBySmallId(unsigned char smallId);
	wstring			GetDisplayNameByGamertag(wstring gamertag);
	INetworkPlayer	*GetHostPlayer();
	void			RegisterPlayerChangedCallback(int iPad, void (*callback)(void *callbackParam, INetworkPlayer *pPlayer, bool leaving), void *callbackParam);
	void			UnRegisterPlayerChangedCallback(int iPad, void (*callback)(void *callbackParam, INetworkPlayer *pPlayer, bool leaving), void *callbackParam);
	void			HandleSignInChange();
	bool			ShouldMessageForFullSession();

	// State management

	bool			IsInSession();
	bool			IsInGameplay();
	bool			IsLeavingGame();
	bool			IsReadyToPlayOrIdle();

	// Hosting and game type

	bool			SetLocalGame(bool isLocal);
	bool			IsLocalGame();
	void			SetPrivateGame(bool isPrivate);
	bool			IsPrivateGame();
	void			HostGame(int localUsersMask, bool bOnlineGame, bool bIsPrivate, unsigned char publicSlots = MINECRAFT_NET_MAX_PLAYERS, unsigned char privateSlots = 0);
	bool			IsHost();
	bool			IsInStatsEnabledSession();

	// Client session discovery

	bool			SessionHasSpace(unsigned int spaceRequired = 1);
	vector<FriendSessionInfo *>	*GetSessionList(int iPad, int localPlayers, bool partyOnly);
	bool			GetGameSessionInfo(int iPad, SessionID sessionId,FriendSessionInfo *foundSession);
	void			SetSessionsUpdatedCallback( void (*SessionsUpdatedCallback)(LPVOID pParam), LPVOID pSearchParam );
	void			GetFullFriendSessionInfo( FriendSessionInfo *foundSession, void (* FriendSessionUpdatedFn)(bool success, void *pParam), void *pParam );
	void			ForceFriendsSessionRefresh();

	// Session joining and leaving

	bool			JoinGameFromInviteInfo( int userIndex, int userMask, const INVITE_INFO *pInviteInfo);
	eJoinGameResult	JoinGame(FriendSessionInfo *searchResult, int localUsersMask);
	static void		CancelJoinGame(LPVOID lpParam); // Not part of the shared interface
	bool			LeaveGame(bool bMigrateHost);
	static int		JoinFromInvite_SignInReturned(void *pParam,bool bContinue, int iPad);
	void			UpdateAndSetGameSessionData(INetworkPlayer *pNetworkPlayerLeaving = NULL);
	void			SendInviteGUI(int iPad);
	void			ResetLeavingGame();

	// Threads

	bool			IsNetworkThreadRunning();
	static int		RunNetworkGameThreadProc( void* lpParameter );
	static int		ServerThreadProc( void* lpParameter );
	static int		ExitAndJoinFromInviteThreadProc( void* lpParam );

#if (defined __PS3__) || (defined __ORBIS__) || (defined __PSVITA__)
	static int		MustSignInReturned_0(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int		PSNSignInReturned_0(void* pParam, bool bContinue, int iPad);

	static int		MustSignInReturned_1(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int		PSNSignInReturned_1(void* pParam, bool bContinue, int iPad);
#endif

	static void		_LeaveGame();
	static int		ChangeSessionTypeThreadProc( void* lpParam );

	// System flags

	void SystemFlagSet(INetworkPlayer *pNetworkPlayer, int index);
	bool SystemFlagGet(INetworkPlayer *pNetworkPlayer, int index);

	// Events

	void ServerReadyCreate(bool create);	// Create the signal (or set to NULL)
	void ServerReady();						// Signal that we are ready
	void ServerReadyWait();					// Wait for the signal
	void ServerReadyDestroy();				// Destroy signal
	bool ServerReadyValid();				// Is non-NULL

	void ServerStoppedCreate(bool create);	// Create the signal
	void ServerStopped();					// Signal that we are ready
	void ServerStoppedWait();				// Wait for the signal
	void ServerStoppedDestroy();			// Destroy signal
	bool ServerStoppedValid();				// Is non-NULL

#ifdef __PSVITA__
	static bool usingAdhocMode();
	static void setAdhocMode(bool bAdhoc);
	static void startAdhocMatching();
#endif
	// Debug output

	wstring GatherStats();
	void renderQueueMeter();
	wstring GatherRTTStats();

	// GUI debug output

	// Used for debugging output
	static const int messageQueue_length = 512;
	static int64_t messageQueue[messageQueue_length];
	static const int byteQueue_length = 512;
    static int64_t byteQueue[byteQueue_length];
	static int messageQueuePos;

	// Methods called from PlatformNetworkManager
private:
	void StateChange_AnyToHosting();
	void StateChange_AnyToJoining();
	void StateChange_JoiningToIdle(CPlatformNetworkManager::eJoinFailedReason reason);
	void StateChange_AnyToStarting();
	void StateChange_AnyToEnding(bool bStateWasPlaying);
	void StateChange_AnyToIdle();
	void CreateSocket( INetworkPlayer *pNetworkPlayer, bool localPlayer );
	void CloseConnection( INetworkPlayer *pNetworkPlayer );
	void PlayerJoining( INetworkPlayer *pNetworkPlayer );
	void PlayerLeaving( INetworkPlayer *pNetworkPlayer );
	void HostChanged();
	void WriteStats( INetworkPlayer *pNetworkPlayer );
	void GameInviteReceived( int userIndex, const INVITE_INFO *pInviteInfo);
	void HandleInviteWhenInMenus( int userIndex, const INVITE_INFO *pInviteInfo);
	void AddLocalPlayerFailed(int idx, bool serverFull = false);
#if defined __PS3__ || defined __PSVITA__ || defined __ORBIS__
	void HandleDisconnect(bool bLostRoomOnly,bool bPSNSignOut);
#else
	void HandleDisconnect(bool bLostRoomOnly);
#endif

	int  GetPrimaryPad();
	int  GetLockedProfile();
	bool IsSignedInLive(int playerIdx);
	bool AllowedToPlayMultiplayer(int playerIdx);
	const char *GetOnlineName(int playerIdx);

	C4JThread::Event*	m_hServerStoppedEvent;
	C4JThread::Event*	m_hServerReadyEvent;
	bool m_bInitialised;

#ifdef _XBOX_ONE
public:
	void SetFullSessionMessageOnNextSessionChange() { m_bFullSessionMessageOnNextSessionChange = true; }
#endif
private:
	float m_lastPlayerEventTimeStart;			// For telemetry
	static CPlatformNetworkManager *s_pPlatformNetworkManager;
	bool			m_bNetworkThreadRunning;
	int GetJoiningReadyPercentage();
	bool m_bLastDisconnectWasLostRoomOnly;
	bool m_bFullSessionMessageOnNextSessionChange;
#if defined __PS3__ || defined __PSVITA__ || defined __ORBIS__
	bool m_bSignedOutofPSN;
#endif

#ifdef __ORBIS__
	PsPlusUpsellWrapper	*m_pUpsell;
	INVITE_INFO			*m_pInviteInfo;
	int					 m_iPlayerInvited;
#endif

public:
#ifndef _XBOX
	void FakeLocalPlayerJoined();		// Temporary method whilst we don't have real networking to make this happen
#endif
};

extern CGameNetworkManager g_NetworkManager;

#ifdef __PS3__
#undef __in
#define __out
#endif
