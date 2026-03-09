#pragma once
using namespace std;

#include "Packet.h"

class MoveEntityPacketSmall : public Packet, public enable_shared_from_this<MoveEntityPacketSmall>
{

	// 4J JEV, static inner/sub classes
public:
	class PosRot;
	class Pos;
	class Rot;

	int id;
    char xa, ya, za, yRot, xRot;
	bool hasRot;

	MoveEntityPacketSmall();
	MoveEntityPacketSmall(int id);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	virtual bool canBeInvalidated();
	virtual bool isInvalidatedBy(shared_ptr<Packet> packet);

public:
	static shared_ptr<Packet> create() { return std::make_shared<MoveEntityPacketSmall>(); }
	virtual int getId() { return 162; }
};

class MoveEntityPacketSmall::PosRot : public MoveEntityPacketSmall
{
public:
	PosRot();
	PosRot(int id, char xa, char ya, char za, char yRot, char xRot);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<MoveEntityPacketSmall::PosRot>(); }
	virtual int getId() { return 165; }
};

class MoveEntityPacketSmall::Pos : public MoveEntityPacketSmall
{
public:
	Pos();
	Pos(int id, char xa, char ya, char za);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<MoveEntityPacketSmall::Pos>(); }
	virtual int getId() { return 163; }

};

class MoveEntityPacketSmall::Rot : public MoveEntityPacketSmall
{
public:
	Rot();
	Rot(int id, char yRot, char xRot);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<MoveEntityPacketSmall::Rot>(); }
	virtual int getId() { return 164; }

};