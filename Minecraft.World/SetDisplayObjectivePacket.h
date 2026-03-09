#pragma once

#include "Packet.h"

class Objective;

class SetDisplayObjectivePacket : public Packet, public enable_shared_from_this<SetDisplayObjectivePacket>
{
public:
	int slot;
	wstring objectiveName;

	SetDisplayObjectivePacket();
	SetDisplayObjectivePacket(int slot, Objective *objective);

	void read(DataInputStream *dis);
	void write(DataOutputStream *dos);
	void handle(PacketListener *listener);
	int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<SetDisplayObjectivePacket>(); }
	virtual int getId() { return 208; }
};