#pragma once
using namespace std;

#include "Packet.h"

class AnimatePacket : public Packet, public enable_shared_from_this<AnimatePacket>
{
public:
	static const int SWING = 1;
	static const int HURT = 2;
	static const int WAKE_UP = 3;
	static const int RESPAWN = 4;	
	static const int EAT = 5; // 1.8.2
	static const int CRITICAL_HIT = 6;
	static const int MAGIC_CRITICAL_HIT = 7;

	int id;
	int action;

	AnimatePacket();
	AnimatePacket(shared_ptr<Entity> e, int action);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	
public:
	static shared_ptr<Packet> create() { return std::make_shared<AnimatePacket>(); }
	virtual int getId() { return 18; }
};