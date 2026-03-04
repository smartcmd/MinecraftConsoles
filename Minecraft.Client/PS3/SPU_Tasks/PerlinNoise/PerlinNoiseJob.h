#pragma once
#include <vectormath\cpp\vectormath_aos.h>
#include "PerlinNoise_SPU.h"





class PerlinNoise_DataIn
{
public:
#ifndef SN_TARGET_PS3_SPU
	void set(PerlinNoise* pNoise, doubleArray& buffer, int x, int y, int z, int xSize, int ySize, int zSize, double xScale, double yScale, double zScale)
	{
		int arraySize = xSize * ySize * zSize;
		// make sure it's 16 byte aligned
		if(arraySize & 1)
			arraySize++;
		if (buffer.data == NULL) 
			buffer = doubleArray(arraySize);
		for (unsigned int i = 0; i < buffer.length; i++)
			buffer[i] = 0;

		m_perlinData.setup(pNoise);

		m_outputBuffer = buffer.data;
		this->x = x;
		this->y = y;
		this->z = z;
		this->xSize = xSize;
		this->ySize = ySize;
		this->zSize = zSize;
		this->xScale = xScale;
		this->yScale = yScale;
		this->zScale = zScale;
	}
	void set(PerlinNoise* pNoise, doubleArray& buffer, int x, int z, int xSize, int zSize, double xScale, double zScale, double pow)
	{
		set(pNoise, buffer, x, 10, z, xSize, 1, zSize, xScale, 1, zScale);
	}
#endif
	PerlinNoise_SPU	m_perlinData;
	double*			m_outputBuffer;

	int x, y, z;
	int xSize, ySize, zSize;
	double xScale, yScale, zScale;
};
