#pragma once

#include "Packet.h"

class Abilities;

class PlayerAbilitiesPacket : public Packet, public enable_shared_from_this<PlayerAbilitiesPacket>
{
private:
	static const int FLAG_INVULNERABLE = 1 << 0;
	static const int FLAG_FLYING = 1 << 1;
	static const int FLAG_CAN_FLY = 1 << 2;
	static const int FLAG_INSTABUILD = 1 << 3;

	bool invulnerable;
	bool _isFlying;
	bool _canFly;
	bool instabuild;
	float flyingSpeed;
	float walkingSpeed;

public:
	PlayerAbilitiesPacket();
	PlayerAbilitiesPacket(Abilities *abilities);

	void read(DataInputStream *dis);
	void write(DataOutputStream *dos);
	void handle(PacketListener *listener);
	int getEstimatedSize();
	//wstring getDebugInfo();
	bool isInvulnerable();
	void setInvulnerable(bool invulnerable);
	bool isFlying();
	void setFlying(bool flying);
	bool canFly();
	void setCanFly(bool canFly);
	bool canInstabuild();
	void setInstabuild(bool instabuild);
	float getFlyingSpeed();
	void setFlyingSpeed(float flySpeed);
	float getWalkingSpeed();
	void setWalkingSpeed(float walkingSpeed);
	bool canBeInvalidated();
	bool isInvalidatedBy(shared_ptr<Packet> packet);

public:
	static shared_ptr<Packet> create() { return std::make_shared<PlayerAbilitiesPacket>(); }
	virtual int getId() { return 202; }
};