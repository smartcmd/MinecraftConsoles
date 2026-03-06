#include "stdafx.h"

#include "Attribute.h"
#include "RangedAttribute.h"
#include "AttributeInstance.h"
#include "ModifiableAttributeInstance.h"
#include "ServersideAttributeMap.h"

AttributeInstance *ServersideAttributeMap::getInstance(Attribute *attribute)
{
	return BaseAttributeMap::getInstance(attribute);
}

AttributeInstance *ServersideAttributeMap::getInstance(eATTRIBUTE_ID id)
{
	AttributeInstance *result = BaseAttributeMap::getInstance(id);

	// 4J: Removed legacy name
	// If we didn't find it, search by legacy name
	/*if (result == NULL)
	{
		auto it = attributesByLegacy.find(name);
		if(it != attributesByLegacy.end())
		{
			result = it->second;
		}
	}*/

	return result;
}

AttributeInstance *ServersideAttributeMap::registerAttribute(Attribute *attribute)
{
    auto it = attributesById.find(attribute->getId());
    if (it != attributesById.end())
	{
		return it->second;
	}

	AttributeInstance *instance = new ModifiableAttributeInstance(this, attribute);
	attributesById.insert(std::pair<eATTRIBUTE_ID, AttributeInstance *>(attribute->getId(), instance));

	// 4J: Removed legacy name
	// If this is a ranged attribute also add to legacy name map
	/*RangedAttribute *rangedAttribute = dynamic_cast<RangedAttribute*>(attribute);
	if (rangedAttribute != NULL && rangedAttribute->getImportLegacyName() != L"")
	{
		attributesByLegacy.insert(std::pair<wstring, AttributeInstance*>(rangedAttribute->getImportLegacyName(), instance));
	}*/

	return instance;
}

void ServersideAttributeMap::onAttributeModified(ModifiableAttributeInstance *attributeInstance)
{
	if (attributeInstance->getAttribute()->isClientSyncable())
	{
		dirtyAttributes.insert(attributeInstance);
	}
}

unordered_set<AttributeInstance *> *ServersideAttributeMap::getDirtyAttributes()
{
	return &dirtyAttributes;
}

unordered_set<AttributeInstance *> *ServersideAttributeMap::getSyncableAttributes()
{
	unordered_set<AttributeInstance *> *result = new unordered_set<AttributeInstance *>();
	vector<AttributeInstance *> atts;
	getAttributes(atts);
	for (int i = 0; i < atts.size(); i++)
	{
		AttributeInstance *instance = atts.at(i);

		if (instance->getAttribute()->isClientSyncable())
		{
			result->insert(instance);
		}
	}

	return result;
}