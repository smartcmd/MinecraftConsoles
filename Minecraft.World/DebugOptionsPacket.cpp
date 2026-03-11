#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.item.h"
#include "PacketListener.h"
#include "DebugOptionsPacket.h"



DebugOptionsPacket::~DebugOptionsPacket()
{
}

DebugOptionsPacket::DebugOptionsPacket()
{
	m_uiVal = 0L;
}

DebugOptionsPacket::DebugOptionsPacket(unsigned int uiVal)
{
	this->m_uiVal = uiVal;
}

void DebugOptionsPacket::handle(PacketListener *listener)
{
	listener->handleDebugOptions(shared_from_this());
}

void DebugOptionsPacket::read(DataInputStream *dis) //throws IOException
{
	m_uiVal = static_cast<unsigned int>(dis->readInt());
}

void DebugOptionsPacket::write(DataOutputStream *dos) // throws IOException
{
	dos->writeInt(static_cast<int>(m_uiVal));
}

int DebugOptionsPacket::getEstimatedSize() 
{
	return sizeof(int);
}
