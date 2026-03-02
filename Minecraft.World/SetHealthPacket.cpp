#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "SetHealthPacket.h"



SetHealthPacket::SetHealthPacket()
{
	this->health = 0;
	this->food = 0;
	this->saturation = 0;

	this->damageSource = eTelemetryChallenges_Unknown;
}

SetHealthPacket::SetHealthPacket(int health, int food, float saturation, ETelemetryChallenges damageSource)
{
	this->health = health;
	this->food = food;
	this->saturation = saturation;
	// this.exhaustion = exhaustion; // 4J - Original comment

	this->damageSource = damageSource;
}

void SetHealthPacket::read(DataInputStream *dis) //throws IOException 
{
	health = dis->readShort();
	food = dis->readShort();
	saturation = dis->readFloat();
	//        exhaustion = dis.readFloat();

	damageSource = (ETelemetryChallenges)dis->readByte();
}

void SetHealthPacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeShort(health);
	dos->writeShort(food);
	dos->writeFloat(saturation);
	//        dos.writeFloat(exhaustion);

	dos->writeByte(damageSource);
}

void SetHealthPacket::handle(PacketListener *listener) 
{
	listener->handleSetHealth(shared_from_this());
}

int SetHealthPacket::getEstimatedSize()
{
	return 9;
}

bool SetHealthPacket::canBeInvalidated()
{
	return true;
}

bool SetHealthPacket::isInvalidatedBy(shared_ptr<Packet> packet)
{
	return true;
}