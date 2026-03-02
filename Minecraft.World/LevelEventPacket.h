#pragma once
using namespace std;

#include "Packet.h"

class LevelEventPacket : public Packet, public enable_shared_from_this<LevelEventPacket>
{
public:
	int type;
    int data;
    int x, y, z;

	LevelEventPacket();
	LevelEventPacket(int type, int x, int y, int z, int data);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new LevelEventPacket()); }
	virtual int getId() { return 61; }
};