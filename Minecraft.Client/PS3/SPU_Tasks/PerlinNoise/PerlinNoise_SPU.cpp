


#include "PerlinNoise_SPU.h"
#include "..\Common\DmaData.h"
#include <alloca.h>

int64_t lfloor(double v)
{
	int64_t i = (int64_t) v;
	return v < i ? i - 1 : i;
}
// 
// class PPUStoreArray
// {
// 	static const int sc_cacheSize = 1024;
// 	double m_localCache[sc_cacheSize];
// 	double* m_pDataPPU;
// 	int m_cachePos;
// 	int m_ppuPos;
// 
// public:
// 	PPUStoreArray(uintptr_t pDataPPU) { m_pDataPPU = (double*)pDataPPU; m_cachePos = 0; m_ppuPos = 0; flush();}
// 
// 	void store(int val)
// 	{
// 		m_localCache[m_cachePos] = val;
// 		m_cachePos++;
// 		if(m_cachePos >= sc_cacheSize)
// 			flush();
// 	}
// 
// 	void flush()
// 	{
// 		if(m_cachePos > 0)
// 		{
// 			// dma the local cache back to PPU and start again
// 			// 			spu_print("DMAing %d bytes from 0x%08x(SPU) to 0x%08x(PPU)\n",(int)( m_cachePos*sizeof(int)), (int)m_localCache, (int)&m_pDataPPU[m_ppuPos]);
// 			DmaData_SPU::put(m_localCache, (uintptr_t)&m_pDataPPU[m_ppuPos], DmaData_SPU::roundUpDMASize(m_cachePos*sizeof(double)));
// 			m_ppuPos += m_cachePos;
// 			m_cachePos = 0;
// 		}
// 		for(int i=0;i<sc_cacheSize;i++)
// 			m_localCache[i] = 0.0;
// 	}
// 	int getSize() { return m_ppuPos; }
// };


void PerlinNoise_SPU::getRegion(double* buffer, int x, int y, int z, int xSize, int ySize, int zSize, double xScale, double yScale, double zScale)
{
// 	if (buffer.data == NULL) buffer = doubleArray(xSize * ySize * zSize);
	int bufLen = xSize * ySize * zSize;
	int bufMemSize = DmaData_SPU::roundUpDMASize(bufLen*sizeof(double));
	double* localBuffer = (double*)alloca(bufMemSize);
	for (unsigned int i = 0; i < bufLen; i++)
		localBuffer[i] = 0;


	double pow = 1;

	for (int i = 0; i < levels; i++)
	{
		//            value += noiseLevels[i].getValue(x * pow, y * pow, z * pow) / pow;
		double xx = x * pow * xScale;
		double yy = y * pow * yScale;
		double zz = z * pow * zScale;
		int64_t xb = lfloor(xx);
		int64_t zb = lfloor(zz);
		xx -= xb;
		zz -= zb;
		xb %= 16777216;
		zb %= 16777216;
		xx += xb;
		zz += zb;
		noiseLevels[i].add(localBuffer, xx, yy, zz, xSize, ySize, zSize, xScale * pow, yScale * pow, zScale * pow, pow);
		pow /= 2;
	}
	DmaData_SPU::putAndWait(localBuffer,(uintptr_t)buffer, bufMemSize);
}

void PerlinNoise_SPU::getRegion(double* sr, int x, int z, int xSize, int zSize, double xScale, double zScale, double pow)
{
	getRegion(sr, x, 10, z, xSize, 1, zSize, xScale, 1, zScale);
}