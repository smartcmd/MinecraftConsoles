#include "stdafx.h"

#include "JavaMath.h"

Random Math::rand = Random();

//Returns a double value with a positive sign, greater than or equal to 0.0 and less than 1.0.
//Returned values are chosen pseudorandomly with (approximately) uniform distribution from that range.
//When this method is first called, it creates a single new pseudorandom-number generator, exactly as if by the expression
//
//new java.util.Random
//This new pseudorandom-number generator is used thereafter for all calls to this method and is used nowhere else.
//This method is properly synchronized to allow correct use by more than one thread. However, if many threads need to
//generate pseudorandom numbers at a great rate, it may reduce contention for each thread to have its own pseudorandom-number generator.
//
//Returns:
//a pseudorandom double greater than or equal to 0.0 and less than 1.0.
double Math::random()
{
	return Math::rand.nextDouble();
}

//Returns the closest long to the argument. The result is rounded to an integer by adding 1/2, taking the floor of the result,
//and casting the result to type long. In other words, the result is equal to the value of the expression:
//(long)Math.floor(a + 0.5d)
//Special cases:
//
//If the argument is NaN, the result is 0.
//If the argument is negative infinity or any value less than or equal to the value of Long.MIN_VALUE, the result is equal to the
//value of Long.MIN_VALUE.
//If the argument is positive infinity or any value greater than or equal to the value of Long.MAX_VALUE, the result is equal to the
//value of Long.MAX_VALUE.
//Parameters:
//a - a floating-point value to be rounded to a long.
//Returns:
//the value of the argument rounded to the nearest long value.
__int64 Math::round( double d )
{
	return (__int64)floor( d + 0.5 );
}

int Math::_max(int a, int b)
{
	return a > b ? a : b;
}

int Math::_min(int a, int b)
{
	return a < b ? a : b;
}

float Math::_max(float a, float b)
{
	return a > b ? a : b;
}

float Math::_min(float a, float b)
{
	return a < b ? a : b;
}

float Math::wrapDegrees(float input) 
{
	while(input>=360.0f)input-=360.0f;
	if (input >= 180.0f) input -= 360.0f;
	if (input < -180.0f) input += 360.0f;
	return input;
}

double Math::wrapDegrees(double input) 
{
	while(input>=360.0)input-=360.0;
	if (input >= 180.0) input -= 360.0;
	if (input < -180.0) input += 360.0;
	return input;
}