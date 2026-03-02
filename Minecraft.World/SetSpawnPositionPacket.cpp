#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "SetSpawnPositionPacket.h"



SetSpawnPositionPacket::SetSpawnPositionPacket() 
{
	x = 0;
	y = 0;
	z = 0;
}

SetSpawnPositionPacket::SetSpawnPositionPacket(int x, int y, int z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

void SetSpawnPositionPacket::read(DataInputStream *dis) //throws IOException
{
	x = dis->readInt();
	y = dis->readInt();
	z = dis->readInt();
}

void SetSpawnPositionPacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeInt(x);
	dos->writeInt(y);
	dos->writeInt(z);
}

void SetSpawnPositionPacket::handle(PacketListener *listener) 
{
	listener->handleSetSpawn(shared_from_this());
}

int SetSpawnPositionPacket::getEstimatedSize() 
{
	return 3*4;
}

bool SetSpawnPositionPacket::canBeInvalidated()
{
	return true;
}

bool SetSpawnPositionPacket::isInvalidatedBy(shared_ptr<Packet> packet)
{
	return true;
}

bool SetSpawnPositionPacket::isAync()
{
	return false;
}