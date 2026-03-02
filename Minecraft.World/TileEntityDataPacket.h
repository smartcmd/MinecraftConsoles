#pragma once
using namespace std;

#include "Packet.h"

class CompoundTag;

class TileEntityDataPacket : public Packet, public enable_shared_from_this<TileEntityDataPacket>
{
public:
	static const int TYPE_MOB_SPAWNER = 1;
	static const int TYPE_ADV_COMMAND = 2;
	static const int TYPE_BEACON = 3;
	static const int TYPE_SKULL = 4;

	int x, y, z;
	int type;
	CompoundTag *tag;

private:
	void _init();

public:
	TileEntityDataPacket();
	~TileEntityDataPacket();
	TileEntityDataPacket(int x, int y, int z, int type, CompoundTag *tag);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new TileEntityDataPacket()); }
	virtual int getId() { return 132; }
};