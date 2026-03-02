#pragma once
using namespace std;

#include "Packet.h"

class ComplexItemDataPacket : public Packet, public enable_shared_from_this<ComplexItemDataPacket>
{
public:
	short itemType;
    short itemId;
    charArray data;

	ComplexItemDataPacket();
	~ComplexItemDataPacket();
	ComplexItemDataPacket(short itemType, short itemId, charArray data);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new ComplexItemDataPacket()); }
	virtual int getId() { return 131; }
};


