#pragma once
using namespace std;

#include "Packet.h"

class TexturePacket : public Packet, public enable_shared_from_this<TexturePacket>
{
public:
	wstring textureName;
	PBYTE pbData;
	DWORD dwBytes;

	TexturePacket();
	~TexturePacket(); 
	TexturePacket(const wstring &textureName, PBYTE pbData, DWORD dwBytes);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new TexturePacket()); }
	virtual int getId() { return 154; }
};