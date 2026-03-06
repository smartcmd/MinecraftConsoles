#pragma once

class FastNoise
{
private:
    uint8_t **noiseMaps;
    int levels;

public:
    FastNoise(int levels);
	FastNoise(Random *random, int levels);

	void init(Random *random, int levels);
	~FastNoise();

	doubleArray getRegion(doubleArray buffer, double x, double y, double z, int xSize, int ySize, int zSize, double xScale, double yScale, double zScale);
};