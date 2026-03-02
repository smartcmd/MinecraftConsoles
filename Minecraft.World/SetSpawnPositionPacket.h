#pragma once
using namespace std;

#include "Packet.h"

class SetSpawnPositionPacket : public Packet, public enable_shared_from_this<SetSpawnPositionPacket>
{
public:
	int x, y, z;

	SetSpawnPositionPacket();
	SetSpawnPositionPacket(int x, int y, int z);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	virtual bool canBeInvalidated();
	virtual bool isInvalidatedBy(shared_ptr<Packet> packet);
	virtual bool isAync();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new SetSpawnPositionPacket()); }
	virtual int getId() { return 6; }
};