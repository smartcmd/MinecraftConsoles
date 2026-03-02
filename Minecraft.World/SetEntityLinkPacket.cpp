#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "net.minecraft.world.entity.h"
#include "SetEntityLinkPacket.h"



SetEntityLinkPacket::SetEntityLinkPacket()
{
	sourceId = -1;
	destId = -1;
	type = -1;
}

SetEntityLinkPacket::SetEntityLinkPacket(int linkType, shared_ptr<Entity> sourceEntity, shared_ptr<Entity> destEntity)
{
	type = linkType;
	this->sourceId = sourceEntity->entityId;
	this->destId = destEntity != NULL ? destEntity->entityId : -1;
}

int SetEntityLinkPacket::getEstimatedSize() 
{
	return 8;
}

void SetEntityLinkPacket::read(DataInputStream *dis) //throws IOException
{
	sourceId = dis->readInt();
	destId = dis->readInt();
	type = dis->readUnsignedByte();
}

void SetEntityLinkPacket::write(DataOutputStream *dos) //throws IOException
{
	dos->writeInt(sourceId);
	dos->writeInt(destId);
	dos->writeByte(type);
}

void SetEntityLinkPacket::handle(PacketListener *listener)
{
	listener->handleEntityLinkPacket(shared_from_this());
}

bool SetEntityLinkPacket::canBeInvalidated()
{
	return true;
}

bool SetEntityLinkPacket::isInvalidatedBy(shared_ptr<Packet> packet)
{
	shared_ptr<SetEntityLinkPacket> target = dynamic_pointer_cast<SetEntityLinkPacket>(packet);
	return target->sourceId == sourceId;
}
