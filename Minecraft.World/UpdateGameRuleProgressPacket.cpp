#include "stdafx.h"
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "UpdateGameRuleProgressPacket.h"



// UpdateGameRuleProgressPacket() default constructor
UpdateGameRuleProgressPacket::UpdateGameRuleProgressPacket() : m_icon(0), m_auxValue(0), m_dataTag(0)
{
}

UpdateGameRuleProgressPacket::UpdateGameRuleProgressPacket(ConsoleGameRules::EGameRuleType definitionType, const wstring &messageId, int icon, int auxValue, int dataTag, void *data, int dataLength)
{
	m_definitionType = definitionType;
	m_messageId = messageId;
	m_icon = icon;
	m_auxValue = auxValue;
	m_dataTag = dataTag;

	if (dataLength > 0 && dataLength <= 65536)
	{
		m_data = byteArray(dataLength);
		memcpy(m_data.data,data,dataLength);
	}
	else
	{
		m_data = byteArray();
	}
}

void UpdateGameRuleProgressPacket::read(DataInputStream *dis) //throws IOException 
{
	m_definitionType = static_cast<ConsoleGameRules::EGameRuleType>(dis->readInt());
	m_messageId = readUtf(dis,64);
	m_icon = dis->readInt();
	m_auxValue = dis->readByte();
	m_dataTag = dis->readInt();
	int dataLength = dis->readInt();

	if(dataLength > 0)
	{
		m_data = byteArray(dataLength);
		dis->readFully(m_data);
	}
	else
	{
		m_data = byteArray();
	}
}

void UpdateGameRuleProgressPacket::write(DataOutputStream *dos) //throws IOException
{
	dos->writeInt(m_definitionType);
	writeUtf(m_messageId,dos);
	dos->writeInt(m_icon);
	dos->writeByte(m_auxValue);
	dos->writeInt(m_dataTag);
	dos->writeInt(m_data.length);
	dos->write(m_data);
}

void UpdateGameRuleProgressPacket::handle(PacketListener *listener)
{
	listener->handleUpdateGameRuleProgressPacket(shared_from_this());
}

int UpdateGameRuleProgressPacket::getEstimatedSize()
{
	return static_cast<int>(m_messageId.length()) + 4 + m_data.length;
}