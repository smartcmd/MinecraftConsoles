#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "EntityEventPacket.h"



EntityEventPacket::EntityEventPacket() 
{
	entityId = 0;
	eventId = 0;
}

EntityEventPacket::EntityEventPacket(int entityId, uint8_t eventId) 
{
	this->entityId = entityId;
	this->eventId = eventId;
}

void EntityEventPacket::read(DataInputStream *dis) //throws IOException 
{
	entityId = dis->readInt();
	eventId = dis->readByte();
}

void EntityEventPacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeInt(entityId);
	dos->writeByte(eventId);
}

void EntityEventPacket::handle(PacketListener *listener) 
{
	listener->handleEntityEvent(shared_from_this());
}

int EntityEventPacket::getEstimatedSize()
{
	return 5;
}
