#pragma once
using namespace std;

#include "Packet.h"

class AddEntityPacket : public Packet, public enable_shared_from_this<AddEntityPacket>
{
public:
	static const int BOAT = 1;
	static const int ITEM = 2;
    static const int MINECART_RIDEABLE = 10;
    static const int MINECART_CHEST = 11;
    static const int MINECART_FURNACE = 12;
    static const int PRIMED_TNT = 50;
	static const int ENDER_CRYSTAL = 51;
    static const int ARROW = 60;
    static const int SNOWBALL = 61;
    static const int EGG = 62;
    static const int FIREBALL = 63;
	static const int SMALL_FIREBALL = 64;
	static const int THROWN_ENDERPEARL = 65;

    static const int FALLING = 70;
	static const int ITEM_FRAME = 71;
	static const int EYEOFENDERSIGNAL = 72;
	static const int THROWN_POTION = 73;
	static const int FALLING_EGG = 74;
	static const int THROWN_EXPBOTTLE = 75;

	static const int FISH_HOOK = 90;

	// 4J Added TU9
	static const int DRAGON_FIRE_BALL = 200;

    int id;
    int x, y, z;
    int xa, ya, za;
    int type;
    int data;
	byte yRot,xRot;	// 4J added

private:
	void _init(shared_ptr<Entity> e, int type, int data, int xp, int yp, int zp, int yRotp, int xRotp );

public:
	AddEntityPacket();
    AddEntityPacket(shared_ptr<Entity> e, int type, int yRotp, int xRotp, int xp, int yp, int zp);
	AddEntityPacket(shared_ptr<Entity> e, int type, int data, int yRotp, int xRotp, int xp, int yp, int zp );

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new AddEntityPacket()); }
	virtual int getId() { return 23; }
};