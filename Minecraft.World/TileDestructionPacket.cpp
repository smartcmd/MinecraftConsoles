#include "stdafx.h"
#include "net.minecraft.network.packet.h"
#include "TileDestructionPacket.h"

TileDestructionPacket::TileDestructionPacket() 
{
	id = 0;
	x = 0;
	y = 0;
	z = 0;
	state = 0;
}

TileDestructionPacket::TileDestructionPacket(int id, int x, int y, int z, int state)
{
	this->id = id;
	this->x = x;
	this->y = y;
	this->z = z;
	this->state = state;
}

void TileDestructionPacket::read(DataInputStream *dis)
{
	id = dis->readInt();
	x = dis->readInt();
	y = dis->readInt();
	z = dis->readInt();
	state = dis->read();
}

void TileDestructionPacket::write(DataOutputStream *dos)
{
	dos->writeInt(id);
	dos->writeInt(x);
	dos->writeInt(y);
	dos->writeInt(z);
	dos->write(state);
}

void TileDestructionPacket::handle(PacketListener *listener)
{
	listener->handleTileDestruction(shared_from_this());
}

int TileDestructionPacket::getEstimatedSize()
{
	return 13;
}

int TileDestructionPacket::getEntityId()
{
	return id;
}

int TileDestructionPacket::getX()
{
	return x;
}

int TileDestructionPacket::getY()
{
	return y;
}

int TileDestructionPacket::getZ()
{
	return z;
}

int TileDestructionPacket::getState()
{
	return state;
}

bool TileDestructionPacket::canBeInvalidated()
{
	return true;
}

bool TileDestructionPacket::isInvalidatedBy(shared_ptr<Packet> packet)
{
	shared_ptr<TileDestructionPacket> target = dynamic_pointer_cast<TileDestructionPacket>(packet);
	return target->id == id;
}