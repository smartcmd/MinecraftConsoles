#pragma once

#include "Packet.h"

class PlayerTeam;

class SetPlayerTeamPacket : public Packet , public enable_shared_from_this<SetPlayerTeamPacket>
{
public:
	static const int METHOD_ADD = 0;
	static const int METHOD_REMOVE = 1;
	static const int METHOD_CHANGE = 2;
	static const int METHOD_JOIN = 3;
	static const int METHOD_LEAVE = 4;

	wstring name;
	wstring displayName;
	wstring prefix;
	wstring suffix;
	vector<wstring> players;
	int method;
	int options;

	SetPlayerTeamPacket();
	SetPlayerTeamPacket(PlayerTeam *team, int method);
	SetPlayerTeamPacket(PlayerTeam *team, vector<wstring> *players, int method);
	void read(DataInputStream *dis);
	void write(DataOutputStream *dos);
	void handle(PacketListener *listener);
	int getEstimatedSize();
	
public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new SetPlayerTeamPacket()); }
	virtual int getId() { return 209; }
};