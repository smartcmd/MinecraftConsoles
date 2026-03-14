#pragma once
using namespace std;

#include "Packet.h"

class ContainerClosePacket : public Packet, public enable_shared_from_this<ContainerClosePacket>
{
public:
	int containerId;

	ContainerClosePacket();
	ContainerClosePacket(int containerId);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<ContainerClosePacket>(); }
	virtual int getId() { return 101; }
};


