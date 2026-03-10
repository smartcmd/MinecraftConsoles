#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "MoveEntityPacketSmall.h"


MoveEntityPacketSmall::MoveEntityPacketSmall() 
{
	hasRot = false;

	id = -1;
	xa = 0;
	ya = 0;
	za = 0;
	yRot = 0;
	xRot = 0;
}

MoveEntityPacketSmall::MoveEntityPacketSmall(int id)
{
	if( (id < 0 ) || (id >= 2048 ) )
	{
		// We shouln't be tracking an entity that doesn't have a short type of id
		__debugbreak();
	}

	this->id = id;
	hasRot = false;

	xa = 0;
	ya = 0;
	za = 0;
	yRot = 0;
	xRot = 0;
}

void MoveEntityPacketSmall::read(DataInputStream *dis) //throws IOException 
{
	id = dis->readShort();
}

void MoveEntityPacketSmall::write(DataOutputStream *dos) //throws IOException
{
	if( (id < 0 ) || (id >= 2048 ) )
	{
		// We shouln't be tracking an entity that doesn't have a short type of id
		__debugbreak();
	}
	dos->writeShort(static_cast<short>(id));
}

void MoveEntityPacketSmall::handle(PacketListener *listener)
{
	listener->handleMoveEntitySmall(shared_from_this());
}

int MoveEntityPacketSmall::getEstimatedSize() 
{
	return 2;
}

bool MoveEntityPacketSmall::canBeInvalidated()
{
	return true;
}

bool MoveEntityPacketSmall::isInvalidatedBy(shared_ptr<Packet> packet)
{
	shared_ptr<MoveEntityPacketSmall> target = dynamic_pointer_cast<MoveEntityPacketSmall>(packet);
	return target != nullptr && target->id == id;
}

MoveEntityPacketSmall::PosRot::PosRot()
{
	hasRot = true;
}

MoveEntityPacketSmall::PosRot::PosRot(int id, char xa, char ya, char za, char yRot, char xRot) : MoveEntityPacketSmall( id )
{
	this->xa = xa;
	this->ya = ya;
	this->za = za;
	this->yRot = yRot;
	this->xRot = xRot;
	hasRot = true;
}

void MoveEntityPacketSmall::PosRot::read(DataInputStream *dis) //throws IOException 
{
	int idAndRot = dis->readShort();
	this->id = idAndRot & 0x07ff;
	this->yRot = idAndRot >> 11;
	int xAndYAndZ = (int)dis->readShort();
	this->xa = xAndYAndZ >> 11; 
	this->ya = (xAndYAndZ << 21 ) >> 26;
	this->za = (xAndYAndZ << 27 ) >> 27;
}

void MoveEntityPacketSmall::PosRot::write(DataOutputStream *dos) //throws IOException 
{
	if( (id < 0 ) || (id >= 2048 ) )
	{
		// We shouln't be tracking an entity that doesn't have a short type of id
		__debugbreak();
	}
	short idAndRot = id | yRot << 11;
	dos->writeShort(idAndRot);
	short xAndYAndZ = ( xa << 11 ) | ( ( ya & 0x3f ) << 5 ) | ( za & 0x1f );
	dos->writeShort(xAndYAndZ);
}

int MoveEntityPacketSmall::PosRot::getEstimatedSize() 
{
	return 4;
}

MoveEntityPacketSmall::Pos::Pos() 
{
}

MoveEntityPacketSmall::Pos::Pos(int id, char xa, char ya, char za) : MoveEntityPacketSmall(id)
{
	this->xa = xa;
	this->ya = ya;
	this->za = za;
}

void MoveEntityPacketSmall::Pos::read(DataInputStream *dis) //throws IOException 
{
	int idAndY = dis->readShort();
	this->id = idAndY & 0x07ff;
	this->ya = idAndY >> 11;
	int XandZ = (int)static_cast<signed char>(dis->readByte());
	xa = XandZ >> 4;
	za = ( XandZ << 28 ) >> 28;
}

void MoveEntityPacketSmall::Pos::write(DataOutputStream *dos) //throws IOException
{
	if( (id < 0 ) || (id >= 2048 ) )
	{
		// We shouln't be tracking an entity that doesn't have a short type of id
		__debugbreak();
	}
	short idAndY = id | ya << 11;
	dos->writeShort(idAndY);
	char XandZ = ( xa << 4 ) | ( za & 0x0f );
	dos->writeByte(XandZ);
}

int MoveEntityPacketSmall::Pos::getEstimatedSize()
{
	return 3;
}

MoveEntityPacketSmall::Rot::Rot() 
{
	hasRot = true;
}

MoveEntityPacketSmall::Rot::Rot(int id, char yRot, char xRot) : MoveEntityPacketSmall(id)
{

	this->yRot = yRot;
	this->xRot = xRot;
	hasRot = true;
}

void MoveEntityPacketSmall::Rot::read(DataInputStream *dis) //throws IOException 
{
	int idAndRot = (int)dis->readShort();
	this->id = idAndRot & 0x07ff;
	this->yRot = idAndRot >> 11;
}

void MoveEntityPacketSmall::Rot::write(DataOutputStream *dos) //throws IOException 
{
	if( (id < 0 ) || (id >= 2048 ) )
	{
		// We shouln't be tracking an entity that doesn't have a short type of id
		__debugbreak();
	}
	short idAndRot = id | yRot << 11;
	dos->writeShort(idAndRot);
}

int MoveEntityPacketSmall::Rot::getEstimatedSize()
{
	return 2;
}
