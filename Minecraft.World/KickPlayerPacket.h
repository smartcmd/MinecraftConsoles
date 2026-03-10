#pragma once
using namespace std;

#include "Packet.h"

class KickPlayerPacket : public Packet, public enable_shared_from_this<KickPlayerPacket>
{
public:
	BYTE m_networkSmallId;

	KickPlayerPacket();
	KickPlayerPacket(BYTE networkSmallId);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<KickPlayerPacket>(); }
	virtual int getId() { return 159; }
};