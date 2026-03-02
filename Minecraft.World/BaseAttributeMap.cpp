#include "stdafx.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "BaseAttributeMap.h"

BaseAttributeMap::~BaseAttributeMap()
{
	for(AUTO_VAR(it,attributesById.begin()); it != attributesById.end(); ++it)
	{
		delete it->second;
	}
}

AttributeInstance *BaseAttributeMap::getInstance(Attribute *attribute)
{
	return getInstance(attribute->getId());
}

AttributeInstance *BaseAttributeMap::getInstance(eATTRIBUTE_ID id)
{
	AUTO_VAR(it,attributesById.find(id));
	if(it != attributesById.end())
	{
		return it->second;
	}
	else
	{
		return NULL;
	}
}

void BaseAttributeMap::getAttributes(vector<AttributeInstance *>& atts)
{
	for(AUTO_VAR(it,attributesById.begin()); it != attributesById.end(); ++it)
	{
		atts.push_back(it->second);
	}
}

void BaseAttributeMap::onAttributeModified(ModifiableAttributeInstance *attributeInstance)
{
}

void BaseAttributeMap::removeItemModifiers(shared_ptr<ItemInstance> item)
{
	attrAttrModMap *modifiers = item->getAttributeModifiers();

	for(AUTO_VAR(it, modifiers->begin()); it != modifiers->end(); ++it)
	{
		AttributeInstance *attribute = getInstance(it->first);
		AttributeModifier *modifier = it->second;

		if (attribute != NULL)
		{
			attribute->removeModifier(modifier);
		}

		delete modifier;
	}

	delete modifiers;
}

void BaseAttributeMap::addItemModifiers(shared_ptr<ItemInstance> item)
{
	attrAttrModMap *modifiers = item->getAttributeModifiers();

	for(AUTO_VAR(it, modifiers->begin()); it != modifiers->end(); ++it)
	{
		AttributeInstance *attribute = getInstance(it->first);
		AttributeModifier *modifier = it->second;

		if (attribute != NULL)
		{
			attribute->removeModifier(modifier);
			attribute->addModifier(new AttributeModifier(*modifier));
		}
		
		delete modifier;
	}

	delete modifiers;
}
