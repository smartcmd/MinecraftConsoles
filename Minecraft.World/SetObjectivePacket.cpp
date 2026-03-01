#include "stdafx.h"
#include "net.minecraft.world.scores.h"
#include "PacketListener.h"
#include "SetObjectivePacket.h"

SetObjectivePacket::SetObjectivePacket()
{
	objectiveName = L"";
	displayName = L"";
	method = 0;
}

SetObjectivePacket::SetObjectivePacket(Objective *objective, int method)
{
	objectiveName = objective->getName();
	displayName = objective->getDisplayName();
	this->method = method;
}

void SetObjectivePacket::read(DataInputStream *dis)
{
	objectiveName = readUtf(dis, Objective::MAX_NAME_LENGTH);
	displayName = readUtf(dis, Objective::MAX_DISPLAY_NAME_LENGTH);
	method = dis->readByte();
}

void SetObjectivePacket::write(DataOutputStream *dos)
{
	writeUtf(objectiveName, dos);
	writeUtf(displayName, dos);
	dos->writeByte(method);
}

void SetObjectivePacket::handle(PacketListener *listener)
{
	listener->handleAddObjective(shared_from_this());
}

int SetObjectivePacket::getEstimatedSize()
{
	return 2 + objectiveName.length() + 2 + displayName.length() + 1;
}