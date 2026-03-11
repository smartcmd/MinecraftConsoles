#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.entity.h"
#include "PacketListener.h"
#include "AddMobPacket.h"



AddMobPacket::AddMobPacket()
{
	id = -1;
	type = 0;
	x = 0;
	y = 0;
	z = 0;
	yRot = 0;
	xRot = 0;
	entityData = nullptr;
	unpack = nullptr;
}

AddMobPacket::~AddMobPacket()
{
	delete unpack;
}

AddMobPacket::AddMobPacket(shared_ptr<LivingEntity> mob, int yRotp, int xRotp, int xp, int yp, int zp, int yHeadRotp)
{
	id = mob->entityId;

	type = static_cast<byte>(EntityIO::getId(mob));
	// 4J Stu - We should add entities at their "last sent" position so that the relative update packets
	// put them in the correct place
	x = xp;//Mth::floor(mob->x * 32);
	y = yp;//Mth::floor(mob->y * 32);
	z = zp;//Mth::floor(mob->z * 32);
	// 4J - changed - send current "previously sent" value of rotations to put this in sync with other clients
	yRot = yRotp;
	xRot = xRotp;
	yHeadRot = yHeadRotp;
	//	yRot = (byte) (mob->yRot * 256 / 360);
	//	xRot = (byte) (mob->xRot * 256 / 360);
	//	yHeadRot = (byte) (mob->yHeadRot * 256 / 360);

	// From SetEntityMotionpacket
	double m = 3.9;
	double xd = mob->xd;
	double yd = mob->yd;
	double zd = mob->zd;
	if (xd < -m) xd = -m;
	if (yd < -m) yd = -m;
	if (zd < -m) zd = -m;
	if (xd > m) xd = m;
	if (yd > m) yd = m;
	if (zd > m) zd = m;
	this->xd = static_cast<int>(xd * 8000.0);
	this->yd = static_cast<int>(yd * 8000.0);
	this->zd = static_cast<int>(zd * 8000.0);

	//	printf("%d: New add mob rot %d\n",id,yRot);

	entityData = mob->getEntityData();
	unpack = nullptr;
}

void AddMobPacket::read(DataInputStream *dis) //throws IOException
{
	id = dis->readShort();
	type = dis->readByte() & 0xff;
#ifdef _LARGE_WORLDS
	x = dis->readInt();
	y = dis->readInt();
	z = dis->readInt();
#else
	x = dis->readShort();
	y = dis->readShort();
	z = dis->readShort();
#endif
	yRot = dis->readByte();
	xRot = dis->readByte();
	yHeadRot = dis->readByte();
	xd = dis->readShort();
	yd = dis->readShort();
	zd = dis->readShort();
	MemSect(1);
	unpack = SynchedEntityData::unpack(dis);
	MemSect(0);
}

void AddMobPacket::write(DataOutputStream *dos) //throws IOException
{
	dos->writeShort(id);
	dos->writeByte(type & 0xff);
#ifdef _LARGE_WORLDS
	dos->writeInt(x);
	dos->writeInt(y);
	dos->writeInt(z);
#else
	dos->writeShort(x);
	dos->writeShort(y);
	dos->writeShort(z);
#endif
	dos->writeByte(yRot);
	dos->writeByte(xRot);
	dos->writeByte(yHeadRot);
	dos->writeShort(xd);
	dos->writeShort(yd);
	dos->writeShort(zd);
	entityData->packAll(dos);
}

void AddMobPacket::handle(PacketListener *listener)
{
	listener->handleAddMob(shared_from_this());
}

int AddMobPacket::getEstimatedSize() 
{
	int size = 11;
	if( entityData != nullptr )
	{
		size += entityData->getSizeInBytes();
	}
	else if( unpack != nullptr )
	{
		// 4J Stu - This is an incoming value which we aren't currently analysing
		//size += unpack->get
	}
	return size;
}

vector<shared_ptr<SynchedEntityData::DataItem> > *AddMobPacket::getUnpackedData() 
{
	if (unpack == nullptr)
	{
		unpack = entityData->getAll();
	}
	return unpack;
}
