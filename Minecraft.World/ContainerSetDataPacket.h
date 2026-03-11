#pragma once
using namespace std;

#include "Packet.h"

class ContainerSetDataPacket : public Packet, public enable_shared_from_this<ContainerSetDataPacket>
{
public:
	int containerId;
    int id;
    int value;

	ContainerSetDataPacket();
	ContainerSetDataPacket(int containerId, int id, int value);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<ContainerSetDataPacket>(); }
	virtual int getId() { return 105; }
};