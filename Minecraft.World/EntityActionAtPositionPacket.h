#pragma once
using namespace std;

#include "Packet.h"

class EntityActionAtPositionPacket : public Packet, public enable_shared_from_this<EntityActionAtPositionPacket>
{
public:
	static const int START_SLEEP;
	int id, x, y, z, action;

	EntityActionAtPositionPacket();
	EntityActionAtPositionPacket(shared_ptr<Entity> e, int action, int x, int y, int z);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<EntityActionAtPositionPacket>(); }
	virtual int getId() { return 17; }
};