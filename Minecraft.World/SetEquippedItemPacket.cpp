#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.item.h"
#include "PacketListener.h"
#include "SetEquippedItemPacket.h"



SetEquippedItemPacket::SetEquippedItemPacket()
{
	entity = 0;
	slot = 0;
	item = nullptr;
}

SetEquippedItemPacket::SetEquippedItemPacket(int entity, int slot, shared_ptr<ItemInstance> item) 
{
	this->entity = entity;
	this->slot = slot;

	// 4J Stu - Brought forward change from 1.3 to fix #64688 - Customer Encountered: TU7: Content: Art: Aura of enchanted item is not displayed for other players in online game
	this->item = item == nullptr ? nullptr : item->copy();
}

void SetEquippedItemPacket::read(DataInputStream *dis) //throws IOException 
{
	entity = dis->readInt();
	slot = dis->readShort();

	// 4J Stu - Brought forward change from 1.3 to fix #64688 - Customer Encountered: TU7: Content: Art: Aura of enchanted item is not displayed for other players in online game
	item = readItem(dis);
}

void SetEquippedItemPacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeInt(entity);
	dos->writeShort(slot);

	// 4J Stu - Brought forward change from 1.3 to fix #64688 - Customer Encountered: TU7: Content: Art: Aura of enchanted item is not displayed for other players in online game
	writeItem(item, dos);
}

void SetEquippedItemPacket::handle(PacketListener *listener) 
{
	listener->handleSetEquippedItem(shared_from_this());
}

int SetEquippedItemPacket::getEstimatedSize()
{
	return 4 + 2 * 2;
}

// 4J Stu - Brought forward from 1.3 to fix #64688 - Customer Encountered: TU7: Content: Art: Aura of enchanted item is not displayed for other players in online game
shared_ptr<ItemInstance> SetEquippedItemPacket::getItem()
{
	return item;
}

bool SetEquippedItemPacket::canBeInvalidated()
{
	return true;
}

bool SetEquippedItemPacket::isInvalidatedBy(shared_ptr<Packet> packet)
{
	shared_ptr<SetEquippedItemPacket> target = dynamic_pointer_cast<SetEquippedItemPacket>(packet);
	return target->entity == entity && target->slot == slot;
}