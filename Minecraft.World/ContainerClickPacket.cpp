#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.item.h"
#include "PacketListener.h"
#include "ContainerClickPacket.h"



ContainerClickPacket::~ContainerClickPacket()
{
}

ContainerClickPacket::ContainerClickPacket()
{
	containerId = 0;
	slotNum = 0;
	buttonNum = 0;
	uid = 0;
	item = nullptr;
	clickType = 0;
}

ContainerClickPacket::ContainerClickPacket(int containerId, int slotNum, int buttonNum, int clickType, shared_ptr<ItemInstance> item, short uid)
{
	this->containerId = containerId;
	this->slotNum = slotNum;
	this->buttonNum = buttonNum;
	this->uid = uid;
	this->clickType = clickType;
	// 4J - make a copy of the relevant bits of this item, as we want our packets to have full ownership of any data they reference
	this->item = item ? item->copy() : nullptr;
}

void ContainerClickPacket::handle(PacketListener *listener)
{
	listener->handleContainerClick(shared_from_this());
}

void ContainerClickPacket::read(DataInputStream *dis) //throws IOException
{
	containerId = dis->readByte();
	slotNum = dis->readShort();
	buttonNum = dis->readByte();
	uid = dis->readShort();
	clickType = dis->readByte();

	item = readItem(dis);
}

void ContainerClickPacket::write(DataOutputStream *dos) // throws IOException
{
	dos->writeByte(containerId);
	dos->writeShort(slotNum);
	dos->writeByte(buttonNum);
	dos->writeShort(uid);
	dos->writeByte(clickType);

	writeItem(item, dos);
}

int ContainerClickPacket::getEstimatedSize() 
{
	return 4 + 4 + 2 + 1;
}
