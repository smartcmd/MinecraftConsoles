#include "stdafx.h"
#include "net.minecraft.world.effect.h"
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "BasicTree.h"
#include "BasicTypeContainers.h"
#include "UpdateMobEffectPacket.h"



UpdateMobEffectPacket::UpdateMobEffectPacket()
{
	entityId = 0;
	effectId = 0;
	effectAmplifier = 0;
	effectDurationTicks = 0;
}

UpdateMobEffectPacket::UpdateMobEffectPacket(int entityId, MobEffectInstance *effect)
{
	this->entityId = entityId;
	effectId = (BYTE) (effect->getId() & 0xff);
	effectAmplifier = (char) (effect->getAmplifier() & 0xff);

	if (effect->getDuration() > Short::MAX_VALUE)
	{
		effectDurationTicks = Short::MAX_VALUE;
	}
	else
	{
		effectDurationTicks = (short) effect->getDuration();
	}
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

bool UpdateMobEffectPacket::isSuperLongDuration()
{
	return effectDurationTicks == Short::MAX_VALUE;
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