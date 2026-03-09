#pragma once
using namespace std;
#include <vector>
#include <qnet.h>
#include "..\..\..\Minecraft.World\C4JThread.h"
#include "NetworkPlayerInterface.h"
#include "SessionInfo.h"

class ClientConnection;
class Minecraft;
class CGameNetworkManager;

// This is the interface to be implemented by the platform-specific versions of the PlatformNetworkManagers. This API is used directly by GameNetworkManager so that
// it can remain as platform independent as possible.

// This value should be incremented if the server version changes, or the game session data changes
#define MINECRAFT_NET_VERSION VER_NETWORK


typedef struct _SearchForGamesData
{
	DWORD sessionIDCount;
	XSESSION_SEARCHRESULT_HEADER *searchBuffer;
	XNQOS **ppQos;
	SessionID *sessionIDList;
	XOVERLAPPED *pOverlapped;
} SearchForGamesData;

class CPlatformNetworkManager
{
	friend class CGameNetworkManager;
public:

	typedef enum
	{
		JOIN_FAILED_SERVER_FULL,
		JOIN_FAILED_INSUFFICIENT_PRIVILEGES,
		JOIN_FAILED_NONSPECIFIC,
	} eJoinFailedReason;

	virtual bool Initialise(CGameNetworkManager *pGameNetworkManager, int flagIndexSize) = 0;
	virtual void Terminate() = 0;
	virtual int GetJoiningReadyPercentage() = 0;
	virtual int CorrectErrorIDS(int IDS) = 0;

	virtual void DoWork() = 0;
	virtual int GetPlayerCount() = 0;
	virtual int GetOnlinePlayerCount() = 0;
	virtual int GetLocalPlayerMask(int playerIndex) = 0;
	virtual bool AddLocalPlayerByUserIndex( int userIndex ) = 0;
	virtual bool RemoveLocalPlayerByUserIndex( int userIndex ) = 0;
	virtual INetworkPlayer *GetLocalPlayerByUserIndex( int userIndex ) = 0;
	virtual INetworkPlayer *GetPlayerByIndex(int playerIndex) = 0;
	virtual INetworkPlayer * GetPlayerByXuid(PlayerUID xuid) = 0;
	virtual INetworkPlayer * GetPlayerBySmallId(unsigned char smallId) = 0;
	virtual bool ShouldMessageForFullSession() = 0;

	virtual INetworkPlayer *GetHostPlayer() = 0;
	virtual bool IsHost() = 0;
	virtual bool JoinGameFromInviteInfo( int userIndex, int userMask, const INVITE_INFO *pInviteInfo) = 0;
	virtual bool LeaveGame(bool bMigrateHost) = 0;

	virtual bool IsInSession() = 0;
	virtual bool IsInGameplay() = 0;
	virtual bool IsReadyToPlayOrIdle() = 0;
	virtual bool IsInStatsEnabledSession() = 0;
	virtual bool SessionHasSpace(unsigned int spaceRequired = 1) = 0;
	virtual void SendInviteGUI(int quadrant) = 0;
	virtual bool IsAddingPlayer() = 0;

	virtual void HostGame(int localUsersMask, bool bOnlineGame, bool bIsPrivate, unsigned char publicSlots = MINECRAFT_NET_MAX_PLAYERS, unsigned char privateSlots = 0) = 0;
	virtual int  JoinGame(FriendSessionInfo *searchResult, int dwLocalUsersMask, int dwPrimaryUserIndex ) = 0;
	virtual void CancelJoinGame() {};
	virtual bool SetLocalGame(bool isLocal) = 0;
	virtual bool IsLocalGame() = 0;
	virtual void SetPrivateGame(bool isPrivate) = 0;
	virtual bool IsPrivateGame() = 0;
	virtual bool IsLeavingGame() = 0;
	virtual void ResetLeavingGame() = 0;

	virtual void RegisterPlayerChangedCallback(int iPad, void (*callback)(void *callbackParam, INetworkPlayer *pPlayer, bool leaving), void *callbackParam) = 0;
	virtual void UnRegisterPlayerChangedCallback(int iPad, void (*callback)(void *callbackParam, INetworkPlayer *pPlayer, bool leaving), void *callbackParam) = 0;

	virtual void HandleSignInChange() = 0;

	virtual bool _RunNetworkGame() = 0;
	virtual void SetGamePlayState() {}

private:
	virtual bool _LeaveGame(bool bMigrateHost, bool bLeaveRoom) = 0;
	virtual void _HostGame(int usersMask, unsigned char publicSlots = MINECRAFT_NET_MAX_PLAYERS, unsigned char privateSlots = 0) = 0;
	virtual bool _StartGame() = 0;


public:
	virtual void UpdateAndSetGameSessionData(INetworkPlayer *pNetworkPlayerLeaving = nullptr) = 0;

private:
	virtual bool RemoveLocalPlayer( INetworkPlayer *pNetworkPlayer ) = 0;

public:
	virtual void SystemFlagSet(INetworkPlayer *pNetworkPlayer, int index) = 0;
	virtual bool SystemFlagGet(INetworkPlayer *pNetworkPlayer, int index) = 0;

	virtual wstring GatherStats() = 0;
	virtual wstring GatherRTTStats() = 0;

private:	
	virtual void SetSessionTexturePackParentId( int id ) = 0;
	virtual void SetSessionSubTexturePackId( int id ) = 0;
	virtual void Notify(int ID, ULONG_PTR Param) = 0;

public:
	virtual vector<FriendSessionInfo *> *GetSessionList(int iPad, int localPlayers, bool partyOnly) = 0;
	virtual bool GetGameSessionInfo(int iPad, SessionID sessionId,FriendSessionInfo *foundSession) = 0;
	virtual void SetSessionsUpdatedCallback( void (*SessionsUpdatedCallback)(LPVOID pParam), LPVOID pSearchParam ) = 0;
	virtual void GetFullFriendSessionInfo( FriendSessionInfo *foundSession, void (* FriendSessionUpdatedFn)(bool success, void *pParam), void *pParam ) = 0;
	virtual void ForceFriendsSessionRefresh() = 0;

#ifndef _XBOX
	virtual void FakeLocalPlayerJoined() {};		// Temporary method whilst we don't have real networking to make this happen
#endif

#ifdef _DURANGO
	virtual wstring GetDisplayNameByGamertag(wstring gamertag) = 0;
#endif
};
