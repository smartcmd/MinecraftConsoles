#pragma once
using namespace std;

#include "TilePos.h"
#include "Packet.h"

class ExplodePacket : public Packet, public enable_shared_from_this<ExplodePacket>
{
public:
	double x, y, z;
    float r;
    vector<TilePos> toBlow;		// 4J - was an unorderedset but doesn't require any features of that apart from making it match the ctor toBlow type
	bool m_bKnockbackOnly;
	
	private:
		float knockbackX;
		float knockbackY;
		float knockbackZ;

	public:

	ExplodePacket();
	ExplodePacket(double x, double y, double z, float r, unordered_set<TilePos, TilePosKeyHash, TilePosKeyEq> *toBlow, Vec3 *knockback, bool knockBackOnly);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

	float getKnockbackX();
	float getKnockbackY();
	float getKnockbackZ();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new ExplodePacket()); }
	virtual int getId() { return 60; }
};