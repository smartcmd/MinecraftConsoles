#include "stdafx.h"
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "BasicTypeContainers.h"
#include "GameCommandPacket.h"

GameCommandPacket::GameCommandPacket()
{
	length = 0;
}

GameCommandPacket::GameCommandPacket(EGameCommand command, byteArray data)
{
	this->command = command;
	this->data = data;
	length = 0;

	if (data.data != nullptr)
	{
		length = data.length;

		if (length > Short::MAX_VALUE)
		{
			app.DebugPrintf("Payload may not be larger than 32K\n");
#ifndef _CONTENT_PACKAGE
			__debugbreak();
#endif
			//throw new IllegalArgumentException("Payload may not be larger than 32k");
		}
	}
}

GameCommandPacket::~GameCommandPacket()
{
	delete [] data.data;
}

void GameCommandPacket::read(DataInputStream *dis)
{
	command = static_cast<EGameCommand>(dis->readInt());
	length = dis->readShort();

	if (length > 0 && length <= Short::MAX_VALUE)
	{
		if(data.data != nullptr)
		{
			delete [] data.data;
		}
		data = byteArray(length);
		dis->readFully(data);
	}
}

void GameCommandPacket::write(DataOutputStream *dos)
{
	dos->writeInt(command);
	dos->writeShort(static_cast<short>(length));
	if (data.data != nullptr)
	{
		dos->write(data);
	}
}

void GameCommandPacket::handle(PacketListener *listener)
{
	listener->handleGameCommand( shared_from_this() );
}

int GameCommandPacket::getEstimatedSize()
{
	return 2 + 2 + length;
}