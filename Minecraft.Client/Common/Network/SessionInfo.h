#pragma once

#if defined(__PS3__) || defined(__ORBIS__)
#include "..\..\Common\Network\Sony\SQRNetworkManager.h"
#endif


// A struct that we store in the QoS data when we are hosting the session. Max size 1020 bytes.
#ifdef _XBOX
typedef struct _GameSessionData
{
	unsigned short netVersion;										//   2 bytes
	char hostName[XUSER_NAME_SIZE];									//  16 bytes ( 16*1 )
	GameSessionUID hostPlayerUID;										//   8 bytes ( 8*1 ) on xbox, 24 bytes on PS3
	GameSessionUID players[MINECRAFT_NET_MAX_PLAYERS];					//  64 bytes ( 8*8 ) on xbox, 192 ( 24*8) on PS3
	char szPlayers[MINECRAFT_NET_MAX_PLAYERS][XUSER_NAME_SIZE];		// 128 bytes ( 8*16)
	unsigned int m_uiGameHostSettings;								//   4 bytes
	unsigned int texturePackParentId;										//   4 bytes
	unsigned char subTexturePackId;									//   1 byte

	bool isJoinable;												//   1 byte

	_GameSessionData()
	{
		netVersion = 0;
		memset(hostName, 0, XUSER_NAME_SIZE);
		memset(players, 0, MINECRAFT_NET_MAX_PLAYERS * sizeof(players[0]));
		memset(szPlayers, 0, MINECRAFT_NET_MAX_PLAYERS * XUSER_NAME_SIZE);
		isJoinable = true;
		m_uiGameHostSettings = 0;
		texturePackParentId = 0;
		subTexturePackId = 0;
	}
} GameSessionData;
#elif defined __PS3__ || defined __ORBIS__ || defined(__PSVITA__)
typedef struct _GameSessionData
{
	unsigned short netVersion;										//   2 bytes
	GameSessionUID hostPlayerUID;										//   8 bytes ( 8*1 ) on xbox, 24 bytes on PS3
	GameSessionUID players[MINECRAFT_NET_MAX_PLAYERS];					//  64 bytes ( 8*8 ) on xbox, 192 ( 24*8) on PS3
	unsigned int m_uiGameHostSettings;								//   4 bytes
	unsigned int texturePackParentId;										//   4 bytes
	unsigned char subTexturePackId;									//   1 byte

	bool isJoinable;												//   1 byte

	unsigned char playerCount;										//   1 byte
	bool isReadyToJoin;												//   1 byte

	_GameSessionData()
	{
		netVersion = 0;
		memset(players, 0, MINECRAFT_NET_MAX_PLAYERS * sizeof(players[0]));
		isJoinable = true;
		m_uiGameHostSettings = 0;
		texturePackParentId = 0;
		subTexturePackId = 0;
		playerCount = 0;
		isReadyToJoin = false;

	}
} GameSessionData;
#else
typedef struct _GameSessionData
{
	unsigned short netVersion;
	unsigned int m_uiGameHostSettings;
	unsigned int texturePackParentId;
	unsigned char subTexturePackId;

	bool isReadyToJoin;
	bool isJoinable;

	char hostIP[64];
	int hostPort;
	wchar_t hostName[XUSER_NAME_SIZE];
	unsigned char playerCount;
	unsigned char maxPlayers;

	_GameSessionData()
	{
		netVersion = 0;
		m_uiGameHostSettings = 0;
		texturePackParentId = 0;
		subTexturePackId = 0;
		isReadyToJoin = false;
		isJoinable = true;
		memset(hostIP, 0, sizeof(hostIP));
		hostPort = 0;
		memset(hostName, 0, sizeof(hostName));
		playerCount = 0;
		maxPlayers = MINECRAFT_NET_MAX_PLAYERS;
	}
} GameSessionData;
#endif

class FriendSessionInfo
{
public:
	SessionID sessionId;
#ifdef _XBOX
	XSESSION_SEARCHRESULT searchResult;
#elif defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__)
	SQRNetworkManager::SessionSearchResult searchResult;
#elif defined(_DURANGO)
	DQRNetworkManager::SessionSearchResult searchResult;
#endif
	wchar_t* displayLabel;
	unsigned char displayLabelLength;
	unsigned char displayLabelViewableStartIndex;
	GameSessionData data;
	bool hasPartyMember;

	FriendSessionInfo()
	{
		displayLabel = nullptr;
		displayLabelLength = 0;
		displayLabelViewableStartIndex = 0;
		hasPartyMember = false;
	}

	FriendSessionInfo(const FriendSessionInfo& other)
	{
		sessionId = other.sessionId;
#ifdef _XBOX
		searchResult = other.searchResult;
#elif defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__)
		searchResult = other.searchResult;
#elif defined(_DURANGO)
		searchResult = other.searchResult;
#endif
		displayLabelLength = other.displayLabelLength;
		displayLabelViewableStartIndex = other.displayLabelViewableStartIndex;
		data = other.data;
		hasPartyMember = other.hasPartyMember;
		if (other.displayLabel != NULL)
		{
			displayLabel = new wchar_t[displayLabelLength + 1];
			wcscpy_s(displayLabel, displayLabelLength + 1, other.displayLabel);
		}
		else
		{
			displayLabel = NULL;
		}
	}

	FriendSessionInfo& operator=(const FriendSessionInfo&) = delete;

	~FriendSessionInfo()
	{
		if (displayLabel != nullptr)
			delete[] displayLabel;
	}
};
