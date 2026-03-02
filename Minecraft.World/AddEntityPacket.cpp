#include "stdafx.h"
#include <exception>
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.entity.h"
#include "PacketListener.h"
#include "AddEntityPacket.h"



void AddEntityPacket::_init(shared_ptr<Entity> e, int type, int data, int xp, int yp, int zp, int yRotp, int xRotp)
{
	id = e->entityId;
	// 4J Stu - We should add entities at their "last sent" position so that the relative update packets
	// put them in the correct place
	x = xp;//(int) floor(e->x * 32);
	y = yp;//(int) floor(e->y * 32);
	z = zp;//(int) floor(e->z * 32);
	yRot = yRotp;
	xRot = xRotp;
	this->type = type;
	this->data = data;
	if (data > -1)	// 4J - changed "no data" value to be -1, we can have a valid entity id of 0
	{
		double xd = e->xd;
		double yd = e->yd;
		double zd = e->zd;
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
	}
}

AddEntityPacket::AddEntityPacket() 
{
}

AddEntityPacket::AddEntityPacket(shared_ptr<Entity> e, int type, int yRotp, int xRotp, int xp, int yp, int zp )
{
	_init(e, type, -1, xp, yp, zp, yRotp, xRotp); // 4J - changed "no data" value to be -1, we can have a valid entity id of 0
}

AddEntityPacket::AddEntityPacket(shared_ptr<Entity> e, int type, int data, int yRotp, int xRotp, int xp, int yp, int zp ) 
{
	_init(e, type, data, xp, yp, zp, yRotp, xRotp);
}

void AddEntityPacket::read(DataInputStream *dis) // throws IOException  TODO 4J JEV add throws statement
{
	id = dis->readShort();
	type = dis->readByte();
#ifdef _LARGE_WORLDS
	x = dis->readInt();
	y = dis->readInt();
	z = dis->readInt();
#else
	x = dis->readShort();
	y = dis->readShort();
	z = dis->readShort();
#endif
	yRot = dis->readByte();
	xRot = dis->readByte();
	data = dis->readInt();
	if (data > -1)	// 4J - changed "no data" value to be -1, we can have a valid entity id of 0
	{
		xa = dis->readShort();
		ya = dis->readShort();
		za = dis->readShort();
	}
}

void AddEntityPacket::write(DataOutputStream *dos) // throws IOException TODO 4J JEV add throws statement
{
	dos->writeShort(id);
	dos->writeByte(type);
#ifdef _LARGE_WORLDS
	dos->writeInt(x);
	dos->writeInt(y);
	dos->writeInt(z);
#else
	dos->writeShort(x);
	dos->writeShort(y);
	dos->writeShort(z);
#endif
	dos->writeByte(yRot);
	dos->writeByte(xRot);
	dos->writeInt(data);
	if (data > -1) // 4J - changed "no data" value to be -1, we can have a valid entity id of 0
	{
		dos->writeShort(xa);
		dos->writeShort(ya);
		dos->writeShort(za);
	}
}

void AddEntityPacket::handle(PacketListener *listener) 
{
	listener->handleAddEntity(shared_from_this());
}

int AddEntityPacket::getEstimatedSize() 
{
	return 11 + data > -1 ? 6 : 0;
}
