#pragma once
using namespace std;

#include "Packet.h"

class PlayerCommandPacket : public Packet, public enable_shared_from_this<PlayerCommandPacket>
{
public:
	static const int START_SNEAKING;
	static const int STOP_SNEAKING;
	static const int STOP_SLEEPING;
	static const int START_SPRINTING;
	static const int STOP_SPRINTING;
	static const int START_IDLEANIM;
	static const int STOP_IDLEANIM;
	static const int RIDING_JUMP;
	static const int OPEN_INVENTORY;

	// 4J Added
	// 4J-PB - Making this host only setting
	/*
	static const int SHOW_ON_MAPS;
	static const int HIDE_ON_MAPS;
	*/

	int id;
	int action;
	int data;

	PlayerCommandPacket();
	PlayerCommandPacket(shared_ptr<Entity> e, int action);
	PlayerCommandPacket(shared_ptr<Entity> e, int action, int data);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new PlayerCommandPacket()); }
	virtual int getId() { return 19; }
};