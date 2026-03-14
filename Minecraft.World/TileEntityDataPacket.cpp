#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "TileEntityDataPacket.h"



void TileEntityDataPacket::_init()
{
	x = y = z = 0;
	type = TYPE_MOB_SPAWNER;
	tag = nullptr;
}


TileEntityDataPacket::TileEntityDataPacket()
{
	_init();
	shouldDelay = true;
}

TileEntityDataPacket::TileEntityDataPacket(int x, int y, int z, int type, CompoundTag *tag)
{
	_init();
	shouldDelay = true;
	this->x = x;
	this->y = y;
	this->z = z;
	this->type = type;
	this->tag = tag;
}

TileEntityDataPacket::~TileEntityDataPacket()
{
	delete tag;
}

void TileEntityDataPacket::read(DataInputStream *dis)
{
	x = dis->readInt();
	y = dis->readShort();
	z = dis->readInt();
	type = dis->readByte();
	tag = readNbt(dis);
}

void TileEntityDataPacket::write(DataOutputStream *dos)
{
	dos->writeInt(x);
	dos->writeShort(y);
	dos->writeInt(z);
	dos->writeByte(static_cast<byte>(type));
	writeNbt(tag, dos);
}

void TileEntityDataPacket::handle(PacketListener *listener)
{
	listener->handleTileEntityData(shared_from_this());
}

int TileEntityDataPacket::getEstimatedSize()
{
	return 6 * 4 + 1;
}