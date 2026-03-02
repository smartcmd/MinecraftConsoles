#pragma once

#include "Packet.h"

class ClientCommandPacket : public Packet, public enable_shared_from_this<ClientCommandPacket>
{
public:
	static const int LOGIN_COMPLETE = 0;
	static const int PERFORM_RESPAWN = 1;

	int action;

	ClientCommandPacket();
	ClientCommandPacket(int action);

	void read(DataInputStream *dis);
	void write(DataOutputStream *dos);
	void handle(PacketListener *listener);
	int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new ClientCommandPacket()); }
	virtual int getId() { return 205; }
};