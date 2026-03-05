#include "stdafx.h"

#include "net.minecraft.world.entity.ai.attributes.h"
#include "PacketListener.h"
#include "UpdateAttributesPacket.h"

UpdateAttributesPacket::UpdateAttributesPacket()
{
	entityId = 0;
}

UpdateAttributesPacket::UpdateAttributesPacket(int entityId, unordered_set<AttributeInstance *> *values)
{
	this->entityId = entityId;

	for (AttributeInstance *value : *values)
	{
		unordered_set<AttributeModifier*> mods;
		value->getModifiers(mods);
		attributes.insert(new AttributeSnapshot(value->getAttribute()->getId(), value->getBaseValue(), &mods));
	}
}

UpdateAttributesPacket::~UpdateAttributesPacket()
{
	// Delete modifiers - these are always copies, either on construction or on read
	for(auto& attribute : attributes)
	{
		delete attribute;
	}
}

void UpdateAttributesPacket::read(DataInputStream *dis)
{
	entityId = dis->readInt();

	int attributeCount = dis->readInt();
	for (int i = 0; i < attributeCount; i++)
	{
		eATTRIBUTE_ID id = static_cast<eATTRIBUTE_ID>(dis->readShort());
		double base = dis->readDouble();
		unordered_set<AttributeModifier *> modifiers = unordered_set<AttributeModifier *>();
		int modifierCount = dis->readShort();

		for (int j = 0; j < modifierCount; j++)
		{
			eMODIFIER_ID id = static_cast<eMODIFIER_ID>(dis->readInt());
			double amount = dis->readDouble();
			byte operation = dis->readByte();
			modifiers.insert(new AttributeModifier(id, /*L"Unknown synced attribute modifier",*/ amount, operation));
		}

		attributes.insert(new AttributeSnapshot(id, base, &modifiers));

		// modifiers is copied in AttributeSnapshot ctor so delete contents
		for(auto& modifier : modifiers)
		{
			delete modifier;
		}
	}
}

void UpdateAttributesPacket::write(DataOutputStream *dos)
{
	dos->writeInt(entityId);
	dos->writeInt(attributes.size());

	for(auto& attribute : attributes)
	{
		unordered_set<AttributeModifier *> *modifiers = attribute->getModifiers();

		dos->writeShort(attribute->getId());
		dos->writeDouble(attribute->getBase());
		dos->writeShort(modifiers->size());

		for (auto& modifier : *modifiers)
		{
			dos->writeInt(modifier->getId());
			dos->writeDouble(modifier->getAmount());
			dos->writeByte(modifier->getOperation());
		}
	}
}

void UpdateAttributesPacket::handle(PacketListener *listener)
{
	listener->handleUpdateAttributes(shared_from_this());
}

int UpdateAttributesPacket::getEstimatedSize()
{
	return 4 + 4 + attributes.size() * (8 + 8 + 8);
}

int UpdateAttributesPacket::getEntityId()
{
	return entityId;
}

unordered_set<UpdateAttributesPacket::AttributeSnapshot *> UpdateAttributesPacket::getValues()
{
	return attributes;
}

UpdateAttributesPacket::AttributeSnapshot::AttributeSnapshot(eATTRIBUTE_ID id, double base, unordered_set<AttributeModifier *> *modifiers)
{
	this->id = id;
	this->base = base;

	for(auto& modifier : *modifiers)
	{
		this->modifiers.insert( new AttributeModifier(modifier->getId(), modifier->getAmount(), modifier->getOperation()));
	}
}

UpdateAttributesPacket::AttributeSnapshot::~AttributeSnapshot()
{
	for(auto& modifier : modifiers)
	{
		delete modifier;
	}
}

eATTRIBUTE_ID UpdateAttributesPacket::AttributeSnapshot::getId()
{
	return id;
}

double UpdateAttributesPacket::AttributeSnapshot::getBase()
{
	return base;
}

unordered_set<AttributeModifier *> *UpdateAttributesPacket::AttributeSnapshot::getModifiers()
{
	return &modifiers;
}