#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "AddExperienceOrbPacket.h"



AddExperienceOrbPacket::AddExperienceOrbPacket()
{
	id = 0;
	x = y = z = 0;
	value = 0;
}

AddExperienceOrbPacket::AddExperienceOrbPacket(shared_ptr<ExperienceOrb> e)
{
	id = e->entityId;
	x = Mth::floor(e->x * 32);
	y = Mth::floor(e->y * 32);
	z = Mth::floor(e->z * 32);
	value = e->getValue();
}

void AddExperienceOrbPacket::read(DataInputStream *dis)
{
	id = dis->readInt();
	x = dis->readInt();
	y = dis->readInt();
	z = dis->readInt();
	value = dis->readShort();
}

void AddExperienceOrbPacket::write(DataOutputStream *dos)
{
	dos->writeInt(id);
	dos->writeInt(x);
	dos->writeInt(y);
	dos->writeInt(z);
	dos->writeShort(value);
}

void AddExperienceOrbPacket::handle(PacketListener *listener)
{
	listener->handleAddExperienceOrb(shared_from_this());
}

int AddExperienceOrbPacket::getEstimatedSize()
{
	return 18;
}