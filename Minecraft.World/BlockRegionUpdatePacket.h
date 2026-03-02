#pragma once
using namespace std;

#include "Packet.h"

class Level;

class BlockRegionUpdatePacket : public Packet, public enable_shared_from_this<BlockRegionUpdatePacket>
{
public:
    int x, y, z;
    int xs, ys, zs;
    byteArray buffer;
	int levelIdx;
	bool bIsFullChunk; // 4J Added

private:
	int size;

public:
	BlockRegionUpdatePacket();
	~BlockRegionUpdatePacket();
	BlockRegionUpdatePacket(int x, int y, int z, int xs, int ys, int zs, Level *level);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new BlockRegionUpdatePacket()); }
	virtual int getId() { return 51; }
};
