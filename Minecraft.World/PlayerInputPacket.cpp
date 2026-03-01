#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "PlayerInputPacket.h"



PlayerInputPacket::PlayerInputPacket()
{
	xxa = 0.0f;
	yya = 0.0f;
	isJumpingVar = false;
	isSneakingVar = false;
}

PlayerInputPacket::PlayerInputPacket(float xxa, float yya, bool isJumpingVar, bool isSneakingVar) 
{
	this->xxa = xxa;
	this->yya = yya;
	this->isJumpingVar = isJumpingVar;
	this->isSneakingVar = isSneakingVar;
}

void PlayerInputPacket::read(DataInputStream *dis) //throws IOException 
{
	xxa = dis->readFloat();
	yya = dis->readFloat();
	isJumpingVar = dis->readBoolean();
	isSneakingVar = dis->readBoolean();
}

void PlayerInputPacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeFloat(xxa);
	dos->writeFloat(yya);
	dos->writeBoolean(isJumpingVar);
	dos->writeBoolean(isSneakingVar);
}

void PlayerInputPacket::handle(PacketListener *listener)
{
	listener->handlePlayerInput(shared_from_this());
}

int PlayerInputPacket::getEstimatedSize()
{
	return 10;
}

float PlayerInputPacket::getXxa()
{
	return xxa;
}

float PlayerInputPacket::getYya() 
{
	return yya;
}

bool PlayerInputPacket::isJumping() 
{
	return isJumpingVar;
}

bool PlayerInputPacket::isSneaking() 
{
	return isSneakingVar;
}
