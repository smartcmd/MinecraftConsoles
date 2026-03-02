#pragma once
using namespace std;

#include "Packet.h"

class PlayerInputPacket : public Packet, public enable_shared_from_this<PlayerInputPacket>
{

private:
	float xa;
    float ya;
    bool isJumpingVar;
    bool isSneakingVar;
    float xRot;
    float yRot;

public:
	PlayerInputPacket();
	PlayerInputPacket(float xa, float ya, bool isJumpingVar, bool isSneakingVar, float xRot, float yRot);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

	float getXa();
	float getXRot();
	float getYa();
	float getYRot();
	bool isJumping();
	bool isSneaking();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new PlayerInputPacket()); }
	virtual int getId() { return 27; }
};