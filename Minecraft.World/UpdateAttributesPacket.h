#pragma once

#include "Packet.h"

class AttributeModifier;
class AttributeInstance;

class UpdateAttributesPacket : public Packet, public enable_shared_from_this<UpdateAttributesPacket>
{
public:
	class AttributeSnapshot
	{
	private:
		eATTRIBUTE_ID id;
		double base;
		unordered_set<AttributeModifier *> modifiers;

	public:
		AttributeSnapshot(eATTRIBUTE_ID id, double base, unordered_set<AttributeModifier *> *modifiers);
		~AttributeSnapshot();

		eATTRIBUTE_ID getId();
		double getBase();
		unordered_set<AttributeModifier *> *getModifiers();
	};

private:
	int entityId;
	unordered_set<AttributeSnapshot *> attributes;

public:
	UpdateAttributesPacket();
	UpdateAttributesPacket(int entityId, unordered_set<AttributeInstance *> *values);
	~UpdateAttributesPacket();

	void read(DataInputStream *dis);
	void write(DataOutputStream *dos);
	void handle(PacketListener *listener);
	int getEstimatedSize();
	int getEntityId();
	unordered_set<AttributeSnapshot *> getValues();

public:
	static shared_ptr<Packet> create() { return std::make_shared<UpdateAttributesPacket>(); }
	virtual int getId() { return 44; }
};