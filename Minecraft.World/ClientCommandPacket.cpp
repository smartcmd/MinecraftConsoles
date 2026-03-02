#include "stdafx.h"
#include "PacketListener.h"
#include "ClientCommandPacket.h"

ClientCommandPacket::ClientCommandPacket()
{
	action = 0;
}

ClientCommandPacket::ClientCommandPacket(int action)
{
	this->action = action;
}

void ClientCommandPacket::read(DataInputStream *dis)
{
	action = dis->readByte();
}

void ClientCommandPacket::write(DataOutputStream *dos)
{
	dos->writeByte(action & 0xff);
}

void ClientCommandPacket::handle(PacketListener *listener)
{
	listener->handleClientCommand(dynamic_pointer_cast<ClientCommandPacket>(shared_from_this()));
}

int ClientCommandPacket::getEstimatedSize()
{
	return 1;
}