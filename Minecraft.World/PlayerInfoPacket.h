#pragma once
using namespace std;
#include "Packet.h"

class ServerPlayer;

class PlayerInfoPacket : public Packet, public enable_shared_from_this<PlayerInfoPacket>
{
	public:
		// 4J Stu - I have re-purposed this packet for our uses
		//wstring name;
		//bool add;
		//int latency;
		short m_networkSmallId;
		short m_playerColourIndex;
		unsigned int m_playerPrivileges;
		int m_entityId;

		PlayerInfoPacket();
		//PlayerInfoPacket(const wstring &name, bool add, int latency);
		PlayerInfoPacket(BYTE networkSmallId, short playerColourIndex, unsigned int playerPrivileges = 0);
		PlayerInfoPacket(shared_ptr<ServerPlayer> player);

		virtual void read(DataInputStream *dis);
		virtual void write(DataOutputStream *dos);
		virtual void handle(PacketListener *listener);
		virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new PlayerInfoPacket()); }
	virtual int getId() { return 201; }
};