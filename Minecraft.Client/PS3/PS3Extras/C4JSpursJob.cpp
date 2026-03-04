#include "stdafx.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cell/spurs.h>
#include "C4JSpursJob.h"
#include "C4JThread_SPU.h"
// #include "PS3\SPU_Tasks\SPUMemTools\SPUMemTools.h"


using namespace cell::Spurs::JobQueue;

static const unsigned int NUM_SUBMIT_JOBS = 128;
#define DMA_ALIGNMENT  (128)
#define JOBHEADER_SYMBOL(JobName) _binary_jqjob_##JobName##_jobbin2_jobheader

C4JSpursJobQueue* C4JSpursJobQueue::m_pMainJobQueue = NULL;

uint16_t C4JSpursJobQueue::Port::s_jobTagBitmask = 0;
C4JSpursJobQueue::Port* C4JSpursJobQueue::Port::s_allocatedPorts[16] = {NULL,NULL,NULL,NULL,
																		NULL,NULL,NULL,NULL,
																		NULL,NULL,NULL,NULL,
																		NULL,NULL,NULL,NULL };
bool C4JSpursJobQueue::Port::s_initialised;
CRITICAL_SECTION C4JSpursJobQueue::Port::s_lock; 


C4JSpursJobQueue::C4JSpursJobQueue()
{
	int ret;
	cell::Spurs::Spurs* spurs = C4JThread_SPU::getSpurs2();

	//E create job descriptor pool
	const CellSpursJobQueueJobDescriptorPool poolDescriptor = {
		0, 0, NUM_SUBMIT_JOBS, 0, 0, 0, 0, 0
	};
	unsigned int sizeOfJobDescriptorPool =
            CELL_SPURS_JOBQUEUE_JOB_DESCRIPTOR_POOL_SIZE(0, 0, NUM_SUBMIT_JOBS, 0, 0, 0, 0, 0);

	pPool = (uint8_t*)memalign(DMA_ALIGNMENT, sizeOfJobDescriptorPool);

	//E create jobQueue
	pJobQueue = (JobQueue<JOB_QUEUE_DEPTH>*)memalign(CELL_SPURS_JOBQUEUE_ALIGN, sizeof(JobQueue<JOB_QUEUE_DEPTH>));
	assert(pJobQueue != NULL);

	ret = JobQueue<JOB_QUEUE_DEPTH>::create(	pJobQueue, 
												spurs, 
												"SampleJobQueue", 
												5, //numSpus
												8, // priority 
												(void*)pPool, 
												&poolDescriptor, 
												4, // maxGrab
												 false, // submitWithEntryLock
												false, // doBusyWaiting
												false, //isHaltOnError
												true, // autoPrxLoad
												1 ); //uint8_t	maxNumJobsOnASpu = CELL_SPURS_JOBQUEUE_DEFAULT_MAX_NUM_JOBS_ON_SPU,



	assert(ret == CELL_OK);
	(void)ret;
}


C4JSpursJobQueue::Port::Port(const char* portName)
{
	m_portName = portName;
	if(!s_initialised)
		InitializeCriticalSection(&s_lock);
	s_initialised = true;

	EnterCriticalSection(&s_lock);

	m_pPort2 = (Port2*)memalign(CELL_SPURS_JOBQUEUE_PORT2_ALIGN, CELL_SPURS_JOBQUEUE_PORT2_SIZE);
	assert(m_pPort2);
	int ret = Port2::create(m_pPort2, C4JSpursJobQueue::getMainJobQueue().pJobQueue);
	assert(ret == CELL_OK);


	m_jobTag = getFreeJobTag();
	m_bDestroyed = false;
	LeaveCriticalSection(&s_lock);
}

C4JSpursJobQueue::Port::~Port()
{
	EnterCriticalSection(&s_lock);
	releaseJobTag(m_jobTag);
	if(!m_bDestroyed)
	{
		int ret = m_pPort2->destroy();
		assert(ret == CELL_OK);
	}
	free(m_pPort2);
	LeaveCriticalSection(&s_lock);

}


void C4JSpursJobQueue::Port::submitJob(C4JSpursJob* pJob)
{
	if(m_bDestroyed)
		return;
	int ret;
	//E submit job
	ret = m_pPort2->copyPushJob(&pJob->m_job256.header, sizeof(CellSpursJob256), sizeof(CellSpursJob256), m_jobTag,
		CELL_SPURS_JOBQUEUE_FLAG_SYNC_JOB);
	if(ret != CELL_OK)
		app.DebugPrintf("copyPushJob failed with error 0x%08x\n", ret);
	assert(ret == CELL_OK);

}

void C4JSpursJobQueue::Port::waitForCompletion()
{
	if(m_bDestroyed)
		return;
	int ret;
	ret = m_pPort2->sync(0);
	assert(ret == CELL_OK);
	(void)ret;		// remove unused var warning
}

bool C4JSpursJobQueue::Port::hasCompleted()
{
	if(m_bDestroyed)
		return true;

	int ret;
	ret = m_pPort2->sync(CELL_SPURS_JOBQUEUE_FLAG_NON_BLOCKING);
	if(ret == CELL_SPURS_JOB_ERROR_AGAIN)
		return false;
	assert(ret == CELL_OK);
	return true;
}


void C4JSpursJobQueue::Port::submitSync()
{
	if(m_bDestroyed)
		return;

	int status;
	status = m_pPort2->pushSync(1u << m_jobTag, 0);
	assert(status == CELL_OK);
	(void)status;		// remove unused var warning
}

int C4JSpursJobQueue::Port::getFreeJobTag()
{
	for(int i=0;i<16;i++)
	{
		if((s_jobTagBitmask & (1<<i)) == 0)
		{
			s_jobTagBitmask |= (1<<i);
			s_allocatedPorts[i] = this;
			return i;
		}
	}
	assert(0);
	return 0;
}

void C4JSpursJobQueue::Port::releaseJobTag( int tag )
{
	s_jobTagBitmask &= ~(1<<tag);
	s_allocatedPorts[tag] = NULL;
}

void C4JSpursJobQueue::Port::destroyAll()
{
	// bit hacky, but the texture blit port seems to stall on destroy if we don't have this
	// presumably because it uses a non-blocking sync
	extern C4JSpursJobQueue::Port* g_texBlitJobQueuePort;
	if(g_texBlitJobQueuePort) 
	{
		g_texBlitJobQueuePort->waitForCompletion();
	}

	for(int i=0;i<16;i++)
	{
		if(s_allocatedPorts[i])
		{
			s_allocatedPorts[i]->m_bDestroyed = true;
			int ret = s_allocatedPorts[i]->m_pPort2->destroy();
			assert(ret == CELL_OK);
		}
	}
#ifndef _CONTENT_PACKAGE
	printf("C4JSpursJobQueue::Port::destroyAll\n");
#endif
}




void C4JSpursJobQueue::shutdown()
{
	Port::destroyAll();
	int ret = pJobQueue->shutdown();
	assert(ret == CELL_OK);
	int	exitCode;
	ret = pJobQueue->join(&exitCode);
	assert(ret == CELL_OK && exitCode == CELL_OK);

	free(pJobQueue);
	free(pPool);
#ifndef _CONTENT_PACKAGE
	printf("C4JSpursJobQueue::shutdown\n");
#endif
}



C4JSpursJob::C4JSpursJob()
{
	__builtin_memset(&m_job256, 0, sizeof(CellSpursJob256));
}



C4JSpursJob_CompressedTile::C4JSpursJob_CompressedTile(TileCompressData_SPU* pDataIn, unsigned char* pDataOut) 
	: C4JSpursJob()
{
	extern const CellSpursJobHeader JOBHEADER_SYMBOL(CompressedTile);
	//E create job
	m_job256.header   = JOBHEADER_SYMBOL(CompressedTile);
	m_job256.header.sizeStack = (64*1024)>>4;
	m_job256.workArea.userData[0] = (uintptr_t)pDataIn;
	m_job256.workArea.userData[1] = (uintptr_t)pDataOut;

}


C4JSpursJob_ChunkUpdate::C4JSpursJob_ChunkUpdate(ChunkRebuildData* pDataIn, ChunkRebuildData* pDataOut) 
: C4JSpursJob()
{
	extern const CellSpursJobHeader JOBHEADER_SYMBOL(ChunkUpdate);
	//E create job
	m_job256.header   = JOBHEADER_SYMBOL(ChunkUpdate);
	m_job256.header.sizeStack = (60*1024)>>4;
	m_job256.workArea.userData[0] = (uintptr_t)pDataIn;
	m_job256.workArea.userData[1] = (uintptr_t)pDataOut;
	assert(cellSpursJobQueueCheckJob(&m_job256, 256, 256) == CELL_OK);

}

C4JSpursJob_LevelRenderer_cull::C4JSpursJob_LevelRenderer_cull(LevelRenderer_cull_DataIn* pDataIn) 
	: C4JSpursJob()
{
	extern const CellSpursJobHeader JOBHEADER_SYMBOL(LevelRenderer_cull);
	//E create job
	m_job256.header   = JOBHEADER_SYMBOL(LevelRenderer_cull);
	m_job256.header.sizeStack = (200*1024)>>4;			// this is extremely tight, global flags take up 164K
	m_job256.workArea.userData[0] = (uintptr_t)pDataIn;

	assert(cellSpursJobQueueCheckJob(&m_job256, 256, 256) == CELL_OK);

}

C4JSpursJob_LevelRenderer_zSort::C4JSpursJob_LevelRenderer_zSort(LevelRenderer_cull_DataIn* pDataIn) 
	: C4JSpursJob()
{
	extern const CellSpursJobHeader JOBHEADER_SYMBOL(LevelRenderer_zSort);
	//E create job
	m_job256.header   = JOBHEADER_SYMBOL(LevelRenderer_zSort);
	m_job256.header.sizeStack = (200*1024)>>4;			// this is extremely tight, global flags take up 164K
	m_job256.workArea.userData[0] = (uintptr_t)pDataIn;

	assert(cellSpursJobQueueCheckJob(&m_job256, 256, 256) == CELL_OK);

}

C4JSpursJob_LevelRenderer_FindNearestChunk::C4JSpursJob_LevelRenderer_FindNearestChunk(LevelRenderer_FindNearestChunk_DataIn* pDataIn) 
	: C4JSpursJob()
{
	extern const CellSpursJobHeader JOBHEADER_SYMBOL(LevelRenderer_FindNearestChunk);
	//E create job
	m_job256.header   = JOBHEADER_SYMBOL(LevelRenderer_FindNearestChunk);
	m_job256.header.sizeStack = (200*1024)>>4;			// this is extremely tight, global flags take up 164K
	m_job256.workArea.userData[0] = (uintptr_t)pDataIn;

	assert(cellSpursJobQueueCheckJob(&m_job256, 256, 256) == CELL_OK);

}



// C4JSpursJob_Renderer_TextureUpdate::C4JSpursJob_Renderer_TextureUpdate(Renderer_TextureUpdate_DataIn* pDataIn) 
// 	: C4JSpursJob()
// {
// 	extern const CellSpursJobHeader JOBHEADER_SYMBOL(Renderer_TextureUpdate);
// 	int ret;
// 	//E create job
// 	m_job256.header   = JOBHEADER_SYMBOL(Renderer_TextureUpdate);
// 	m_job256.header.sizeStack = (200*1024)>>4;	
// 	m_job256.workArea.userData[0] = (uintptr_t)pDataIn;
// 
// 	assert(cellSpursJobQueueCheckJob(&m_job256, 256, 256) == CELL_OK);
// 
// }
// 

C4JSpursJob_Texture_blit::C4JSpursJob_Texture_blit(Texture_blit_DataIn* pDataIn) 
	: C4JSpursJob()
{
	extern const CellSpursJobHeader JOBHEADER_SYMBOL(Texture_blit);
	//E create job
	m_job256.header   = JOBHEADER_SYMBOL(Texture_blit);
	m_job256.header.sizeStack = (200*1024)>>4;		
	m_job256.workArea.userData[0] = (uintptr_t)pDataIn;

	assert(cellSpursJobQueueCheckJob(&m_job256, 256, 256) == CELL_OK);

}

C4JSpursJob_CompressedTileStorage_compress::C4JSpursJob_CompressedTileStorage_compress(CompressedTileStorage_compress_dataIn* pDataIn) 
	: C4JSpursJob()
{
	extern const CellSpursJobHeader JOBHEADER_SYMBOL(CompressedTileStorage_compress);
	//E create job
	m_job256.header   = JOBHEADER_SYMBOL(CompressedTileStorage_compress);
	m_job256.header.sizeStack = (200*1024)>>4;		
	m_job256.workArea.userData[0] = (uintptr_t)pDataIn;

	assert(cellSpursJobQueueCheckJob(&m_job256, 256, 256) == CELL_OK);

}

/*
C4JSpursJob_CompressedTileStorage_getData::C4JSpursJob_CompressedTileStorage_getData(unsigned char* pIdxAndData, int dataSize, unsigned char* pDst, unsigned int retOffset) 
	: C4JSpursJob()
{
	extern const CellSpursJobHeader JOBHEADER_SYMBOL(CompressedTileStorage_getData);
	int ret;
	//E create job
	m_job256.header   = JOBHEADER_SYMBOL(CompressedTileStorage_getData);
	m_job256.header.sizeStack = (200*1024)>>4;		
	m_job256.workArea.userData[0] = (uintptr_t)pIdxAndData;
	m_job256.workArea.userData[1] = (uintptr_t)dataSize;
	m_job256.workArea.userData[2] = (uintptr_t)pDst;
	m_job256.workArea.userData[3] = (uintptr_t)retOffset;

	assert(cellSpursJobQueueCheckJob(&m_job256, 256, 256) == CELL_OK);

} */

// C4JSpursJob_GameRenderer_updateLightTexture::C4JSpursJob_GameRenderer_updateLightTexture(GameRenderer_updateLightTexture_dataIn* pDataIn) 
// 	: C4JSpursJob()
// {
// 	extern const CellSpursJobHeader JOBHEADER_SYMBOL(GameRenderer_updateLightTexture);
// 	int ret;
// 	//E create job
// 	m_job256.header   = JOBHEADER_SYMBOL(GameRenderer_updateLightTexture);
// 	m_job256.header.sizeStack = (200*1024)>>4;		
// 	m_job256.workArea.userData[0] = (uintptr_t)pDataIn;
// 
// 	assert(cellSpursJobQueueCheckJob(&m_job256, 256, 256) == CELL_OK);
// 
// }

C4JSpursJob_PerlinNoise::C4JSpursJob_PerlinNoise(PerlinNoise_DataIn* pDataIn) 
	: C4JSpursJob()
{
	extern const CellSpursJobHeader JOBHEADER_SYMBOL(PerlinNoise);
	//E create job
	m_job256.header   = JOBHEADER_SYMBOL(PerlinNoise);
	m_job256.header.sizeStack = (200*1024)>>4;		
	m_job256.workArea.userData[0] = (uintptr_t)pDataIn;

	assert(cellSpursJobQueueCheckJob(&m_job256, 256, 256) == CELL_OK);
}

// C4JSpursJob_MemSet::C4JSpursJob_MemSet(void* pMem, int numBytes, uint8_t val) 
// 	: C4JSpursJob()
// {
// 	extern const CellSpursJobHeader JOBHEADER_SYMBOL(SPUMemTools);
// 	int ret;
// 	//E create job
// 	m_job256.header   = JOBHEADER_SYMBOL(SPUMemTools);
// 	m_job256.header.sizeStack = (200*1024)>>4;		
// 	m_job256.workArea.userData[0] = (uintptr_t)e_SPUMemToolsFunc_MemSet;
// 	m_job256.workArea.userData[1] = (uintptr_t)pMem;
// 	m_job256.workArea.userData[2] = (uintptr_t)numBytes;
// 	m_job256.workArea.userData[3] = (uintptr_t)val;
// 
// 	assert(cellSpursJobQueueCheckJob(&m_job256, 256, 256) == CELL_OK);
// }

// C4JSpursJob_RLECompress::C4JSpursJob_RLECompress(void* pSrc, int srcSize, void* pDst, int* pDstSize)
// 	: C4JSpursJob()
// {
// 	extern const CellSpursJobHeader JOBHEADER_SYMBOL(RLECompress);
// 	int ret;
// 	//E create job
// 	m_job256.header   = JOBHEADER_SYMBOL(RLECompress);
// 	m_job256.header.sizeStack = (200*1024)>>4;		
// 	m_job256.workArea.userData[0] = (uintptr_t)pSrc;
// 	m_job256.workArea.userData[1] = (uintptr_t)srcSize;
// 	m_job256.workArea.userData[2] = (uintptr_t)pDst;
// 	m_job256.workArea.userData[3] = (uintptr_t)pDstSize;
// 
// 	assert(cellSpursJobQueueCheckJob(&m_job256, 256, 256) == CELL_OK);
// }
// 
