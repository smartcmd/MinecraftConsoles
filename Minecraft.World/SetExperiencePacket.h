#pragma once

#include "Packet.h"

class SetExperiencePacket : public Packet, public enable_shared_from_this<SetExperiencePacket>
{
public:
	float experienceProgress;
	int totalExperience;
	int experienceLevel;

	SetExperiencePacket();
	SetExperiencePacket(float experienceProgress, int totalExperience, int experienceLevel);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	virtual bool canBeInvalidated();
	virtual bool isInvalidatedBy(shared_ptr<Packet> packet);
	virtual bool isAync();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new SetExperiencePacket()); }
	virtual int getId() { return 43; }
};