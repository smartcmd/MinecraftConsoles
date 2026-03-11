#pragma once
using namespace std;

#include "Packet.h"
#include "SynchedEntityData.h"

class LivingEntity;

class AddMobPacket : public Packet, public enable_shared_from_this<AddMobPacket>
{
public:
	int id;
	int type;
	int x, y, z;
	int xd, yd, zd;
	byte yRot, xRot, yHeadRot;

private:
	shared_ptr<SynchedEntityData> entityData;
	vector<shared_ptr<SynchedEntityData::DataItem> > *unpack;

public:
	AddMobPacket();
	~AddMobPacket();
	AddMobPacket(shared_ptr<LivingEntity> mob, int yRotp, int xRotp, int xp, int yp, int zp, int yHeadRotp);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

	vector<shared_ptr<SynchedEntityData::DataItem> > *getUnpackedData();

public:
	static shared_ptr<Packet> create() { return std::make_shared<AddMobPacket>(); }
	virtual int getId() { return 24; }
};
