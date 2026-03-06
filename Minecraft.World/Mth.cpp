#include "stdafx.h"
#include "Mth.h"
#include "Random.h"
#include "StringHelpers.h"

const int Mth::BIG_ENOUGH_INT = 1024;
const float Mth::BIG_ENOUGH_FLOAT = BIG_ENOUGH_INT;
const float Mth::DEGRAD = PI / 180.0f;
const float Mth::RADDEG = 180.0f / PI;
const float Mth::RAD_TO_GRAD = PI / 180.0f;

float *Mth::_sin = NULL;

const float Mth::sinScale = 65536.0f / (float) (PI * 2);

// 4J - added - was in static constructor
void Mth::init()
{
	_sin = new float[65536];
	for (int i = 0; i < 65536; i++)
	{
		_sin[i] = (float) ::sin(i * PI * 2 / 65536.0f);
	}
}

float Mth::sin(float i)
{
	if(_sin == NULL) init();		// 4J - added
	return _sin[(int) (i * sinScale) & 65535];
}

float Mth::cos(float i)
{
	if(_sin == NULL) init();		// 4J - added
	return _sin[(int) (i * sinScale + 65536 / 4) & 65535];
}

float Mth::sqrt(float x)
{
	return (float) ::sqrt(x);
}

float Mth::sqrt(double x)
{
	return (float) ::sqrt(x);
}

int Mth::floor(float v)
{
	int i = (int) v;
	return v < i ? i - 1 : i;
}

int64_t Mth::lfloor(double v)
{
	int64_t i = (int64_t) v;
	return v < i ? i - 1 : i;
}

int Mth::fastFloor(double x)
{
	return (int) (x + BIG_ENOUGH_FLOAT) - BIG_ENOUGH_INT;
}

int Mth::floor(double v)
{
	int i = (int) v;
	return v < i ? i - 1 : i;
}

int Mth::absFloor(double v)
{
	return (int) (v >= 0 ? v : -v + 1);
}

float Mth::abs(float v)
{
	return v >= 0 ? v : -v;
}

int Mth::abs(int v)
{
	return v >= 0 ? v : -v;
}

int Mth::ceil(float v)
{
	int i = (int) v;
	return v > i ? i + 1 : i;
}

int Mth::clamp(int value, int min, int max)
{
	if (value < min)
	{
		return min;
	}
	if (value > max)
	{
		return max;
	}
	return value;
}

float Mth::clamp(float value, float min, float max)
{
	if (value < min)
	{
		return min;
	}
	if (value > max)
	{
		return max;
	}
	return value;
}

double Mth::asbMax(double a, double b)
{
	if (a < 0) a = -a;
	if (b < 0) b = -b;
	return a > b ? a : b;
}

int Mth::intFloorDiv(int a, int b)
{
	if (a < 0) return -((-a - 1) / b) - 1;
	return a / b;
}


int Mth::nextInt(Random *random, int minInclusive, int maxInclusive)
{
	if (minInclusive >= maxInclusive)
	{
		return minInclusive;
	}
	return random->nextInt(maxInclusive - minInclusive + 1) + minInclusive;
}

float Mth::nextFloat(Random *random, float min, float max)
{
	if (min >= max) return min;
	return (random->nextFloat() * (max - min)) + min;
}

double Mth::nextDouble(Random *random, double min, double max)
{
	if (min >= max) return min;
	return (random->nextDouble() * (max - min)) + min;
}

float Mth::wrapDegrees(float input)
{
	//input %= 360;
	while (input >= 180)
	{
		input -= 360;
	}
	while (input < -180)
	{
		input += 360;
	}
	return input;
}

double Mth::wrapDegrees(double input)
{
	//input %= 360;
	while (input >= 180)
	{
		input -= 360;
	}
	while (input < -180)
	{
		input += 360;
	}
	return input;
}

int Mth::getInt(const wstring &input, int def)
{
	int result = def;

	result = _fromString<int>(input);

	return result;
}

int Mth::getInt(const wstring &input, int def, int min)
{
	int result = def;

	result = _fromString<int>(input);

	if (result < min) result = min;
	return result;
}

double Mth::getDouble(const wstring &input, double def)
{
	double result = def;

	result = _fromString<double>(input);

	return result;
}

double Mth::getDouble(const wstring &input, double def, double min)
{
	double result = def;

	result = _fromString<double>(input);

	if (result < min) result = min;
	return result;
}

// 4J Changed this to remove the use of the actuall UUID type
wstring Mth::createInsecureUUID(Random *random)
{
	wchar_t output[33];
	output[32] = 0;
	int64_t high = (random->nextLong() & ~UUID_VERSION) | UUID_VERSION_TYPE_4;
	int64_t low = (random->nextLong() & ~UUID_VARIANT) | UUID_VARIANT_2;
	for(int i = 0; i < 16; i++ )
	{
		wchar_t nybbleHigh = high & 0xf;
		wchar_t nybbleLow = low & 0xf;
		nybbleHigh = (nybbleHigh > 9 ) ? ( nybbleHigh + (L'a'-10) ) : ( nybbleHigh + L'0' );
		nybbleLow =  (nybbleLow > 9 ) ?  ( nybbleLow +  (L'a'-10) ) : ( nybbleLow + L'0' );
		high >>= 4;
		low >>= 4;
		output[31 - i]	= nybbleLow;
		output[15 - i]	= nybbleHigh;
	}
	return wstring(output);
}


// 4J Added
bool Mth::almostEquals( double double1, double double2, double precision)
{
	return (std::abs(double1 - double2) <= precision);
}