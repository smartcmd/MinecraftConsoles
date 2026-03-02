#pragma once
using namespace std;

#include "Packet.h"

class SetRidingPacket : public Packet, public enable_shared_from_this<SetRidingPacket>
{
public:
	int riderId, riddenId;

	SetRidingPacket();
	SetRidingPacket(shared_ptr<Entity> rider, shared_ptr<Entity> riding);

	virtual int getEstimatedSize();
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(shared_ptr<Packet> packet);

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new SetRidingPacket()); }
	virtual int getId() { return 39; }

};