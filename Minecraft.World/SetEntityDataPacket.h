#pragma once
using namespace std;

#include "Packet.h"
#include "SynchedEntityData.h"

class SetEntityDataPacket : public Packet, public enable_shared_from_this<SetEntityDataPacket>
{
public:
	int id;

private:
	vector<shared_ptr<SynchedEntityData::DataItem> > *packedItems;

public:
	SetEntityDataPacket();
	~SetEntityDataPacket();
	SetEntityDataPacket(int id, shared_ptr<SynchedEntityData>, bool notJustDirty);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

	vector<shared_ptr<SynchedEntityData::DataItem> > *getUnpackedData();

public:
	static shared_ptr<Packet> create() { return std::make_shared<SetEntityDataPacket>(); }
	virtual int getId() { return 40; }
};