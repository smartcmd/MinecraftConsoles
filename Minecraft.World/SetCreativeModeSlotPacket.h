#pragma once

#include "Packet.h"

class SetCreativeModeSlotPacket : public Packet, public enable_shared_from_this<SetCreativeModeSlotPacket>
{
	public:
		int slotNum;
		shared_ptr<ItemInstance> item;

		SetCreativeModeSlotPacket();
		SetCreativeModeSlotPacket(int slotNum, shared_ptr<ItemInstance> item);

		virtual void handle(PacketListener *listener);
		virtual void read(DataInputStream *dis);
		virtual void write(DataOutputStream *dos);
		virtual int getEstimatedSize();


public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new SetCreativeModeSlotPacket()); }
	virtual int getId() { return 107; }
};