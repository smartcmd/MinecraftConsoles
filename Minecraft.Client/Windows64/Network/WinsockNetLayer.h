// Code implemented by LCEMP, credit if used on other repos
// https://github.com/LCEMP/LCEMP
#pragma once

#ifdef _WINDOWS64

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include "..\..\Common\Network\NetworkPlayerInterface.h"

#pragma comment(lib, "Ws2_32.lib")

#define WIN64_NET_DEFAULT_PORT 25565
#define WIN64_NET_MAX_CLIENTS 255
#define WIN64_SMALLID_REJECT 0xFF
#define WIN64_NET_RECV_BUFFER_SIZE 65536
#define WIN64_NET_MAX_PACKET_SIZE (4 * 1024 * 1024)
#define WIN64_LAN_DISCOVERY_PORT 25566
#define WIN64_LAN_BROADCAST_MAGIC 0x4D434C4E

class Socket;

#pragma pack(push, 1)
struct Win64LANBroadcast
{
	DWORD magic;
	WORD netVersion;
	WORD gamePort;
	wchar_t hostName[32];
	BYTE playerCount;
	BYTE maxPlayers;
	DWORD gameHostSettings;
	DWORD texturePackParentId;
	BYTE subTexturePackId;
	BYTE isJoinable;
};
#pragma pack(pop)

struct Win64LANSession
{
	char hostIP[64];
	int hostPort;
	wchar_t hostName[32];
	unsigned short netVersion;
	unsigned char playerCount;
	unsigned char maxPlayers;
	unsigned int gameHostSettings;
	unsigned int texturePackParentId;
	unsigned char subTexturePackId;
	bool isJoinable;
	DWORD lastSeenTick;
};

struct Win64RemoteConnection
{
	SOCKET tcpSocket;
	BYTE smallId;
	HANDLE recvThread;
	volatile bool active;
};

class WinsockNetLayer
{
public:
	static bool Initialize();
	static void Shutdown();

	static bool HostGame(int port, const char* bindIp = nullptr);
	static bool JoinGame(const char* ip, int port);

	static bool SendToSmallId(BYTE targetSmallId, const void* data, int dataSize);
	static bool SendOnSocket(SOCKET sock, const void* data, int dataSize);

	// Non-host split-screen: additional TCP connections to host, one per pad
	static bool JoinSplitScreen(int padIndex, BYTE* outSmallId);
	static void CloseSplitScreenConnection(int padIndex);
	static SOCKET GetLocalSocket(BYTE senderSmallId);
	static BYTE GetSplitScreenSmallId(int padIndex);

	static bool IsHosting() { return s_isHost; }
	static bool IsConnected() { return s_connected; }
	static bool IsActive() { return s_active; }

	static BYTE GetLocalSmallId() { return s_localSmallId; }
	static BYTE GetHostSmallId() { return s_hostSmallId; }

	static SOCKET GetSocketForSmallId(BYTE smallId);

	static void HandleDataReceived(BYTE fromSmallId, BYTE toSmallId, unsigned char* data, unsigned int dataSize);

	static bool PopDisconnectedSmallId(BYTE* outSmallId);
	static void PushFreeSmallId(BYTE smallId);
	static void CloseConnectionBySmallId(BYTE smallId);

	static bool StartAdvertising(int gamePort, const wchar_t* hostName, unsigned int gameSettings, unsigned int texPackId, unsigned char subTexId, unsigned short netVer);
	static void StopAdvertising();
	static void UpdateAdvertisePlayerCount(BYTE count);
	static void UpdateAdvertiseMaxPlayers(BYTE maxPlayers);
	static void UpdateAdvertiseJoinable(bool joinable);

	static bool StartDiscovery();
	static void StopDiscovery();
	static std::vector<Win64LANSession> GetDiscoveredSessions();

	static int GetHostPort() { return s_hostGamePort; }

private:
	static DWORD WINAPI AcceptThreadProc(LPVOID param);
	static DWORD WINAPI RecvThreadProc(LPVOID param);
	static DWORD WINAPI ClientRecvThreadProc(LPVOID param);
	static DWORD WINAPI SplitScreenRecvThreadProc(LPVOID param);
	static DWORD WINAPI AdvertiseThreadProc(LPVOID param);
	static DWORD WINAPI DiscoveryThreadProc(LPVOID param);

	static SOCKET s_listenSocket;
	static SOCKET s_hostConnectionSocket;
	static HANDLE s_acceptThread;
	static HANDLE s_clientRecvThread;

	static bool s_isHost;
	static bool s_connected;
	static bool s_active;
	static bool s_initialized;

	static BYTE s_localSmallId;
	static BYTE s_hostSmallId;
	static unsigned int s_nextSmallId;

	static CRITICAL_SECTION s_sendLock;
	static CRITICAL_SECTION s_connectionsLock;

	static std::vector<Win64RemoteConnection> s_connections;

	static SOCKET s_advertiseSock;
	static HANDLE s_advertiseThread;
	static volatile bool s_advertising;
	static Win64LANBroadcast s_advertiseData;
	static CRITICAL_SECTION s_advertiseLock;
	static int s_hostGamePort;

	static SOCKET s_discoverySock;
	static HANDLE s_discoveryThread;
	static volatile bool s_discovering;
	static CRITICAL_SECTION s_discoveryLock;
	static std::vector<Win64LANSession> s_discoveredSessions;

	static CRITICAL_SECTION s_disconnectLock;
	static std::vector<BYTE> s_disconnectedSmallIds;

	static CRITICAL_SECTION s_freeSmallIdLock;
	static std::vector<BYTE> s_freeSmallIds;
	// O(1) smallId -> socket lookup so we don't scan s_connections (which never shrinks) on every send
	static SOCKET s_smallIdToSocket[256];
	static CRITICAL_SECTION s_smallIdToSocketLock;

	// Per-pad split-screen TCP connections (client-side, non-host only)
	static SOCKET s_splitScreenSocket[XUSER_MAX_COUNT];
	static BYTE s_splitScreenSmallId[XUSER_MAX_COUNT];
	static HANDLE s_splitScreenRecvThread[XUSER_MAX_COUNT];

public:
	static void ClearSocketForSmallId(BYTE smallId);
};

extern bool g_Win64MultiplayerHost;
extern bool g_Win64MultiplayerJoin;
extern int g_Win64MultiplayerPort;
extern char g_Win64MultiplayerIP[256];
extern bool g_Win64DedicatedServer;
extern int g_Win64DedicatedServerPort;
extern char g_Win64DedicatedServerBindIP[256];

#endif
