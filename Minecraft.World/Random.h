#pragma once

class Random
{
private:
	__int64 seed;
	bool haveNextNextGaussian;
	double nextNextGaussian;
protected:
	int next(int bits);
public:
	Random();
	Random(__int64 seed);
	void setSeed(__int64 s);
	void nextBytes(byte *bytes, unsigned int count);
	double nextDouble();
	double nextGaussian();
	int nextInt();
	int nextInt(int to);
	float nextFloat();
	__int64 nextLong();
	bool nextBoolean();
};