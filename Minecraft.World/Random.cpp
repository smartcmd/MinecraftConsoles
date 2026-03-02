#include "stdafx.h"	
#include "Random.h"
#include "System.h"

Random::Random()
{
	// 4J - jave now uses the system nanosecond counter added to a "seedUniquifier" to get an initial seed. Our nanosecond timer is actually only millisecond accuate, so
	// use QueryPerformanceCounter here instead
	__int64 seed;
	QueryPerformanceCounter((LARGE_INTEGER *)&seed);
	seed += 8682522807148012LL;

	setSeed(seed);
}

Random::Random(__int64 seed)
{
	setSeed(seed);
}

void Random::setSeed(__int64 s)
{
    this->seed = (s ^ 0x5DEECE66DLL) & ((1LL << 48) - 1);
    haveNextNextGaussian = false;
}

int Random::next(int bits)
{
    seed = (seed * 0x5DEECE66DLL + 0xBLL) & ((1LL << 48) - 1);
    return (int)(seed >> (48 - bits));
}

void Random::nextBytes(byte *bytes, unsigned int count)
{
	for(unsigned int i = 0; i < count; i++ )
	{
		bytes[i] = (byte)next(8);
	}
}

double Random::nextDouble()
{

    return (((__int64)next(26) << 27) + next(27))
        / (double)(1LL << 53);
}

double Random::nextGaussian()
{
    if (haveNextNextGaussian)
	{
        haveNextNextGaussian = false;
        return nextNextGaussian;
    }
	else
	{
        double v1, v2, s;
        do
		{ 
            v1 = 2 * nextDouble() - 1;   // between -1.0 and 1.0
            v2 = 2 * nextDouble() - 1;   // between -1.0 and 1.0
            s = v1 * v1 + v2 * v2;
        } while (s >= 1 || s == 0);
        double multiplier = sqrt(-2 * log(s)/s);
        nextNextGaussian = v2 * multiplier;
        haveNextNextGaussian = true;
        return v1 * multiplier;
    }
}

int Random::nextInt()
{
	return next(32);
}

int Random::nextInt(int n)
{
    assert (n>0);


    if ((n & -n) == n)  // i.e., n is a power of 2
        return (int)(((__int64)next(31) * n) >> 31); // 4J Stu - Made __int64 instead of long

    int bits, val;
    do
	{
        bits = next(31);
        val = bits % n;
    } while(bits - val + (n-1) < 0);
    return val;
}

float Random::nextFloat()
{
	return next(24) / ((float)(1 << 24));
}

__int64 Random::nextLong()
{
	return ((__int64)next(32) << 32) + next(32);
}

bool Random::nextBoolean()
{
	return next(1) != 0;
}