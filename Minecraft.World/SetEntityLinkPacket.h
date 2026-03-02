#pragma once
using namespace std;

#include "Packet.h"

class SetEntityLinkPacket : public Packet, public enable_shared_from_this<SetEntityLinkPacket>
{
public:
	static const int RIDING = 0;
	static const int LEASH = 1;

	int type;
	int sourceId, destId;

	SetEntityLinkPacket();
	SetEntityLinkPacket(int linkType, shared_ptr<Entity> sourceEntity, shared_ptr<Entity> destEntity);

	virtual int getEstimatedSize();
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(shared_ptr<Packet> packet);

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new SetEntityLinkPacket()); }
	virtual int getId() { return 39; }

};