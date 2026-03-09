#pragma once
using namespace std;

#include "Packet.h"

class InteractPacket : public Packet, public enable_shared_from_this<InteractPacket>
{
public:
	static const int INTERACT;
    static const int ATTACK;

    int source, target, action;

	InteractPacket();
	InteractPacket(int source, int target, int action);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<InteractPacket>(); }
	virtual int getId() { return 7; }
};