#pragma once
using namespace std;

#include "CommandsEnum.h"
#include "Packet.h"

class GameCommandPacket : public Packet, public enable_shared_from_this<GameCommandPacket>
{
public:
	EGameCommand command;
	int length;
	byteArray data;

	GameCommandPacket();
	GameCommandPacket(EGameCommand command, byteArray data);
	~GameCommandPacket();

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<GameCommandPacket>(); }
	virtual int getId() { return 167; }
};