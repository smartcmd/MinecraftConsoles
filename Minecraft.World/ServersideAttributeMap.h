#pragma once

#include "BaseAttributeMap.h"

class ServersideAttributeMap : public BaseAttributeMap
{
private:
	unordered_set<AttributeInstance *> dirtyAttributes;

protected:
	// 4J: Remove legacy name
	//unordered_map<eATTRIBUTE_ID, AttributeInstance *> attributesByLegacy;

public:

	// 4J-JEV: Changed from ModifiableAttributeInstance to AttributeInstance as they are not 'covariant' on PS4.
	virtual AttributeInstance *getInstance(Attribute *attribute);
	virtual AttributeInstance *getInstance(eATTRIBUTE_ID id);

	virtual AttributeInstance *registerAttribute(Attribute *attribute);
	virtual void onAttributeModified(ModifiableAttributeInstance *attributeInstance);
	virtual unordered_set<AttributeInstance *> *getDirtyAttributes();
	virtual unordered_set<AttributeInstance *> *getSyncableAttributes();
};