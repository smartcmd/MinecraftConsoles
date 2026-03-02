#pragma once
using namespace std;

#include "Packet.h"

class MoveEntityPacket : public Packet, public enable_shared_from_this<MoveEntityPacket>
{

	// 4J JEV, static inner/sub classes
public:
	class PosRot;
	class Pos;
	class Rot;

	int id;
	char xa, ya, za, yRot, xRot;
	bool hasRot;

	MoveEntityPacket();
	MoveEntityPacket(int id);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	virtual bool canBeInvalidated();
	virtual bool isInvalidatedBy(shared_ptr<Packet> packet);

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new MoveEntityPacket()); }
	virtual int getId() { return 30; }
};

class MoveEntityPacket::PosRot : public MoveEntityPacket
{
public:
	PosRot();
	PosRot(int id, char xa, char ya, char za, char yRot, char xRot);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new MoveEntityPacket::PosRot()); }
	virtual int getId() { return 33; }
};

class MoveEntityPacket::Pos : public MoveEntityPacket
{
public:
	Pos();
	Pos(int id, char xa, char ya, char za);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new MoveEntityPacket::Pos()); }
	virtual int getId() { return 31; }
};

class MoveEntityPacket::Rot : public MoveEntityPacket
{
public:
	Rot();
	Rot(int id, char yRot, char xRot);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new MoveEntityPacket::Rot()); }
	virtual int getId() { return 32; }

};