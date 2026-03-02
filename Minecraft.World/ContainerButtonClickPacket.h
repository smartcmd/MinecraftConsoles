#pragma once

#include "Packet.h"

class ContainerButtonClickPacket : public Packet, public enable_shared_from_this<ContainerButtonClickPacket>
{
public:
	int containerId;
	int buttonId;

	ContainerButtonClickPacket();
	ContainerButtonClickPacket(int containerId, int buttonId);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new ContainerButtonClickPacket()); }
	virtual int getId() { return 108; }
};