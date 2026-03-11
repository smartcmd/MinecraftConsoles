#pragma once
using namespace std;

#include "Packet.h"

class ContainerSetSlotPacket : public Packet, public enable_shared_from_this<ContainerSetSlotPacket>
{
public:
	static const int CONTAINER;
    static const int WORKBENCH;
    static const int FURNACE;

    int containerId;
    int slot;
    shared_ptr<ItemInstance> item;

	ContainerSetSlotPacket();
	ContainerSetSlotPacket(int containerId, int slot, shared_ptr<ItemInstance> item);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<ContainerSetSlotPacket>(); }
	virtual int getId() { return 103; }
};


