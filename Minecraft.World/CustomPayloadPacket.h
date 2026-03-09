#pragma once
using namespace std;

#include "Packet.h"

class CustomPayloadPacket : public Packet, public enable_shared_from_this<CustomPayloadPacket>
{
public:

	// Mojang-defined custom packets
	static const wstring CUSTOM_BOOK_PACKET;
	static const wstring CUSTOM_BOOK_SIGN_PACKET;
	static const wstring TEXTURE_PACK_PACKET;
	static const wstring TRADER_LIST_PACKET;
	static const wstring TRADER_SELECTION_PACKET;
	static const wstring SET_ADVENTURE_COMMAND_PACKET;
	static const wstring SET_BEACON_PACKET;
	static const wstring SET_ITEM_NAME_PACKET;

	wstring identifier;
	int length;
	byteArray data;

	CustomPayloadPacket();
	CustomPayloadPacket(const wstring &identifier, byteArray data);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<CustomPayloadPacket>(); }
	virtual int getId() { return 250; }
};