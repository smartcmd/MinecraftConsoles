/* SCE CONFIDENTIAL
PlayStation(R)3 Programmer Tool Runtime Library 430.001
* Copyright (C) 2007 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

/* common headers */
#include <stdint.h>
#include <stdlib.h>
#include <alloca.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>
#include <spu_printf.h>
#include <cell/dma.h>
#include <cell/spurs/job_queue.h>

#include "../Common/DmaData.h"
#include <vectormath/c/vectormath_aos_v.h>


static const bool sc_verbose = false;
CellSpursJobContext2* g_pSpursJobContext;


class PPULoadArray
{
	static const int sc_cacheSize = 16384;
	unsigned char m_localCache[sc_cacheSize];
	unsigned char* m_pDataPPU;
	int m_cachePos;
	int m_cacheFilled;
	int m_ppuPos;
	int m_dataSize;

public:
	PPULoadArray(uintptr_t pDataPPU, int dataSize) 
	{ 
		m_pDataPPU = (unsigned char*)pDataPPU; 
		m_cachePos = 0;
		m_ppuPos = 0; 
		m_dataSize = dataSize;
		fillCache();
	}

	unsigned char getCurrent()
	{
		unsigned char val = m_localCache[m_cachePos];
		return val;
	}
	unsigned char getNext()
	{
		m_cachePos++;
		if(m_cachePos >= sc_cacheSize)
			loadMore();
		unsigned char val = m_localCache[m_cachePos];
		return val;
	}

	int getPos() { return m_ppuPos + m_cachePos; }

	void loadMore()
	{
		m_ppuPos += sc_cacheSize;
		fillCache();
	}
	void fillCache()
	{
		// dma data from PPU
		// 			spu_printf("DMAing %d bytes from 0x%08x(SPU) to 0x%08x(PPU)\n",(int)( m_cachePos*sizeof(int)), (int)m_localCache, (int)&m_pDataPPU[m_ppuPos]);
		int dmaSize = (m_dataSize - m_ppuPos);
		if(dmaSize > sc_cacheSize)
			dmaSize = sc_cacheSize;
		dmaSize = DmaData_SPU::roundUpDMASize(dmaSize);
		DmaData_SPU::getAndWait(m_localCache, (uintptr_t)&m_pDataPPU[m_ppuPos], dmaSize);
		m_cachePos = 0;
	}
};

class PPUStoreArray
{
	static const int sc_cacheSize = 16384;
	unsigned char m_localCache[sc_cacheSize];
	unsigned char* m_pDataPPU;
	int m_cachePos;
	int m_ppuPos;

public:
	PPUStoreArray(uintptr_t pDataPPU) { m_pDataPPU = (unsigned char*)pDataPPU; m_cachePos = 0; m_ppuPos = 0;}

	void store(unsigned char val)
	{
		m_localCache[m_cachePos] = val;
		m_cachePos++;
		if(m_cachePos >= sc_cacheSize)
			flush();
	}

	void flush()
	{
		if(m_cachePos > 0)
		{
			// dma the local cache back to PPU and start again
			// 			spu_printf("DMAing %d bytes from 0x%08x(SPU) to 0x%08x(PPU)\n",(int)( m_cachePos*sizeof(int)), (int)m_localCache, (int)&m_pDataPPU[m_ppuPos]);
			DmaData_SPU::putAndWait(m_localCache, (uintptr_t)&m_pDataPPU[m_ppuPos], DmaData_SPU::roundUpDMASize(m_cachePos));
// 			if(m_ppuPos == 0)
// 				spu_printf("first 4 - %d %d %d %d\n", m_localCache[0], m_localCache[1], m_localCache[2], m_localCache[3]);
			m_ppuPos += m_cachePos;
			m_cachePos = 0;
		}
	}
	int getSize() { return m_ppuPos; }
};


void RLECompress(void *pPPUSrc, int srcSize, void* pPPUDst, int* pDstSize)
{

	PPULoadArray srcBuffer((uintptr_t)pPPUSrc, srcSize);
// 	PPUStoreArray dstBuffer((uintptr_t)pPPUDst);

	unsigned char dstBuffer[1024*100];
	int dstPos = 0;
	int endPos = srcSize-1;

	// Compress with RLE first:
	// 0 - 254 - encodes a single byte
	// 255 followed by 0, 1, 2 - encodes a 1, 2, or 3 255s
	// 255 followed by 3-255, followed by a byte - encodes a run of n + 1 bytes
	do
	{
		unsigned char thisOne = srcBuffer.getCurrent();

		unsigned int count = 1;
		while( ( srcBuffer.getPos() != endPos ) && ( srcBuffer.getNext() == thisOne ) && ( count < 256 ) )
		{
			count++;
		}

		if( count <= 3 )
		{
			if( thisOne == 255 )
			{
				dstBuffer[dstPos++] = 255;
				dstBuffer[dstPos++] =  count - 1;
			}
			else
			{
				for( unsigned int i = 0; i < count ; i++ )
				{
					dstBuffer[dstPos++] = thisOne;
				}
			}
		}
		else
		{
			dstBuffer[dstPos++] = 255;
			dstBuffer[dstPos++] = count - 1;
			dstBuffer[dstPos++] = thisOne;
		}
	} while (srcBuffer.getPos() != endPos);
	DmaData_SPU::wait();

	DmaData_SPU::putValue32(dstPos, (uintptr_t)pDstSize);
	int dstDmaSize = DmaData_SPU::roundUpDMASize(dstPos);
	DmaData_SPU::putAndWait(dstBuffer, (uintptr_t)pPPUDst, dstDmaSize);

}




void cellSpursJobQueueMain(CellSpursJobContext2 *pContext, CellSpursJob256 *pJob)
{
	// 	CellSpursTaskId idTask = cellSpursGetTaskId();
	unsigned int idSpu = cellSpursGetCurrentSpuId();

	if(sc_verbose)
		spu_printf("PerlinNoiseJob [SPU#%u] start\n", idSpu);

	g_pSpursJobContext = pContext;
	void* pPPUSrc = (void*)pJob->workArea.userData[0];		
	int srcSize = (int)pJob->workArea.userData[1];		
	void* pPPUDst = (void*)pJob->workArea.userData[2];		
	int* pDstSize = (int*)pJob->workArea.userData[3];		
// 
// 	spu_printf("pPPUSrc : 0x%08x\n", pPPUSrc);
// 	spu_printf("srcSize : %d\n", srcSize);
// 	spu_printf("pPPUDst : 0x%08x\n", pPPUDst);
// 	spu_printf("pDstSize : 0x%08x\n", pDstSize);

	RLECompress(pPPUSrc, srcSize, pPPUDst, pDstSize);


	if(sc_verbose)
		spu_printf("PerlinNoiseJob [SPU#%u] exit\n", idSpu);
}

