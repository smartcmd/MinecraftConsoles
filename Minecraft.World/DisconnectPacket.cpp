#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "SharedConstants.h"
#include "PacketListener.h"
#include "DisconnectPacket.h"



DisconnectPacket::DisconnectPacket()
{
	reason = eDisconnect_None;
}

DisconnectPacket::DisconnectPacket(eDisconnectReason reason)
{
	this->reason = reason;
}

void DisconnectPacket::read(DataInputStream *dis) //throws IOException
{
	reason = static_cast<eDisconnectReason>(dis->readInt());
}

void DisconnectPacket::write(DataOutputStream *dos) //throws IOException
{
	dos->writeInt((int)reason);
}

void DisconnectPacket::handle(PacketListener *listener)
{
	listener->handleDisconnect(shared_from_this());
}

int DisconnectPacket::getEstimatedSize() 
{
	return sizeof(eDisconnectReason);
}

bool DisconnectPacket::canBeInvalidated()
{
	return true;
}

bool DisconnectPacket::isInvalidatedBy(shared_ptr<Packet> packet)
{
	return true;
}