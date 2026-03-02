#pragma once
class Random;
class Mth
{
private:
	static const int BIG_ENOUGH_INT;
    static const float BIG_ENOUGH_FLOAT;

public:
    static const float RAD_TO_GRAD;
public:
	static const float DEGRAD;
	static const float RADDEG;
private:
	static float *_sin;
private:
	static const float sinScale;
public :
	static void init();	// 4J added
	static float sin(float i);
    static float cos(float i);
    static float sqrt(float x);
    static float sqrt(double x);
    static int floor(float v);
	static __int64 lfloor(double v);
    static int fastFloor(double x);
    static int floor(double v);
    static int absFloor(double v);
    static float abs(float v);
	static int abs(int v);
    static int ceil(float v);
	static int clamp(int value, int min, int max) ;
	static float clamp(float value, float min, float max);
    static double asbMax(double a, double b);
    static int intFloorDiv(int a, int b);
    static int nextInt(Random *random, int minInclusive, int maxInclusive);
	static float wrapDegrees(float input);
	static double wrapDegrees(double input);

	// 4J Added
	static bool almostEquals( double double1, double double2, double precision);
};