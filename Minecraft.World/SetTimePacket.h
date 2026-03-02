#pragma once
using namespace std;

#include "Packet.h"

class SetTimePacket : public Packet, public enable_shared_from_this<SetTimePacket>
{
public:
	__int64 time;

	SetTimePacket();
	SetTimePacket(__int64 time);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	virtual bool canBeInvalidated();
	virtual bool isInvalidatedBy(shared_ptr<Packet> packet);
	virtual bool isAync();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new SetTimePacket()); }
	virtual int getId() { return 4; }
};