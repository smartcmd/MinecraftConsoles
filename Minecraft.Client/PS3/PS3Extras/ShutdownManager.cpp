#include "stdafx.h"
#include "ShutdownManager.h"
#include "../../Common/Leaderboards/LeaderboardManager.h"
#include "../../MinecraftServer.h"
#ifdef __PS3__
#include "C4JSpursJob.h"


bool ShutdownManager::s_threadShouldRun[ShutdownManager::eThreadIdCount];
int ShutdownManager::s_threadRunning[ShutdownManager::eThreadIdCount];
CRITICAL_SECTION ShutdownManager::s_threadRunningCS;
C4JThread::EventArray *ShutdownManager::s_eventArray[eThreadIdCount];
#endif

// Initialises the shutdown manager - this needs to be called as soon as the game is started so it can respond as quickly as possible to shut down requests
void ShutdownManager::Initialise()
{
#ifdef __PS3__
	cellSysutilRegisterCallback( 1, SysUtilCallback, NULL );
	for( int i = 0; i < eThreadIdCount; i++ )
	{
		s_threadShouldRun[i] = true;
		s_threadRunning[i] = 0;
		s_eventArray[i] = NULL;
	}
	// Special case for storage manager, which we will manually set now to be considered as running - this will be unset by StorageManager.ExitRequest if required
	s_threadRunning[eStorageManagerThreads] = true;
	InitializeCriticalSection(&s_threadRunningCS);
#endif
}

// Called in response to a system request to exit the game. This just requests that the main thread should stop, and then the main thread is responsible for calling MainThreadHandleShutdown which
// starts the rest of the shut down process, then waits until it is complete.
void ShutdownManager::StartShutdown()
{
#ifdef __PS3__
	s_threadShouldRun[ eMainThread ] = false;
#endif
}

// This should be called from the main thread after it has been requested to shut down (ShouldRun for main thread returns false), and before it returns control to the kernel. This is responsible for
// signalling to all other threads to stop, and wait until their completion before returning.
void ShutdownManager::MainThreadHandleShutdown()
{
#ifdef __PS3__
	// Set flags for each thread which will be reset when they are complete
	s_threadRunning[ eMainThread ] = false;

	// Second wave of things we would like to shut down (after main)
	LeaderboardManager::Instance()->CancelOperation();
	RequestThreadToStop( eLeaderboardThread );
	RequestThreadToStop( eCommerceThread );
	RequestThreadToStop( ePostProcessThread );
	RequestThreadToStop( eRunUpdateThread );
	RequestThreadToStop( eRenderChunkUpdateThread );
	RequestThreadToStop( eConnectionReadThreads );
	RequestThreadToStop( eConnectionWriteThreads );
	RequestThreadToStop( eEventQueueThreads );
	app.DebugPrintf("Shutdown manager: waiting on first batch of threads requested to terminate...\n");
	WaitForSignalledToComplete();
	app.DebugPrintf("Shutdown manager: terminated.\n");

	// Now shut down the server thread
	MinecraftServer::HaltServer();
	RequestThreadToStop( eServerThread );
	app.DebugPrintf("Shutdown manager: waiting on server to terminate...\n");
	WaitForSignalledToComplete();
	app.DebugPrintf("Shutdown manager: terminated.\n");

	//And shut down the storage manager
	RequestThreadToStop( eStorageManagerThreads );
	StorageManager.ExitRequest(&StorageManagerCompleteFn);
	app.DebugPrintf("Shutdown manager: waiting on storage manager to terminate...\n");
	WaitForSignalledToComplete();
	app.DebugPrintf("Shutdown manager: terminated.\n");

	// Audio system shutdown
	app.DebugPrintf("Shutdown manager: Audio shutdown.\n");
	AIL_shutdown();

	// Trophy system shutdown
	app.DebugPrintf("Shutdown manager: Trophy system shutdown.\n");
	ProfileManager.Terminate();

	// Network manager shutdown
	app.DebugPrintf("Shutdown manager: Network manager shutdown.\n");
	g_NetworkManager.Terminate();

	// Finally shut down the spurs job queue - leaving until last so there should be nothing else dependent on this still running
	app.DebugPrintf("Shutdown manager: SPURS shutdown.\n");
	C4JSpursJobQueue::getMainJobQueue().shutdown();
	app.DebugPrintf("Shutdown manager: Complete.\n");
#endif
}

void ShutdownManager::HasStarted(ShutdownManager::EThreadId threadId)
{
#ifdef __PS3__
	EnterCriticalSection(&s_threadRunningCS);
	s_threadRunning[threadId]++;
	LeaveCriticalSection(&s_threadRunningCS);
#endif
}

void ShutdownManager::HasStarted(ShutdownManager::EThreadId threadId, C4JThread::EventArray *eventArray)
{
#ifdef __PS3__
	EnterCriticalSection(&s_threadRunningCS);
	s_threadRunning[threadId]++;
	LeaveCriticalSection(&s_threadRunningCS);
	s_eventArray[threadId] = eventArray;
#endif
}

bool ShutdownManager::ShouldRun(ShutdownManager::EThreadId threadId)
{
#ifdef __PS3__
	return s_threadShouldRun[threadId];
#else
	return true;
#endif
}

void ShutdownManager::HasFinished(ShutdownManager::EThreadId threadId)
{
#ifdef __PS3__
	EnterCriticalSection(&s_threadRunningCS);
	s_threadRunning[threadId]--;
	LeaveCriticalSection(&s_threadRunningCS);
#endif
}

#ifdef __PS3__
void ShutdownManager::SysUtilCallback(uint64_t status, uint64_t param, void *userdata)
{
	Minecraft *minecraft = Minecraft::GetInstance();
	switch(status)
	{
	case CELL_SYSUTIL_REQUEST_EXITGAME:
		app.DebugPrintf("CELL_SYSUTIL_REQUEST_EXITGAME\n");
		StartShutdown();
		break;
	case CELL_SYSUTIL_SYSTEM_MENU_OPEN:
		// Tell the game UI to stop processing
		StorageManager.SetSystemUIDisplaying(true);
		break;
	case CELL_SYSUTIL_DRAWING_END:
		StorageManager.SetSystemUIDisplaying(false);
		break;
	case CELL_SYSUTIL_DRAWING_BEGIN:
	case CELL_SYSUTIL_SYSTEM_MENU_CLOSE:
		break;
	case CELL_SYSUTIL_BGMPLAYBACK_PLAY:
		if( minecraft )
		{
			minecraft->soundEngine->updateSystemMusicPlaying(true);
		}
		app.DebugPrintf("BGM playing\n");
		break;
	case CELL_SYSUTIL_BGMPLAYBACK_STOP:
		if( minecraft )
		{
			minecraft->soundEngine->updateSystemMusicPlaying(false);
		}
		app.DebugPrintf("BGM stopped\n");
		break;
	}
}
#endif

#ifdef __PS3__
void ShutdownManager::WaitForSignalledToComplete()
{
	bool allComplete;
	do
	{
		cellSysutilCheckCallback();
		Sleep(10);
		allComplete = true;
		for( int i = 0; i < eThreadIdCount; i++ )
		{
			if( !s_threadShouldRun[i] )
			{
				if( s_threadRunning[i] != 0 ) allComplete = false;
			}
		}
	} while( !allComplete);
	
}

void ShutdownManager::RequestThreadToStop(int i)
{
	s_threadShouldRun[i] = false;
	if( s_eventArray[i] )
	{
		s_eventArray[i]->Cancel();
	}
}

void ShutdownManager::StorageManagerCompleteFn()
{
	HasFinished(eStorageManagerThreads);
}
#endif