#pragma once
using namespace std;

#include "Packet.h"

class Level;

class ChunkTilesUpdatePacket : public Packet, public enable_shared_from_this<ChunkTilesUpdatePacket>
{
public:
	int xc, zc;
    shortArray positions;
    byteArray blocks;
    byteArray data;
    byte count; // 4J Was int but never has a value higher than 10
	int levelIdx;

	ChunkTilesUpdatePacket();
	~ChunkTilesUpdatePacket();
	ChunkTilesUpdatePacket(int xc, int zc, shortArray positions, byte count, Level *level);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new ChunkTilesUpdatePacket()); }
	virtual int getId() { return 52; }
};

