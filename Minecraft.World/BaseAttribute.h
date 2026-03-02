#pragma once

#include "Attribute.h"

class BaseAttribute : public Attribute
{
private:
	eATTRIBUTE_ID id;
	double defaultValue;
	bool syncable;

protected:
	BaseAttribute(eATTRIBUTE_ID id, double defaultValue);

public:
	virtual eATTRIBUTE_ID getId();
	virtual double getDefaultValue();
	virtual bool isClientSyncable();
	virtual BaseAttribute *setSyncable(bool syncable);
};