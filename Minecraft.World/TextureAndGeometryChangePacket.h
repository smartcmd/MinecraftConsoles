#pragma once
using namespace std;

#include "Packet.h"

class TextureAndGeometryChangePacket : public Packet, public enable_shared_from_this<TextureAndGeometryChangePacket>
{
public:

    int id;
	wstring path;
	DWORD dwSkinID;

	TextureAndGeometryChangePacket();
	TextureAndGeometryChangePacket(shared_ptr<Entity> e, const wstring &path);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new TextureAndGeometryChangePacket()); }
	virtual int getId() { return 161; }
};