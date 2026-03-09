#pragma once
using namespace std;

#include "Packet.h"

class PreLoginPacket : public Packet, public enable_shared_from_this<PreLoginPacket>
{
    // the login key is username client->server and sessionid server->client
public:
	static const int m_iSaveNameLen=14;
	//4J Added more info to this packet so that we can check if anyone has a UGC privilege that won't let us
	// join, and so that we can inform the server if we have that privilege set. Anyone with UGC turned off completely
	// can't play the game online at all, so we only need to specify players with friends only set
	PlayerUID *m_playerXuids;
	DWORD m_dwPlayerCount;
	BYTE m_friendsOnlyBits;
	DWORD m_ugcPlayersVersion;
	BYTE m_szUniqueSaveName[m_iSaveNameLen]; // added for checking if the level is in the ban list
	DWORD m_serverSettings; // A bitfield of server settings constructed with the MAKE_SERVER_SETTINGS macro
	BYTE m_hostIndex; // Rather than sending the xuid of the host again, send an index into the m_playerXuids array
	DWORD m_texturePackId;
	SHORT m_netcodeVersion;

	wstring loginKey;

	PreLoginPacket();
	PreLoginPacket(wstring userName);
	PreLoginPacket(wstring userName, PlayerUID *playerXuids, DWORD playerCount, BYTE friendsOnlyBits, DWORD ugcPlayersVersion,char *pszUniqueSaveName, DWORD serverSettings, BYTE hostIndex, DWORD texturePackId);
	~PreLoginPacket();

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<PreLoginPacket>(); }
	virtual int getId() { return 2; }
};