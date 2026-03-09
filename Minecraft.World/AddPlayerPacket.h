#pragma once
using namespace std;

#include "Packet.h"
#include "SynchedEntityData.h"

class Player;

class AddPlayerPacket : public Packet, public enable_shared_from_this<AddPlayerPacket>
{

private:
	shared_ptr<SynchedEntityData> entityData;
    vector<shared_ptr<SynchedEntityData::DataItem> > *unpack;

public:
	int id;
    wstring name;
    int x, y, z;
    char yRot, xRot;
    int carriedItem;
	PlayerUID xuid; // 4J Added
	PlayerUID OnlineXuid; // 4J Added
	BYTE m_playerIndex; // 4J Added
	DWORD m_skinId; // 4J Added
	DWORD m_capeId; // 4J Added
	unsigned int m_uiGamePrivileges; // 4J Added
	byte yHeadRot; // 4J Added

	AddPlayerPacket();
	~AddPlayerPacket();
	AddPlayerPacket(shared_ptr<Player> player, PlayerUID xuid, PlayerUID OnlineXuid,int xp, int yp, int zp, int yRotp, int xRotp, int yHeadRotp);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

	vector<shared_ptr<SynchedEntityData::DataItem> > *getUnpackedData();
public:
	static shared_ptr<Packet> create() { return std::make_shared<AddPlayerPacket>(); }
	virtual int getId() { return 20; }
};
