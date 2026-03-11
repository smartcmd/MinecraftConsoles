#include "stdafx.h"
#include <stdlib.h>
#include "EdgeZLib.h"
#include "edge/zlib/edgezlib_ppu.h"

static CellSpurs* s_pSpurs = nullptr;

//Set this to 5 if you want deflate/inflate to run in parallel on 5 SPUs.

const uint32_t kNumDeflateTasks			= 1;
const uint32_t kMaxNumDeflateQueueEntries = 64;
static CellSpursEventFlag s_eventFlagDeflate;		//Cannot be on stack
static CellSpursTaskset s_taskSetDeflate;			//Cannot be on stack
static EdgeZlibDeflateQHandle s_deflateQueue;
static void* s_pDeflateQueueBuffer = nullptr;
static void* s_pDeflateTaskContext[kNumDeflateTasks];
static uint32_t s_numElementsToCompress;	//Cannot be on stack


const uint32_t kNumInflateTasks			= 1;
const uint32_t kMaxNumInflateQueueEntries = 64;
static CellSpursEventFlag s_eventFlagInflate;		//Cannot be on stack
static CellSpursTaskset s_taskSetInflate;			//Cannot be on stack
static EdgeZlibInflateQHandle s_inflateQueue;
static void* s_pInflateQueueBuffer = nullptr;
static void* s_pInflateTaskContext[kNumInflateTasks];
static uint32_t s_numElementsToDecompress;	//Cannot be on stack


static CRITICAL_SECTION s_critSect;

static const bool sc_verbose = false;

#define printf_verbose(...) { if(sc_verbose){ printf(__VA_ARGS__);} }



void EdgeZLib::Init(CellSpurs* pSpurs)
{
	s_pSpurs = pSpurs;
	InitializeCriticalSection(&s_critSect);

	//////////////////////////////////////////////////////////////////////////
	//
	//	Initialize Deflate Queue.
	//	This will hold the queue of work that the Deflate Task(s) on SPU will
	//	pull work from.
	//
	//////////////////////////////////////////////////////////////////////////
	{

		// register taskSet
		CellSpursTasksetAttribute taskSetAttribute;
		uint8_t prios[8] = {15, 15, 15, 15, 15, 15, 0, 0};
		int ret = cellSpursTasksetAttributeInitialize( &taskSetAttribute, 0, prios, 8 );
		PS3_ASSERT_CELL_ERROR(ret);
		ret = cellSpursTasksetAttributeSetName(&taskSetAttribute, "edgeZlibDeflateTaskSet");
		PS3_ASSERT_CELL_ERROR(ret);
		ret = cellSpursCreateTasksetWithAttribute( pSpurs, &s_taskSetDeflate, &taskSetAttribute );
		PS3_ASSERT_CELL_ERROR(ret);

		ret = cellSpursEventFlagInitializeIWL(	pSpurs,
			&s_eventFlagDeflate,
			CELL_SPURS_EVENT_FLAG_CLEAR_AUTO,
			CELL_SPURS_EVENT_FLAG_SPU2PPU );
		PS3_ASSERT_CELL_ERROR(ret);

		ret = cellSpursEventFlagAttachLv2EventQueue( &s_eventFlagDeflate );
		PS3_ASSERT_CELL_ERROR(ret);
#ifndef _CONTENT_PACKAGE
		printf_verbose( "PPU: Event flag created\n" );
#endif

		uint32_t deflateQueueBuffSize = edgeZlibGetDeflateQueueSize( kMaxNumDeflateQueueEntries );
		s_pDeflateQueueBuffer = memalign( EDGE_ZLIB_DEFLATE_QUEUE_ALIGN, deflateQueueBuffSize );
		s_deflateQueue = edgeZlibCreateDeflateQueue(
			s_pSpurs,
			kMaxNumDeflateQueueEntries,
			s_pDeflateQueueBuffer,
			deflateQueueBuffSize );
#ifndef _CONTENT_PACKAGE
		printf_verbose( "PPU: Deflate Queue created\n" );
#endif

		//////////////////////////////////////////////////////////////////////////
		//
		//	Build Deflate Tasks.
		//	We want the compression to be able to run in parallel on multiple
		//	SPUs so we create as many tasks as SPUs that we want it to run
		//	on (kNumDeflateTasks).
		//
		//////////////////////////////////////////////////////////////////////////

		s_pDeflateTaskContext[kNumDeflateTasks];
		for( uint32_t taskNum = 0 ; taskNum < kNumDeflateTasks ; taskNum++ )
		{
			uint32_t contextSaveSize = edgeZlibGetDeflateTaskContextSaveSize();
			s_pDeflateTaskContext[taskNum] = memalign( CELL_SPURS_TASK_CONTEXT_ALIGN, contextSaveSize );
			edgeZlibCreateDeflateTask( &s_taskSetDeflate, s_pDeflateTaskContext[taskNum], s_deflateQueue );
		}
#ifndef _CONTENT_PACKAGE
		printf_verbose( "PPU: %d Deflate Task(s) started\n", kNumDeflateTasks );
#endif
	}



	{



		//////////////////////////////////////////////////////////////////////////
		//
		//	Initialize taskset and event flag
		//
		//////////////////////////////////////////////////////////////////////////

		// register taskSet
		CellSpursTasksetAttribute taskSetAttribute;
		uint8_t prios[8] = {15, 15, 15, 15, 15, 15, 0, 0};
		int ret = cellSpursTasksetAttributeInitialize( &taskSetAttribute, 0, prios, 8 );
		PS3_ASSERT_CELL_ERROR(ret);
		ret = cellSpursTasksetAttributeSetName(&taskSetAttribute, "edgeZlibInflateTaskSet");
		PS3_ASSERT_CELL_ERROR(ret);
		ret = cellSpursCreateTasksetWithAttribute( s_pSpurs, &s_taskSetInflate, &taskSetAttribute );
		PS3_ASSERT_CELL_ERROR(ret);
#ifndef _CONTENT_PACKAGE
		printf_verbose( "PPU: Inflate Taskset created\n" );
#endif

		ret = cellSpursEventFlagInitializeIWL(	s_pSpurs,
			&s_eventFlagInflate,
			CELL_SPURS_EVENT_FLAG_CLEAR_AUTO,
			CELL_SPURS_EVENT_FLAG_SPU2PPU );
		PS3_ASSERT_CELL_ERROR(ret);

		ret = cellSpursEventFlagAttachLv2EventQueue( &s_eventFlagInflate );
		PS3_ASSERT_CELL_ERROR(ret);
#ifndef _CONTENT_PACKAGE
		printf_verbose( "PPU: Event flag created\n" );
#endif

		//////////////////////////////////////////////////////////////////////////
		//
		//	Initialize Inflate Queue.
		//	This will hold the queue of work that the Inflate Task(s) on SPU will
		//	pull work from.
		//
		//////////////////////////////////////////////////////////////////////////

		uint32_t inflateQueueBuffSize = edgeZlibGetInflateQueueSize( kMaxNumInflateQueueEntries );
		s_pInflateQueueBuffer = memalign( EDGE_ZLIB_INFLATE_QUEUE_ALIGN, inflateQueueBuffSize );
		s_inflateQueue = edgeZlibCreateInflateQueue(
			s_pSpurs,
			kMaxNumInflateQueueEntries,
			s_pInflateQueueBuffer,
			inflateQueueBuffSize );
#ifndef _CONTENT_PACKAGE
		printf_verbose( "PPU: Inflate Queue created\n" );
#endif
		//////////////////////////////////////////////////////////////////////////
		//
		//	Build Inflate Tasks.
		//	We want the compression to be able to run in parallel on multiple
		//	SPUs so we create as many tasks as SPUs that we want it to run
		//	on (kNumInflateTasks).
		//
		//////////////////////////////////////////////////////////////////////////

		for( uint32_t taskNum = 0 ; taskNum < kNumInflateTasks ; taskNum++ )
		{
			uint32_t contextSaveSize = edgeZlibGetInflateTaskContextSaveSize();
			s_pInflateTaskContext[taskNum] = memalign( CELL_SPURS_TASK_CONTEXT_ALIGN, contextSaveSize );
			edgeZlibCreateInflateTask( &s_taskSetInflate, s_pInflateTaskContext[taskNum], s_inflateQueue );
		}
#ifndef _CONTENT_PACKAGE
		printf_verbose( "PPU: %d Inflate Task(s) started\n", kNumInflateTasks );
#endif
	}

}






bool EdgeZLib::Compress(void* pDestination, uint32_t* pDestSize, const void* pSource, uint32_t SrcSize)
{
	EnterCriticalSection(&s_critSect);

	//////////////////////////////////////////////////////////////////////////
	//
	//	Add one item to the Deflate Queue.
	//	The Deflate Task will wake up and process this work.
	//
	//////////////////////////////////////////////////////////////////////////
	uint32_t* pDst = nullptr;
	bool findingSizeOnly = false;
	if(pDestination == nullptr && *pDestSize == 0)
	{
		pDst = (uint32_t*)malloc(SrcSize);
		findingSizeOnly = true;
		*pDestSize = SrcSize;
	}
	else
	{
		pDst = ((uint32_t*)pDestination);
	}

	pDst[0] = SrcSize;		// 4 byte header added for source size


	s_numElementsToCompress = 1;	//Must be set correctly before any elements are added

	uint16_t eventFlagBits = 1;

	uint32_t compressionLevel = 9;
	static uint32_t s_compressedSize;			//Cannot be on stack

	// create one task queue entry (max 64K deflate per entry)
	edgeZlibAddDeflateQueueElement(		s_deflateQueue,
		pSource, SrcSize,
		&pDst[1], *pDestSize,
		&s_compressedSize,
		&s_numElementsToCompress,	//This will be decremented by 1 when this element is compressed
		&s_eventFlagDeflate,		//When s_numElementsToCompress decrements to zero, this event will be triggered
		eventFlagBits,
		compressionLevel,
		kEdgeZlibDeflateTask_DeflateStoreCompressed
		);

#ifndef _CONTENT_PACKAGE
	printf_verbose( "PPU: Deflate element(s) added to queue\n" );

	printf_verbose( "PPU: Wait for event flag acknowledgment\n" );
#endif

	//////////////////////////////////////////////////////////////////////////
	//
	//	Wait for the event flag to be acknowledged in order to tell us the
	//	work is done.
	//
	//////////////////////////////////////////////////////////////////////////

	int ret = cellSpursEventFlagWait( &s_eventFlagDeflate, &eventFlagBits, CELL_SPURS_EVENT_FLAG_AND );
	PS3_ASSERT_CELL_ERROR(ret );
	assert( s_numElementsToCompress == 0 );	//Should have reached zero by now

#ifndef _CONTENT_PACKAGE
	printf_verbose( "PPU: Edge Zlib event acknowledged\n" );
#endif
	// ensure Deflate task told us it stored the compressed data
	assert( (s_compressedSize & kEdgeZlibDeflateTask_HowStoredMask) == kEdgeZlibDeflateTask_CompressedWasStored );
	// remove bit from s_compressedSize
	s_compressedSize &= ~kEdgeZlibDeflateTask_HowStoredMask;




	//////////////////////////////////////////////////////////////////////////
	//
	//	Print stats
	//
	//////////////////////////////////////////////////////////////////////////

#ifndef _CONTENT_PACKAGE
	printf_verbose( "\n uncompSiz    compSiz    compress%% numTasks\n" );
	printf_verbose( "___________________________________________\n" );
#endif
	float compressionPercent = (1.f - (float)s_compressedSize / (float)SrcSize) * 100.f;

#ifndef _CONTENT_PACKAGE
	printf_verbose( "   0x%05x    0x%05x   %8.2f%%         %d\n\n", 
		(int)SrcSize, (int)s_compressedSize,	compressionPercent, kNumDeflateTasks );
#endif

	*pDestSize = s_compressedSize + 4;		// 4 byte header for size
	if(findingSizeOnly)
		free(pDst);
	LeaveCriticalSection(&s_critSect);

	return true;
}

void EdgeZLib::Shutdown()
{
	//////////////////////////////////////////////////////////////////////////
	//
	//	Shutdown Deflate Queue, event flag and taskset.
	//
	//////////////////////////////////////////////////////////////////////////

#ifndef _CONTENT_PACKAGE
	printf_verbose( "PPU: Shutdown Deflate Queue...\n" );
#endif
	edgeZlibShutdownDeflateQueue( s_deflateQueue );

	int ret = cellSpursEventFlagDetachLv2EventQueue( &s_eventFlagDeflate );
	PS3_ASSERT_CELL_ERROR(ret);

	// shutdown taskSet
#ifndef _CONTENT_PACKAGE
	printf_verbose( "PPU: Wait for taskset shutdown...\n" );
#endif
	ret = cellSpursShutdownTaskset( &s_taskSetDeflate );
	PS3_ASSERT_CELL_ERROR(ret);
	// wait for all tasks to finish
	ret = cellSpursJoinTaskset( &s_taskSetDeflate );
	PS3_ASSERT_CELL_ERROR(ret);
#ifndef _CONTENT_PACKAGE
	printf_verbose( "PPU: Shutdown taskset completed.\n" );
#endif
	// free alloc'd buffers
	for( uint32_t taskNum = 0 ; taskNum < kNumDeflateTasks ; taskNum++ )
	{
		free( s_pDeflateTaskContext[taskNum] );
	}
	free( s_pDeflateQueueBuffer );


	//////////////////////////////////////////////////////////////////////////
	//
	//	Shutdown Inflate Queue, event flag and taskset.
	//
	//////////////////////////////////////////////////////////////////////////

#ifndef _CONTENT_PACKAGE
	printf_verbose( "PPU: Shutdown Inflate Queue...\n" );
#endif
	edgeZlibShutdownInflateQueue( s_inflateQueue );

	ret = cellSpursEventFlagDetachLv2EventQueue( &s_eventFlagInflate );
	PS3_ASSERT_CELL_ERROR(ret);

	// shutdown taskSet
#ifndef _CONTENT_PACKAGE
	printf_verbose( "PPU: Wait for taskset shutdown...\n" );
#endif
	ret = cellSpursShutdownTaskset( &s_taskSetInflate );
	PS3_ASSERT_CELL_ERROR(ret);
	// wait for all tasks to finish
	ret = cellSpursJoinTaskset( &s_taskSetInflate );
	PS3_ASSERT_CELL_ERROR(ret);
#ifndef _CONTENT_PACKAGE
	printf_verbose( "PPU: Shutdown taskset completed.\n" );
#endif

	// free alloc'd buffers
	for( uint32_t taskNum = 0 ; taskNum < kNumInflateTasks ; taskNum++ )
	{
		free( s_pInflateTaskContext[taskNum] );
	}
	free( s_pInflateQueueBuffer );


}

bool EdgeZLib::Decompress(void* pDestination, uint32_t* pDestSize, const void* pSource, uint32_t SrcSize)
{	

#if 1
	EnterCriticalSection(&s_critSect);

	const uint32_t* pSrc = (uint32_t*)pSource;
	uint32_t uncompressedSize = pSrc[0];
	assert(uncompressedSize <= (*pDestSize));



	//////////////////////////////////////////////////////////////////////////
	//
	//	Add one item to the Inflate Queue.
	//	The Inflate Task will wake up and process this work.
	//
	//////////////////////////////////////////////////////////////////////////

	s_numElementsToDecompress = 1;	//Must be set correctly before any elements are added

	uint16_t eventFlagBits = 1;

	// create one task queue entry (max 64K inflate per entry)
	edgeZlibAddInflateQueueElement(	s_inflateQueue,
		&pSrc[1], SrcSize,
		pDestination, uncompressedSize,
		&s_numElementsToDecompress,	//This will be decremented by 1 when this element is compressed
		&s_eventFlagInflate,				//When s_numElementsToDecompress decrements to zero, this event will be triggered
		eventFlagBits,
		kEdgeZlibInflateTask_Inflate
		);	

#ifndef _CONTENT_PACKAGE
	printf_verbose( "PPU: Inflate element(s) added to queue\n" );

	printf_verbose( "PPU: Wait for event flag acknowledgment\n" );
#endif

	//////////////////////////////////////////////////////////////////////////
	//
	//	Wait for the event flag to be acknowledged in order to tell us the
	//	work is done.
	//
	//////////////////////////////////////////////////////////////////////////

	int ret = cellSpursEventFlagWait( &s_eventFlagInflate, &eventFlagBits, CELL_SPURS_EVENT_FLAG_AND );
	PS3_ASSERT_CELL_ERROR(ret);
	assert( s_numElementsToDecompress == 0 );	//Should have reached zero by now

#ifndef _CONTENT_PACKAGE
	printf_verbose( "PPU: Edge Zlib event acknowledged\n" );
#endif

	*pDestSize = uncompressedSize;
	LeaveCriticalSection(&s_critSect);
	
	return true;

#endif // 0
}