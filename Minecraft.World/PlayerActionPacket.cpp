#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "PlayerActionPacket.h"

const int PlayerActionPacket::START_DESTROY_BLOCK = 0;
const int PlayerActionPacket::ABORT_DESTROY_BLOCK = 1;
const int PlayerActionPacket::STOP_DESTROY_BLOCK = 2;
const int PlayerActionPacket::DROP_ALL_ITEMS = 3;
const int PlayerActionPacket::DROP_ITEM = 4;
const int PlayerActionPacket::RELEASE_USE_ITEM = 5;

PlayerActionPacket::PlayerActionPacket()
{
	x = 0;
	y = 0;
	z = 0;
	face = 0;
	action = 0;
}

PlayerActionPacket::PlayerActionPacket(int action, int x, int y, int z, int face)
{
	this->action = action;
	this->x = x;
	this->y = y;
	this->z = z;
	this->face = face;
}

void PlayerActionPacket::read(DataInputStream *dis) //throws IOException 
{
	action = dis->readUnsignedByte();
	x = dis->readInt();
	y = dis->readUnsignedByte();
	z = dis->readInt();
	face = dis->readUnsignedByte();
}

void PlayerActionPacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->write(action);
	dos->writeInt(x);
	dos->write(y);
	dos->writeInt(z);
	dos->write(face);
}

void PlayerActionPacket::handle(PacketListener *listener) 
{
	listener->handlePlayerAction(shared_from_this());
}

int PlayerActionPacket::getEstimatedSize()
{
	return 11;
}
