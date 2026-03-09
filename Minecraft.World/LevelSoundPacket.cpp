#include "stdafx.h"
#include "BasicTypeContainers.h"
#include "InputOutputStream.h"
#include "net.minecraft.network.packet.h"
#include "LevelSoundPacket.h"

const float LevelSoundPacket::PITCH_ACCURACY = Byte::MAX_VALUE / 2.0f;
const float LevelSoundPacket::LOCATION_ACCURACY = 8.0f;

LevelSoundPacket::LevelSoundPacket()
{
	sound = 0;
	x = 0;
	y = Integer::MAX_VALUE;
	z = 0;
	volume = 0.0f;
	pitch = 0;
}

LevelSoundPacket::LevelSoundPacket(int sound, double x, double y, double z, float volume, float pitch)
{
	this->sound = sound;
	this->x = static_cast<int>(x * LOCATION_ACCURACY);
	this->y = static_cast<int>(y * LOCATION_ACCURACY);
	this->z = static_cast<int>(z * LOCATION_ACCURACY);
	this->volume = volume;
	// 4J-PB - Let's make the pitch a float so it doesn't get mangled and make the noteblock people unhappy
	//this->pitch = (int) (pitch * PITCH_ACCURACY);
	this->pitch = pitch;

// 	if (this->pitch < 0) this->pitch = 0;
// 	if (this->pitch > 255) this->pitch = 255;
}

void LevelSoundPacket::read(DataInputStream *dis)
{
	sound = dis->readInt();
	x = dis->readInt();
	y = dis->readInt();
	z = dis->readInt();
	volume = dis->readFloat();
	//pitch = dis->readUnsignedByte();
	pitch = dis->readFloat();
}

void LevelSoundPacket::write(DataOutputStream *dos)
{
	dos->writeInt(sound);
	dos->writeInt(x);
	dos->writeInt(y);
	dos->writeInt(z);
	dos->writeFloat(volume);
	//dos->writeByte(pitch);
	dos->writeFloat(pitch);
}

int LevelSoundPacket::getSound()
{
	return sound;
}

double LevelSoundPacket::getX()
{
	return x / LOCATION_ACCURACY;
}

double LevelSoundPacket::getY()
{
	return y / LOCATION_ACCURACY;
}

double LevelSoundPacket::getZ()
{
	return z / LOCATION_ACCURACY;
}

float LevelSoundPacket::getVolume()
{
	return volume;
}

float LevelSoundPacket::getPitch()
{
	//return pitch / PITCH_ACCURACY;
	return pitch;
}

void LevelSoundPacket::handle(PacketListener *listener)
{
	listener->handleSoundEvent(shared_from_this());
}

int LevelSoundPacket::getEstimatedSize()
{
	return 4 * 6;
}
