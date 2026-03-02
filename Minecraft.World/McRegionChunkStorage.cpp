#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "ConsoleSaveFileIO.h"
#include "LevelData.h"
#include "McRegionChunkStorage.h"

CRITICAL_SECTION		McRegionChunkStorage::cs_memory;

std::deque<DataOutputStream *> McRegionChunkStorage::s_chunkDataQueue;
int McRegionChunkStorage::s_runningThreadCount = 0;
C4JThread *McRegionChunkStorage::s_saveThreads[3];


McRegionChunkStorage::McRegionChunkStorage(ConsoleSaveFile *saveFile, const wstring &prefix) : m_prefix( prefix )
{
	m_saveFile = saveFile;

	// Make sure that if there are any files for regions to be created, that they are created in the order that suits us for making the initial level save work fast
	if( prefix == L"" )
	{
		m_saveFile->createFile(ConsoleSavePath(L"DIM-1r.-1.-1.mcr"));
		m_saveFile->createFile(ConsoleSavePath(L"DIM-1r.0.-1.mcr"));
		m_saveFile->createFile(ConsoleSavePath(L"DIM-1r.0.0.mcr"));
		m_saveFile->createFile(ConsoleSavePath(L"DIM-1r.-1.0.mcr"));
		m_saveFile->createFile(ConsoleSavePath(L"DIM1/r.-1.-1.mcr"));
		m_saveFile->createFile(ConsoleSavePath(L"DIM1/r.0.-1.mcr"));
		m_saveFile->createFile(ConsoleSavePath(L"DIM1/r.0.0.mcr"));
		m_saveFile->createFile(ConsoleSavePath(L"DIM1/r.-1.0.mcr"));
		m_saveFile->createFile(ConsoleSavePath(L"r.-1.-1.mcr"));
		m_saveFile->createFile(ConsoleSavePath(L"r.0.-1.mcr"));
		m_saveFile->createFile(ConsoleSavePath(L"r.0.0.mcr"));
		m_saveFile->createFile(ConsoleSavePath(L"r.-1.0.mcr"));
	}

	
#ifdef SPLIT_SAVES
	ConsoleSavePath currentFile = ConsoleSavePath( m_prefix + wstring( L"entities.dat" ) );

	if(m_saveFile->doesFileExist(currentFile))
	{
		ConsoleSaveFileInputStream fis = ConsoleSaveFileInputStream( m_saveFile, currentFile );
		DataInputStream dis(&fis);

		int count = dis.readInt();

		for(int i = 0; i < count; ++i)
		{
			__int64 index = dis.readLong();
			CompoundTag *tag = NbtIo::read(&dis);

			ByteArrayOutputStream bos;
			DataOutputStream dos(&bos);
			NbtIo::write(tag, &dos);
			delete tag;

			byteArray savedData(bos.size());
			memcpy(savedData.data, bos.buf.data, bos.size());

			m_entityData[index] = savedData;
		}
	}
#endif
}

McRegionChunkStorage::~McRegionChunkStorage()
{
	for(AUTO_VAR(it,m_entityData.begin()); it != m_entityData.end(); ++it)
	{
		delete it->second.data;
	}
}

LevelChunk *McRegionChunkStorage::load(Level *level, int x, int z)
{
	DataInputStream *regionChunkInputStream = RegionFileCache::getChunkDataInputStream(m_saveFile, m_prefix, x, z);

#ifdef SPLIT_SAVES
	// If we can't find the chunk in the save file, then we should remove any entities we might have for that chunk
	if(regionChunkInputStream == NULL)
	{
		__int64 index = ((__int64)(x) << 32) | (((__int64)(z))&0x00000000FFFFFFFF);

		AUTO_VAR(it, m_entityData.find(index));
		if(it != m_entityData.end())
		{
			delete it->second.data;
			m_entityData.erase(it);
		}
	}
#endif

	LevelChunk *levelChunk = NULL;

	if(m_saveFile->getOriginalSaveVersion() >= SAVE_FILE_VERSION_COMPRESSED_CHUNK_STORAGE)
	{
		if (regionChunkInputStream != NULL)
		{
			MemSect(9);
			levelChunk = OldChunkStorage::load(level, regionChunkInputStream);
			loadEntities(level, levelChunk);
			MemSect(0);
			regionChunkInputStream->deleteChildStream();
			delete regionChunkInputStream;
		}
	}
	else
	{
		CompoundTag *chunkData;
		if (regionChunkInputStream != NULL)
		{
			MemSect(8);
			chunkData = NbtIo::read((DataInput *)regionChunkInputStream);
			MemSect(0);
		} else
		{
			return NULL;
		}

		regionChunkInputStream->deleteChildStream();
		delete regionChunkInputStream;

		if (!chunkData->contains(L"Level"))
		{
			char buf[256];
			sprintf(buf,"Chunk file at %d, %d is missing level data, skipping\n",x, z);
			app.DebugPrintf(buf);
			delete chunkData;
			return NULL;
		}
		if (!chunkData->getCompound(L"Level")->contains(L"Blocks"))
		{
			char buf[256];
			sprintf(buf,"Chunk file at %d, %d is missing block data, skipping\n",x, z);
			app.DebugPrintf(buf);
			delete chunkData;
			return NULL;
		}
		MemSect(9);
		levelChunk = OldChunkStorage::load(level, chunkData->getCompound(L"Level"));
		MemSect(0);
		if (!levelChunk->isAt(x, z))
		{
			char buf[256];
			sprintf(buf,"Chunk file at %d, %d is in the wrong location; relocating. Expected %d, %d, got %d, %d\n",
				x, z, x, z, levelChunk->x, levelChunk->z);
			app.DebugPrintf(buf);
			delete levelChunk;
			delete chunkData;
			return NULL;

			// 4J Stu - We delete the data within OldChunkStorage::load, so we can never reload from it
			//chunkData->putInt(L"xPos", x);
			//chunkData->putInt(L"zPos", z);
			//MemSect(10);
			//levelChunk = OldChunkStorage::load(level, chunkData->getCompound(L"Level"));
			//MemSect(0);
		}
#ifdef SPLIT_SAVES
		loadEntities(level, levelChunk);
#endif
		delete chunkData;
	}
	return levelChunk;

}

void McRegionChunkStorage::save(Level *level, LevelChunk *levelChunk)
{
	level->checkSession();

	// 4J - removed try/catch
//    try {

	// Note - have added use of a critical section round sections of code that do a lot of memory alloc/free operations. This is because
	// when we are running saves on multiple threads these sections have a lot of contention and thrash the memory system's critical sections
	// Better to let each thread have its turn at a higher level of granularity.
	MemSect(30);
	PIXBeginNamedEvent(0,"Getting output stream\n");
	DataOutputStream *output = RegionFileCache::getChunkDataOutputStream(m_saveFile, m_prefix, levelChunk->x, levelChunk->z);
	PIXEndNamedEvent();

	if(m_saveFile->getOriginalSaveVersion() >= SAVE_FILE_VERSION_COMPRESSED_CHUNK_STORAGE)
	{
		PIXBeginNamedEvent(0,"Writing chunk data");
		OldChunkStorage::save(levelChunk, level, output);
		PIXEndNamedEvent();

		PIXBeginNamedEvent(0,"Updating chunk queue");
		EnterCriticalSection(&cs_memory);
		s_chunkDataQueue.push_back(output);
		LeaveCriticalSection(&cs_memory);
		PIXEndNamedEvent();
	}
	else
	{
		EnterCriticalSection(&cs_memory);
		PIXBeginNamedEvent(0,"Creating tags\n");
		CompoundTag *tag = new CompoundTag();
		CompoundTag *levelData = new CompoundTag();
		tag->put(L"Level", levelData);
		OldChunkStorage::save(levelChunk, level, levelData);
		PIXEndNamedEvent();
		PIXBeginNamedEvent(0,"NbtIo writing\n");
		NbtIo::write(tag, output);
		PIXEndNamedEvent();
		LeaveCriticalSection(&cs_memory);
		PIXBeginNamedEvent(0,"Output closing\n");
		output->close();
		PIXEndNamedEvent();


		// 4J Stu - getChunkDataOutputStream makes a new DataOutputStream that points to a new ChunkBuffer( ByteArrayOutputStream )
		// We should clean these up when we are done
		EnterCriticalSection(&cs_memory);
		PIXBeginNamedEvent(0,"Cleaning up\n");
		output->deleteChildStream();
		delete output;
		delete tag;
		LeaveCriticalSection(&cs_memory);
		PIXEndNamedEvent();
	}
	MemSect(0);

	LevelData *levelInfo = level->getLevelData();

	// 4J Stu - Override this with our save file size to stop all the RegionFileCache lookups
	//levelInfo->setSizeOnDisk(levelInfo->getSizeOnDisk() + RegionFileCache::getSizeDelta(m_saveFile, m_prefix, levelChunk->x, levelChunk->z));
	levelInfo->setSizeOnDisk( this->m_saveFile->getSizeOnDisk() );
//    } catch (Exception e) {
//        e.printStackTrace();
//    }
}

void McRegionChunkStorage::saveEntities(Level *level, LevelChunk *levelChunk)
{
#ifdef SPLIT_SAVES
	PIXBeginNamedEvent(0,"Saving entities");
	__int64 index = ((__int64)(levelChunk->x) << 32) | (((__int64)(levelChunk->z))&0x00000000FFFFFFFF);

	delete m_entityData[index].data;

	CompoundTag *newTag = new CompoundTag();
	bool savedEntities = OldChunkStorage::saveEntities(levelChunk, level, newTag);

	if(savedEntities)
	{
		ByteArrayOutputStream bos;
		DataOutputStream dos(&bos);
		NbtIo::write(newTag, &dos);

		byteArray savedData(bos.size());
		memcpy(savedData.data, bos.buf.data, bos.size());

		m_entityData[index] = savedData;
	}
	else
	{
		AUTO_VAR(it, m_entityData.find(index));
		if(it != m_entityData.end())
		{
			m_entityData.erase(it);
		}
	}
	delete newTag;
	PIXEndNamedEvent();
#endif
}

void McRegionChunkStorage::loadEntities(Level *level, LevelChunk *levelChunk)
{
#ifdef SPLIT_SAVES
	__int64 index = ((__int64)(levelChunk->x) << 32) | (((__int64)(levelChunk->z))&0x00000000FFFFFFFF);
	
	AUTO_VAR(it, m_entityData.find(index));
	if(it != m_entityData.end())
	{
		ByteArrayInputStream bais(it->second);
		DataInputStream dis(&bais);
		CompoundTag *tag = NbtIo::read(&dis);
		OldChunkStorage::loadEntities(levelChunk, level, tag);
		bais.reset();
		delete tag;
	}
#endif
}

void McRegionChunkStorage::tick()
{
	m_saveFile->tick();
}

void McRegionChunkStorage::flush()
{
#ifdef SPLIT_SAVES
	PIXBeginNamedEvent(0, "Flushing entity data");
	ConsoleSavePath currentFile = ConsoleSavePath( m_prefix + wstring( L"entities.dat" ) );
	ConsoleSaveFileOutputStream fos = ConsoleSaveFileOutputStream( m_saveFile, currentFile );
	BufferedOutputStream bos(&fos, 1024*1024);
	DataOutputStream dos(&bos);

	PIXBeginNamedEvent(0,"Writing to stream");
	dos.writeInt(m_entityData.size());

	for(AUTO_VAR(it,m_entityData.begin()); it != m_entityData.end(); ++it)
	{
		dos.writeLong(it->first);
		dos.write(it->second,0,it->second.length);
	}
	bos.flush();
	PIXEndNamedEvent();
	PIXEndNamedEvent();
#endif
}


void McRegionChunkStorage::staticCtor()
{
	InitializeCriticalSectionAndSpinCount(&cs_memory,5120);

	for(unsigned int i = 0; i < 3; ++i)
	{
		char threadName[256];
		sprintf(threadName,"McRegion Save thread %d\n",i);
		SetThreadName(0, threadName);

		//saveThreads[j] = CreateThread(NULL,0,runSaveThreadProc,&threadData[j],CREATE_SUSPENDED,&threadId[j]);
		s_saveThreads[i] = new C4JThread(runSaveThreadProc,NULL,threadName);


		//app.DebugPrintf("Created new thread: %s\n",threadName);

		// Threads 1,3 and 5 are generally idle so use them
		if(i == 0) s_saveThreads[i]->SetProcessor(CPU_CORE_SAVE_THREAD_A);
		else if(i == 1)
		{
			s_saveThreads[i]->SetProcessor(CPU_CORE_SAVE_THREAD_B);
#ifdef __ORBIS__
			s_saveThreads[i]->SetPriority(THREAD_PRIORITY_BELOW_NORMAL);	// On Orbis, this core is also used for Matching 2, and that priority of that seems to be always at default no matter what we set it to. Prioritise this below Matching 2.
#endif
		}
		else if(i == 2) s_saveThreads[i]->SetProcessor(CPU_CORE_SAVE_THREAD_C);

		//ResumeThread( saveThreads[j] );
		s_saveThreads[i]->Run();
	}
}

int McRegionChunkStorage::runSaveThreadProc(LPVOID lpParam)
{
	Compression::CreateNewThreadStorage();

	bool running = true;
	size_t lastQueueSize = 0;

	DataOutputStream *dos = NULL;
	while(running)
	{
		if( TryEnterCriticalSection(&cs_memory) )
		{
			lastQueueSize = s_chunkDataQueue.size();
			if(lastQueueSize > 0)
			{
				dos = s_chunkDataQueue.front();
				s_chunkDataQueue.pop_front();
			}
			s_runningThreadCount++;
			LeaveCriticalSection(&cs_memory);

			if(dos)
			{
				PIXBeginNamedEvent(0,"Saving chunk");
				//app.DebugPrintf("Compressing chunk data (%d left)\n", lastQueueSize - 1);
				dos->close();
				dos->deleteChildStream();
				PIXEndNamedEvent();
			}
			delete dos;
			dos = NULL;

			EnterCriticalSection(&cs_memory);
			s_runningThreadCount--;
			LeaveCriticalSection(&cs_memory);
		}

		// If there was more than one thing in the queue last time we checked, then we want to spin round again soon
		// Otherwise wait a bit longer
		if( (lastQueueSize -1) > 0) Sleep(1); // Sleep 1 to yield
		else Sleep(100);
	}

	Compression::ReleaseThreadStorage();

	return 0;
}

void McRegionChunkStorage::WaitForAll()
{
	WaitForAllSaves();
}

void McRegionChunkStorage::WaitIfTooManyQueuedChunks()
{
	WaitForSaves();
}

// Static
void McRegionChunkStorage::WaitForAllSaves()
{
	// Wait for there to be no more tasks to be processed...
	EnterCriticalSection(&cs_memory);
	size_t queueSize = s_chunkDataQueue.size();
	LeaveCriticalSection(&cs_memory);

	while(queueSize > 0)
	{
		Sleep(10);

		EnterCriticalSection(&cs_memory);
		queueSize = s_chunkDataQueue.size();
		LeaveCriticalSection(&cs_memory);
	}

	// And then wait for there to be no running threads that are processing these tasks
	EnterCriticalSection(&cs_memory);
	int runningThreadCount = s_runningThreadCount;
	LeaveCriticalSection(&cs_memory);

	while(runningThreadCount > 0)
	{
		Sleep(10);

		EnterCriticalSection(&cs_memory);
		runningThreadCount = s_runningThreadCount;
		LeaveCriticalSection(&cs_memory);
	}
}

// Static
void McRegionChunkStorage::WaitForSaves()
{
	static const int MAX_QUEUE_SIZE = 12;
	static const int DESIRED_QUEUE_SIZE = 6;

	// Wait for the queue to reduce to a level where we should add more elements
	EnterCriticalSection(&cs_memory);
	size_t queueSize = s_chunkDataQueue.size();
	LeaveCriticalSection(&cs_memory);

	if( queueSize > MAX_QUEUE_SIZE )
	{
		while( queueSize > DESIRED_QUEUE_SIZE )
		{
			Sleep(10);

			EnterCriticalSection(&cs_memory);
			queueSize = s_chunkDataQueue.size();
			LeaveCriticalSection(&cs_memory);
		}
	}
}
