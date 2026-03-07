#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "RespawnPacket.h"
#include "LevelType.h"

RespawnPacket::RespawnPacket()
{
	this->dimension = 0;
	this->difficulty = 1;
	this->mapSeed = 0;
	this->mapHeight = 0;
	this->playerGameType = NULL;
	this->m_newSeaLevel = false;
	m_pLevelType = NULL;
	m_newEntityId = 0;
	m_xzSize = LEVEL_MAX_WIDTH;
	m_hellScale = HELL_LEVEL_MAX_SCALE;
}

RespawnPacket::RespawnPacket(char dimension, int64_t mapSeed, int mapHeight, GameType *playerGameType, char difficulty, LevelType *pLevelType, bool newSeaLevel, int newEntityId, int xzSize, int hellScale)
{
	this->dimension = dimension;
	this->mapSeed = mapSeed;
	this->mapHeight = mapHeight;
	this->playerGameType = playerGameType;
	this->difficulty = difficulty;
	this->m_newSeaLevel = newSeaLevel;
	this->m_pLevelType=pLevelType;
	this->m_newEntityId = newEntityId;
	m_xzSize = xzSize;
	m_hellScale = hellScale;
	app.DebugPrintf("RespawnPacket - Difficulty = %d\n",difficulty);

}

void RespawnPacket::handle(PacketListener *listener)
{
	listener->handleRespawn(shared_from_this());
}

void RespawnPacket::read(DataInputStream *dis) //throws IOException
{
	dimension = dis->readByte();
	playerGameType = GameType::byId(dis->readByte());
	mapHeight = dis->readShort();
	wstring typeName = readUtf(dis, 16);
	m_pLevelType = LevelType::getLevelType(typeName);
	if (m_pLevelType == NULL)
	{
		m_pLevelType = LevelType::lvl_normal;
	}
	mapSeed = dis->readLong();
	difficulty = dis->readByte();
	m_newSeaLevel = dis->readBoolean();
	m_newEntityId = dis->readShort();
#ifdef _LARGE_WORLDS
	m_xzSize = dis->readShort();
	m_hellScale = dis->read();
#endif
	app.DebugPrintf("RespawnPacket::read - Difficulty = %d\n",difficulty);

}

void RespawnPacket::write(DataOutputStream *dos) //throws IOException
{
	dos->writeByte(dimension);
	dos->writeByte(playerGameType->getId());
	dos->writeShort(mapHeight);
	if (m_pLevelType == NULL)
	{
		writeUtf(L"", dos);
	}
	else
	{
		writeUtf(m_pLevelType->getGeneratorName(), dos);
	}
	dos->writeLong(mapSeed);
	dos->writeByte(difficulty);
	dos->writeBoolean(m_newSeaLevel);
	dos->writeShort(m_newEntityId);
#ifdef _LARGE_WORLDS
	dos->writeShort(m_xzSize);
	dos->write(m_hellScale);
#endif
}

int RespawnPacket::getEstimatedSize()
{
	int length=0;
	if (m_pLevelType != NULL)
	{
		length = (int)m_pLevelType->getGeneratorName().length();
	}
	return 13+length;
}
