#pragma once
using namespace std;

#include "Packet.h"

class LevelEventPacket : public Packet, public enable_shared_from_this<LevelEventPacket>
{
public:
	int type;
    int data;
    int x, y, z;
	bool globalEvent;

	LevelEventPacket();
	LevelEventPacket(int type, int x, int y, int z, int data, bool globalEvent);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	bool isGlobalEvent();

public:
	static shared_ptr<Packet> create() { return std::make_shared<LevelEventPacket>(); }
	virtual int getId() { return 61; }
};