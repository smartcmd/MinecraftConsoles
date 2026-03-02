#include "stdafx.h"
#include "net.minecraft.world.effect.h"
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "UpdateMobEffectPacket.h"



UpdateMobEffectPacket::UpdateMobEffectPacket()
{
	this->entityId = 0;
	this->effectId = 0;
	this->effectAmplifier = 0;
	this->effectDurationTicks = 0;
}

UpdateMobEffectPacket::UpdateMobEffectPacket(int entityId, MobEffectInstance *effect)
{
	this->entityId = entityId;
	this->effectId = (BYTE) (effect->getId() & 0xff);
	this->effectAmplifier = (char) (effect->getAmplifier() & 0xff);
	this->effectDurationTicks = (short) effect->getDuration();
}

void UpdateMobEffectPacket::read(DataInputStream *dis)
{
	entityId = dis->readInt();
	effectId = dis->readByte();
	effectAmplifier = dis->readByte();
	effectDurationTicks = dis->readShort();
}

void UpdateMobEffectPacket::write(DataOutputStream *dos)
{
	dos->writeInt(entityId);
	dos->writeByte(effectId);
	dos->writeByte(effectAmplifier);
	dos->writeShort(effectDurationTicks);
}

void UpdateMobEffectPacket::handle(PacketListener *listener)
{
	listener->handleUpdateMobEffect(shared_from_this());
}

int UpdateMobEffectPacket::getEstimatedSize()
{
	return 8;
}

bool UpdateMobEffectPacket::canBeInvalidated()
{
	return true;
}

bool UpdateMobEffectPacket::isInvalidatedBy(shared_ptr<Packet> packet)
{
	shared_ptr<UpdateMobEffectPacket> target = dynamic_pointer_cast<UpdateMobEffectPacket>(packet);
	return target->entityId == entityId && target->effectId == effectId;
}