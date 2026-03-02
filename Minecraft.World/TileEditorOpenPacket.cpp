#include "stdafx.h"

#include "PacketListener.h"
#include "TileEditorOpenPacket.h"

TileEditorOpenPacket::TileEditorOpenPacket()
{
	editorType = 0;
	x = y = z = 0;
}

TileEditorOpenPacket::TileEditorOpenPacket(int editorType, int x, int y, int z)
{
	this->editorType = editorType;
	this->x = x;
	this->y = y;
	this->z = z;
}

void TileEditorOpenPacket::handle(PacketListener *listener)
{
	listener->handleTileEditorOpen(shared_from_this());
}

void TileEditorOpenPacket::read(DataInputStream *dis)
{
	this->editorType = dis->readByte();
	this->x = dis->readInt();
	this->y = dis->readInt();
	this->z = dis->readInt();
}

void TileEditorOpenPacket::write(DataOutputStream *dos)
{
	dos->writeByte(editorType);
	dos->writeInt(x);
	dos->writeInt(y);
	dos->writeInt(z);
}

int TileEditorOpenPacket::getEstimatedSize()
{
	return 1 + 3 * 4;
}