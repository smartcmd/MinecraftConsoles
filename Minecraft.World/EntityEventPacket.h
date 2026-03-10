#pragma once
using namespace std;

#include "Packet.h"

class EntityEventPacket : public Packet, public enable_shared_from_this<EntityEventPacket>
{
public:
	int entityId;
    byte eventId;

	EntityEventPacket();
	EntityEventPacket(int entityId, byte eventId);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener) ;
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<EntityEventPacket>(); }
	virtual int getId() { return 38; }
};


