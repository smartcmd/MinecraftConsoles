#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "TileEventPacket.h"
#include "net.minecraft.world.level.tile.h"

TileEventPacket::TileEventPacket() 
{
	x = 0;
	y = 0;
	z = 0;
	b0 = 0;
	b1 = 0;
	tile = 0;
}

TileEventPacket::TileEventPacket(int x, int y, int z, int tile, int b0, int b1)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->b0 = b0;
	this->b1 = b1;
	this->tile = tile;
}

void TileEventPacket::read(DataInputStream *dis) //throws IOException 
{
	x = dis->readInt();
	y = dis->readShort();
	z = dis->readInt();
	b0 = dis->readUnsignedByte();
	b1 = dis->readUnsignedByte();
	tile = dis->readShort() & Tile::TILE_NUM_MASK;
}

void TileEventPacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeInt(x);
	dos->writeShort(y);
	dos->writeInt(z);
	dos->write(b0);
	dos->write(b1);
	dos->writeShort(tile & Tile::TILE_NUM_MASK);
}

void TileEventPacket::handle(PacketListener *listener) 
{
	listener->handleTileEvent(shared_from_this());
}

int TileEventPacket::getEstimatedSize() 
{
	return 2 * 4 + 2 + 2 + 2;
}
