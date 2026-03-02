#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.entity.h"
#include "PacketListener.h"
#include "SetEntityMotionPacket.h"



void SetEntityMotionPacket::_init(int id, double xd, double yd, double zd)
{
	this->id = id;
	double m = 3.9;
	if (xd < -m) xd = -m;
	if (yd < -m) yd = -m;
	if (zd < -m) zd = -m;
	if (xd > m) xd = m;
	if (yd > m) yd = m;
	if (zd > m) zd = m;
	xa = (int) (xd * 8000.0);
	ya = (int) (yd * 8000.0);
	za = (int) (zd * 8000.0);
	// 4J - if we could transmit this as bytes (in 1/16 accuracy) then flag to do so
	if( ( xa >= (-128 * 16 ) ) && ( ya >= (-128 * 16 ) ) && ( za >= (-128 * 16 ) ) &&
		( xa < (128 * 16 ) ) && ( ya < (128 * 16 ) ) && ( za < (128 * 16 ) ) )
	{
		useBytes = true;
	}
	else
	{
		useBytes = false;
	}
}

SetEntityMotionPacket::SetEntityMotionPacket() 
{
	_init(0, 0.0f, 0.0f, 0.0f);
}

SetEntityMotionPacket::SetEntityMotionPacket(shared_ptr<Entity> e)
{
	_init(e->entityId, e->xd, e->yd, e->zd);
}

SetEntityMotionPacket::SetEntityMotionPacket(int id, double xd, double yd, double zd)
{
	_init(id, xd, yd, zd);   
}

void SetEntityMotionPacket::read(DataInputStream *dis) //throws IOException 
{
	short idAndFlag = dis->readShort();
	id = idAndFlag & 0x07ff;
	if( idAndFlag & 0x0800 )
	{
		xa = (int)dis->readByte();
		ya = (int)dis->readByte();
		za = (int)dis->readByte();
		xa = ( xa << 24 ) >> 24;
		ya = ( ya << 24 ) >> 24;
		za = ( za << 24 ) >> 24;
		xa *= 16;
		ya *= 16;
		za *= 16;
		useBytes = true;
	}
	else
	{
		xa = dis->readShort();
		ya = dis->readShort();
		za = dis->readShort();
		useBytes = false;
	}
}

void SetEntityMotionPacket::write(DataOutputStream *dos) //throws IOException 
{
	if( useBytes )
	{
		dos->writeShort(id | 0x800);
		dos->writeByte(xa/16);
		dos->writeByte(ya/16);
		dos->writeByte(za/16);
	}
	else
	{
		dos->writeShort(id);
		dos->writeShort(xa);
		dos->writeShort(ya);
		dos->writeShort(za);
	}
}

void SetEntityMotionPacket::handle(PacketListener *listener)
{
	listener->handleSetEntityMotion(shared_from_this());
}

int SetEntityMotionPacket::getEstimatedSize()
{
	return useBytes ? 5 : 8;
}

bool SetEntityMotionPacket::canBeInvalidated()
{
	return true;
}

bool SetEntityMotionPacket::isInvalidatedBy(shared_ptr<Packet> packet)
{
	shared_ptr<SetEntityMotionPacket> target = dynamic_pointer_cast<SetEntityMotionPacket>(packet);
	return target->id == id;
}
