/* SCE CONFIDENTIAL
PlayStation(R)3 Programmer Tool Runtime Library 430.001
* Copyright (C) 2007 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

/* common headers */
#include "spu_assert.h"
#include <cell/dma.h>
#include <cell/spurs/job_queue.h>

extern CellSpursJobContext2* g_pSpursJobContext;

class DmaData_SPU
{
public:
	static const bool sc_verbose = false;

	static int roundUpDMASize(int size)
	{
		return (size + 0x0f) & (~0x0f);
	}

	static void get(void* dest, uintptr_t ea, unsigned int dmaSize)
	{
		spu_assert((ea % 0x10) == 0); //  make sure we're 16 uint8_t aligned
		spu_assert((uint32_t(dest) % 0x10) == 0); //  make sure we're 16 uint8_t aligned
		spu_assert((dmaSize % 0x10) == 0); //  and that the transfer is a multiple of 16 bytes
		spu_assert(ea >256*1024); //  and that we're not targetting SPU memory
		// start memory transfer
		if(sc_verbose)
			spu_print("DMA PPU->SPU start: 0x%08x -> 0x%08x : size : %d bytes : tag %d .... ", (unsigned int)ea, (unsigned int)dest, dmaSize, g_pSpursJobContext->dmaTag);

		cellDmaLargeGet(dest, ea, dmaSize, g_pSpursJobContext->dmaTag, 0, 0);
	}

	static void getUnaligned(void* dest, uintptr_t ea, unsigned int dmaSize)
	{
		spu_assert(ea >256*1024); //  and that we're not targetting SPU memory
		// start memory transfer
		if(sc_verbose)
			spu_print("DMA PPU->SPU start: 0x%08x -> 0x%08x : size : %d bytes : tag %d .... ", (unsigned int)ea, (unsigned int)dest, dmaSize, g_pSpursJobContext->dmaTag);

		cellDmaUnalignedGet(dest, ea, dmaSize, g_pSpursJobContext->dmaTag, 0, 0);
	}


	static uint32_t getValue32(uintptr_t ea)
	{
		return cellDmaGetUint32(ea, g_pSpursJobContext->dmaTag, 0, 0);
	}
	static uint32_t getValue16(uintptr_t ea)
	{
		return cellDmaGetUint16(ea, g_pSpursJobContext->dmaTag, 0, 0);
	}
		static uint32_t getValue64(uintptr_t ea)
	{
		return cellDmaGetUint64(ea, g_pSpursJobContext->dmaTag, 0, 0);
	}


	static void wait()
	{
		cellDmaWaitTagStatusAll(1 << g_pSpursJobContext->dmaTag);
		if(sc_verbose)
			spu_print("DMA PPU->SPU done!\n");
	}

	static void getAndWait(void* dest, uintptr_t ea, unsigned int dmaSize)
	{
		get(dest, ea, dmaSize);
		wait();
	}

	static void getAndWaitUnaligned(void* dest, uintptr_t ea, unsigned int dmaSize)
	{
		getUnaligned(dest, ea, dmaSize);
		wait();
	}

	static void put(void* src, uintptr_t ea, unsigned int dmaSize)
	{
		if(sc_verbose)
			spu_print("DMA SPU->PPU  start: 0x%08x -> 0x%08x : size : %d bytes .... ", (unsigned int)src, (unsigned int)ea, dmaSize);
		spu_assert((ea % 0x10) == 0); //  make sure we're 16 uint8_t aligned
		spu_assert((uint32_t(src) % 0x10) == 0); //  make sure we're 16 uint8_t aligned
	spu_assert((dmaSize % 0x10) == 0); //  and that the transfer is a multiple of 16 bytes
		spu_assert(ea >256*1024); //  and that we're not targetting SPU memory
		cellDmaLargePut(src, ea, dmaSize, g_pSpursJobContext->dmaTag, 0, 0);
	}

	static void putAndWait(void* src, uintptr_t ea, unsigned int dmaSize)
	{
		put(src, ea, dmaSize);
		wait();
	}
	static void putAndWaitUnaligned(void* src, uintptr_t ea, unsigned int dmaSize)
	{
		if(sc_verbose)
			spu_print("DMA SPU->PPU : 0x%08x -> 0x%08x : size : %d bytes\n", (unsigned int)src, (unsigned int)ea, dmaSize);
		spu_assert((ea % 0x10) == 0); //  make sure we're 16 uint8_t aligned
		spu_assert((uint32_t(src) % 0x10) == 0); //  make sure we're 16 uint8_t aligned
		spu_assert((dmaSize % 0x10) == 0); //  and that the transfer is a multiple of 16 bytes
		spu_assert(ea >256*1024); //  and that we're not targetting SPU memory
		cellDmaUnalignedPut(src, ea, dmaSize, g_pSpursJobContext->dmaTag, 0, 0);
		cellDmaWaitTagStatusAll(1 << g_pSpursJobContext->dmaTag);
		if(sc_verbose)
			spu_print("DMA SPU->PPU complete\n");
	}

	static void putValue32(uint32_t val, uintptr_t ea)
	{
		cellDmaPutUint32(val, ea, g_pSpursJobContext->dmaTag, 0, 0);
	}

};

