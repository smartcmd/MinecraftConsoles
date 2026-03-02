#pragma once
using namespace std;

#include "Packet.h"
class LevelType;

class LoginPacket : public Packet, public enable_shared_from_this<LoginPacket>
{
public:
	int clientVersion;
	wstring userName;
	__int64 seed;
	char dimension;
	PlayerUID m_offlineXuid, m_onlineXuid;			// 4J Added
	char difficulty;	// 4J Added	
	bool m_friendsOnlyUGC; // 4J Added
	DWORD m_ugcPlayersVersion; // 4J Added
	INT m_multiplayerInstanceId; //4J Added for sentient
	BYTE m_playerIndex; // 4J Added
	DWORD m_playerSkinId, m_playerCapeId; // 4J Added
	bool m_isGuest; // 4J Added
	bool m_newSeaLevel; // 4J Added
	LevelType *m_pLevelType;
	unsigned int m_uiGamePrivileges;
	int m_xzSize; // 4J Added
	int m_hellScale; // 4J Added

	// 1.8.2
	int gameType;
	BYTE mapHeight;
	BYTE maxPlayers;

	LoginPacket();
	LoginPacket(const wstring& userName, int clientVersion, LevelType *pLevelType, __int64 seed, int gameType, char dimension, BYTE mapHeight, BYTE maxPlayers, char difficulty, INT m_multiplayerInstanceId, BYTE playerIndex, bool newSeaLevel, unsigned int uiGamePrivileges, int xzSize, int hellScale); // Server -> Client
	LoginPacket(const wstring& userName, int clientVersion, PlayerUID offlineXuid, PlayerUID onlineXuid, bool friendsOnlyUGC, DWORD ugcPlayersVersion, DWORD skinId, DWORD capeId, bool isGuest); // Client -> Server

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new LoginPacket()); }
	virtual int getId() { return 1; }
};
