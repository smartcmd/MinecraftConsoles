#pragma once
using namespace std;

#include "Packet.h"

class TextureChangePacket : public Packet, public enable_shared_from_this<TextureChangePacket>
{
public:
	enum ETextureChangeType
	{
		e_TextureChange_Skin = 0,
		e_TextureChange_Cape,
	};

    int id;
    ETextureChangeType action;
	wstring path;

	TextureChangePacket();
	TextureChangePacket(shared_ptr<Entity> e, ETextureChangeType action, const wstring &path);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<TextureChangePacket>(); }
	virtual int getId() { return 157; }
};