#pragma once

#include "AttributeInstance.h"

class ModifiableAttributeInstance : public AttributeInstance
{
private:
	BaseAttributeMap *attributeMap;
	Attribute *attribute;
	unordered_set<AttributeModifier *> modifiers [AttributeModifier::TOTAL_OPERATIONS];
	unordered_map<unsigned int, AttributeModifier *> modifierById;
	double baseValue;
	bool dirty;
	double cachedValue;

public:
	ModifiableAttributeInstance(BaseAttributeMap *attributeMap, Attribute *attribute);
	~ModifiableAttributeInstance();

	Attribute *getAttribute();
	double getBaseValue();
	void setBaseValue(double baseValue);
	unordered_set<AttributeModifier *> *getModifiers(int operation);
	void getModifiers(unordered_set<AttributeModifier *>& result);
	AttributeModifier *getModifier(eMODIFIER_ID id);
	void addModifiers(unordered_set<AttributeModifier *> *modifiers);
	void addModifier(AttributeModifier *modifier);

private:
	void setDirty();

public:
	void removeModifier(AttributeModifier *modifier);
	void removeModifier(eMODIFIER_ID id);
	void removeModifiers();
	double getValue();

private:
	double calculateValue();
};