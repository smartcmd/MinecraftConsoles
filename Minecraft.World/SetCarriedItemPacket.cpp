#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "SetCarriedItemPacket.h"



SetCarriedItemPacket::SetCarriedItemPacket() 
{
	slot = 0;
}

SetCarriedItemPacket::SetCarriedItemPacket(int slot) 
{
	this->slot = slot;
}

void SetCarriedItemPacket::read(DataInputStream *dis) //throws IOException
{
	slot = dis->readShort();
}

void SetCarriedItemPacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeShort(slot);
}

void SetCarriedItemPacket::handle(PacketListener *listener) 
{
	listener->handleSetCarriedItem(shared_from_this());
}

int SetCarriedItemPacket::getEstimatedSize() 
{
	return 2;
}

bool SetCarriedItemPacket::canBeInvalidated() 
{
	return true;
}

bool SetCarriedItemPacket::isInvalidatedBy(shared_ptr<Packet> packet)
{
	return true;
}