#pragma once

#include "Packet.h"

class TileDestructionPacket : public Packet, public enable_shared_from_this<TileDestructionPacket>
{
private:
	int id;
	int x;
	int y;
	int z;
	int state;

public:
	TileDestructionPacket();
	TileDestructionPacket(int id, int x, int y, int z, int state);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

	int getEntityId();
	int getX();
	int getY();
	int getZ();
	int getState();

	virtual bool canBeInvalidated();
	virtual bool isInvalidatedBy(shared_ptr<Packet> packet);

public:
	static shared_ptr<Packet> create() { return std::make_shared<TileDestructionPacket>(); }
	virtual int getId() { return 55; }
};