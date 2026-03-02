#pragma once
class AttributeModifier;

// 4J: This ID is serialised into save data so new attributes must always be added after existing ones
enum eATTRIBUTE_ID
{
	// 1.6.4
	eAttributeId_GENERIC_MAXHEALTH,
	eAttributeId_GENERIC_FOLLOWRANGE,
	eAttributeId_GENERIC_KNOCKBACKRESISTANCE,
	eAttributeId_GENERIC_MOVEMENTSPEED,
	eAttributeId_GENERIC_ATTACKDAMAGE,
	eAttributeId_HORSE_JUMPSTRENGTH,
	eAttributeId_ZOMBIE_SPAWNREINFORCEMENTS,

	// 1.8+
	// New attributes go here

	eAttributeId_COUNT
};

class Attribute
{
public:
	static const int MAX_NAME_LENGTH = 64;

	/**
	* 4J: Changed this from a string name to an ID
	* Gets the ID of this attribute, for serialization.
	*
	* @return Name of this attribute.
	*/
	virtual eATTRIBUTE_ID getId() = 0;

	/**
	* Sanitizes an attribute value, making sure it's not out of range and is an acceptable amount.
	*
	*
	* @param value Value to sanitize.
	* @return Sanitized value, safe for use.
	*/
	virtual double sanitizeValue(double value) = 0;

	/**
	* Get the default value of this attribute, to be used upon creation.
	*
	* @return Default value.
	*/
	virtual double getDefaultValue() = 0;

	/**
	* Checks if this attribute should be synced to the client.
	*
	* Attributes should be serverside only unless the client needs to know about it.
	*
	* @return True if the client should know about this attribute.
	*/
	virtual bool isClientSyncable() = 0;

	// 4J: Added to retrieve string ID for attribute
	static int getName(eATTRIBUTE_ID id);

protected:
	static const int AttributeNames [];
};

#ifdef __ORBIS__
typedef unordered_map<eATTRIBUTE_ID, AttributeModifier *, std::hash<int>> attrAttrModMap;
#else
typedef unordered_map<eATTRIBUTE_ID, AttributeModifier *> attrAttrModMap;
#endif