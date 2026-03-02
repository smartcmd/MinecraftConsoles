#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.network.packet.h"
#include "PlayerAbilitiesPacket.h"

const float PlayerAbilitiesPacket::SPEED_ACCURACY = 255.0f;

PlayerAbilitiesPacket::PlayerAbilitiesPacket()
{
	invulnerable = false;
	_isFlying = false;
	_canFly = false;
	instabuild = false;
	flyingSpeed = 0.0f;
	walkingSpeed = 0.0f;
}

PlayerAbilitiesPacket::PlayerAbilitiesPacket(Abilities *abilities)
{
	this->setInvulnerable(abilities->invulnerable);
	this->setFlying(abilities->flying);
	this->setCanFly(abilities->mayfly);
	this->setInstabuild(abilities->instabuild);
	this->setFlyingSpeed(abilities->getFlyingSpeed());
	this->setWalkingSpeed(abilities->getWalkingSpeed());
}

void PlayerAbilitiesPacket::read(DataInputStream *dis)
{
	byte bitfield = dis->readByte();

	this->setInvulnerable((bitfield & FLAG_INVULNERABLE) > 0);
	this->setFlying((bitfield & FLAG_FLYING) > 0);
	this->setCanFly((bitfield & FLAG_CAN_FLY) > 0);
	this->setInstabuild((bitfield & FLAG_INSTABUILD) > 0);
	this->setFlyingSpeed(dis->readByte() / SPEED_ACCURACY);
	this->setWalkingSpeed(dis->readByte() / SPEED_ACCURACY);
}

void PlayerAbilitiesPacket::write(DataOutputStream *dos)
{
	byte bitfield = 0;

	if (isInvulnerable()) bitfield |= FLAG_INVULNERABLE;
	if (isFlying()) bitfield |= FLAG_FLYING;
	if (canFly()) bitfield |= FLAG_CAN_FLY;
	if (canInstabuild()) bitfield |= FLAG_INSTABUILD;

	dos->writeByte(bitfield);
	dos->writeByte((int) (flyingSpeed * SPEED_ACCURACY));
	dos->writeByte((int) (walkingSpeed * SPEED_ACCURACY));
}

void PlayerAbilitiesPacket::handle(PacketListener *listener)
{
	listener->handlePlayerAbilities(shared_from_this());
}

int PlayerAbilitiesPacket::getEstimatedSize()
{
	return 2;
}

//wstring getDebugInfo()
//{
//	return String.format("invuln=%b, flying=%b, canfly=%b, instabuild=%b, flyspeed=%.4f, walkspped=%.4f", isInvulnerable(), isFlying(), canFly(), canInstabuild(), getFlyingSpeed(), getWalkingSpeed());
//}

bool PlayerAbilitiesPacket::isInvulnerable()
{
	return invulnerable;
}

void PlayerAbilitiesPacket::setInvulnerable(bool invulnerable)
{
	this->invulnerable = invulnerable;
}

bool PlayerAbilitiesPacket::isFlying()
{
	return _isFlying;
}

void PlayerAbilitiesPacket::setFlying(bool flying)
{
	_isFlying = flying;
}

bool PlayerAbilitiesPacket::canFly()
{
	return _canFly;
}

void PlayerAbilitiesPacket::setCanFly(bool canFly)
{
	this->_canFly = canFly;
}

bool PlayerAbilitiesPacket::canInstabuild()
{
	return instabuild;
}

void PlayerAbilitiesPacket::setInstabuild(bool instabuild)
{
	this->instabuild = instabuild;
}

float PlayerAbilitiesPacket::getFlyingSpeed()
{
	return flyingSpeed;
}

void PlayerAbilitiesPacket::setFlyingSpeed(float flySpeed)
{
	this->flyingSpeed = flySpeed;
}

float PlayerAbilitiesPacket::getWalkingSpeed()
{
	return walkingSpeed;
}

void PlayerAbilitiesPacket::setWalkingSpeed(float walkingSpeed)
{
	this->walkingSpeed = walkingSpeed;
}

bool PlayerAbilitiesPacket::canBeInvalidated()
{
	return true;
}

bool PlayerAbilitiesPacket::isInvalidatedBy(shared_ptr<Packet> packet)
{
	return true;
}