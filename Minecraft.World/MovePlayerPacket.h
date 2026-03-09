#pragma once
using namespace std;

#include "Packet.h"

class MovePlayerPacket : public Packet, public enable_shared_from_this<MovePlayerPacket>
{
public:
	class PosRot;
	class Pos;
	class Rot;

	double x, y, z, yView;
	float yRot, xRot;
	bool onGround;
	bool hasPos, hasRot;
	bool isFlying; // 4J Added

	MovePlayerPacket();
	MovePlayerPacket(bool onGround, bool isFlying);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();
	virtual bool canBeInvalidated();
	virtual bool isInvalidatedBy(shared_ptr<Packet> packet);

public:
	static shared_ptr<Packet> create() { return std::make_shared<MovePlayerPacket>(); }
	virtual int getId() { return 10; }
};

class MovePlayerPacket::PosRot : public MovePlayerPacket
{
public:
	PosRot();
	PosRot(double x, double y, double yView, double z, float yRot, float xRot, bool onGround, bool isFlying);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<MovePlayerPacket::PosRot>(); }
	virtual int getId() { return 13; }
};

class MovePlayerPacket::Pos : public MovePlayerPacket
{
public:
	Pos();
	Pos(double x, double y, double yView, double z, bool onGround, bool isFlying);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<MovePlayerPacket::Pos>(); }
	virtual int getId() { return 11; }

};

class MovePlayerPacket::Rot : public MovePlayerPacket
{
public:
	Rot();
	Rot(float yRot, float xRot, bool onGround, bool isFlying);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();
public:
	static shared_ptr<Packet> create() { return std::make_shared<MovePlayerPacket::Rot>(); }
	virtual int getId() { return 12; }

};