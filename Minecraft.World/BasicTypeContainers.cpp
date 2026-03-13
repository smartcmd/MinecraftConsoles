#include "stdafx.h"
#include "BasicTypeContainers.h"
#include <limits>


/* 4J Jev TODO,
	this is different to Float::MAX_VALUE, javas floats
	seem to actually allow values of infinity which c++ does not.
*/
//A constant holding the positive infinity of type float. It is equal to the value returned by Float.intBitsToFloat(0x7f800000).
const float Float::POSITIVE_INFINITY = 0x7f800000;

const float Float::MAX_VALUE = FLT_MAX;

const double Double::MAX_VALUE = DBL_MAX;

const double Double::MIN_NORMAL = DBL_MIN;

int Integer::parseInt(wstring &str, int radix /* = 10*/)
{
	return wcstol( str.c_str(), nullptr, radix );
}