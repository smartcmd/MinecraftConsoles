#pragma once

// 4J ADDED THIS PACKET

using namespace std;

#include "Packet.h"

class DebugOptionsPacket : public Packet, public enable_shared_from_this<DebugOptionsPacket>
{
public:
	unsigned int m_uiVal;

	DebugOptionsPacket();
	~DebugOptionsPacket();
	DebugOptionsPacket(unsigned int uiVal);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new DebugOptionsPacket()); }
	virtual int getId() { return 152; }
};
