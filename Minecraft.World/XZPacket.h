#pragma once

// 4J ADDED THIS PACKET

using namespace std;

#include "Packet.h"

class XZPacket : public Packet, public enable_shared_from_this<XZPacket>
{
public:
	static const int STRONGHOLD;

	char action;
	int x;
	int z;

	XZPacket();
	~XZPacket();
	XZPacket(char action, int x, int z);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new XZPacket()); }
	virtual int getId() { return 166; }
};