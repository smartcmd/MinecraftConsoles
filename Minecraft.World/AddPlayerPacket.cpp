#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "PacketListener.h"
#include "AddPlayerPacket.h"



AddPlayerPacket::AddPlayerPacket()
{
	id = -1;
	name = L"";
	x = 0;
	y = 0;
	z = 0;
	yRot = 0;
	xRot = 0;
	carriedItem = 0;
	xuid = INVALID_XUID;
	m_playerIndex = 0;
	m_skinId = 0;
	m_capeId = 0;
	m_uiGamePrivileges = 0;
	entityData = nullptr;
	unpack = NULL;
}

AddPlayerPacket::~AddPlayerPacket()
{
	if(unpack != NULL) delete unpack;
}

AddPlayerPacket::AddPlayerPacket(shared_ptr<Player> player, PlayerUID xuid, PlayerUID OnlineXuid,int xp, int yp, int zp, int yRotp, int xRotp, int yHeadRotp) 
{
	id = player->entityId;
	name = player->name;

	// 4J Stu - Send "previously sent" value of position as well so that we stay in sync
	x = xp;//Mth::floor(player->x * 32);
	y = yp;//Mth::floor(player->y * 32);
	z = zp;//Mth::floor(player->z * 32);
	// 4J - changed - send current "previously sent" value of rotations to put this in sync with other clients
	yRot = yRotp;
	xRot = xRotp;
	yHeadRot = yHeadRotp; // 4J Added
	//    yRot = (byte) (player->yRot * 256 / 360);
	//    xRot = (byte) (player->xRot * 256 / 360);

	//printf("%d: New add player (%f,%f,%f) : (%d,%d,%d) : xRot %d, yRot %d\n",id,player->x,player->y,player->z,x,y,z,xRot,yRot);

	shared_ptr<ItemInstance> itemInstance = player->inventory->getSelected();
	carriedItem = itemInstance == NULL ? 0 : itemInstance->id;

	this->xuid = xuid;
	this->OnlineXuid = OnlineXuid;
	m_playerIndex = (BYTE)player->getPlayerIndex();
	m_skinId = player->getCustomSkin();
	m_capeId = player->getCustomCape();
	m_uiGamePrivileges = player->getAllPlayerGamePrivileges();

	entityData = player->getEntityData();
	unpack = NULL;
}

void AddPlayerPacket::read(DataInputStream *dis) //throws IOException
{
	id = dis->readInt();
	name = readUtf(dis, Player::MAX_NAME_LENGTH);
	x = dis->readInt();
	y = dis->readInt();
	z = dis->readInt();
	yRot = dis->readByte();
	xRot = dis->readByte();	
	yHeadRot = dis->readByte(); // 4J Added
	carriedItem = dis->readShort();
	xuid = dis->readPlayerUID();
	OnlineXuid = dis->readPlayerUID();
	m_playerIndex = dis->readByte();
	INT skinId = dis->readInt();
	m_skinId = *(DWORD *)&skinId;	
	INT capeId = dis->readInt();
	m_capeId = *(DWORD *)&capeId;
	INT privileges = dis->readInt();
	m_uiGamePrivileges = *(unsigned int *)&privileges;
	MemSect(1);
	unpack = SynchedEntityData::unpack(dis);
	MemSect(0);
}

void AddPlayerPacket::write(DataOutputStream *dos) //throws IOException
{
	dos->writeInt(id);
	writeUtf(name, dos);
	dos->writeInt(x);
	dos->writeInt(y);
	dos->writeInt(z);
	dos->writeByte(yRot);
	dos->writeByte(xRot);
	dos->writeByte(yHeadRot); // 4J Added
	dos->writeShort(carriedItem);
	dos->writePlayerUID(xuid);
	dos->writePlayerUID(OnlineXuid);
	dos->writeByte(m_playerIndex);
	dos->writeInt(m_skinId);
	dos->writeInt(m_capeId);
	dos->writeInt(m_uiGamePrivileges);
	entityData->packAll(dos);

}

void AddPlayerPacket::handle(PacketListener *listener)
{
	listener->handleAddPlayer(shared_from_this());
}

int AddPlayerPacket::getEstimatedSize()
{
	int iSize= sizeof(int) + Player::MAX_NAME_LENGTH + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(BYTE) + sizeof(BYTE) +sizeof(short) + sizeof(PlayerUID) + sizeof(PlayerUID) + sizeof(int) + sizeof(BYTE) + sizeof(unsigned int) + sizeof(byte);

	if( entityData != NULL )
	{
		iSize += entityData->getSizeInBytes();
	}
	else if( unpack != NULL )
	{
		// 4J Stu - This is an incoming value which we aren't currently analysing
		//iSize += unpack->get
	}

	return iSize;
}

vector<shared_ptr<SynchedEntityData::DataItem> > *AddPlayerPacket::getUnpackedData() 
{
	if (unpack == NULL)
	{
		unpack = entityData->getAll();
	}
	return unpack;
}
