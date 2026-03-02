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
	static const int HOPPER = 9;
	static const int DROPPER = 10;
	static const int HORSE = 11;
	static const int FIREWORKS = 12; // 4J Added
	static const int BONUS_CHEST = 13; // 4J Added
	static const int LARGE_CHEST = 14; // 4J Added
	static const int ENDER_CHEST = 15; // 4J Added
	static const int MINECART_CHEST = 16; // 4J Added
	static const int MINECART_HOPPER = 17; // 4J Added

	int containerId;
	int type;
	int size;
	bool customName;
	wstring title;
	int entityId;

private:
	void _init(int containerId, int type, const wstring &title, int size, bool customName, int entityId);

public:
	ContainerOpenPacket();
	ContainerOpenPacket(int containerId, int type, const wstring &title, int size, bool customName);
	ContainerOpenPacket(int containerId, int type, const wstring &title, int size, bool customName, int entityId);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new ContainerOpenPacket()); }
	virtual int getId() { return 100; }
};


