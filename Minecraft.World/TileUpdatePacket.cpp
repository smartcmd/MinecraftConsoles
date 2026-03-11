#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.level.h"
#include "PacketListener.h"
#include "TileUpdatePacket.h"
#include "Dimension.h"



TileUpdatePacket::TileUpdatePacket() 
{
	shouldDelay = true;
}

TileUpdatePacket::TileUpdatePacket(int x, int y, int z, Level *level)
{
	shouldDelay = true;
	this->x = x;
	this->y = y;
	this->z = z;
	block = level->getTile(x, y, z);
	data = level->getData(x, y, z);
	levelIdx = ( ( level->dimension->id == 0 ) ? 0 : ( (level->dimension->id == -1) ? 1 : 2 ) );
}

void TileUpdatePacket::read(DataInputStream *dis) //throws IOException 
{
#ifdef _LARGE_WORLDS
	x = dis->readInt();
	y = dis->readUnsignedByte();
	z = dis->readInt();
	
	block = static_cast<int>(dis->readShort()) & 0xffff;

	BYTE dataLevel = dis->readByte();
	data = dataLevel & 0xf;
	levelIdx = (dataLevel>>4) & 0xf;
#else
	// 4J - See comments in write for packing
	int xyzdata = dis->readInt();
	x = ( xyzdata >> 22 ) & 0x3ff;
	y = ( xyzdata >> 14 ) & 0xff;
	z = ( xyzdata >> 4 ) & 0x3ff;
	x = ( x << 22 ) >> 22;
	z = ( z << 22 ) >> 22;
	data = xyzdata & 0xf;
	block = (int)dis->readShort() & 0xffff;
	//levelIdx = ( xyzdata >> 31 ) & 1;

	// Can't pack this as it's now 2 bits
	levelIdx = (int)dis->readByte();
#endif
}

void TileUpdatePacket::write(DataOutputStream *dos) //throws IOException
{
#ifdef _LARGE_WORLDS
	dos->writeInt(x);
	dos->write(y);
	dos->writeInt(z);
	dos->writeShort(block);

	BYTE dataLevel = ((levelIdx & 0xf ) << 4) | (data & 0xf);
	dos->writeByte(dataLevel);
#else
	// 4J - for our fixed size map, we can pack x & z into 10 bits each (-512 -> 511), y into 8 bits (0 to 255)
	// block type could really be 7 bits but leaving that as 8 for future ease of expansion. Data only needs to be 4-bits as that is how it 
	// is ultimately stored
	int xyzdata = ( ( x & 0x3ff ) << 22 ) | ( ( y & 0xff ) << 14 ) | ( ( z & 0x3ff ) << 4 ) | ( data & 0xf);
	//xyzdata |= levelIdx << 31;
	dos->writeInt(xyzdata);
	dos->writeShort(block);

	// Can't pack this as it's now 2 bits
	dos->write(levelIdx);
#endif
}

void TileUpdatePacket::handle(PacketListener *listener) 
{
	listener->handleTileUpdate(shared_from_this());
}

int TileUpdatePacket::getEstimatedSize() 
{
#ifdef _LARGE_WORLDS
	return 12;
#else
	return 5;
#endif
}
