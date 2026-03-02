#pragma once
using namespace std;

#include "Packet.h"

class LevelType;
class GameType;

class RespawnPacket : public Packet, public enable_shared_from_this<RespawnPacket>
{
public:
	char dimension;
	char difficulty;
	__int64 mapSeed;
    int mapHeight;
    GameType *playerGameType;
	bool m_newSeaLevel;	// 4J added
	LevelType *m_pLevelType;
	int m_newEntityId;
	int m_xzSize; // 4J Added
	int m_hellScale; // 4J Added

	RespawnPacket();
	RespawnPacket(char dimension, __int64 mapSeed, int mapHeight, GameType *playerGameType, char difficulty, LevelType *pLevelType, bool newSeaLevel, int newEntityId, int xzSize, int hellScale);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new RespawnPacket()); }
	virtual int getId() { return 9; }
};
