#pragma once
using namespace std;

#include "Packet.h"

class ContainerClickPacket : public Packet, public enable_shared_from_this<ContainerClickPacket>
{
public:
	int containerId;
    int slotNum;
    int buttonNum;
    short uid;
    shared_ptr<ItemInstance> item;
    bool quickKey;

	ContainerClickPacket();
	~ContainerClickPacket();
    ContainerClickPacket(int containerId, int slotNum, int buttonNum, bool quickKey, shared_ptr<ItemInstance> item, short uid);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new ContainerClickPacket()); }
	virtual int getId() { return 102; }
};


