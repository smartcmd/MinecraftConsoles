#pragma once
using namespace std;

#include "Packet.h"

class TeleportEntityPacket : public Packet, public enable_shared_from_this<TeleportEntityPacket>
{
public:
	int id;
    int x, y, z;
    uint8_t yRot, xRot;

	TeleportEntityPacket();
	TeleportEntityPacket(shared_ptr<Entity> e);
	TeleportEntityPacket(int id, int x, int y, int z, uint8_t yRot, uint8_t xRot);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(shared_ptr<Packet> packet);

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new TeleportEntityPacket()); }
	virtual int getId() { return 34; }
};