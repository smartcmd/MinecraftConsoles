#include "stdafx.h"
#include <iostream>
#include "PacketListener.h"
#include "PreloginPacket.h"
#include "InputOutputStream.h"



PreLoginPacket::PreLoginPacket() 
{
	loginKey = L"";
	m_playerXuids = nullptr;
	m_dwPlayerCount = 0;
	m_friendsOnlyBits = 0;
	m_ugcPlayersVersion = 0;
	ZeroMemory(m_szUniqueSaveName,m_iSaveNameLen);
	m_serverSettings = 0;
	m_hostIndex = 0;
	m_texturePackId = 0;
	m_netcodeVersion = 0;
}

PreLoginPacket::PreLoginPacket(wstring userName) 
{
	this->loginKey = userName;
	m_playerXuids = nullptr;
	m_dwPlayerCount = 0;
	m_friendsOnlyBits = 0;
	m_ugcPlayersVersion = 0;
	ZeroMemory(m_szUniqueSaveName,m_iSaveNameLen);
	m_serverSettings = 0;
	m_hostIndex = 0;
	m_texturePackId = 0;
	m_netcodeVersion = 0;
}

PreLoginPacket::PreLoginPacket(wstring userName, PlayerUID *playerXuids, DWORD playerCount, BYTE friendsOnlyBits, DWORD ugcPlayersVersion,char *pszUniqueSaveName, DWORD serverSettings, BYTE hostIndex, DWORD texturePackId) 
{
	this->loginKey = userName;
	m_playerXuids = playerXuids;
	m_dwPlayerCount = playerCount;
	m_friendsOnlyBits = friendsOnlyBits;
	m_ugcPlayersVersion = ugcPlayersVersion;
	memcpy(m_szUniqueSaveName,pszUniqueSaveName,m_iSaveNameLen);
	m_serverSettings = serverSettings;
	m_hostIndex = hostIndex;
	m_texturePackId = texturePackId;
	m_netcodeVersion = 0;
}

PreLoginPacket::~PreLoginPacket()
{
	if( m_playerXuids != nullptr ) delete [] m_playerXuids;
}

void PreLoginPacket::read(DataInputStream *dis) //throws IOException 
{
	m_netcodeVersion = dis->readShort();

	loginKey = readUtf(dis, 32);

	m_friendsOnlyBits = dis->readByte();
	m_ugcPlayersVersion = dis->readInt();
	m_dwPlayerCount = dis->readByte();
	if( m_dwPlayerCount > MINECRAFT_NET_MAX_PLAYERS ) m_dwPlayerCount = MINECRAFT_NET_MAX_PLAYERS;
	if( m_dwPlayerCount > 0 )
	{
		m_playerXuids = new PlayerUID[m_dwPlayerCount];
		for(DWORD i = 0; i < m_dwPlayerCount; ++i)
		{
			m_playerXuids[i] = dis->readPlayerUID();
		}
	}
	for(DWORD i = 0; i < m_iSaveNameLen; ++i)
	{
		m_szUniqueSaveName[i]=dis->readByte();
	}
	// m_szUniqueSaveName[m_iSaveNameLen - 1] = 0; // LCEMP does this but I have no idea why, TODO: why?
	m_serverSettings = dis->readInt();
	m_hostIndex = dis->readByte();
	
	INT texturePackId = dis->readInt();
	m_texturePackId = *(DWORD *)&texturePackId;

	// Set the name of the map so we can check it for players banned lists
	app.SetUniqueMapName((char *)m_szUniqueSaveName);
}

void PreLoginPacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeShort(MINECRAFT_NET_VERSION);

	writeUtf(loginKey, dos);
	
	dos->writeByte(m_friendsOnlyBits);
	dos->writeInt(m_ugcPlayersVersion);
	dos->writeByte(static_cast<byte>(m_dwPlayerCount));
	for(DWORD i = 0; i < m_dwPlayerCount; ++i)
	{
		dos->writePlayerUID( m_playerXuids[i] );
	}

	app.DebugPrintf("*** PreLoginPacket::write - %s\n",m_szUniqueSaveName);
	for(DWORD i = 0; i < m_iSaveNameLen; ++i)
	{
		dos->writeByte(m_szUniqueSaveName[i]);
	}
	dos->writeInt(m_serverSettings);
	dos->writeByte(m_hostIndex);
	dos->writeInt(m_texturePackId);
}

void PreLoginPacket::handle(PacketListener *listener) 
{
	listener->handlePreLogin(shared_from_this());
}

int PreLoginPacket::getEstimatedSize() 
{
	return 4 + 4 + static_cast<int>(loginKey.length()) + 4 +14 + 4 + 1 + 4;
}
