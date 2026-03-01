#pragma once
using namespace std;

#include "Packet.h"

class ChatPacket : public Packet, public enable_shared_from_this<ChatPacket>
{
	// longest allowed string is "<" + name + "> " + message
private:
	static const unsigned int MAX_LENGTH;

public:
	// 4J - We want to be able to localise the messages sent. The enum also allows for the posibility that there
	// may be different versions playing the game, so the enum should map to a string id which may be different on
	// different versions
	enum EChatPacketMessage
	{
		e_ChatCustom = 0, // No localised string, only the text passed in
		e_ChatBedOccupied,
		e_ChatBedNoSleep,
		e_ChatBedNotValid,
		e_ChatBedNotSafe,
		e_ChatBedPlayerSleep,
		e_ChatBedMeSleep,
		e_ChatPlayerLeftGame,
		e_ChatPlayerJoinedGame,
		e_ChatPlayerKickedFromGame,
		e_ChatCannotPlaceLava,

		e_ChatDeathInFire,
		e_ChatDeathOnFire,
		e_ChatDeathLava,
		e_ChatDeathInWall,
		e_ChatDeathDrown,
		e_ChatDeathStarve,
		e_ChatDeathCactus,
		e_ChatDeathFall,
		e_ChatDeathOutOfWorld,
		e_ChatDeathGeneric,
		e_ChatDeathExplosion,
		e_ChatDeathMagic,
		e_ChatDeathMob,
		e_ChatDeathPlayer,
		e_ChatDeathArrow,
		e_ChatDeathFireball,
		e_ChatDeathThrown,
		e_ChatDeathIndirectMagic,
		e_ChatDeathDragonBreath,
		e_ChatDeathAnvil,
		e_ChatDeathFallingBlock,
		e_ChatDeathThorns,

		e_ChatDeathFellAccidentLadder,
		e_ChatDeathFellAccidentVines,
		e_ChatDeathFellAccidentWater,
		e_ChatDeathFellAccidentGeneric,
		e_ChatDeathFellKiller,
		e_ChatDeathFellAssist,
		e_ChatDeathFellAssistItem,
		e_ChatDeathFellFinish,
		e_ChatDeathFellFinishItem,
		e_ChatDeathInFirePlayer,
		e_ChatDeathOnFirePlayer,
		e_ChatDeathLavaPlayer,
		e_ChatDeathDrownPlayer,
		e_ChatDeathCactusPlayer,
		e_ChatDeathExplosionPlayer,
		e_ChatDeathWither,
		e_ChatDeathPlayerItem,
		e_ChatDeathArrowItem,
		e_ChatDeathFireballItem,
		e_ChatDeathThrownItem,
		e_ChatDeathIndirectMagicItem,

		e_ChatPlayerEnteredEnd,
		e_ChatPlayerLeftEnd,

		e_ChatPlayerMaxPigsSheepCows, // tell the players they can't use the spawn egg
		e_ChatPlayerMaxChickens, // tell the players they can't use the spawn egg
		e_ChatPlayerMaxSquid, // tell the players they can't use the spawn egg
		e_ChatPlayerMaxMooshrooms, // tell the players they can't use the spawn egg
		e_ChatPlayerMaxWolves, // tell the players they can't use the spawn egg
		e_ChatPlayerMaxAnimals, // tell the players they can't use the spawn egg
		e_ChatPlayerMaxEnemies, // tell the players they can't use the spawn egg
		e_ChatPlayerMaxVillagers, // tell the players they can't use the spawn egg
		e_ChatPlayerMaxHangingEntities, // tell the players they hit the picture/itemframe limit
		e_ChatPlayerCantSpawnInPeaceful, // Tell the player they can't spawn enemies in peaceful mode
		e_ChatPlayerMaxBredAnimals, // Tell the player they can't put this animal in love mode because no breeding can be done
		e_ChatPlayerMaxBredPigsSheepCows, // Tell the player they can't put this animal in love mode because no breeding can be done
		e_ChatPlayerMaxBredChickens, // Tell the player they can't put this animal in love mode because no breeding can be done
		e_ChatPlayerMaxBredMooshrooms, // Tell the player they can't put this animal in love mode because no breeding can be done
		e_ChatPlayerMaxBredWolves, // Tell the player they can't put this wolf in love mode because no breeding can be done
		e_ChatPlayerCantShearMooshroom, // Tell the player they can't shear because the limits have been reached
		e_ChatPlayerMaxBoats,
		e_ChatPlayerMaxBats,

		e_ChatCommandTeleportSuccess,
		e_ChatCommandTeleportMe,
		e_ChatCommandTeleportToMe,

	};

public:
	vector<wstring> m_stringArgs;
	vector<int> m_intArgs;
	EChatPacketMessage m_messageType;

	ChatPacket();

	// 4J: Seperated the one convoluted ctor into three more readable ctors. The last two ctors are only used for death messages and I'd really
	// like to consolodate them and/or the logic that uses them at some point.
	ChatPacket(const wstring& message, EChatPacketMessage type = e_ChatCustom, int customData = -1);
	ChatPacket(const wstring& message, EChatPacketMessage type, int sourceEntityType, const wstring& sourceName); 
	ChatPacket(const wstring& message, EChatPacketMessage type, int sourceEntityType, const wstring& sourceName, const wstring& itemName);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new ChatPacket()); }
	virtual int getId() { return 3; }
};

