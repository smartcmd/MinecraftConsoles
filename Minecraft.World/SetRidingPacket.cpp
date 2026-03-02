#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "net.minecraft.world.entity.h"
#include "SetRidingPacket.h"



SetRidingPacket::SetRidingPacket()
{
	riderId = -1;
	riddenId = -1;
}

SetRidingPacket::SetRidingPacket(shared_ptr<Entity> rider, shared_ptr<Entity> riding)
{
	this->riderId = rider->entityId;
	this->riddenId = riding != NULL ? riding->entityId : -1;
}

int SetRidingPacket::getEstimatedSize() 
{
	return 8;
}

void SetRidingPacket::read(DataInputStream *dis) //throws IOException
{
	riderId = dis->readInt();
	riddenId = dis->readInt();
}

void SetRidingPacket::write(DataOutputStream *dos) //throws IOException
{
	dos->writeInt(riderId);
	dos->writeInt(riddenId);
}

void SetRidingPacket::handle(PacketListener *listener)
{
	listener->handleRidePacket(shared_from_this());
}

bool SetRidingPacket::canBeInvalidated()
{
	return true;
}

bool SetRidingPacket::isInvalidatedBy(shared_ptr<Packet> packet)
{
	shared_ptr<SetRidingPacket> target = dynamic_pointer_cast<SetRidingPacket>(packet);
	return target->riderId == riderId;
}
