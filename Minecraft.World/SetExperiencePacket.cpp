#include "stdafx.h"
#include "SetExperiencePacket.h"
#include "PacketListener.h"
#include "InputOutputStream.h"



SetExperiencePacket::SetExperiencePacket()
{
	this->experienceProgress = 0;
	this->totalExperience = 0;
	this->experienceLevel = 0;
}

SetExperiencePacket::SetExperiencePacket(float experienceProgress, int totalExperience, int experienceLevel)
{
	this->experienceProgress = experienceProgress;
	this->totalExperience = totalExperience;
	this->experienceLevel = experienceLevel;
}

void SetExperiencePacket::read(DataInputStream *dis)
{
	experienceProgress = dis->readFloat();
	experienceLevel = dis->readShort();
	totalExperience = dis->readShort();
}

void SetExperiencePacket::write(DataOutputStream *dos)
{
	dos->writeFloat(experienceProgress);
	dos->writeShort(experienceLevel);
	dos->writeShort(totalExperience);
}

void SetExperiencePacket::handle(PacketListener *listener)
{
	listener->handleSetExperience(shared_from_this());
}

int SetExperiencePacket::getEstimatedSize()
{
	return 8;
}

bool SetExperiencePacket::canBeInvalidated()
{
	return true;
}

bool SetExperiencePacket::isInvalidatedBy(shared_ptr<Packet> packet)
{
	return true;
}

bool SetExperiencePacket::isAync()
{
	return true;
}