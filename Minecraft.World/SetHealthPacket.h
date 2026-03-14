#pragma once
using namespace std;

#include "Packet.h"

class SetHealthPacket : public Packet, public enable_shared_from_this<SetHealthPacket>
{
public:
	float health;
	int food;
	float saturation;

	ETelemetryChallenges damageSource; // 4J Added

	SetHealthPacket();
	SetHealthPacket(float health, int food, float saturation, ETelemetryChallenges damageSource);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	virtual bool canBeInvalidated();
	virtual bool isInvalidatedBy(shared_ptr<Packet> packet);

public:
	static shared_ptr<Packet> create() { return std::make_shared<SetHealthPacket>(); }
	virtual int getId() { return 8; }
};

