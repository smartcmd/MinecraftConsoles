#pragma once

class MobEffectInstance;

#include "Packet.h"

class RemoveMobEffectPacket : public Packet, public enable_shared_from_this<RemoveMobEffectPacket>
{
	public:
		int entityId;
		char effectId;

		RemoveMobEffectPacket();
		RemoveMobEffectPacket(int entityId, MobEffectInstance *effect);

		virtual void read(DataInputStream *dis);
		virtual void write(DataOutputStream *dos);
		virtual void handle(PacketListener *listener);
		virtual int getEstimatedSize();
public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new RemoveMobEffectPacket()); }
	virtual int getId() { return 42; }
};