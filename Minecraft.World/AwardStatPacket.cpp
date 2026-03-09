#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "AwardStatPacket.h"



AwardStatPacket::AwardStatPacket()
{
	this->m_paramData.data = nullptr;
	this->m_paramData.length = 0;
}

AwardStatPacket::AwardStatPacket(int statId, int count)
{
	this->statId = statId;

	this->m_paramData.data = (byte *) new int(count);
	this->m_paramData.length = sizeof(int);
}

AwardStatPacket::AwardStatPacket(int statId, byteArray paramData)
{
	this->statId = statId;
	this->m_paramData = paramData;
}

AwardStatPacket::~AwardStatPacket()
{
	if (m_paramData.data != nullptr) 
	{
		delete [] m_paramData.data;
		m_paramData.data = nullptr;
	}
}

void AwardStatPacket::handle(PacketListener *listener) 
{
	listener->handleAwardStat(shared_from_this());
	m_paramData.data = nullptr;
}

void AwardStatPacket::read(DataInputStream *dis) //throws IOException 
{
	statId = dis->readInt();

	// Read parameter blob.
	int length = dis->readInt();
	if(length > 0)
	{
		m_paramData = byteArray(length);
		dis->readFully(m_paramData);
	}
}

void AwardStatPacket::write(DataOutputStream *dos) //throws IOException
{
	dos->writeInt(statId);
	dos->writeInt(m_paramData.length);
	if(m_paramData.length > 0) dos->write(m_paramData);
}

int AwardStatPacket::getEstimatedSize()
{
	return 6;
}

bool AwardStatPacket::isAync()
{
	return true;
}

// On most platforms we only store 'count' in an AwardStatPacket.
int AwardStatPacket::getCount()
{
#ifdef _DURANGO
	assert(false); // Method not supported on Durango.
	return 0;
#else
	return *((int*)this->m_paramData.data);
#endif
}

// On Durango we store 'Event' parameters here in a blob.
byteArray AwardStatPacket::getParamData()
{
	return m_paramData;
}
