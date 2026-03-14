#include "stdafx.h"
#include "net.minecraft.world.effect.h"
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "RemoveMobEffectPacket.h"



RemoveMobEffectPacket::RemoveMobEffectPacket()
{
}

RemoveMobEffectPacket::RemoveMobEffectPacket(int entityId, MobEffectInstance *effect)
{
	this->entityId = entityId;
	this->effectId = static_cast<byte>(effect->getId() & 0xff);
}

void RemoveMobEffectPacket::read(DataInputStream *dis)
{
	entityId = dis->readInt();
	effectId = dis->readByte();
}

void RemoveMobEffectPacket::write(DataOutputStream *dos)
{
	dos->writeInt(entityId);
	dos->writeByte(effectId);
}

void RemoveMobEffectPacket::handle(PacketListener *listener)
{
	listener->handleRemoveMobEffect(shared_from_this());
}

int RemoveMobEffectPacket::getEstimatedSize()
{
	return 5;
}