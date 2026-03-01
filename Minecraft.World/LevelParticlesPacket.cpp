#include "stdafx.h"
#include "PacketListener.h"
#include "LevelParticlesPacket.h"

LevelParticlesPacket::LevelParticlesPacket()
{
	this->name = L"";
	this->x = 0.0f;
	this->y = 0.0f;
	this->z = 0.0f;
	this->xDist = 0.0f;
	this->yDist = 0.0f;
	this->zDist = 0.0f;
	this->maxSpeed = 0.0f;
	this->count = 0;
}

LevelParticlesPacket::LevelParticlesPacket(const wstring &name, float x, float y, float z, float xDist, float yDist, float zDist, float maxSpeed, int count)
{
	this->name = name;
	this->x = x;
	this->y = y;
	this->z = z;
	this->xDist = xDist;
	this->yDist = yDist;
	this->zDist = zDist;
	this->maxSpeed = maxSpeed;
	this->count = count;
}

void LevelParticlesPacket::read(DataInputStream *dis)
{
	name = readUtf(dis, 64);
	x = dis->readFloat();
	y = dis->readFloat();
	z = dis->readFloat();
	xDist = dis->readFloat();
	yDist = dis->readFloat();
	zDist = dis->readFloat();
	maxSpeed = dis->readFloat();
	count = dis->readInt();
}

void LevelParticlesPacket::write(DataOutputStream *dos)
{
	writeUtf(name, dos);
	dos->writeFloat(x);
	dos->writeFloat(y);
	dos->writeFloat(z);
	dos->writeFloat(xDist);
	dos->writeFloat(yDist);
	dos->writeFloat(zDist);
	dos->writeFloat(maxSpeed);
	dos->writeInt(count);
}

wstring LevelParticlesPacket::getName()
{
	return name;
}

double LevelParticlesPacket::getX()
{
	return x;
}

double LevelParticlesPacket::getY()
{
	return y;
}

double LevelParticlesPacket::getZ()
{
	return z;
}

float LevelParticlesPacket::getXDist()
{
	return xDist;
}

float LevelParticlesPacket::getYDist()
{
	return yDist;
}

float LevelParticlesPacket::getZDist()
{
	return zDist;
}

float LevelParticlesPacket::getMaxSpeed()
{
	return maxSpeed;
}

int LevelParticlesPacket::getCount()
{
	return count;
}

void LevelParticlesPacket::handle(PacketListener *listener)
{
	listener->handleParticleEvent(shared_from_this());
}

int LevelParticlesPacket::getEstimatedSize()
{
	return 4 * 2 + 7 * 8;
}