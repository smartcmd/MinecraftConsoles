#pragma once
using namespace std;

#include "Packet.h"

class Painting;

class AddPaintingPacket : public Packet, public enable_shared_from_this<AddPaintingPacket>
{
public:
	int id;
    int x, y, z;
    int dir;
    wstring motive;

public:
	AddPaintingPacket();
	AddPaintingPacket(shared_ptr<Painting> e);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new AddPaintingPacket()); }
	virtual int getId() { return 25; }
};
