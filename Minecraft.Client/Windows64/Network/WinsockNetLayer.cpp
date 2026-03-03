// Code implemented by LCEMP, credit if used on other repos
// https://github.com/LCEMP/LCEMP

#include "stdafx.h"

#ifdef _WINDOWS64

#include "WinsockNetLayer.h"
#include "..\..\Common\Network\PlatformNetworkManagerStub.h"
#include "..\..\..\Minecraft.World\Socket.h"

SOCKET WinsockNetLayer::s_listenSocket = INVALID_SOCKET;
SOCKET WinsockNetLayer::s_hostConnectionSocket = INVALID_SOCKET;
HANDLE WinsockNetLayer::s_acceptThread = NULL;
HANDLE WinsockNetLayer::s_clientRecvThread = NULL;

bool WinsockNetLayer::s_isHost = false;
bool WinsockNetLayer::s_connected = false;
bool WinsockNetLayer::s_active = false;
bool WinsockNetLayer::s_initialized = false;

BYTE WinsockNetLayer::s_localSmallId = 0;
BYTE WinsockNetLayer::s_hostSmallId = 0;
BYTE WinsockNetLayer::s_nextSmallId = 1;

CRITICAL_SECTION WinsockNetLayer::s_sendLock;
CRITICAL_SECTION WinsockNetLayer::s_connectionsLock;

std::vector<Win64RemoteConnection> WinsockNetLayer::s_connections;

SOCKET WinsockNetLayer::s_advertiseSock = INVALID_SOCKET;
HANDLE WinsockNetLayer::s_advertiseThread = NULL;
volatile bool WinsockNetLayer::s_advertising = false;
Win64LANBroadcast WinsockNetLayer::s_advertiseData = {};
CRITICAL_SECTION WinsockNetLayer::s_advertiseLock;
int WinsockNetLayer::s_hostGamePort = WIN64_NET_DEFAULT_PORT;

SOCKET WinsockNetLayer::s_discoverySock = INVALID_SOCKET;
HANDLE WinsockNetLayer::s_discoveryThread = NULL;
volatile bool WinsockNetLayer::s_discovering = false;
CRITICAL_SECTION WinsockNetLayer::s_discoveryLock;
std::vector<Win64LANSession> WinsockNetLayer::s_discoveredSessions;

CRITICAL_SECTION WinsockNetLayer::s_disconnectLock;
std::vector<BYTE> WinsockNetLayer::s_disconnectedSmallIds;

CRITICAL_SECTION WinsockNetLayer::s_freeSmallIdLock;
std::vector<BYTE> WinsockNetLayer::s_freeSmallIds;

bool g_Win64MultiplayerHost = false;
bool g_Win64MultiplayerJoin = false;
int g_Win64MultiplayerPort = WIN64_NET_DEFAULT_PORT;
char g_Win64MultiplayerIP[256] = "127.0.0.1";

bool WinsockNetLayer::Initialize()
{
	if (s_initialized) return true;

	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		app.DebugPrintf("WSAStartup failed: %d\n", result);
		return false;
	}

	InitializeCriticalSection(&s_sendLock);
	InitializeCriticalSection(&s_connectionsLock);
	InitializeCriticalSection(&s_advertiseLock);
	InitializeCriticalSection(&s_discoveryLock);
	InitializeCriticalSection(&s_disconnectLock);
	InitializeCriticalSection(&s_freeSmallIdLock);

	s_initialized = true;

	StartDiscovery();

	return true;
}

void WinsockNetLayer::Shutdown()
{
	StopAdvertising();
	StopDiscovery();

	s_active = false;
	s_connected = false;

	if (s_listenSocket != INVALID_SOCKET)
	{
		closesocket(s_listenSocket);
		s_listenSocket = INVALID_SOCKET;
	}

	if (s_hostConnectionSocket != INVALID_SOCKET)
	{
		closesocket(s_hostConnectionSocket);
		s_hostConnectionSocket = INVALID_SOCKET;
	}

	EnterCriticalSection(&s_connectionsLock);
	for (size_t i = 0; i < s_connections.size(); i++)
	{
		s_connections[i].active = false;
		if (s_connections[i].tcpSocket != INVALID_SOCKET)
		{
			closesocket(s_connections[i].tcpSocket);
		}
	}
	s_connections.clear();
	LeaveCriticalSection(&s_connectionsLock);

	if (s_acceptThread != NULL)
	{
		WaitForSingleObject(s_acceptThread, 2000);
		CloseHandle(s_acceptThread);
		s_acceptThread = NULL;
	}

	if (s_clientRecvThread != NULL)
	{
		WaitForSingleObject(s_clientRecvThread, 2000);
		CloseHandle(s_clientRecvThread);
		s_clientRecvThread = NULL;
	}

	if (s_initialized)
	{
		DeleteCriticalSection(&s_sendLock);
		DeleteCriticalSection(&s_connectionsLock);
		DeleteCriticalSection(&s_advertiseLock);
		DeleteCriticalSection(&s_discoveryLock);
		DeleteCriticalSection(&s_disconnectLock);
		s_disconnectedSmallIds.clear();
		DeleteCriticalSection(&s_freeSmallIdLock);
		s_freeSmallIds.clear();
		WSACleanup();
		s_initialized = false;
	}
}

bool WinsockNetLayer::HostGame(int port)
{
	if (!s_initialized && !Initialize()) return false;

	s_isHost = true;
	s_localSmallId = 0;
	s_hostSmallId = 0;
	s_nextSmallId = 1;
	s_hostGamePort = port;

	EnterCriticalSection(&s_freeSmallIdLock);
	s_freeSmallIds.clear();
	LeaveCriticalSection(&s_freeSmallIdLock);

	struct addrinfo hints = {};
	struct addrinfo* result = NULL;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	char portStr[16];
	sprintf_s(portStr, "%d", port);

	int iResult = getaddrinfo(NULL, portStr, &hints, &result);
	if (iResult != 0)
	{
		app.DebugPrintf("getaddrinfo failed: %d\n", iResult);
		return false;
	}

	s_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (s_listenSocket == INVALID_SOCKET)
	{
		app.DebugPrintf("socket() failed: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		return false;
	}

	int opt = 1;
	setsockopt(s_listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

	iResult = ::bind(s_listenSocket, result->ai_addr, (int)result->ai_addrlen);
	freeaddrinfo(result);
	if (iResult == SOCKET_ERROR)
	{
		app.DebugPrintf("bind() failed: %d\n", WSAGetLastError());
		closesocket(s_listenSocket);
		s_listenSocket = INVALID_SOCKET;
		return false;
	}

	iResult = listen(s_listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		app.DebugPrintf("listen() failed: %d\n", WSAGetLastError());
		closesocket(s_listenSocket);
		s_listenSocket = INVALID_SOCKET;
		return false;
	}

	s_active = true;
	s_connected = true;

	s_acceptThread = CreateThread(NULL, 0, AcceptThreadProc, NULL, 0, NULL);

	app.DebugPrintf("Win64 LAN: Hosting on port %d\n", port);
	return true;
}

bool WinsockNetLayer::JoinGame(const char* ip, int port)
{
	if (!s_initialized && !Initialize()) return false;

	s_isHost = false;
	s_hostSmallId = 0;
	s_connected = false;
	s_active = false;

	if (s_hostConnectionSocket != INVALID_SOCKET)
	{
		closesocket(s_hostConnectionSocket);
		s_hostConnectionSocket = INVALID_SOCKET;
	}

	struct addrinfo hints = {};
	struct addrinfo* result = NULL;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	char portStr[16];
	sprintf_s(portStr, "%d", port);

	int iResult = getaddrinfo(ip, portStr, &hints, &result);
	if (iResult != 0)
	{
		app.DebugPrintf("getaddrinfo failed for %s:%d - %d\n", ip, port, iResult);
		return false;
	}

	bool connected = false;
	BYTE assignedSmallId = 0;
	const int maxAttempts = 12;

	for (int attempt = 0; attempt < maxAttempts; ++attempt)
	{
		s_hostConnectionSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (s_hostConnectionSocket == INVALID_SOCKET)
		{
			app.DebugPrintf("socket() failed: %d\n", WSAGetLastError());
			break;
		}

		int noDelay = 1;
		setsockopt(s_hostConnectionSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&noDelay, sizeof(noDelay));

		iResult = connect(s_hostConnectionSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			int err = WSAGetLastError();
			app.DebugPrintf("connect() to %s:%d failed (attempt %d/%d): %d\n", ip, port, attempt + 1, maxAttempts, err);
			closesocket(s_hostConnectionSocket);
			s_hostConnectionSocket = INVALID_SOCKET;
			Sleep(200);
			continue;
		}

		BYTE assignBuf[1];
		int bytesRecv = recv(s_hostConnectionSocket, (char*)assignBuf, 1, 0);
		if (bytesRecv != 1)
		{
			app.DebugPrintf("Failed to receive small ID assignment from host (attempt %d/%d)\n", attempt + 1, maxAttempts);
			closesocket(s_hostConnectionSocket);
			s_hostConnectionSocket = INVALID_SOCKET;
			Sleep(200);
			continue;
		}

		assignedSmallId = assignBuf[0];
		connected = true;
		break;
	}
	freeaddrinfo(result);

	if (!connected)
	{
		return false;
	}
	s_localSmallId = assignedSmallId;

	app.DebugPrintf("Win64 LAN: Connected to %s:%d, assigned smallId=%d\n", ip, port, s_localSmallId);

	s_active = true;
	s_connected = true;

	s_clientRecvThread = CreateThread(NULL, 0, ClientRecvThreadProc, NULL, 0, NULL);

	return true;
}

bool WinsockNetLayer::SendOnSocket(SOCKET sock, const void* data, int dataSize)
{
	if (sock == INVALID_SOCKET || dataSize <= 0) return false;

	EnterCriticalSection(&s_sendLock);

	BYTE header[4];
	header[0] = (BYTE)((dataSize >> 24) & 0xFF);
	header[1] = (BYTE)((dataSize >> 16) & 0xFF);
	header[2] = (BYTE)((dataSize >> 8) & 0xFF);
	header[3] = (BYTE)(dataSize & 0xFF);

	int totalSent = 0;
	int toSend = 4;
	while (totalSent < toSend)
	{
		int sent = send(sock, (const char*)header + totalSent, toSend - totalSent, 0);
		if (sent == SOCKET_ERROR || sent == 0)
		{
			LeaveCriticalSection(&s_sendLock);
			return false;
		}
		totalSent += sent;
	}

	totalSent = 0;
	while (totalSent < dataSize)
	{
		int sent = send(sock, (const char*)data + totalSent, dataSize - totalSent, 0);
		if (sent == SOCKET_ERROR || sent == 0)
		{
			LeaveCriticalSection(&s_sendLock);
			return false;
		}
		totalSent += sent;
	}

	LeaveCriticalSection(&s_sendLock);
	return true;
}

bool WinsockNetLayer::SendToSmallId(BYTE targetSmallId, const void* data, int dataSize)
{
	if (!s_active) return false;

	if (s_isHost)
	{
		SOCKET sock = GetSocketForSmallId(targetSmallId);
		if (sock == INVALID_SOCKET) return false;
		return SendOnSocket(sock, data, dataSize);
	}
	else
	{
		return SendOnSocket(s_hostConnectionSocket, data, dataSize);
	}
}

SOCKET WinsockNetLayer::GetSocketForSmallId(BYTE smallId)
{
	EnterCriticalSection(&s_connectionsLock);
	for (size_t i = 0; i < s_connections.size(); i++)
	{
		if (s_connections[i].smallId == smallId && s_connections[i].active)
		{
			SOCKET sock = s_connections[i].tcpSocket;
			LeaveCriticalSection(&s_connectionsLock);
			return sock;
		}
	}
	LeaveCriticalSection(&s_connectionsLock);
	return INVALID_SOCKET;
}

static bool RecvExact(SOCKET sock, BYTE* buf, int len)
{
	int totalRecv = 0;
	while (totalRecv < len)
	{
		int r = recv(sock, (char*)buf + totalRecv, len - totalRecv, 0);
		if (r <= 0) return false;
		totalRecv += r;
	}
	return true;
}

void WinsockNetLayer::HandleDataReceived(BYTE fromSmallId, BYTE toSmallId, unsigned char* data, unsigned int dataSize)
{
	INetworkPlayer* pPlayerFrom = g_NetworkManager.GetPlayerBySmallId(fromSmallId);
	INetworkPlayer* pPlayerTo = g_NetworkManager.GetPlayerBySmallId(toSmallId);

	if (pPlayerFrom == NULL || pPlayerTo == NULL) return;

	if (s_isHost)
	{
		::Socket* pSocket = pPlayerFrom->GetSocket();
		if (pSocket != NULL)
			pSocket->pushDataToQueue(data, dataSize, false);
	}
	else
	{
		::Socket* pSocket = pPlayerTo->GetSocket();
		if (pSocket != NULL)
			pSocket->pushDataToQueue(data, dataSize, true);
	}
}

DWORD WINAPI WinsockNetLayer::AcceptThreadProc(LPVOID param)
{
	while (s_active)
	{
		SOCKET clientSocket = accept(s_listenSocket, NULL, NULL);
		if (clientSocket == INVALID_SOCKET)
		{
			if (s_active)
				app.DebugPrintf("accept() failed: %d\n", WSAGetLastError());
			break;
		}

		int noDelay = 1;
		setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&noDelay, sizeof(noDelay));

		extern QNET_STATE _iQNetStubState;
		if (_iQNetStubState != QNET_STATE_GAME_PLAY)
		{
			app.DebugPrintf("Win64 LAN: Rejecting connection, game not ready\n");
			closesocket(clientSocket);
			continue;
		}

		BYTE assignedSmallId;
		EnterCriticalSection(&s_freeSmallIdLock);
		if (!s_freeSmallIds.empty())
		{
			assignedSmallId = s_freeSmallIds.back();
			s_freeSmallIds.pop_back();
		}
		else if (s_nextSmallId < MINECRAFT_NET_MAX_PLAYERS)
		{
			assignedSmallId = s_nextSmallId++;
		}
		else
		{
			LeaveCriticalSection(&s_freeSmallIdLock);
			app.DebugPrintf("Win64 LAN: Server full, rejecting connection\n");
			closesocket(clientSocket);
			continue;
		}
		LeaveCriticalSection(&s_freeSmallIdLock);

		BYTE assignBuf[1] = { assignedSmallId };
		int sent = send(clientSocket, (const char*)assignBuf, 1, 0);
		if (sent != 1)
		{
			app.DebugPrintf("Failed to send small ID to client\n");
			closesocket(clientSocket);
			continue;
		}

		Win64RemoteConnection conn;
		conn.tcpSocket = clientSocket;
		conn.smallId = assignedSmallId;
		conn.active = true;
		conn.recvThread = NULL;

		EnterCriticalSection(&s_connectionsLock);
		s_connections.push_back(conn);
		int connIdx = (int)s_connections.size() - 1;
		LeaveCriticalSection(&s_connectionsLock);

		app.DebugPrintf("Win64 LAN: Client connected, assigned smallId=%d\n", assignedSmallId);

		IQNetPlayer* qnetPlayer = &IQNet::m_player[assignedSmallId];

		extern void Win64_SetupRemoteQNetPlayer(IQNetPlayer * player, BYTE smallId, bool isHost, bool isLocal);
		Win64_SetupRemoteQNetPlayer(qnetPlayer, assignedSmallId, false, false);

		extern CPlatformNetworkManagerStub* g_pPlatformNetworkManager;
		g_pPlatformNetworkManager->NotifyPlayerJoined(qnetPlayer);

		DWORD* threadParam = new DWORD;
		*threadParam = connIdx;
		HANDLE hThread = CreateThread(NULL, 0, RecvThreadProc, threadParam, 0, NULL);

		EnterCriticalSection(&s_connectionsLock);
		if (connIdx < (int)s_connections.size())
			s_connections[connIdx].recvThread = hThread;
		LeaveCriticalSection(&s_connectionsLock);
	}
	return 0;
}

DWORD WINAPI WinsockNetLayer::RecvThreadProc(LPVOID param)
{
	DWORD connIdx = *(DWORD*)param;
	delete (DWORD*)param;

	EnterCriticalSection(&s_connectionsLock);
	if (connIdx >= (DWORD)s_connections.size())
	{
		LeaveCriticalSection(&s_connectionsLock);
		return 0;
	}
	SOCKET sock = s_connections[connIdx].tcpSocket;
	BYTE clientSmallId = s_connections[connIdx].smallId;
	LeaveCriticalSection(&s_connectionsLock);

	std::vector<BYTE> recvBuf;
	recvBuf.resize(WIN64_NET_RECV_BUFFER_SIZE);

	while (s_active)
	{
		BYTE header[4];
		if (!RecvExact(sock, header, 4))
		{
			app.DebugPrintf("Win64 LAN: Client smallId=%d disconnected (header)\n", clientSmallId);
			break;
		}

		int packetSize =
			((uint32_t)header[0] << 24) |
			((uint32_t)header[1] << 16) |
			((uint32_t)header[2] << 8) |
			((uint32_t)header[3]);

		if (packetSize <= 0 || packetSize > WIN64_NET_MAX_PACKET_SIZE)
		{
			app.DebugPrintf("Win64 LAN: Invalid packet size %d from client smallId=%d (max=%d)\n",
				packetSize,
				clientSmallId,
				(int)WIN64_NET_MAX_PACKET_SIZE);
			break;
		}

		if ((int)recvBuf.size() < packetSize)
		{
			recvBuf.resize(packetSize);
			app.DebugPrintf("Win64 LAN: Resized host recv buffer to %d bytes for client smallId=%d\n", packetSize, clientSmallId);
		}

		if (!RecvExact(sock, &recvBuf[0], packetSize))
		{
			app.DebugPrintf("Win64 LAN: Client smallId=%d disconnected (body)\n", clientSmallId);
			break;
		}

		HandleDataReceived(clientSmallId, s_hostSmallId, &recvBuf[0], packetSize);
	}

	EnterCriticalSection(&s_connectionsLock);
	for (size_t i = 0; i < s_connections.size(); i++)
	{
		if (s_connections[i].smallId == clientSmallId)
		{
			s_connections[i].active = false;
			if (s_connections[i].tcpSocket != INVALID_SOCKET)
			{
				closesocket(s_connections[i].tcpSocket);
				s_connections[i].tcpSocket = INVALID_SOCKET;
			}
			break;
		}
	}
	LeaveCriticalSection(&s_connectionsLock);

	EnterCriticalSection(&s_disconnectLock);
	s_disconnectedSmallIds.push_back(clientSmallId);
	LeaveCriticalSection(&s_disconnectLock);

	return 0;
}

bool WinsockNetLayer::PopDisconnectedSmallId(BYTE* outSmallId)
{
	bool found = false;
	EnterCriticalSection(&s_disconnectLock);
	if (!s_disconnectedSmallIds.empty())
	{
		*outSmallId = s_disconnectedSmallIds.back();
		s_disconnectedSmallIds.pop_back();
		found = true;
	}
	LeaveCriticalSection(&s_disconnectLock);
	return found;
}

void WinsockNetLayer::PushFreeSmallId(BYTE smallId)
{
	EnterCriticalSection(&s_freeSmallIdLock);
	s_freeSmallIds.push_back(smallId);
	LeaveCriticalSection(&s_freeSmallIdLock);
}

void WinsockNetLayer::CloseConnectionBySmallId(BYTE smallId)
{
	EnterCriticalSection(&s_connectionsLock);
	for (size_t i = 0; i < s_connections.size(); i++)
	{
		if (s_connections[i].smallId == smallId && s_connections[i].active && s_connections[i].tcpSocket != INVALID_SOCKET)
		{
			closesocket(s_connections[i].tcpSocket);
			s_connections[i].tcpSocket = INVALID_SOCKET;
			app.DebugPrintf("Win64 LAN: Force-closed TCP connection for smallId=%d\n", smallId);
			break;
		}
	}
	LeaveCriticalSection(&s_connectionsLock);
}

DWORD WINAPI WinsockNetLayer::ClientRecvThreadProc(LPVOID param)
{
	std::vector<BYTE> recvBuf;
	recvBuf.resize(WIN64_NET_RECV_BUFFER_SIZE);

	while (s_active && s_hostConnectionSocket != INVALID_SOCKET)
	{
		BYTE header[4];
		if (!RecvExact(s_hostConnectionSocket, header, 4))
		{
			app.DebugPrintf("Win64 LAN: Disconnected from host (header)\n");
			break;
		}

		int packetSize = (header[0] << 24) | (header[1] << 16) | (header[2] << 8) | header[3];

		if (packetSize <= 0 || packetSize > WIN64_NET_MAX_PACKET_SIZE)
		{
			app.DebugPrintf("Win64 LAN: Invalid packet size %d from host (max=%d)\n",
				packetSize,
				(int)WIN64_NET_MAX_PACKET_SIZE);
			break;
		}

		if ((int)recvBuf.size() < packetSize)
		{
			recvBuf.resize(packetSize);
			app.DebugPrintf("Win64 LAN: Resized client recv buffer to %d bytes\n", packetSize);
		}

		if (!RecvExact(s_hostConnectionSocket, &recvBuf[0], packetSize))
		{
			app.DebugPrintf("Win64 LAN: Disconnected from host (body)\n");
			break;
		}

		HandleDataReceived(s_hostSmallId, s_localSmallId, &recvBuf[0], packetSize);
	}

	s_connected = false;
	return 0;
}

bool WinsockNetLayer::StartAdvertising(int gamePort, const wchar_t* hostName, unsigned int gameSettings, unsigned int texPackId, unsigned char subTexId, unsigned short netVer)
{
	if (s_advertising) return true;
	if (!s_initialized) return false;

	EnterCriticalSection(&s_advertiseLock);
	memset(&s_advertiseData, 0, sizeof(s_advertiseData));
	s_advertiseData.magic = WIN64_LAN_BROADCAST_MAGIC;
	s_advertiseData.netVersion = netVer;
	s_advertiseData.gamePort = (WORD)gamePort;
	wcsncpy_s(s_advertiseData.hostName, 32, hostName, _TRUNCATE);
	s_advertiseData.playerCount = 1;
	s_advertiseData.maxPlayers = MINECRAFT_NET_MAX_PLAYERS;
	s_advertiseData.gameHostSettings = gameSettings;
	s_advertiseData.texturePackParentId = texPackId;
	s_advertiseData.subTexturePackId = subTexId;
	s_advertiseData.isJoinable = 0;
	s_hostGamePort = gamePort;
	LeaveCriticalSection(&s_advertiseLock);

	s_advertiseSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s_advertiseSock == INVALID_SOCKET)
	{
		app.DebugPrintf("Win64 LAN: Failed to create advertise socket: %d\n", WSAGetLastError());
		return false;
	}

	BOOL broadcast = TRUE;
	setsockopt(s_advertiseSock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof(broadcast));

	s_advertising = true;
	s_advertiseThread = CreateThread(NULL, 0, AdvertiseThreadProc, NULL, 0, NULL);

	app.DebugPrintf("Win64 LAN: Started advertising on UDP port %d\n", WIN64_LAN_DISCOVERY_PORT);
	return true;
}

void WinsockNetLayer::StopAdvertising()
{
	s_advertising = false;

	if (s_advertiseSock != INVALID_SOCKET)
	{
		closesocket(s_advertiseSock);
		s_advertiseSock = INVALID_SOCKET;
	}

	if (s_advertiseThread != NULL)
	{
		WaitForSingleObject(s_advertiseThread, 2000);
		CloseHandle(s_advertiseThread);
		s_advertiseThread = NULL;
	}
}

void WinsockNetLayer::UpdateAdvertisePlayerCount(BYTE count)
{
	EnterCriticalSection(&s_advertiseLock);
	s_advertiseData.playerCount = count;
	LeaveCriticalSection(&s_advertiseLock);
}

void WinsockNetLayer::UpdateAdvertiseJoinable(bool joinable)
{
	EnterCriticalSection(&s_advertiseLock);
	s_advertiseData.isJoinable = joinable ? 1 : 0;
	LeaveCriticalSection(&s_advertiseLock);
}

DWORD WINAPI WinsockNetLayer::AdvertiseThreadProc(LPVOID param)
{
	struct sockaddr_in broadcastAddr;
	memset(&broadcastAddr, 0, sizeof(broadcastAddr));
	broadcastAddr.sin_family = AF_INET;
	broadcastAddr.sin_port = htons(WIN64_LAN_DISCOVERY_PORT);
	broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

	while (s_advertising)
	{
		EnterCriticalSection(&s_advertiseLock);
		Win64LANBroadcast data = s_advertiseData;
		LeaveCriticalSection(&s_advertiseLock);

		int sent = sendto(s_advertiseSock, (const char*)&data, sizeof(data), 0,
			(struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));

		if (sent == SOCKET_ERROR && s_advertising)
		{
			app.DebugPrintf("Win64 LAN: Broadcast sendto failed: %d\n", WSAGetLastError());
		}

		Sleep(1000);
	}

	return 0;
}

bool WinsockNetLayer::StartDiscovery()
{
	if (s_discovering) return true;
	if (!s_initialized) return false;

	s_discoverySock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s_discoverySock == INVALID_SOCKET)
	{
		app.DebugPrintf("Win64 LAN: Failed to create discovery socket: %d\n", WSAGetLastError());
		return false;
	}

	BOOL reuseAddr = TRUE;
	setsockopt(s_discoverySock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseAddr, sizeof(reuseAddr));

	struct sockaddr_in bindAddr;
	memset(&bindAddr, 0, sizeof(bindAddr));
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(WIN64_LAN_DISCOVERY_PORT);
	bindAddr.sin_addr.s_addr = INADDR_ANY;

	if (::bind(s_discoverySock, (struct sockaddr*)&bindAddr, sizeof(bindAddr)) == SOCKET_ERROR)
	{
		app.DebugPrintf("Win64 LAN: Discovery bind failed: %d\n", WSAGetLastError());
		closesocket(s_discoverySock);
		s_discoverySock = INVALID_SOCKET;
		return false;
	}

	DWORD timeout = 500;
	setsockopt(s_discoverySock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

	s_discovering = true;
	s_discoveryThread = CreateThread(NULL, 0, DiscoveryThreadProc, NULL, 0, NULL);

	app.DebugPrintf("Win64 LAN: Listening for LAN games on UDP port %d\n", WIN64_LAN_DISCOVERY_PORT);
	return true;
}

void WinsockNetLayer::StopDiscovery()
{
	s_discovering = false;

	if (s_discoverySock != INVALID_SOCKET)
	{
		closesocket(s_discoverySock);
		s_discoverySock = INVALID_SOCKET;
	}

	if (s_discoveryThread != NULL)
	{
		WaitForSingleObject(s_discoveryThread, 2000);
		CloseHandle(s_discoveryThread);
		s_discoveryThread = NULL;
	}

	EnterCriticalSection(&s_discoveryLock);
	s_discoveredSessions.clear();
	LeaveCriticalSection(&s_discoveryLock);
}

std::vector<Win64LANSession> WinsockNetLayer::GetDiscoveredSessions()
{
	std::vector<Win64LANSession> result;
	EnterCriticalSection(&s_discoveryLock);
	result = s_discoveredSessions;
	LeaveCriticalSection(&s_discoveryLock);
	return result;
}

DWORD WINAPI WinsockNetLayer::DiscoveryThreadProc(LPVOID param)
{
	char recvBuf[512];

	while (s_discovering)
	{
		struct sockaddr_in senderAddr;
		int senderLen = sizeof(senderAddr);

		int recvLen = recvfrom(s_discoverySock, recvBuf, sizeof(recvBuf), 0,
			(struct sockaddr*)&senderAddr, &senderLen);

		if (recvLen == SOCKET_ERROR)
		{
			continue;
		}

		if (recvLen < (int)sizeof(Win64LANBroadcast))
			continue;

		Win64LANBroadcast* broadcast = (Win64LANBroadcast*)recvBuf;
		if (broadcast->magic != WIN64_LAN_BROADCAST_MAGIC)
			continue;

		char senderIP[64];
		inet_ntop(AF_INET, &senderAddr.sin_addr, senderIP, sizeof(senderIP));

		DWORD now = GetTickCount();

		EnterCriticalSection(&s_discoveryLock);

		bool found = false;
		for (size_t i = 0; i < s_discoveredSessions.size(); i++)
		{
			if (strcmp(s_discoveredSessions[i].hostIP, senderIP) == 0 &&
				s_discoveredSessions[i].hostPort == (int)broadcast->gamePort)
			{
				s_discoveredSessions[i].netVersion = broadcast->netVersion;
				wcsncpy_s(s_discoveredSessions[i].hostName, 32, broadcast->hostName, _TRUNCATE);
				s_discoveredSessions[i].playerCount = broadcast->playerCount;
				s_discoveredSessions[i].maxPlayers = broadcast->maxPlayers;
				s_discoveredSessions[i].gameHostSettings = broadcast->gameHostSettings;
				s_discoveredSessions[i].texturePackParentId = broadcast->texturePackParentId;
				s_discoveredSessions[i].subTexturePackId = broadcast->subTexturePackId;
				s_discoveredSessions[i].isJoinable = (broadcast->isJoinable != 0);
				s_discoveredSessions[i].lastSeenTick = now;
				found = true;
				break;
			}
		}

		if (!found)
		{
			Win64LANSession session;
			memset(&session, 0, sizeof(session));
			strncpy_s(session.hostIP, sizeof(session.hostIP), senderIP, _TRUNCATE);
			session.hostPort = (int)broadcast->gamePort;
			session.netVersion = broadcast->netVersion;
			wcsncpy_s(session.hostName, 32, broadcast->hostName, _TRUNCATE);
			session.playerCount = broadcast->playerCount;
			session.maxPlayers = broadcast->maxPlayers;
			session.gameHostSettings = broadcast->gameHostSettings;
			session.texturePackParentId = broadcast->texturePackParentId;
			session.subTexturePackId = broadcast->subTexturePackId;
			session.isJoinable = (broadcast->isJoinable != 0);
			session.lastSeenTick = now;
			s_discoveredSessions.push_back(session);

			app.DebugPrintf("Win64 LAN: Discovered game \"%ls\" at %s:%d\n",
				session.hostName, session.hostIP, session.hostPort);
		}

		for (size_t i = s_discoveredSessions.size(); i > 0; i--)
		{
			if (now - s_discoveredSessions[i - 1].lastSeenTick > 5000)
			{
				app.DebugPrintf("Win64 LAN: Session \"%ls\" at %s timed out\n",
					s_discoveredSessions[i - 1].hostName, s_discoveredSessions[i - 1].hostIP);
				s_discoveredSessions.erase(s_discoveredSessions.begin() + (i - 1));
			}
		}

		LeaveCriticalSection(&s_discoveryLock);
	}

	return 0;
}

#endif