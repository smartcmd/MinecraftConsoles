#pragma once

#include "Packet.h"

class Objective;

class SetObjectivePacket : public Packet, public enable_shared_from_this<SetObjectivePacket>
{
public:
	static const int METHOD_ADD = 0;
	static const int METHOD_REMOVE = 1;
	static const int METHOD_CHANGE = 2;

	wstring objectiveName;
	wstring displayName;
	int method;

	SetObjectivePacket();
	SetObjectivePacket(Objective *objective, int method);
	void read(DataInputStream *dis);
	void write(DataOutputStream *dos);
	void handle(PacketListener *listener);
	int getEstimatedSize();
	
public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new SetObjectivePacket()); }
	virtual int getId() { return 206; }
};