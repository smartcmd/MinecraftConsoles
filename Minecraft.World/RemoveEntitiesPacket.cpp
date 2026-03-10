#include "stdafx.h"
#include <iostream>
#include "ArrayWithLength.h"
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "RemoveEntitiesPacket.h"

RemoveEntitiesPacket::RemoveEntitiesPacket()
{
}

RemoveEntitiesPacket::RemoveEntitiesPacket(intArray ids) 
{
	this->ids = ids;
}

RemoveEntitiesPacket::~RemoveEntitiesPacket()
{
	delete ids.data;
}

void RemoveEntitiesPacket::read(DataInputStream *dis) //throws IOException 
{
	int count = dis->readByte();
	if(count < 0) count = 0;
	ids = intArray(count);
	for(unsigned int i = 0; i < ids.length; ++i)
	{
		ids[i] = dis->readInt();
	}
}

void RemoveEntitiesPacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeByte(ids.length);
	for(unsigned int i = 0; i < ids.length; ++i)
	{
		dos->writeInt(ids[i]);
	}
}

void RemoveEntitiesPacket::handle(PacketListener *listener)
{
	listener->handleRemoveEntity(shared_from_this());
}

int RemoveEntitiesPacket::getEstimatedSize()
{
	return 1 + (ids.length * 4);
}

/*
	4J: These are necesary on the PS3.
		(and 4).
*/
#if (defined __PS3__ || defined __ORBIS__ || defined __PSVITA__)
const int RemoveEntitiesPacket::MAX_PER_PACKET;
#endif
