#pragma once

#include "Packet.h"

class ExperienceOrb;

class AddExperienceOrbPacket : public Packet, public enable_shared_from_this<AddExperienceOrbPacket>
{
public:
	int id;
	int x, y, z;
	int value;

	AddExperienceOrbPacket();
	AddExperienceOrbPacket(shared_ptr<ExperienceOrb> e);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

	static shared_ptr<Packet> create() { return std::make_shared<AddExperienceOrbPacket>(); }
	virtual int getId() { return 26; }
};