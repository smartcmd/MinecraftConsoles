#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.entity.h"
#include "PacketListener.h"
#include "AnimatePacket.h"



AnimatePacket::AnimatePacket()
{
	id = -1;
	action = 0;
}

AnimatePacket::AnimatePacket(shared_ptr<Entity> e, int action)
{
	id = e->entityId;
	this->action = action;
}

void AnimatePacket::read(DataInputStream *dis) //throws IOException 
{
	id = dis->readInt();
	action = dis->readByte();
}

void AnimatePacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeInt(id);
	dos->writeByte(action);
}

void AnimatePacket::handle(PacketListener *listener) 
{
	listener->handleAnimate(shared_from_this());
}

int AnimatePacket::getEstimatedSize() 
{
	return 5;
}
