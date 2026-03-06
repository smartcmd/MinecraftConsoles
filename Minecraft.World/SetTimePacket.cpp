#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "SetTimePacket.h"

SetTimePacket::SetTimePacket()
{
	gameTime = 0;
	dayTime = 0;
}

SetTimePacket::SetTimePacket(int64_t gameTime, int64_t dayTime, bool tickDayTime)
{
	this->gameTime = gameTime;
	this->dayTime = dayTime;

	// 4J: We send daylight cycle rule with host options so don't need this
	/*if (!tickDayTime)
	{
		this->dayTime = -this->dayTime;
		if (this->dayTime == 0)
		{
			this->dayTime = -1;
		}
	}*/
}

void SetTimePacket::read(DataInputStream *dis) //throws IOException
{
	gameTime = dis->readLong();
	dayTime = dis->readLong();
}

void SetTimePacket::write(DataOutputStream *dos) //throws IOException
{
	dos->writeLong(gameTime);
	dos->writeLong(dayTime);
}

void SetTimePacket::handle(PacketListener *listener)
{
	listener->handleSetTime(shared_from_this());
}

int SetTimePacket::getEstimatedSize()
{
	return 16;
}

bool SetTimePacket::canBeInvalidated()
{
	return true;
}

bool SetTimePacket::isInvalidatedBy(shared_ptr<Packet> packet)
{
	return true;
}

bool SetTimePacket::isAync()
{
	return true;
}