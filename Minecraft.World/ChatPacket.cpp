#include "stdafx.h"
#include <iostream>
#include "net.minecraft.h"
#include "net.minecraft.world.entity.player.h"
#include "PacketListener.h"
#include "ChatPacket.h"

// longest allowed string is "<" + name + "> " + message
const unsigned int ChatPacket::MAX_LENGTH = SharedConstants::maxChatLength + Player::MAX_NAME_LENGTH + 3;

ChatPacket::ChatPacket() 
{
	m_messageType = e_ChatCustom;
}

ChatPacket::ChatPacket(const wstring& message, EChatPacketMessage type /*= e_ChatCustom*/, int customData /*= -1*/)
{
	m_messageType = type;
	if (customData != -1) m_intArgs.push_back(customData);

	m_stringArgs.push_back(message);
}

ChatPacket::ChatPacket(const wstring& message, EChatPacketMessage type, int sourceEntityType, const wstring& sourceName)
{
	m_messageType = type;
	if (sourceEntityType != -1) m_intArgs.push_back(sourceEntityType);

	m_stringArgs.push_back(message);
	m_stringArgs.push_back(sourceName);
}

ChatPacket::ChatPacket(const wstring& message, EChatPacketMessage type, int sourceEntityType, const wstring& sourceName, const wstring& itemName)
{
	m_messageType = type;
	if (sourceEntityType != -1) m_intArgs.push_back(sourceEntityType);

	m_stringArgs.push_back(message);
	m_stringArgs.push_back(sourceName);
	m_stringArgs.push_back(itemName);
}

// Read chat packet (throws IOException)
void ChatPacket::read(DataInputStream *dis) 
{
	m_messageType = static_cast<EChatPacketMessage>(dis->readShort());

	short packedCounts = dis->readShort();
	int stringCount = (packedCounts >> 4) & 0xF;
	int intCount = (packedCounts >> 0) & 0xF;
	
	for(int i = 0; i < stringCount; i++)
	{
		m_stringArgs.push_back(readUtf(dis, MAX_LENGTH));
	}

	for(int i = 0; i < intCount; i++)
	{
		m_intArgs.push_back(dis->readInt());
	}
}

// Write chat packet (throws IOException)
void ChatPacket::write(DataOutputStream *dos)
{
	dos->writeShort(m_messageType);

	short packedCounts = 0;
	packedCounts |= (m_stringArgs.size() & 0xF) << 4;
	packedCounts |= (m_intArgs.size() & 0xF) << 0;

	dos->writeShort(packedCounts);

	for(size_t i = 0; i < m_stringArgs.size(); i++)
	{
		writeUtf(m_stringArgs[i], dos);
	}

	for(size_t i = 0; i < m_intArgs.size(); i++)
	{
		dos->writeInt(m_intArgs[i]);
	}
}

// Handle chat packet
void ChatPacket::handle(PacketListener *listener)
{
	listener->handleChat(shared_from_this());
}

// Get an estimated size of the packet 
int ChatPacket::getEstimatedSize() 
{
	int stringsSize = 0;
	for(size_t i = 0; i < m_stringArgs.size(); i++)
	{
		stringsSize += m_stringArgs[i].length();
	}

	return 
		sizeof(EChatPacketMessage) + // message type 
		sizeof(short) + // packed arg counts 
		stringsSize + // string args
		(m_intArgs.size() * sizeof(int)); // int args
}