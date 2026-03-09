#pragma once
using namespace std;

#include "Packet.h"

class SetEntityMotionPacket : public Packet, public enable_shared_from_this<SetEntityMotionPacket>
{
public:
	int id;
    int xa, ya, za;
	bool useBytes;		// 4J added

private:
	void _init(int id, double xd, double yd, double zd);

public:
	SetEntityMotionPacket();
	SetEntityMotionPacket(shared_ptr<Entity> e);
	SetEntityMotionPacket(int id, double xd, double yd, double zd);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(shared_ptr<Packet> packet);

public:
	static shared_ptr<Packet> create() { return std::make_shared<SetEntityMotionPacket>(); }
	virtual int getId() { return 28; }
};