#pragma once
using namespace std;

#include "Packet.h"

class DisconnectPacket : public Packet, public enable_shared_from_this<DisconnectPacket>
{
public:

	enum eDisconnectReason
	{
		eDisconnect_None = 0,
		eDisconnect_Quitting,
		eDisconnect_Closed,

		eDisconnect_LoginTooLong,
		eDisconnect_IllegalStance,
		eDisconnect_IllegalPosition,
		eDisconnect_MovedTooQuickly,
		eDisconnect_NoFlying,
		eDisconnect_Kicked,

		eDisconnect_TimeOut,
		eDisconnect_Overflow,
		eDisconnect_EndOfStream,
		eDisconnect_ServerFull,
		eDisconnect_OutdatedServer,
		eDisconnect_OutdatedClient,
		eDisconnect_UnexpectedPacket,

		eDisconnect_ConnectionCreationFailed,
		eDisconnect_NoMultiplayerPrivilegesHost,
		eDisconnect_NoMultiplayerPrivilegesJoin,

		eDisconnect_NoUGC_AllLocal,
		eDisconnect_NoUGC_Single_Local,
		eDisconnect_ContentRestricted_AllLocal,
		eDisconnect_ContentRestricted_Single_Local,
#ifndef __PS3__
		eDisconnect_NoUGC_Remote,
#endif

		eDisconnect_NoFriendsInGame,
		eDisconnect_Banned,
		eDisconnect_NotFriendsWithHost,
		eDisconnect_NATMismatch,
#ifdef __ORBIS__
		eDisconnect_NetworkError,
#endif
#ifdef _XBOX_ONE
		eDisconnect_ExitedGame,
#endif
	};

	// 4J Stu - The reason was a string, but we need to send a non-locale specific reason
	eDisconnectReason reason;

	DisconnectPacket();
	DisconnectPacket(eDisconnectReason reason);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	virtual bool canBeInvalidated();
	virtual bool isInvalidatedBy(shared_ptr<Packet> packet);

public:
	static shared_ptr<Packet> create() { return std::make_shared<DisconnectPacket>(); }
	virtual int getId() { return 255; }
};


