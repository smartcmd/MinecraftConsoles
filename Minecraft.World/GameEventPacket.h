#pragma once
using namespace std;

#include "Packet.h"

class GameEventPacket : public Packet, public enable_shared_from_this<GameEventPacket>
{

public:
	static const int NO_RESPAWN_BED_AVAILABLE;
	static const int START_RAINING;
	static const int STOP_RAINING;
	static const int CHANGE_GAME_MODE; // 1.8.2
	static const int WIN_GAME; // 1.0.01
	static const int DEMO_EVENT; // 1.3.2
	static const int SUCCESSFUL_BOW_HIT = 6;

	static const int DEMO_PARAM_INTRO; // 1.3.2
	static const int DEMO_PARAM_HINT_1; // 1.3.2
	static const int DEMO_PARAM_HINT_2; // 1.3.2
	static const int DEMO_PARAM_HINT_3; // 1.3.2

	// 4J Added
	static const int START_SAVING;
	static const int STOP_SAVING;

	static const int EVENT_LANGUAGE_ID_LENGTH = 6;
	static const int EVENT_LANGUAGE_ID[EVENT_LANGUAGE_ID_LENGTH];


	int _event;
	int param;

	GameEventPacket();
	GameEventPacket(int evnt, int param);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new GameEventPacket()); }
	virtual int getId() { return 70; }
};