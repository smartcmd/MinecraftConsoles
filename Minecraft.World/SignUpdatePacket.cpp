#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "SignUpdatePacket.h"



SignUpdatePacket::SignUpdatePacket()
{
	shouldDelay = true;
	m_bVerified=false;
	m_bCensored=false;
	x = 0;
	y = 0;
	z = 0;

}

SignUpdatePacket::SignUpdatePacket(int x, int y, int z, bool bVerified, bool bCensored, wstring lines[]) 
{
	shouldDelay = true;
	this->m_bVerified=bVerified;
	this->m_bCensored=bCensored;
	this->x = x;
	this->y = y;
	this->z = z;
	for( int i = 0; i < MAX_SIGN_LINES; i++ ) this->lines[i] = lines[i];
}

void SignUpdatePacket::read(DataInputStream *dis) //throws IOException
{
	x = dis->readInt();
	y = dis->readShort();
	z = dis->readInt();
	this->m_bVerified=dis->readBoolean();
	this->m_bCensored=dis->readBoolean();;
	for (int i = 0; i < MAX_SIGN_LINES; i++)
		lines[i] = readUtf(dis, SignTileEntity::MAX_LINE_LENGTH);
}

void SignUpdatePacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeInt(x);
	dos->writeShort(y);
	dos->writeInt(z);
	dos->writeBoolean(m_bVerified);
	dos->writeBoolean(m_bCensored);
	for (int i = 0; i < MAX_SIGN_LINES; i++)
		writeUtf(lines[i], dos);
}

void SignUpdatePacket::handle(PacketListener *listener)
{
	listener->handleSignUpdate(shared_from_this());
}

int SignUpdatePacket::getEstimatedSize() 
{
	int l = 0;
	l+=sizeof(int);
	l+=sizeof(short);
	l+=sizeof(int);
	l+=sizeof(byte);
	l+=sizeof(byte);

	for (int i = 0; i < MAX_SIGN_LINES; i++)
		l += static_cast<int>(lines[i].length());
	return l;
}
