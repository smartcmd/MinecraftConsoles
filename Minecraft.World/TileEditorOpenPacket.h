#pragma once

#include "Packet.h"

class TileEditorOpenPacket : public Packet, public enable_shared_from_this<TileEditorOpenPacket>
{
public:
	static const int SIGN = 0;
	static const int COMMAND_BLOCK = 1;

	int editorType;
	int x, y, z;

	TileEditorOpenPacket();
	TileEditorOpenPacket(int editorType, int x, int y, int z);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<TileEditorOpenPacket>(); }
	virtual int getId() { return 133; }
};