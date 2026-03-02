#include "stdafx.h"
#include "Mth.h"
#include "Random.h"

const int Mth::BIG_ENOUGH_INT = 1024;
const float Mth::BIG_ENOUGH_FLOAT = BIG_ENOUGH_INT;
const float Mth::RAD_TO_GRAD = PI / 180.0f;
const float Mth::DEGRAD = PI / 180.0f;
const float Mth::RADDEG = 180.0f / PI;

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

__int64 Mth::lfloor(double v)
{
    __int64 i = (__int64) v;
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

// 4J Added
bool Mth::almostEquals( double double1, double double2, double precision)
{
	return (std::abs(double1 - double2) <= precision);
}