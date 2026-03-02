#pragma once

class ModifiableAttributeInstance;

class BaseAttributeMap
{
protected:
	//unordered_map<Attribute *, AttributeInstance *> attributesByObject;
#ifdef __ORBIS__
	unordered_map<eATTRIBUTE_ID, AttributeInstance *, std::hash<int> > attributesById;
#else
	unordered_map<eATTRIBUTE_ID, AttributeInstance *> attributesById;
#endif

public :
	virtual ~BaseAttributeMap();

	virtual AttributeInstance *getInstance(Attribute *attribute);
	virtual AttributeInstance *getInstance(eATTRIBUTE_ID name);

	virtual AttributeInstance *registerAttribute(Attribute *attribute) = 0;

	virtual void getAttributes(vector<AttributeInstance *>& atts);
	virtual void onAttributeModified(ModifiableAttributeInstance *attributeInstance);

	// 4J: Changed these into specialised functions for adding/removing the modifiers of an item (it's cleaner)
	virtual void removeItemModifiers(shared_ptr<ItemInstance> item);
	virtual void addItemModifiers(shared_ptr<ItemInstance> item);
};