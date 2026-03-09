#include "stdafx.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "BasicTypeContainers.h"
#include "SharedMonsterAttributes.h"

Attribute *SharedMonsterAttributes::MAX_HEALTH = (new RangedAttribute(eAttributeId_GENERIC_MAXHEALTH, 20, 0, Double::MAX_VALUE))->setSyncable(true);
Attribute *SharedMonsterAttributes::FOLLOW_RANGE = (new RangedAttribute(eAttributeId_GENERIC_FOLLOWRANGE, 32, 0, 2048));
Attribute *SharedMonsterAttributes::KNOCKBACK_RESISTANCE = (new RangedAttribute(eAttributeId_GENERIC_KNOCKBACKRESISTANCE, 0, 0, 1));
Attribute *SharedMonsterAttributes::MOVEMENT_SPEED = (new RangedAttribute(eAttributeId_GENERIC_MOVEMENTSPEED, 0.7f, 0, Double::MAX_VALUE))->setSyncable(true);
Attribute *SharedMonsterAttributes::ATTACK_DAMAGE = new RangedAttribute(eAttributeId_GENERIC_ATTACKDAMAGE, 2, 0, Double::MAX_VALUE);

ListTag<CompoundTag> *SharedMonsterAttributes::saveAttributes(BaseAttributeMap *attributes)
{
	ListTag<CompoundTag> *list = new ListTag<CompoundTag>();

	vector<AttributeInstance *> atts;
	attributes->getAttributes(atts);
	for (auto& attribute : atts)
	{
		list->add(saveAttribute(attribute));
	}

	return list;
}

CompoundTag *SharedMonsterAttributes::saveAttribute(AttributeInstance *instance)
{
	CompoundTag *tag = new CompoundTag();
	Attribute *attribute = instance->getAttribute();

	tag->putInt(L"ID", attribute->getId());
	tag->putDouble(L"Base", instance->getBaseValue());

	unordered_set<AttributeModifier *> modifiers;
	instance->getModifiers(modifiers);

	if (!modifiers.empty())
	{
		ListTag<CompoundTag> *list = new ListTag<CompoundTag>();

		for (auto& modifier : modifiers)
		{
			if (modifier->isSerializable())
			{
				list->add(saveAttributeModifier(modifier));
			}
		}

		tag->put(L"Modifiers", list);
	}

	return tag;
}

CompoundTag *SharedMonsterAttributes::saveAttributeModifier(AttributeModifier *modifier)
{
	CompoundTag *tag = new CompoundTag();

	tag->putDouble(L"Amount", modifier->getAmount());
	tag->putInt(L"Operation", modifier->getOperation());
	tag->putInt(L"UUID", modifier->getId());

	return tag;
}

void SharedMonsterAttributes::loadAttributes(BaseAttributeMap *attributes, ListTag<CompoundTag> *list)
{
	for (int i = 0; i < list->size(); i++)
	{
		CompoundTag *tag = list->get(i);
		AttributeInstance *instance = attributes->getInstance(static_cast<eATTRIBUTE_ID>(tag->getInt(L"ID")));

		if (instance != nullptr)
		{
			loadAttribute(instance, tag);
		}
		else
		{
			app.DebugPrintf("Ignoring unknown attribute '%d'", tag->getInt(L"ID") );
		}
	}
}

void SharedMonsterAttributes::loadAttribute(AttributeInstance *instance, CompoundTag *tag)
{
	instance->setBaseValue(tag->getDouble(L"Base"));

	if (tag->contains(L"Modifiers"))
	{
		ListTag<CompoundTag> *list = (ListTag<CompoundTag> *) tag->getList(L"Modifiers");

		for (int i = 0; i < list->size(); i++)
		{
			AttributeModifier *modifier = loadAttributeModifier(list->get(i));
			AttributeModifier *old = instance->getModifier(modifier->getId());
			if (old != nullptr) instance->removeModifier(old);
			instance->addModifier(modifier);
		}
	}
}

AttributeModifier *SharedMonsterAttributes::loadAttributeModifier(CompoundTag *tag)
{
	eMODIFIER_ID id = static_cast<eMODIFIER_ID>(tag->getInt(L"UUID"));
	return new AttributeModifier(id, tag->getDouble(L"Amount"), tag->getInt(L"Operation"));
}