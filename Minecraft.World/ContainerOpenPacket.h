#pragma once
using namespace std;

#include "Packet.h"

class ContainerOpenPacket : public Packet, public enable_shared_from_this<ContainerOpenPacket>
{
public:
	static const int CONTAINER = 0;
	static const int WORKBENCH = 1;
	static const int FURNACE = 2;
	static const int TRAP = 3;
	static const int ENCHANTMENT = 4;
	static const int BREWING_STAND = 5;
	static const int TRADER_NPC = 6;
	static const int BEACON = 7;
	static const int REPAIR_TABLE = 8;

	int containerId;
	int type;
	int title; // 4J Stu - Changed from string
	int size;

	ContainerOpenPacket();
	ContainerOpenPacket(int containerId, int type, int title, int size);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new ContainerOpenPacket()); }
	virtual int getId() { return 100; }
};


