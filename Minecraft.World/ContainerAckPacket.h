#pragma once
using namespace std;

#include "Packet.h"

#include "stdafx.h"
#include <iostream>
#include "PacketListener.h"

class ContainerAckPacket : public Packet, public enable_shared_from_this<ContainerAckPacket>
{
public:
	int containerId;
    short uid;
    bool accepted;

	ContainerAckPacket();
	ContainerAckPacket(int containerId, short uid, bool accepted);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<ContainerAckPacket>(); }
	virtual int getId() { return 106; }
};


