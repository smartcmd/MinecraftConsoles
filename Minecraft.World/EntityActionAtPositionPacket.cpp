#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.entity.h"
#include "PacketListener.h"
#include "EntityActionAtPositionPacket.h"



const int EntityActionAtPositionPacket::START_SLEEP = 0;

EntityActionAtPositionPacket::EntityActionAtPositionPacket()
{
	id = -1;
	x = 0;
	y = 0;
	z = 0;
	action = 0;
}

EntityActionAtPositionPacket::EntityActionAtPositionPacket(shared_ptr<Entity> e, int action, int x, int y, int z)
{
	this->action = action;
	this->x = x;
	this->y = y;
	this->z = z;
	this->id = e->entityId;
}

void EntityActionAtPositionPacket::read(DataInputStream *dis) //throws IOException 
{
	id = dis->readInt();
	action = dis->readByte();
	x = dis->readInt();
	y = dis->readByte();
	z = dis->readInt();
}

void EntityActionAtPositionPacket::write(DataOutputStream *dos) //throws IOException
{
	dos->writeInt(id);
	dos->writeByte(action);
	dos->writeInt(x);
	dos->writeByte(y);
	dos->writeInt(z);
}

void EntityActionAtPositionPacket::handle(PacketListener *listener)
{
	listener->handleEntityActionAtPosition(shared_from_this());
}

int EntityActionAtPositionPacket::getEstimatedSize()
{
	return 14;
}
