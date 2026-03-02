#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "LevelEventPacket.h"



LevelEventPacket::LevelEventPacket() 
{
	type = 0;
	data = 0;
	x = 0;
	y = 0;
	z = 0;
}

LevelEventPacket::LevelEventPacket(int type, int x, int y, int z, int data, bool globalEvent)
{
	this->type = type;
	this->x = x;
	this->y = y;
	this->z = z;
	this->data = data;
	this->globalEvent = globalEvent;
}

void LevelEventPacket::read(DataInputStream *dis) //throws IOException
{
	type = dis->readInt();
	x = dis->readInt();
	y = dis->readByte() & 0xff;
	z = dis->readInt();
	data = dis->readInt();
	globalEvent = dis->readBoolean();
}

void LevelEventPacket::write(DataOutputStream *dos) //throws IOException
{
	dos->writeInt(type);
	dos->writeInt(x);
	dos->writeByte(y & 0xff);
	dos->writeInt(z);
	dos->writeInt(data);
	dos->writeBoolean(globalEvent);
}

void LevelEventPacket::handle(PacketListener *listener) 
{
	listener->handleLevelEvent(shared_from_this());
}

int LevelEventPacket::getEstimatedSize()
{
	return 4 * 5 + 1;
}

bool LevelEventPacket::isGlobalEvent()
{
	return globalEvent;
}
