#pragma once

#include "Packet.h"

class LevelParticlesPacket : public Packet, public enable_shared_from_this<LevelParticlesPacket>
{
private:
	wstring name;
	float x;
	float y;
	float z;
	float xDist;
	float yDist;
	float zDist;
	float maxSpeed;
	int count;

public:
	LevelParticlesPacket();
	LevelParticlesPacket(const wstring &name, float x, float y, float z, float xDist, float yDist, float zDist, float maxSpeed, int count);

	void read(DataInputStream *dis);
	void write(DataOutputStream *dos);
	wstring getName();
	double getX();
	double getY();
	double getZ();
	float getXDist();
	float getYDist();
	float getZDist();
	float getMaxSpeed();
	int getCount();
	void handle(PacketListener *listener);
	int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new LevelParticlesPacket()); }
	virtual int getId() { return 63; }
};