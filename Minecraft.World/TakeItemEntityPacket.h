#pragma once
using namespace std;

#include "Packet.h"

class TakeItemEntityPacket : public Packet, public enable_shared_from_this<TakeItemEntityPacket>
{
public:
	int itemId, playerId;

	TakeItemEntityPacket();
	TakeItemEntityPacket(int itemId, int playerId);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<TakeItemEntityPacket>(); }
	virtual int getId() { return 22; }
};