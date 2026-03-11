#pragma once

// 4J ADDED THIS PACKET

using namespace std;

#include "Packet.h"

class CraftItemPacket : public Packet, public enable_shared_from_this<CraftItemPacket>
{
public:
	int recipe;
	short uid;

	CraftItemPacket();
	~CraftItemPacket();
	CraftItemPacket(int recipe, short uid);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<CraftItemPacket>(); }
	virtual int getId() { return 150; }
};