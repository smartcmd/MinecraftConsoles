#pragma once
using namespace std;

#include "Packet.h"

class SetHealthPacket : public Packet, public enable_shared_from_this<SetHealthPacket>
{
public:
	int health;
	int food;
	float saturation;
	//    public float exhaustion; // 4J - Original comment

	ETelemetryChallenges damageSource; // 4J Added

	SetHealthPacket();
	SetHealthPacket(int health, int food, float saturation, ETelemetryChallenges damageSource);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	virtual bool canBeInvalidated();
	virtual bool isInvalidatedBy(shared_ptr<Packet> packet);

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new SetHealthPacket()); }
	virtual int getId() { return 8; }
};

