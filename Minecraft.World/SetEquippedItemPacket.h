#pragma once
using namespace std;

#include "Packet.h"

class SetEquippedItemPacket : public Packet, public enable_shared_from_this<SetEquippedItemPacket>
{
public:
	int entity;
    int slot;

private:
	// 4J Stu - Brought forward from 1.3 to fix #64688 - Customer Encountered: TU7: Content: Art: Aura of enchanted item is not displayed for other players in online game
	shared_ptr<ItemInstance> item;

public:
	SetEquippedItemPacket();
	SetEquippedItemPacket(int entity, int slot, shared_ptr<ItemInstance> item);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(shared_ptr<Packet> packet);
	
	// 4J Stu - Brought forward from 1.3 to fix #64688 - Customer Encountered: TU7: Content: Art: Aura of enchanted item is not displayed for other players in online game
	shared_ptr<ItemInstance> getItem();

public:
	static shared_ptr<Packet> create() { return std::make_shared<SetEquippedItemPacket>(); }
	virtual int getId() { return 5; }
};