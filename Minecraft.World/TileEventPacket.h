#pragma once
using namespace std;

#include "Packet.h"

class TileEventPacket : public Packet, public enable_shared_from_this<TileEventPacket>
{
public:
	int x, y, z, b0, b1, tile;

	TileEventPacket();
	TileEventPacket(int x, int y, int z, int tile, int b0, int b1);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new TileEventPacket()); }
	virtual int getId() { return 54; }
};