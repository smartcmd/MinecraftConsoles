#pragma once
using namespace std;

#include "Packet.h"

class EntityEventPacket : public Packet, public enable_shared_from_this<EntityEventPacket>
{
public:
	int entityId;
    uint8_t eventId;

	EntityEventPacket();
	EntityEventPacket(int entityId, uint8_t eventId);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener) ;
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new EntityEventPacket()); }
	virtual int getId() { return 38; }
};


