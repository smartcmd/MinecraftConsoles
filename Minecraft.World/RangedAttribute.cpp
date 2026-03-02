#include "stdafx.h"

#include "RangedAttribute.h"

RangedAttribute::RangedAttribute(eATTRIBUTE_ID id, double defaultValue, double minValue, double maxValue) : BaseAttribute(id, defaultValue)
{
	this->minValue = minValue;
	this->maxValue = maxValue;

	//if (minValue > maxValue) throw new IllegalArgumentException("Minimum value cannot be bigger than maximum value!");
	//if (defaultValue < minValue) throw new IllegalArgumentException("Default value cannot be lower than minimum value!");
	//if (defaultValue > maxValue) throw new IllegalArgumentException("Default value cannot be bigger than maximum value!");
}

double RangedAttribute::getMinValue()
{
	return minValue;
}

double RangedAttribute::getMaxValue()
{
	return maxValue;
}

double RangedAttribute::sanitizeValue(double value)
{
	if (value < minValue) value = minValue;
	if (value > maxValue) value = maxValue;

	return value;
}