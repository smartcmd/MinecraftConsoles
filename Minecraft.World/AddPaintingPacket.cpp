#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.entity.h"
#include "PacketListener.h"
#include "AddPaintingPacket.h"



AddPaintingPacket::AddPaintingPacket()
{
	id = -1;
	x = 0;
	y = 0;
	z = 0;
	dir = 0;
	motive = L"";
}

AddPaintingPacket::AddPaintingPacket(shared_ptr<Painting> e)
{
	id = e->entityId;
	x = e->xTile;
	y = e->yTile;
	z = e->zTile;
	dir = e->dir;
	motive = e->motive->name;
}

void AddPaintingPacket::read(DataInputStream *dis)  //throws IOException
{
	id = dis->readInt();
	motive = readUtf(dis, Painting::Motive::MAX_MOTIVE_NAME_LENGTH);
	x = dis->readInt();
	y = dis->readInt();
	z = dis->readInt();
	dir = dis->readInt();
}

void AddPaintingPacket::write(DataOutputStream *dos) //throws IOException
{
	dos->writeInt(id);
	writeUtf(motive, dos);
	dos->writeInt(x);
	dos->writeInt(y);
	dos->writeInt(z);
	dos->writeInt(dir);
}

void AddPaintingPacket::handle(PacketListener *listener)
{
	listener->handleAddPainting(shared_from_this());
}

int AddPaintingPacket::getEstimatedSize()
{
	return 24;
}
