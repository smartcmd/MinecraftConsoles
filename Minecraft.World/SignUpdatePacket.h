#pragma once
using namespace std;

#include "Packet.h"

class SignUpdatePacket : public Packet, public enable_shared_from_this<SignUpdatePacket>
{
public:
	int x, y, z;
	bool m_bVerified;
	bool m_bCensored;
    wstring lines[4];

	SignUpdatePacket();
	SignUpdatePacket(int x, int y, int z, bool bVerified, bool bCensored, wstring lines[]);
	bool GetVerified() {return m_bVerified;}
	bool GetCensored() {return m_bCensored;}
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new SignUpdatePacket()); }
	virtual int getId() { return 130; }
};