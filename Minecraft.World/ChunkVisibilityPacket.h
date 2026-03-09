#pragma once
using namespace std;

#include "Packet.h"

#include "stdafx.h"
#include <iostream>
#include "PacketListener.h"

class ChunkVisibilityPacket : public Packet, public enable_shared_from_this<ChunkVisibilityPacket>
{
public:
	int x, z;
    bool visible;

	ChunkVisibilityPacket();
	ChunkVisibilityPacket(int x, int z, bool visible);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<ChunkVisibilityPacket>(); }
	virtual int getId() { return 50; }
};


