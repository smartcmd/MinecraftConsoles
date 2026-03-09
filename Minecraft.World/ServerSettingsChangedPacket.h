#pragma once

// 4J ADDED THIS PACKET

using namespace std;

#include "Packet.h"

class ServerSettingsChangedPacket : public Packet, public enable_shared_from_this<ServerSettingsChangedPacket>
{
public:
	static const int HOST_DIFFICULTY;
	static const int HOST_OPTIONS;
	static const int HOST_IN_GAME_SETTINGS;

	char action;
	unsigned int data;

	ServerSettingsChangedPacket();
	~ServerSettingsChangedPacket();
	ServerSettingsChangedPacket(char action, unsigned int data);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<ServerSettingsChangedPacket>(); }
	virtual int getId() { return 153; }
};