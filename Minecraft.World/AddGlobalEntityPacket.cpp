#include "stdafx.h"
#include <exception>
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.global.h"
#include "PacketListener.h"
#include "AddGlobalEntityPacket.h"



const int AddGlobalEntityPacket::LIGHTNING = 1;

AddGlobalEntityPacket::AddGlobalEntityPacket()
{
	id = -1;
	x = 0;
	y = 0;
	x = 0;
	type = 0;
}

AddGlobalEntityPacket::AddGlobalEntityPacket(shared_ptr<Entity> e) 
{
	id = e->entityId;
    x = Mth::floor(e->x * 32);
    y = Mth::floor(e->y * 32);
    z = Mth::floor(e->z * 32);
	if ( e->instanceof(eTYPE_LIGHTNINGBOLT) ) 
	{
		type = LIGHTNING;
	}
	else
	{
		type = 0;
	}
}

void AddGlobalEntityPacket::read(DataInputStream *dis) // throws IOException
{
	id = dis->readInt();
	type = dis->readByte();
	x = dis->readInt();
	y = dis->readInt();
	z = dis->readInt();
}

void AddGlobalEntityPacket::write(DataOutputStream *dos) // throws IOException 
{
	dos->writeInt(id);
	dos->writeByte(type);
	dos->writeInt(x);
	dos->writeInt(y);
	dos->writeInt(z);
}

void AddGlobalEntityPacket::handle(PacketListener *listener)
{
	listener->handleAddGlobalEntity(shared_from_this());
}

int AddGlobalEntityPacket::getEstimatedSize()
{
	return 17;
}
