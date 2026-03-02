#pragma once

#include "InputOutputStream.h"
#include "PacketListener.h"
#include "Packet.h"

class RotateHeadPacket : public Packet, public enable_shared_from_this<RotateHeadPacket>
{
public:
	int id;
	char yHeadRot;

	RotateHeadPacket();
	RotateHeadPacket(int id, char yHeadRot);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	virtual bool canBeInvalidated();
	virtual bool isInvalidatedBy(shared_ptr<Packet> packet);
	virtual bool isAync();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new RotateHeadPacket()); }
	virtual int getId() { return 35; }
};