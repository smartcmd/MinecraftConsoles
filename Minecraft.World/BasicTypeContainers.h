#pragma once


class Byte
{
public:
	static const char MAX_VALUE = CHAR_MAX;
	static const char MIN_VALUE = CHAR_MIN;
};

class Short
{
public:
	static const short MAX_VALUE = SHRT_MAX;
	static const short MIN_VALUE = SHRT_MIN;
};

class Integer
{
public:
	static const int MAX_VALUE = INT_MAX;
	static int parseInt(wstring &str, int radix = 10);
};

class Float
{
public:
	static const float MAX_VALUE;
	static int floatToIntBits( float value )
	{
		return *(int *)&value;
	}
	static int floatToRawIntBits( float value )
	{
		return *(int *)&value;
	}

	static float intBitsToFloat( int bits )
	{
		return *(float *)&bits;
	}

	static const float POSITIVE_INFINITY;
};

class Double
{
public:
	static const double MAX_VALUE;

	static bool isNaN( double a ) {
#ifdef __PS3__
		return isnan(a);
#else
		return ( a != a );
#endif
	}
	static bool isInfinite( double a ) { return false; /*4J TODO*/ }

	static double longBitsToDouble( __int64 bits )
	{
		return *(double *)&bits;
	}

	static __int64 doubleToLongBits( double d )
	{
		return *(__int64 *)&d;
	}
};

// 4J Stu - The String class should only be used if we need to use the BaseClass::class type
// As such I have renamed it so that we don't confuse it with places where we should use std::string
class _String
{
};