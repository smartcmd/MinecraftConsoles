#pragma once

#include "Packet.h"

class LevelSoundPacket : public Packet, public enable_shared_from_this<LevelSoundPacket>
{
public:
	static const float PITCH_ACCURACY;
	static const float LOCATION_ACCURACY;

private:
	int sound;
	int x;
	int y;
	int z;
	float volume;
	//int pitch;
	float pitch;

public:
	LevelSoundPacket();
	LevelSoundPacket(int iSound, double x, double y, double z, float volume, float pitch);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getSound();
	double getX();
	double getY();
	double getZ();
	float getVolume();
	float getPitch();
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new LevelSoundPacket()); }
	virtual int getId() { return 62; }
};
