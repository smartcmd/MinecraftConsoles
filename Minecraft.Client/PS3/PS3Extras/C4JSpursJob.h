

#pragma once
#include <cell/spurs.h>


#define JOBHEADER_SYMBOL(JobName) _binary_jqjob_##JobName##_jobbin2_jobheader

// class C4JSpursJobEventQueue
// {
// 	C4JThread* m_pollingThread;
// 	sys_event_port_t		m_basicEventPort;
// 	sys_event_queue_t		m_basicEventQueue;
// 
// 	typedef int (C4JSpursJobEventQueueFunc)(uint64_t data1, uint64_t data2);
// 
// 	int eventPoll(void* lpParam)
// 	{
// 		sys_event_t event;
// 		do
// 		{
// 			sys_event_queue_receive(m_basicEventQueue, &event, 0);
// 			C4JSpursJobEventQueueFunc* func = (C4JSpursJobEventQueueFunc*)event.data1;
// 			func(event.data2, event.data3);
// 		} while(1);
// 		return 0;
// 	}
// 
// 	void  CompressedTileStorage::compress_SPUSendEvent(int upgradeBlock/*=-1*/)
// 	{
// 		if(g_pCompressSPUThread == nullptr)
// 		{
// 			sys_event_port_create(&g_basicEventPort, SYS_EVENT_PORT_LOCAL, SYS_EVENT_PORT_NO_NAME);
// 			sys_event_queue_attribute_t queue_attr = {SYS_SYNC_PRIORITY, SYS_PPU_QUEUE};
// 			sys_event_queue_create(&g_basicEventQueue, &queue_attr, SYS_EVENT_QUEUE_LOCAL, 127);
// 			sys_event_port_connect_local(g_basicEventPort, g_basicEventQueue);
// 
// 			g_pCompressSPUThread = new C4JThread(compress_SPUEventPoll, this, "compress_SPU");
// 			g_pCompressSPUThread->Run();
// 		}
// 
// 		sys_event_port_send(g_basicEventPort, (std::uint64_t)this, upgradeBlock, 0);
// 	}
// 
// };

class C4JSpursJob
{
public:
	CellSpursJob256 m_job256;

	C4JSpursJob();
};

class TileCompressData_SPU; 
class C4JSpursJob_CompressedTile : public C4JSpursJob
{
public:
	C4JSpursJob_CompressedTile();
	C4JSpursJob_CompressedTile(TileCompressData_SPU* pDataIn, unsigned char* pDataOut);
};

class ChunkRebuildData; 
class C4JSpursJob_ChunkUpdate : public C4JSpursJob
{
public:
	C4JSpursJob_ChunkUpdate();
	C4JSpursJob_ChunkUpdate(ChunkRebuildData* pDataIn, ChunkRebuildData* pDataOut);
};

class LevelRenderer_cull_DataIn;
class C4JSpursJob_LevelRenderer_cull : public C4JSpursJob
{
public:
	C4JSpursJob_LevelRenderer_cull();
	C4JSpursJob_LevelRenderer_cull(LevelRenderer_cull_DataIn* pDataIn);
};

class C4JSpursJob_LevelRenderer_zSort : public C4JSpursJob
{
public:
	C4JSpursJob_LevelRenderer_zSort();
	C4JSpursJob_LevelRenderer_zSort(LevelRenderer_cull_DataIn* pDataIn);
};

class LevelRenderer_FindNearestChunk_DataIn;
class C4JSpursJob_LevelRenderer_FindNearestChunk : public C4JSpursJob
{
public:
	C4JSpursJob_LevelRenderer_FindNearestChunk();
	C4JSpursJob_LevelRenderer_FindNearestChunk(LevelRenderer_FindNearestChunk_DataIn* pDataIn);
};


// class Renderer_TextureUpdate_DataIn;
// class C4JSpursJob_Renderer_TextureUpdate : public C4JSpursJob
// {
// public:
// 	C4JSpursJob_Renderer_TextureUpdate();
// 	C4JSpursJob_Renderer_TextureUpdate(Renderer_TextureUpdate_DataIn* pDataIn);
// };

class Texture_blit_DataIn;
class C4JSpursJob_Texture_blit : public C4JSpursJob
{
public:
	C4JSpursJob_Texture_blit();
	C4JSpursJob_Texture_blit(Texture_blit_DataIn* pDataIn);
};

class CompressedTileStorage_compress_dataIn;
class C4JSpursJob_CompressedTileStorage_compress : public C4JSpursJob
{
public:
	C4JSpursJob_CompressedTileStorage_compress();
	C4JSpursJob_CompressedTileStorage_compress(CompressedTileStorage_compress_dataIn* pDataIn);
};

class C4JSpursJob_CompressedTileStorage_getData : public C4JSpursJob
{
public:
	C4JSpursJob_CompressedTileStorage_getData();
	C4JSpursJob_CompressedTileStorage_getData(unsigned char* pIdxAndData, int dataSize, unsigned char* pDst, unsigned int retOffset);
};




// class GameRenderer_updateLightTexture_dataIn;
// class C4JSpursJob_GameRenderer_updateLightTexture : public C4JSpursJob
// {
// public:
// 	C4JSpursJob_GameRenderer_updateLightTexture();
// 	C4JSpursJob_GameRenderer_updateLightTexture(GameRenderer_updateLightTexture_dataIn* pDataIn);
// };


class PerlinNoise_DataIn;
class C4JSpursJob_PerlinNoise : public C4JSpursJob
{
public:
	C4JSpursJob_PerlinNoise();
	C4JSpursJob_PerlinNoise(PerlinNoise_DataIn* pDataIn);
};


// class C4JSpursJob_MemSet : public C4JSpursJob
// {
// public:
// 	C4JSpursJob_MemSet();
// 	C4JSpursJob_MemSet(void* pMem, int numBytes, uint8_t val);
// };

// class C4JSpursJob_RLECompress : public C4JSpursJob
// {
// public:
// 	C4JSpursJob_RLECompress();
// 	C4JSpursJob_RLECompress(void* pSrc, int srcSize, void* pDst, int* pDstSize);
// };



class C4JSpursJobQueue
{
public:
	class Port
	{
		static bool s_initialised;
		static CRITICAL_SECTION s_lock; 
		static uint16_t s_jobTagBitmask;
		static Port*	s_allocatedPorts[16];

		cell::Spurs::JobQueue::Port2* m_pPort2;
		int m_jobTag;
		bool m_bDestroyed;
		const char* m_portName;
		int getFreeJobTag();
		void releaseJobTag(int tag);
	public:
		Port(const char* portName);
		~Port();

		static void destroyAll();
		void submitJob(C4JSpursJob* pJob);
		void waitForCompletion();
		bool hasCompleted();
		void submitSync();
	};

private:
	static const unsigned int JOB_QUEUE_DEPTH = 256;
	static C4JSpursJobQueue* m_pMainJobQueue;
	uint8_t *pPool;
	cell::Spurs::JobQueue::JobQueue<JOB_QUEUE_DEPTH> *pJobQueue;

public:
	static C4JSpursJobQueue& getMainJobQueue() {if(m_pMainJobQueue == nullptr) m_pMainJobQueue = new C4JSpursJobQueue; return *m_pMainJobQueue;}
	
	C4JSpursJobQueue();
	void shutdown();
	void submitJob(C4JSpursJob* pJob);
	void waitForCompletion();
	void submitSync();
};

