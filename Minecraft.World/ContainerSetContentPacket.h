#pragma once
using namespace std;

#include "Packet.h"

class ContainerSetContentPacket : public Packet, public enable_shared_from_this<ContainerSetContentPacket>
{
public:
	int containerId;
    ItemInstanceArray items;

	ContainerSetContentPacket();
	~ContainerSetContentPacket();
	ContainerSetContentPacket(int containerId, vector<shared_ptr<ItemInstance> > *newItems);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<ContainerSetContentPacket>(); }
	virtual int getId() { return 104; }
};


