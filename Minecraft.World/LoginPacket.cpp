#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.entity.player.h"
#include "PacketListener.h"
#include "LoginPacket.h"
#include "LevelType.h"



LoginPacket::LoginPacket()
{
	this->userName = L"";
	this->clientVersion = 0;
	this->seed = 0;
	this->dimension = 0;
	this->gameType = 0;
	this->mapHeight = 0;
	this->maxPlayers = 0;

	this->difficulty = 1;

	this->m_offlineXuid = INVALID_XUID;
	this->m_onlineXuid = INVALID_XUID;
	m_friendsOnlyUGC = false;
	m_ugcPlayersVersion = 0;
	m_multiplayerInstanceId = 0;
	m_playerIndex = 0;
	m_playerSkinId = 0;
	m_playerCapeId = 0;
	m_isGuest = false;
	m_newSeaLevel = false;
	m_pLevelType = nullptr;
	m_uiGamePrivileges = 0;
	m_xzSize = LEVEL_MAX_WIDTH;
	m_hellScale = HELL_LEVEL_MAX_SCALE;
}

// Client -> Server
LoginPacket::LoginPacket(const wstring& userName, int clientVersion, PlayerUID offlineXuid, PlayerUID onlineXuid,  bool friendsOnlyUGC, DWORD ugcPlayersVersion, DWORD skinId, DWORD capeId, bool isGuest)
{
	this->userName = userName;
	this->clientVersion = clientVersion;
	this->seed = 0;
	this->dimension = 0;
	this->gameType = 0;
	this->mapHeight = 0;
	this->maxPlayers = 0;

	this->difficulty = 1;

	this->m_offlineXuid = offlineXuid;
	this->m_onlineXuid = onlineXuid;
	m_friendsOnlyUGC = friendsOnlyUGC;
	m_ugcPlayersVersion = ugcPlayersVersion;
	m_multiplayerInstanceId = 0;
	m_playerIndex = 0;
	m_playerSkinId = skinId;
	m_playerCapeId = capeId;
	m_isGuest = isGuest;
	m_newSeaLevel = false;
	m_pLevelType = nullptr;
	m_uiGamePrivileges = 0;
	m_xzSize = LEVEL_MAX_WIDTH;
	m_hellScale = HELL_LEVEL_MAX_SCALE;
}

// Server -> Client
LoginPacket::LoginPacket(const wstring& userName, int clientVersion, LevelType *pLevelType, int64_t seed, int gameType, char dimension, BYTE mapHeight, BYTE maxPlayers, char difficulty, INT multiplayerInstanceId, BYTE playerIndex, bool newSeaLevel, unsigned int uiGamePrivileges, int xzSize, int hellScale)
{
	this->userName = userName;
	this->clientVersion = clientVersion;
	this->seed = seed;
	this->dimension = dimension;
	this->gameType = gameType;
	this->mapHeight = mapHeight;
	this->maxPlayers = maxPlayers;

	this->difficulty = difficulty;

	this->m_offlineXuid = INVALID_XUID;
	this->m_onlineXuid = INVALID_XUID;
	m_friendsOnlyUGC = false;
	m_ugcPlayersVersion = 0;
	m_multiplayerInstanceId = multiplayerInstanceId;
	this->m_playerIndex = playerIndex;
	m_playerSkinId = 0;
	m_playerCapeId = 0;
	m_isGuest = false;
	m_newSeaLevel = newSeaLevel;
	this->m_pLevelType=pLevelType;
	m_uiGamePrivileges = uiGamePrivileges;
	m_xzSize = xzSize;
	m_hellScale = hellScale;
}

void LoginPacket::read(DataInputStream *dis) //throws IOException
{
	clientVersion = dis->readInt();
	userName = readUtf(dis, Player::MAX_NAME_LENGTH);
	wstring typeName = readUtf(dis, 16);
	m_pLevelType = LevelType::getLevelType(typeName);
	if (m_pLevelType == nullptr)
	{
		m_pLevelType = LevelType::lvl_normal;
	}
	seed = dis->readLong();
	gameType = dis->readInt();
	dimension = dis->readByte();
	mapHeight = dis->readByte();
	maxPlayers = dis->readByte();
	m_offlineXuid = dis->readPlayerUID();
	m_onlineXuid = dis->readPlayerUID();
	m_friendsOnlyUGC = dis->readBoolean();
	m_ugcPlayersVersion = dis->readInt();
	difficulty = dis->readByte();
	m_multiplayerInstanceId = dis->readInt();
	m_playerIndex = dis->readByte();
	INT skinId = dis->readInt();
	m_playerSkinId = *(DWORD *)&skinId;
	INT capeId = dis->readInt();
	m_playerCapeId = *(DWORD *)&capeId;
	m_isGuest = dis->readBoolean();
	m_newSeaLevel = dis->readBoolean();
	m_uiGamePrivileges = dis->readInt();
#ifdef _LARGE_WORLDS
	m_xzSize = dis->readShort();
	m_hellScale = dis->read();
#endif
	app.DebugPrintf("LoginPacket::read - Difficulty = %d\n",difficulty);

}

void LoginPacket::write(DataOutputStream *dos) //throws IOException
{
	dos->writeInt(clientVersion);
	writeUtf(userName, dos);
	if (m_pLevelType == nullptr)
	{
		writeUtf(L"", dos);
	}
	else
	{
		writeUtf(m_pLevelType->getGeneratorName(), dos);
	}
	dos->writeLong(seed);
	dos->writeInt(gameType);
	dos->writeByte(dimension);
	dos->writeByte(mapHeight);
	dos->writeByte(maxPlayers);
	dos->writePlayerUID(m_offlineXuid);
	dos->writePlayerUID(m_onlineXuid);
	dos->writeBoolean(m_friendsOnlyUGC);
	dos->writeInt(m_ugcPlayersVersion);
	dos->writeByte(difficulty);
	dos->writeInt(m_multiplayerInstanceId);
	dos->writeByte(m_playerIndex);
	dos->writeInt(m_playerSkinId);
	dos->writeInt(m_playerCapeId);
	dos->writeBoolean(m_isGuest);
	dos->writeBoolean(m_newSeaLevel);
	dos->writeInt(m_uiGamePrivileges);
#ifdef _LARGE_WORLDS
	dos->writeShort(m_xzSize);
	dos->write(m_hellScale);
#endif
}

void LoginPacket::handle(PacketListener *listener)
{
	listener->handleLogin(shared_from_this());
}

int LoginPacket::getEstimatedSize()
{
	int length=0;
	if (m_pLevelType != nullptr)
	{
		length = static_cast<int>(m_pLevelType->getGeneratorName().length());
	}

	return static_cast<int>(sizeof(int) + userName.length() + 4 + 6 + sizeof(int64_t) + sizeof(char) + sizeof(int) + (2 * sizeof(PlayerUID)) + 1 + sizeof(char) + sizeof(BYTE) + sizeof(bool) + sizeof(bool) + length + sizeof(unsigned int));
}
