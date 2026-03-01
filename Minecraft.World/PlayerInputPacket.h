#pragma once
using namespace std;

#include "Packet.h"

class PlayerInputPacket : public Packet, public enable_shared_from_this<PlayerInputPacket>
{

private:
	float xxa;
    float yya;
    bool isJumpingVar;
    bool isSneakingVar;

public:
	PlayerInputPacket();
	PlayerInputPacket(float xxa, float yya, bool isJumpingVar, bool isSneakingVar);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

	float getXxa();
	float getYya();
	bool isJumping();
	bool isSneaking();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new PlayerInputPacket()); }
	virtual int getId() { return 27; }
};