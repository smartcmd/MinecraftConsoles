

#include "stdafx.h"

#include "C4JThread_SPU.h"
#include "PS3/SPU_Tasks/ChunkUpdate/ChunkRebuildData.h"
#include "PS3/SPU_Tasks/CompressedTile/CompressedTileStorage_SPU.h"
#include "PS3/SPU_Tasks/LevelRenderChunks/LevelRenderChunks.h"

#define SPURS_MAX_SPU					6
#define SPURS_PPU_THREAD_PRIORITY		2
#define SPURS_SPU_THREAD_PRIORITY		100
#define SPURS_PREFIX					"Minecraft"
#define PRIMARY_PPU_THREAD_PRIORITY		1001
#define PRIMARY_PPU_THREAD_STACK_SIZE	65536
#define SPU_PRINTF_HANDLER_PRIORITY		999

static const bool sc_verbose = true;

cell::Spurs::Spurs2* C4JThread_SPU::ms_spurs2Object = NULL;

void C4JThread_SPU::initSPURS()
{
	int ret;
#ifndef _CONTENT_PACKAGE
	// initialize spu_printf server
	ret = spu_printf_initialize(SPU_PRINTF_HANDLER_PRIORITY, 0);
	if (ret)
	{
		std::printf("Error: spu_printf_initialize(): %#x\n", ret);
		std::printf("## libspurs : FAILED ##\n");
		std::abort();
	}
#endif

	// initialize SPURS attribute
	cell::Spurs::SpursAttribute attribute;
	ret = cell::Spurs::SpursAttribute::initialize(&attribute, SPURS_MAX_SPU - 1, SPURS_SPU_THREAD_PRIORITY, SPURS_PPU_THREAD_PRIORITY, false);
	if (ret) 
	{
#ifndef _CONTENT_PACKAGE
		std::printf("Error: cell::Spurs::SpursAttribute::initialize(): %#x\n", ret);
		std::printf("## libspurs : FAILED ##\n");
#endif
		std::abort();
	}

	ret = attribute.setNamePrefix(SPURS_PREFIX, std::strlen(SPURS_PREFIX));
	if (ret) 
	{
#ifndef _CONTENT_PACKAGE
		std::printf("Error: attribute.setNamePrefix(): %#x\n", ret);
		std::printf("## libspurs : FAILED ##\n");
#endif
		std::abort();
	}

	ret = attribute.setSpuThreadGroupType(SYS_SPU_THREAD_GROUP_TYPE_EXCLUSIVE_NON_CONTEXT);
	if (ret) 
	{
#ifndef _CONTENT_PACKAGE
		std::printf("Error: attribute.setSpuThreadGroupType(): %#x\n", ret);
		std::printf("## libspurs : FAILED ##\n");
#endif
		std::abort();
	}

#ifndef _CONTENT_PACKAGE
	ret = attribute.enableSpuPrintfIfAvailable();
	if (ret)
	{
		std::printf("Error: attribute.enableSpuPrintfIfAvailable(): %#x\n", ret);
		std::printf("## libspurs : FAILED ##\n");
		std::abort();
	}
#endif

	// allocate memory
	ms_spurs2Object = new cell::Spurs::Spurs2;
	if (ms_spurs2Object == 0)
	{
#ifndef _CONTENT_PACKAGE
		std::printf("Error: new cell::Spurs::Spurs2\n");
		std::printf("## libspurs : FAILED ##\n");
#endif
		std::abort();
	}

	// create SPURS instance
	ret = cell::Spurs::Spurs2::initialize(ms_spurs2Object, &attribute);
	if (ret) 
	{
#ifndef _CONTENT_PACKAGE
		std::printf("Error: cell::Spurs::Spurs2::initialize(): %#x\n", ret);
		std::printf("## libspurs : FAILED ##\n");
#endif
		std::abort();
	}

}


void C4JThread_SPU::shutdownSPURS()
{
	int ret;
	// destroy SPURS instance
	ret = ms_spurs2Object->finalize();
	if (ret) 
	{
#ifndef _CONTENT_PACKAGE
		std::printf("Error: spurs->finalize(): %#x\n", ret);
		std::printf("## libspurs : FAILED ##\n");
#endif		
		std::abort();

	}

	// free memory
	delete ms_spurs2Object;

#ifndef _CONTENT_PACKAGE
	// finalize spu_printf server
	ret = spu_printf_finalize();
	if (ret) 
	{
		std::printf("Error: spu_printf_finalize(): %#x\n", ret);
		std::printf("## libspurs : FAILED ##\n");
		std::abort();
	}
#endif 
}





