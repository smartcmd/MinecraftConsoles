#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.item.h"
#include "PacketListener.h"
#include "CraftItemGridPacket.h"



CraftItemGridPacket::~CraftItemGridPacket()
{
}

CraftItemGridPacket::CraftItemGridPacket()
{
	recipe = -1;
	uid = 0;
	is2x2 = false;
	for(int i = 0; i < 9; i++) gridData[i] = -1;
}

CraftItemGridPacket::CraftItemGridPacket(int recipe, short uid, bool is2x2, int gridData[9])
{
	this->recipe = recipe;
	this->uid = uid;
	this->is2x2 = is2x2;
	for(int i = 0; i < 9; i++) this->gridData[i] = gridData[i];
}

void CraftItemGridPacket::handle(PacketListener *listener)
{
	listener->handleCraftItemGrid(shared_from_this());
}

void CraftItemGridPacket::read(DataInputStream *dis)
{
	uid = dis->readShort();
	recipe = dis->readInt();
	is2x2 = dis->readBoolean();
	for(int i = 0; i < 9; i++) gridData[i] = dis->readInt();
}

void CraftItemGridPacket::write(DataOutputStream *dos)
{
	dos->writeShort(uid);
	dos->writeInt(recipe);
	dos->writeBoolean(is2x2);
	for(int i = 0; i < 9; i++) dos->writeInt(gridData[i]);
}

int CraftItemGridPacket::getEstimatedSize() 
{
	return 2 + 4 + 1 + (9 * 4);
}
