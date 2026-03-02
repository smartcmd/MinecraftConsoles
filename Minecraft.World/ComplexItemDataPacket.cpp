#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "ComplexItemDataPacket.h"



ComplexItemDataPacket::~ComplexItemDataPacket()
{
	delete [] data.data;
}

ComplexItemDataPacket::ComplexItemDataPacket()
{
	shouldDelay = true;
	itemType = 0;
}

ComplexItemDataPacket::ComplexItemDataPacket(short itemType, short itemId, charArray data)
{
	shouldDelay = true;
	this->itemType = itemType;
	this->itemId = itemId;
	// Take copy of array passed in as we want the packets to have full ownership of any data they reference
	this->data = charArray(data.length);
	memcpy(this->data.data, data.data, data.length);
}

void ComplexItemDataPacket::read(DataInputStream *dis) //throws IOException
{
	itemType = dis->readShort();
	itemId = dis->readShort();

	data = charArray(dis->readUnsignedShort() & 0xffff);
	dis->readFully(data);
}

void ComplexItemDataPacket::write(DataOutputStream *dos) //throws IOException
{
	dos->writeShort(itemType);
	dos->writeShort(itemId);
	dos->writeUnsignedShort(data.length);

	byteArray ba( (byte*)data.data, data.length );
	dos->write(ba);
}

void ComplexItemDataPacket::handle(PacketListener *listener)
{
	listener->handleComplexItemData( shared_from_this() );
}

int ComplexItemDataPacket::getEstimatedSize()
{
	return 2+2+2+ data.length;
}
