#pragma once
#include <vectormath\cpp\vectormath_aos.h>
#include "ImprovedNoise_SPU.h"

#ifndef SN_TARGET_PS3_SPU
#include "PerlinNoise.h"
#endif

class PerlinNoise_SPU//: public Synth
{
private:
	static const int MAX_NOISE_LEVELS = 16;

	ImprovedNoise_SPU noiseLevels[MAX_NOISE_LEVELS];
	int levels;

public:

	void getRegion(double* buffer, int x, int y, int z, int xSize, int ySize, int zSize, double xScale, double yScale, double zScale);
	void getRegion(double* sr, int x, int z, int xSize, int zSize, double xScale, double zScale, double pow);

#ifndef SN_TARGET_PS3_SPU
	void setup(PerlinNoise* pNoise)
	{
		levels = pNoise->levels;
		for(int i=0;i<pNoise->levels;i++)
		{
			ImprovedNoise_SPU& noiseSPU = noiseLevels[i];
			ImprovedNoise* noisePPU = pNoise->noiseLevels[i];
			noiseSPU.scale = noisePPU->scale;
			noiseSPU.xo = noisePPU->xo;
			noiseSPU.yo = noisePPU->yo;
			noiseSPU.zo = noisePPU->zo;
			memcpy(noiseSPU.p, noisePPU->p, sizeof(noiseSPU.p));
		}
	}
#endif
};
