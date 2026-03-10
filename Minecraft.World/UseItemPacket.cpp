#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.item.h"
#include "PacketListener.h"
#include "UseItemPacket.h"

const float UseItemPacket::CLICK_ACCURACY = 16.0f;

UseItemPacket::~UseItemPacket() 
{
}

UseItemPacket::UseItemPacket() 
{
	x = 0;
	y = 0;
	z = 0;
	face = 0;
	item = nullptr;
	clickX = 0.0f;
	clickY = 0.0f;
	clickZ = 0.0f;
}

UseItemPacket::UseItemPacket(int x, int y, int z, int face, shared_ptr<ItemInstance> item, float clickX, float clickY, float clickZ) 
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->face = face;
	// 4J - take copy of item as we want our packets to have full ownership of any referenced data
	this->item = item ? item->copy() : shared_ptr<ItemInstance>();
	this->clickX = clickX;
	this->clickY = clickY;
	this->clickZ = clickZ;
}

void UseItemPacket::read(DataInputStream *dis) //throws IOException 
{
	x = dis->readInt();
	y = dis->readUnsignedByte();
	z = dis->readInt();
	face = dis->read();
	item = readItem(dis);
	clickX = dis->readUnsignedByte() / CLICK_ACCURACY;
	clickY = dis->readUnsignedByte() / CLICK_ACCURACY;
	clickZ = dis->readUnsignedByte() / CLICK_ACCURACY;
}

void UseItemPacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeInt(x);
	dos->write(y);
	dos->writeInt(z);
	dos->write(face);

	writeItem(item, dos);
	dos->write(static_cast<int>(clickX * CLICK_ACCURACY));
	dos->write(static_cast<int>(clickY * CLICK_ACCURACY));
	dos->write(static_cast<int>(clickZ * CLICK_ACCURACY));
}

void UseItemPacket::handle(PacketListener *listener)
{
	listener->handleUseItem(shared_from_this());
}

int UseItemPacket::getEstimatedSize()
{
	return 15;
}

int UseItemPacket::getX()
{
	return x;
}

int UseItemPacket::getY()
{
	return y;
}

int UseItemPacket::getZ()
{
	return z;
}

int UseItemPacket::getFace()
{
	return face;
}

shared_ptr<ItemInstance> UseItemPacket::getItem()
{
	return item;
}

float UseItemPacket::getClickX()
{
	return clickX;
}

float UseItemPacket::getClickY()
{
	return clickY;
}

float UseItemPacket::getClickZ()
{
	return clickZ;
}
