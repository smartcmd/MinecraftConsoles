#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "SetTimePacket.h"



SetTimePacket::SetTimePacket() 
{
	time = 0;
}

SetTimePacket::SetTimePacket(__int64 time)
{
	this->time = time;
}

void SetTimePacket::read(DataInputStream *dis) //throws IOException
{
	time = dis->readLong();
}

void SetTimePacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeLong(time);
}

void SetTimePacket::handle(PacketListener *listener)
{
	listener->handleSetTime(shared_from_this());
}

int SetTimePacket::getEstimatedSize()
{
	return 8;
}

bool SetTimePacket::canBeInvalidated()
{
	return true;
}

bool SetTimePacket::isInvalidatedBy(shared_ptr<Packet> packet)
{
	return true;
}

bool SetTimePacket::isAync()
{
	return true;
}