#pragma once

#include "BaseAttribute.h"

class RangedAttribute : public BaseAttribute
{
private:
	double minValue;
	double maxValue;

public:
	RangedAttribute(eATTRIBUTE_ID id, double defaultValue, double minValue, double maxValue);

	double getMinValue();
	double getMaxValue();
	double sanitizeValue(double value);
	
	// 4J: Removed legacy name
	//RangedAttribute *importLegacyName(const wstring &name);
	//wstring getImportLegacyName();
};