#pragma once
#include "Random.h"

class Math
{
private:
	static Random rand;

public:
	static double random();
	static __int64 round( double d );
	static int _max(int a, int b);
	static float _max(float a, float b);
	static int _min(int a, int b);
	static float _min(float a, float b);

	static float wrapDegrees(float input);
	static double wrapDegrees(double input);
};