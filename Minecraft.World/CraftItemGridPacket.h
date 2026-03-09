#pragma once

using namespace std;

#include "Packet.h"

class CraftItemGridPacket : public Packet, public enable_shared_from_this<CraftItemGridPacket>
{
public:
	int recipe;
	short uid;
	bool is2x2;
	int gridData[9];

	CraftItemGridPacket();
	~CraftItemGridPacket();
	CraftItemGridPacket(int recipe, short uid, bool is2x2, int gridData[9]);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<CraftItemGridPacket>(); }
	virtual int getId() { return 169; }
};
