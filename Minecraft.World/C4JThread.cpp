#include "stdafx.h"


#include "C4JThread.h"
#ifdef __PSVITA__
#include "../Minecraft.Client/PSVita/PSVitaExtras/ShutdownManager.h"
#include "../Minecraft.Client/PSVita/PSVitaExtras/PSVitaTLSStorage.h"

// AP - this comes from the low level user_malloc.c file used to overide the default memory functions. These must be called when a thread is started/stopped
extern "C" {
extern void user_registerthread();
extern void user_removethread();
}
#else
#include "../Minecraft.Client/PS3/PS3Extras/ShutdownManager.h"

#endif

std::vector<C4JThread*> C4JThread::ms_threadList;
CRITICAL_SECTION C4JThread::ms_threadListCS;

#ifdef _XBOX_ONE
	// 4J Stu - On XboxOne the main thread is not the one that does all the static init, so we have to set this up later
C4JThread *C4JThread::m_mainThread = nullptr;

void C4JThread::StaticInit()
{
	m_mainThread = new C4JThread("Main thread");
}
#else
C4JThread	C4JThread::m_mainThread("Main thread");
#endif

#ifdef __ORBIS__
__thread SceKernelCpumask C4JThread::m_oldAffinityMask;
#endif


#if __PSVITA__
static SceInt32 g_DefaultCPU;
static SceInt32 g_DefaultPriority;
#endif

C4JThread::C4JThread( C4JThreadStartFunc* startFunc, void* param, const char* threadName, int stackSize/* = 0*/ )
{
	m_startFunc = startFunc;
	m_threadParam = param;
	m_stackSize = stackSize;

	// to match XBox, if the stack size is zero, use the default 64k
	if(m_stackSize == 0)
		m_stackSize = 65536 * 2;
	// make sure it's at least 16K
	if(m_stackSize < 16384)
		m_stackSize = 16384;

#ifdef __PS3__
	sprintf(m_threadName, "(4J) %s", threadName );
#else
	sprintf_s(m_threadName,64, "(4J) %s", threadName );
#endif

	m_isRunning = false;
	m_hasStarted = false;

	m_exitCode = STILL_ACTIVE;

#ifdef __PS3__
	m_completionFlag = new Event(Event::e_modeManualClear);
	m_threadID = 0;
	m_lastSleepTime = 0;
	m_priority = 1002;	// main thread has priority 1001
#elif defined __ORBIS__
	m_completionFlag = new Event(Event::e_modeManualClear);
	m_threadID = 0;
	m_lastSleepTime = 0;
	scePthreadAttrInit(&m_threadAttr);
	int err = scePthreadAttrSetaffinity(&m_threadAttr, 63); // set the thread affinity to all cores to start with
	assert(err == SCE_OK);
	m_oldAffinityMask = 0;
	m_priority = SCE_KERNEL_PRIO_FIFO_DEFAULT;
#elif defined __PSVITA__
	m_completionFlag = new Event(Event::e_modeManualClear);
	m_threadID = 0;
	m_lastSleepTime = 0;
	m_priority = g_DefaultPriority;
	//m_CPUMask = SCE_KERNEL_CPU_MASK_USER_ALL;

	// AP - I had trouble getting the cpu to change once the thread was created so I've hard coded them here
	// The main work division is...
	// 0 - Main
	// 1 - Chunk/Tile Update
	// 2 - Server/Audio
	// These three can sometimes consume ALL the CPU time so they are set to below average priority so as not to block other critical threads
	int CPU = SCE_KERNEL_CPU_MASK_USER_ALL;
	if( !strcmp(threadName, "Chunk update") )
	{
		CPU = SCE_KERNEL_CPU_MASK_USER_2;
		m_priority = g_DefaultPriority + 1; 
	}
	if( !strcmp(threadName, "Server" ) )
	{
		CPU = SCE_KERNEL_CPU_MASK_USER_1;
		m_priority = g_DefaultPriority + 1; 
	}
	// make sure Tile Update doesn't go on cpu 0 because it will hold up the main thread. And it can't go on cpu 1 because Chunk Update crashes.
	if( !strcmp(threadName, "Tile update") )
	{
		CPU = SCE_KERNEL_CPU_MASK_USER_1;
	}

	m_threadID = sceKernelCreateThread(m_threadName, entryPoint, g_DefaultPriority, m_stackSize, 0, CPU, nullptr);
	app.DebugPrintf("***************************** start thread %s **************************\n", m_threadName);
#else
	m_threadID = 0;
	m_threadHandle = 0;
	m_threadHandle = CreateThread(nullptr, m_stackSize, entryPoint, this, CREATE_SUSPENDED, &m_threadID);
#endif
	EnterCriticalSection(&ms_threadListCS);
	ms_threadList.push_back(this);
	LeaveCriticalSection(&ms_threadListCS);
}

// only used for the main thread
C4JThread::C4JThread( const char* mainThreadName)
{
#ifdef __PSVITA__
	user_registerthread();
#endif

	m_startFunc = nullptr;
	m_threadParam = nullptr;
	m_stackSize = 0;

#ifdef __PS3__
	sprintf(m_threadName, "(4J) %s", mainThreadName);
#else
	sprintf_s(m_threadName, 64, "(4J) %s", mainThreadName);
#endif
	m_isRunning = true;
	m_hasStarted = true;
	m_lastSleepTime = System::currentTimeMillis();

	// should be the first thread to be created, so init the static critical section for the threadlist here
	InitializeCriticalSection(&ms_threadListCS);


#ifdef __PS3__
	m_completionFlag = new Event(Event::e_modeManualClear);
	sys_ppu_thread_get_id(&m_threadID);
#elif defined __ORBIS__
	m_completionFlag = new Event(Event::e_modeManualClear);
	m_threadID = scePthreadSelf();
	m_priority = SCE_KERNEL_PRIO_FIFO_DEFAULT;
#elif defined __PSVITA__
	m_completionFlag = new Event(Event::e_modeManualClear);
	g_DefaultPriority = sceKernelGetThreadCurrentPriority();
	m_threadID = sceKernelGetThreadId();
	int err = sceKernelChangeThreadCpuAffinityMask(m_threadID, SCE_KERNEL_CPU_MASK_USER_0);
//	sceKernelChangeThreadPriority(m_threadID, g_DefaultPriority + 1);
	g_DefaultCPU = SCE_KERNEL_CPU_MASK_USER_ALL;//sceKernelGetThreadCpuAffinityMask(m_threadID);
#else
	m_threadID = GetCurrentThreadId();
	m_threadHandle = GetCurrentThread();
#endif
#ifdef _XBOX_ONE
	SetThreadName(-1, m_threadName);
#endif
	EnterCriticalSection(&ms_threadListCS);
	ms_threadList.push_back(this);
	LeaveCriticalSection(&ms_threadListCS);
}

C4JThread::~C4JThread()
{
#if defined __PS3__ || defined __ORBIS__ || defined __PSVITA__
	delete m_completionFlag;
#endif

#if defined __ORBIS__
	scePthreadJoin(m_threadID, nullptr);
#endif

	EnterCriticalSection(&ms_threadListCS);

	for (auto it = ms_threadList.begin(); it != ms_threadList.end(); it++)
	{
		if( (*it) == this )
		{
			ms_threadList.erase(it);			
			LeaveCriticalSection(&ms_threadListCS);
			return;
		}
	}

	LeaveCriticalSection(&ms_threadListCS);
}

#ifdef __PS3__
void C4JThread::entryPoint(uint64_t param)
{
	C4JThread* pThread = (C4JThread*)param;
	pThread->m_exitCode = (*pThread->m_startFunc)(pThread->m_threadParam);
	pThread->m_completionFlag->Set();
	pThread->m_isRunning = false;
	sys_ppu_thread_exit(0);
}
#elif defined __ORBIS__
void * C4JThread::entryPoint(void *param)
{
	C4JThread* pThread = (C4JThread*)param;
	pThread->m_exitCode = (*pThread->m_startFunc)(pThread->m_threadParam);
	pThread->m_completionFlag->Set();
	pThread->m_isRunning = false;
	scePthreadExit(nullptr);
}
#elif defined __PSVITA__
struct StrArg {
	C4JThread* Thread;
};

SceInt32 C4JThread::entryPoint(SceSize argSize, void *pArgBlock)
{
	StrArg *strArg = (StrArg*)pArgBlock;
	C4JThread* pThread = strArg->Thread;
	user_registerthread();
	pThread->m_exitCode = (*pThread->m_startFunc)(pThread->m_threadParam);
	app.DebugPrintf("***************************** thread exit %s **************************\n", pThread->m_threadName);
	pThread->m_completionFlag->Set();
	pThread->m_isRunning = false;

	// AP - make sure we clean up this thread's storage and memory
	PSVitaTLSStorage::RemoveThread(pThread->m_threadID);
	user_removethread();

	sceKernelExitDeleteThread(nullptr);

	return pThread->m_exitCode;
}
#else
DWORD WINAPI	C4JThread::entryPoint(LPVOID lpParam)
{
	C4JThread* pThread = static_cast<C4JThread *>(lpParam);
	SetThreadName(-1, pThread->m_threadName);
	pThread->m_exitCode = (*pThread->m_startFunc)(pThread->m_threadParam);
	pThread->m_isRunning = false;
	return pThread->m_exitCode;
}
#endif




void C4JThread::Run()
{
#ifdef __PS3__
	//		prio specifies the priority value of the PPU thread within the range from 0 to 3071 where 0 is the highest.
	// One of the following values is set to flags:
	// 0 - non-joinable non-interrupt thread 
	// SYS_PPU_THREAD_CREATE_JOINABLE - Create a joinable thread 
	// SYS_PPU_THREAD_CREATE_INTERRUPT - Create an interrupt thread 
	uint64_t flags = 0;
	int err = sys_ppu_thread_create(&m_threadID, entryPoint, (uint64_t)this, m_priority, m_stackSize, flags, m_threadName);
#elif defined __ORBIS__
	scePthreadAttrSetstacksize(&m_threadAttr, m_stackSize);
	scePthreadAttrSetguardsize(&m_threadAttr, 1024);
	int ret = scePthreadCreate(&m_threadID, &m_threadAttr, entryPoint, this, m_threadName);
	assert( ret == SCE_OK );
	scePthreadSetprio(m_threadID,m_priority);
	scePthreadAttrDestroy(&m_threadAttr);
#elif defined __PSVITA__
	StrArg strArg = {this};
//	m_threadID = sceKernelCreateThread(m_threadName, entryPoint, m_priority, m_stackSize, 0, m_CPUMask, nullptr);
	sceKernelStartThread( m_threadID, sizeof(strArg), &strArg);
#else
	ResumeThread(m_threadHandle);
#endif
	m_lastSleepTime = System::currentTimeMillis();
	m_isRunning = true;
	m_hasStarted = true;
}

void C4JThread::SetProcessor( int proc )
{
#ifdef __PS3__
	// does nothing since we only have the 1 processor
#elif defined __ORBIS__
	scePthreadAttrSetaffinity(&m_threadAttr, 1 << proc);
#elif defined __PSVITA__
	int Proc = proc >> 1;			// convert from 360's 3 cores * 2 hardware threads to Vita's 3 cores
	int Mask = SCE_KERNEL_CPU_MASK_USER_0 << Proc;
	//m_CPUMask = Mask;
//	int err = sceKernelChangeThreadCpuAffinityMask(m_threadID, Mask);
	int Newmask = sceKernelGetThreadCpuAffinityMask(m_threadID);
	app.DebugPrintf("***************************** set thread proc %s %d %d %d **************************\n", m_threadName, proc, Mask, Newmask);
#elif defined _DURANGO
	SetThreadAffinityMask(m_threadHandle, 1 << proc );
#else
	XSetThreadProcessor( m_threadHandle, proc);
#endif
}

void C4JThread::SetPriority( int priority )
{
#ifdef __PS3__
	switch(priority)
	{
	case THREAD_PRIORITY_LOWEST:			m_priority = 1003; break;  
	case THREAD_PRIORITY_BELOW_NORMAL:		m_priority = 1002; break;  
	case THREAD_PRIORITY_NORMAL:			m_priority = 1001; break;  // same as main thread
	case THREAD_PRIORITY_ABOVE_NORMAL:		m_priority = 1000; break; 
	case THREAD_PRIORITY_HIGHEST:			m_priority = 999; break; 
	}
	if(m_threadID != 0)
		sys_ppu_thread_set_priority(m_threadID, m_priority);
	//int erro = sys_ppu_thread_set_priority(m_threadID, priority);
#elif defined __ORBIS__

	switch(priority)
	{
		case THREAD_PRIORITY_LOWEST:			m_priority = SCE_KERNEL_PRIO_FIFO_LOWEST; break;  
		case THREAD_PRIORITY_BELOW_NORMAL:		m_priority = SCE_KERNEL_PRIO_FIFO_LOWEST + ((SCE_KERNEL_PRIO_FIFO_DEFAULT-SCE_KERNEL_PRIO_FIFO_LOWEST)/2); break;  
		case THREAD_PRIORITY_NORMAL:			m_priority = SCE_KERNEL_PRIO_FIFO_DEFAULT; break;  // same as main thread
		case THREAD_PRIORITY_ABOVE_NORMAL:		m_priority = SCE_KERNEL_PRIO_FIFO_DEFAULT + ((SCE_KERNEL_PRIO_FIFO_HIGHEST-SCE_KERNEL_PRIO_FIFO_DEFAULT)/2); break; 
		case THREAD_PRIORITY_HIGHEST:			m_priority = SCE_KERNEL_PRIO_FIFO_HIGHEST; break; 
	}

	if( m_threadID != 0 )
	{
		scePthreadSetprio(m_threadID,m_priority);
	}
#elif defined __PSVITA__
	int Mid = g_DefaultPriority;//(SCE_KERNEL_LOWEST_PRIORITY_USER + SCE_KERNEL_HIGHEST_PRIORITY_USER) / 2;
	switch(priority)
	{
		case THREAD_PRIORITY_LOWEST:			
			m_priority = SCE_KERNEL_LOWEST_PRIORITY_USER; 
			break;  
		case THREAD_PRIORITY_BELOW_NORMAL:		
			m_priority = Mid + 1; 
			break;  
		case THREAD_PRIORITY_NORMAL:			
			m_priority = Mid; 
			break;  // same as main thread
		case THREAD_PRIORITY_ABOVE_NORMAL:		
			m_priority = Mid - 1; 
			break; 
		case THREAD_PRIORITY_HIGHEST:			
			m_priority = SCE_KERNEL_HIGHEST_PRIORITY_USER; 
			break; 
	}

//	sceKernelChangeThreadPriority(m_threadID, m_priority);
	app.DebugPrintf("***************************** set thread prio %s %d %d **************************\n", m_threadName, priority, m_priority);
#else
	SetThreadPriority(m_threadHandle, priority);
#endif // __PS3__
}

DWORD C4JThread::WaitForCompletion( int timeoutMs )
{
#ifdef __PS3__
	if(timeoutMs == INFINITE)
		timeoutMs = SYS_NO_TIMEOUT ;
	return m_completionFlag->WaitForSignal(timeoutMs);
#elif defined __ORBIS__
	return m_completionFlag->WaitForSignal( timeoutMs );
#elif defined __PSVITA__
	return m_completionFlag->WaitForSignal( timeoutMs );
/*	SceUInt32 Timeout = timeoutMs * 1000;
	SceInt32 err = sceKernelWaitThreadEnd(m_threadID, &m_exitCode, &Timeout);
	if( err == 0 )
	{
		return m_exitCode;
	}
	else
	{
		if( err == SCE_KERNEL_ERROR_WAIT_TIMEOUT )
		{
			return WAIT_TIMEOUT;
		}
		else
		{
			// AP - not sure what to do here
			return 0;
		}
	}*/

//	return m_exitCode;
#else
	return WaitForSingleObject(m_threadHandle, timeoutMs);
#endif // __PS3__
}

int C4JThread::GetExitCode()
{
#if defined  __PS3__ || defined __ORBIS__ || defined __PSVITA__
	return m_exitCode;
#else
	DWORD exitcode = 0;
	GetExitCodeThread(m_threadHandle, &exitcode);

	return *((int *)&exitcode);
#endif
}

void C4JThread::Sleep( int millisecs )
{
#ifdef __PS3__
	if(millisecs == 0)
	{
		// https://ps3.scedev.net/forums/thread/116470/
		// "sys_timer_usleep(0) does not yield the CPU."
		sys_ppu_thread_yield();
	}
	else
		sys_timer_usleep(millisecs * 1000);
#elif defined __ORBIS__
	sceKernelUsleep(((SceKernelUseconds)millisecs) * 1000);
#elif defined __PSVITA__
	// 4J Stu - 0 is an error, so add a tiny sleep when we just want to yield
	sceKernelDelayThread(millisecs * 1000 + 1);
#else
	::Sleep(millisecs);
#endif // __PS3__
}

C4JThread* C4JThread::getCurrentThread()
{
#ifdef __PS3__
	sys_ppu_thread_t currThreadID;
	sys_ppu_thread_get_id(&currThreadID);
#elif defined __ORBIS__
	ScePthread currThreadID = scePthreadSelf();
#elif defined __PSVITA__
	SceUID currThreadID = sceKernelGetThreadId();
#else
	DWORD currThreadID = GetCurrentThreadId();
#endif //__PS3__
	EnterCriticalSection(&ms_threadListCS);

	for(size_t i=0;i<ms_threadList.size(); i++)
	{
		if(currThreadID == ms_threadList[i]->m_threadID)
		{
			LeaveCriticalSection(&ms_threadListCS);
			return ms_threadList[i];
		}
	}

	LeaveCriticalSection(&ms_threadListCS);

	return nullptr;
}

bool C4JThread::isMainThread()
{
#ifdef _XBOX_ONE
	return getCurrentThread() == m_mainThread;
#else
	return getCurrentThread() == &m_mainThread;
#endif
}

C4JThread::Event::Event(EMode mode/* = e_modeAutoClear*/)
{
	m_mode = mode;
#ifdef __PS3__
	sys_event_flag_attribute_t attr;
	// default values taken from sys_event_flag_attribute_initialize
	attr.attr_protocol = SYS_SYNC_PRIORITY;
	attr.attr_pshared = SYS_SYNC_NOT_PROCESS_SHARED;
	attr.key = 0;
	attr.flags = 0;
	attr.type = SYS_SYNC_WAITER_SINGLE; 
	attr.name[0] = '\0';
	sys_event_flag_attribute_initialize(attr);
	
	int err = sys_event_flag_create(&m_event, &attr, 0);

#elif defined __ORBIS__
	char name[1] = {0};
	sceKernelCreateEventFlag( &m_event, name, SCE_KERNEL_EVF_ATTR_TH_FIFO | SCE_KERNEL_EVF_ATTR_MULTI, 0, nullptr);
#elif defined __PSVITA__
	char name[1] = {0};
	m_event = sceKernelCreateEventFlag( name, SCE_KERNEL_EVF_ATTR_TH_FIFO | SCE_KERNEL_EVF_ATTR_MULTI, 0, nullptr);
#else
	m_event = CreateEvent( nullptr, (m_mode == e_modeManualClear), FALSE, nullptr );
#endif //__PS3__
}


C4JThread::Event::~Event()
{
#ifdef __PS3__
	sys_event_flag_destroy(m_event);
#elif defined __ORBIS__
	sceKernelDeleteEventFlag(m_event);
#elif defined __PSVITA__
	sceKernelDeleteEventFlag(m_event);
#else
	CloseHandle( m_event );
#endif // __PS3__
}


void C4JThread::Event::Set()
{
#ifdef __PS3__
	int err =sys_event_flag_set(m_event, 1);
#elif defined __ORBIS__
	sceKernelSetEventFlag(m_event, 1);
#elif defined __PSVITA__
	sceKernelSetEventFlag(m_event, 1);
#else
	SetEvent(m_event);
#endif //__PS3__
}

void C4JThread::Event::Clear()
{
#ifdef __PS3__
	int err =sys_event_flag_clear(m_event, ~(1));
#elif defined __ORBIS__
	sceKernelClearEventFlag(m_event, ~(1));
#elif defined __PSVITA__
	sceKernelClearEventFlag(m_event, ~1);
#else
	ResetEvent(m_event);
#endif //__PS3__
}

DWORD C4JThread::Event::WaitForSignal( int timeoutMs )
{
#ifdef __PS3__
	if(timeoutMs == INFINITE)
		timeoutMs = SYS_NO_TIMEOUT ;
	int timoutMicrosecs = timeoutMs * 1000;
	uint32_t mode = SYS_EVENT_FLAG_WAIT_AND;
	if(m_mode == e_modeAutoClear)
		mode |= SYS_EVENT_FLAG_WAIT_CLEAR;
	int err = sys_event_flag_wait(m_event, 1, mode, 0, timoutMicrosecs);

	switch(err)
	{
	case CELL_OK: return WAIT_OBJECT_0;
	case ETIMEDOUT: return WAIT_TIMEOUT;
	case ECANCELED: return WAIT_ABANDONED;
	default: return WAIT_FAILED;
	}

#elif defined __ORBIS__
	SceKernelUseconds timeoutMicrosecs;
	SceKernelUseconds *pTimeoutMicrosecs;
	if( timeoutMs == INFINITE )
	{
		pTimeoutMicrosecs = nullptr;
	}
	else
	{
		timeoutMicrosecs = ((SceKernelUseconds)timeoutMs) * 1000;
		pTimeoutMicrosecs = &timeoutMicrosecs;
	}
	uint32_t waitMode = SCE_KERNEL_EVF_WAITMODE_AND;
	if(m_mode == e_modeAutoClear)
	{
		waitMode |= SCE_KERNEL_EVF_WAITMODE_CLEAR_PAT;
	}
	int err = sceKernelWaitEventFlag(m_event, 1, waitMode, nullptr, pTimeoutMicrosecs);
	switch(err)
	{
		case SCE_OK: return WAIT_OBJECT_0;
		case SCE_KERNEL_ERROR_ETIMEDOUT: return WAIT_TIMEOUT;
		case SCE_KERNEL_ERROR_ECANCELED: return WAIT_ABANDONED;
		default: return WAIT_FAILED;
	}
#elif defined __PSVITA__
	SceUInt32 timeoutMicrosecs;
	SceUInt32 *pTimeoutMicrosecs;
	if( timeoutMs == INFINITE )
	{
		pTimeoutMicrosecs = nullptr;
	}
	else
	{
		timeoutMicrosecs = ((SceInt32)timeoutMs) * 1000;
		pTimeoutMicrosecs = &timeoutMicrosecs;
	}
	uint32_t waitMode = SCE_KERNEL_EVF_WAITMODE_AND;
	if(m_mode == e_modeAutoClear)
	{
		waitMode |= SCE_KERNEL_EVF_WAITMODE_CLEAR_ALL;
	}
	int err = sceKernelWaitEventFlag(m_event, 1, waitMode, nullptr, pTimeoutMicrosecs);
	switch(err)
	{
		case SCE_OK: return WAIT_OBJECT_0;
		case SCE_KERNEL_ERROR_WAIT_TIMEOUT: return WAIT_TIMEOUT;
		case SCE_KERNEL_ERROR_WAIT_CANCEL: return WAIT_ABANDONED;
		default: return WAIT_FAILED;
	}
#else
	return WaitForSingleObject(m_event, timeoutMs);
#endif // __PS3__
}

C4JThread::EventArray::EventArray( int size, EMode mode/* = e_modeAutoClear*/)
{
	assert(size<32);
	m_size = size;
	m_mode = mode;
#ifdef __PS3__
	sys_event_flag_attribute_t attr;
	// default values taken from sys_event_flag_attribute_initialize
	attr.attr_protocol = SYS_SYNC_PRIORITY;
	attr.attr_pshared = SYS_SYNC_NOT_PROCESS_SHARED;
	attr.key = 0;
	attr.flags = 0;
	attr.type = SYS_SYNC_WAITER_SINGLE; 
	attr.name[0] = '\0';
	sys_event_flag_attribute_initialize(attr);
	int err = sys_event_flag_create(&m_events, &attr, 0);
	assert(err == CELL_OK);
#elif defined __ORBIS__
	char name[1] = {0};
	sceKernelCreateEventFlag( &m_events, name, SCE_KERNEL_EVF_ATTR_TH_FIFO | SCE_KERNEL_EVF_ATTR_MULTI, 0, nullptr);
#elif defined __PSVITA__
	char name[1] = {0};
	m_events = sceKernelCreateEventFlag( name, SCE_KERNEL_EVF_ATTR_TH_FIFO | SCE_KERNEL_EVF_ATTR_MULTI, 0, nullptr);
#else
	m_events = new HANDLE[size];
	for(int i=0;i<size;i++)
	{
		m_events[i]  = CreateEvent(nullptr, (m_mode == e_modeManualClear), FALSE, nullptr );
	}
#endif // __PS3__
}


void C4JThread::EventArray::Set(int index)
{
#ifdef __PS3__
	int err =sys_event_flag_set(m_events, 1<<index);
	assert(err == CELL_OK);
#elif defined __ORBIS__
	sceKernelSetEventFlag(m_events, 1<<index);
#elif defined __PSVITA__
	sceKernelSetEventFlag(m_events, 1<<index);
#else
	SetEvent(m_events[index]);
#endif //__PS3__
}

void C4JThread::EventArray::Clear(int index)
{
#ifdef __PS3__
	int err =sys_event_flag_clear(m_events, ~(1<<index));
	assert(err == CELL_OK);
#elif defined __ORBIS__
	sceKernelClearEventFlag(m_events, ~(1<<index));
#elif defined __PSVITA__
	sceKernelClearEventFlag(m_events, ~(1<<index));
#else
	ResetEvent(m_events[index]);
#endif //__PS3__
}

void C4JThread::EventArray::SetAll()
{
	for(int i=0;i<m_size;i++)
		Set(i);
}

void C4JThread::EventArray::ClearAll()
{
	for(int i=0;i<m_size;i++)
		Clear(i);
}

DWORD C4JThread::EventArray::WaitForSingle(int index, int timeoutMs )
{
	DWORD retVal;
#ifdef __PS3__
	int timeoutMicrosecs;
	if(timeoutMs == INFINITE)
		timeoutMicrosecs = SYS_NO_TIMEOUT;
	else
		timeoutMicrosecs = timeoutMs * 1000;
	uint32_t mode = SYS_EVENT_FLAG_WAIT_AND;
	if(m_mode == e_modeAutoClear)
		mode |= SYS_EVENT_FLAG_WAIT_CLEAR;

	int err = sys_event_flag_wait(m_events, 1<<index, mode, 0, timeoutMicrosecs);

	switch(err)
	{
	case CELL_OK:
		retVal = WAIT_OBJECT_0;
		break;
	case ETIMEDOUT:
		retVal = WAIT_TIMEOUT;
		break;
	case ECANCELED:
		retVal = WAIT_ABANDONED;
		break;
	default:
		assert(0);
		retVal = WAIT_FAILED;
		break;
	}
#elif defined __ORBIS__
	SceKernelUseconds timeoutMicrosecs;
	SceKernelUseconds *pTimeoutMicrosecs;
	if( timeoutMs == INFINITE )
	{
		pTimeoutMicrosecs = nullptr;
	}
	else
	{
		timeoutMicrosecs = ((SceKernelUseconds)timeoutMs) * 1000;
		pTimeoutMicrosecs = &timeoutMicrosecs;
	}
	uint32_t waitMode = SCE_KERNEL_EVF_WAITMODE_AND;
	if(m_mode == e_modeAutoClear)
	{
		waitMode |= SCE_KERNEL_EVF_WAITMODE_CLEAR_PAT;
	}
	uint64_t resultPat;
	int err = sceKernelWaitEventFlag(m_events, 1<<index, waitMode, &resultPat, pTimeoutMicrosecs);
	assert(err != SCE_KERNEL_ERROR_ETIMEDOUT);
	switch(err)
	{
		case SCE_OK:
			retVal = WAIT_OBJECT_0;
			break;
		case SCE_KERNEL_ERROR_ETIMEDOUT:
			retVal = WAIT_TIMEOUT;
			break;
		case SCE_KERNEL_ERROR_ECANCELED:
			retVal = WAIT_ABANDONED;
			break;
		default:
			retVal = WAIT_FAILED;
			break;
	}
#elif defined __PSVITA__
	SceUInt32 timeoutMicrosecs;
	SceUInt32 *pTimeoutMicrosecs;
	if( timeoutMs == INFINITE )
	{
		pTimeoutMicrosecs = nullptr;
	}
	else
	{
		timeoutMicrosecs = ((SceUInt32)timeoutMs) * 1000;
		pTimeoutMicrosecs = &timeoutMicrosecs;
	}
	uint32_t waitMode = SCE_KERNEL_EVF_WAITMODE_AND;
	if(m_mode == e_modeAutoClear)
	{
		waitMode |= SCE_KERNEL_EVF_WAITMODE_CLEAR_ALL;
	}
	int err = sceKernelWaitEventFlag(m_events, 1<<index, waitMode, nullptr, pTimeoutMicrosecs);
	switch(err)
	{
		case SCE_OK: return WAIT_OBJECT_0;
		case SCE_KERNEL_ERROR_WAIT_TIMEOUT: return WAIT_TIMEOUT;
		case SCE_KERNEL_ERROR_WAIT_CANCEL: return WAIT_ABANDONED;
		default: return WAIT_FAILED;
	}
#else
	retVal = WaitForSingleObject(m_events[index], timeoutMs);
#endif // __PS3__

	return retVal;
}

DWORD C4JThread::EventArray::WaitForAll(int timeoutMs )
{
	DWORD retVal;
#ifdef __PS3__
	if(timeoutMs == INFINITE)
		timeoutMs = SYS_NO_TIMEOUT ;
	int timoutMicrosecs = timeoutMs * 1000;
	unsigned int bitmask = 0;
	for(int i=0;i<m_size;i++)
		bitmask |= (1<<i);

	uint32_t mode = SYS_EVENT_FLAG_WAIT_AND;
	if(m_mode == e_modeAutoClear)
		mode |= SYS_EVENT_FLAG_WAIT_CLEAR;

	int err = sys_event_flag_wait(m_events, bitmask, mode, 0, timoutMicrosecs);

	switch(err)
	{
	case CELL_OK:
		retVal = WAIT_OBJECT_0;
		break;
	case ETIMEDOUT:
		retVal = WAIT_TIMEOUT;
		break;
	case ECANCELED:
		retVal = WAIT_ABANDONED;
		break;
	default:
		assert(0);
		retVal = WAIT_FAILED;
		break;
	}

#elif defined __ORBIS__
	SceKernelUseconds timeoutMicrosecs;
	SceKernelUseconds *pTimeoutMicrosecs;
	if( timeoutMs == INFINITE )
	{
		pTimeoutMicrosecs = nullptr;
	}
	else
	{
		timeoutMicrosecs = ((SceKernelUseconds)timeoutMs) * 1000;
		pTimeoutMicrosecs = &timeoutMicrosecs;
	}
	unsigned int bitmask = 0;
	for(int i=0;i<m_size;i++)
		bitmask |= (1<<i);
	uint32_t waitMode = SCE_KERNEL_EVF_WAITMODE_AND;
	if(m_mode == e_modeAutoClear)
	{
		waitMode |= SCE_KERNEL_EVF_WAITMODE_CLEAR_PAT;
	}
	int err = sceKernelWaitEventFlag(m_events, bitmask, waitMode, nullptr, pTimeoutMicrosecs);
	switch(err)
	{
		case SCE_OK:
			retVal = WAIT_OBJECT_0;
			break;
		case SCE_KERNEL_ERROR_ETIMEDOUT:
			retVal = WAIT_TIMEOUT;
			break;
		case SCE_KERNEL_ERROR_ECANCELED:
			retVal = WAIT_ABANDONED;
			break;
		default:
			retVal = WAIT_FAILED;
			break;
	}
#elif defined __PSVITA__
	SceUInt32 timeoutMicrosecs;
	SceUInt32 *pTimeoutMicrosecs;
	if( timeoutMs == INFINITE )
	{
		pTimeoutMicrosecs = nullptr;
	}
	else
	{
		timeoutMicrosecs = ((SceUInt32)timeoutMs) * 1000;
		pTimeoutMicrosecs = &timeoutMicrosecs;
	}
	unsigned int bitmask = 0;
	for(int i=0;i<m_size;i++)
		bitmask |= (1<<i);
	uint32_t waitMode = SCE_KERNEL_EVF_WAITMODE_AND;
	if(m_mode == e_modeAutoClear)
	{
		waitMode |= SCE_KERNEL_EVF_WAITMODE_CLEAR_ALL;
	}
	int err = sceKernelWaitEventFlag(m_events, bitmask, waitMode, nullptr, pTimeoutMicrosecs);
	switch(err)
	{
		case SCE_OK: return WAIT_OBJECT_0;
		case SCE_KERNEL_ERROR_WAIT_TIMEOUT: return WAIT_TIMEOUT;
		case SCE_KERNEL_ERROR_WAIT_CANCEL: return WAIT_ABANDONED;
		default: return WAIT_FAILED;
	}
#else
	retVal = WaitForMultipleObjects(m_size, m_events, true, timeoutMs);
#endif // __PS3__

	return retVal;
}

DWORD C4JThread::EventArray::WaitForAny(int timeoutMs )
{
#ifdef __PS3__
	if(timeoutMs == INFINITE)
		timeoutMs = SYS_NO_TIMEOUT ;
	int timoutMicrosecs = timeoutMs * 1000;
	unsigned int bitmask = 0;
	for(int i=0;i<m_size;i++)
		bitmask |= (1<<i);

	uint32_t mode = SYS_EVENT_FLAG_WAIT_OR;
	if(m_mode == e_modeAutoClear)
		mode |= SYS_EVENT_FLAG_WAIT_CLEAR;

	int err = sys_event_flag_wait(m_events, bitmask, mode, 0, timoutMicrosecs);

	switch(err)
	{
	case CELL_OK: return WAIT_OBJECT_0;
	case ETIMEDOUT: return WAIT_TIMEOUT;
	case ECANCELED: return WAIT_ABANDONED;
	default:
		assert(0);
		return WAIT_FAILED;
	}

#elif defined __ORBIS__
	SceKernelUseconds timeoutMicrosecs;
	SceKernelUseconds *pTimeoutMicrosecs;
	if( timeoutMs == INFINITE )
	{
		pTimeoutMicrosecs = nullptr;
	}
	else
	{
		timeoutMicrosecs = ((SceKernelUseconds)timeoutMs) * 1000;
		pTimeoutMicrosecs = &timeoutMicrosecs;
	}
	unsigned int bitmask = 0;
	for(int i=0;i<m_size;i++)
		bitmask |= (1<<i);
	uint32_t waitMode = SCE_KERNEL_EVF_WAITMODE_OR;
	if(m_mode == e_modeAutoClear)
	{
		waitMode |= SCE_KERNEL_EVF_WAITMODE_CLEAR_PAT;
	}
	int err = sceKernelWaitEventFlag(m_events, bitmask, waitMode, nullptr, pTimeoutMicrosecs);
	switch(err)
	{
		case SCE_OK: return WAIT_OBJECT_0;
		case SCE_KERNEL_ERROR_ETIMEDOUT: return WAIT_TIMEOUT;
		case SCE_KERNEL_ERROR_ECANCELED: return WAIT_ABANDONED;
		default: return WAIT_FAILED;
	}
#elif defined __PSVITA__
	SceUInt32 timeoutMicrosecs;
	SceUInt32 *pTimeoutMicrosecs;
	if( timeoutMs == INFINITE )
	{
		pTimeoutMicrosecs = nullptr;
	}
	else
	{
		timeoutMicrosecs = ((SceUInt32)timeoutMs) * 1000;
		pTimeoutMicrosecs = &timeoutMicrosecs;
	}
	unsigned int bitmask = 0;
	for(int i=0;i<m_size;i++)
		bitmask |= (1<<i);
	uint32_t waitMode = SCE_KERNEL_EVF_WAITMODE_OR;
	if(m_mode == e_modeAutoClear)
	{
		waitMode |= SCE_KERNEL_EVF_WAITMODE_CLEAR_ALL;
	}
	int err = sceKernelWaitEventFlag(m_events, bitmask, waitMode, nullptr, pTimeoutMicrosecs);
	switch(err)
	{
		case SCE_OK: return WAIT_OBJECT_0;
		case SCE_KERNEL_ERROR_WAIT_TIMEOUT: return WAIT_TIMEOUT;
		case SCE_KERNEL_ERROR_WAIT_CANCEL: return WAIT_ABANDONED;
		default: return WAIT_FAILED;
	}
#else
	return WaitForMultipleObjects(m_size, m_events, false, timeoutMs);
#endif // __PS3__
}

#ifdef __PS3__
void C4JThread::EventArray::Cancel()
{
	sys_event_flag_cancel(m_events, nullptr);
}
#endif 




C4JThread::EventQueue::EventQueue( UpdateFunc* updateFunc, ThreadInitFunc threadInitFunc, const char* szThreadName)
{
	m_updateFunc = updateFunc;
	m_threadInitFunc = threadInitFunc;
	strcpy(m_threadName, szThreadName);
	m_thread = nullptr;
	m_startEvent = nullptr;
	m_finishedEvent = nullptr;
	m_processor = -1;
	m_priority = THREAD_PRIORITY_HIGHEST+1;
}

void C4JThread::EventQueue::init()
{
	m_startEvent = new C4JThread::EventArray(1);
	m_finishedEvent = new C4JThread::Event();
	InitializeCriticalSection(&m_critSect);
	m_thread = new C4JThread(threadFunc, this, m_threadName);
	if(m_processor >= 0)
		m_thread->SetProcessor(m_processor);
	if(m_priority != THREAD_PRIORITY_HIGHEST+1)
		m_thread->SetPriority(m_priority);
	m_thread->Run();
}

void C4JThread::EventQueue::sendEvent( Level* pLevel )
{
	if(m_thread == nullptr)
		init();
	EnterCriticalSection(&m_critSect);
	m_queue.push(pLevel);
	m_startEvent->Set(0);
	m_finishedEvent->Clear();
	LeaveCriticalSection(&m_critSect);
}

void C4JThread::EventQueue::waitForFinish()
{
	if(m_thread == nullptr)
		init();
	EnterCriticalSection(&m_critSect);
	if(m_queue.empty())
	{
		LeaveCriticalSection((&m_critSect));
		return;
	}
	LeaveCriticalSection((&m_critSect));
	m_finishedEvent->WaitForSignal(INFINITE);
}

int C4JThread::EventQueue::threadFunc( void* lpParam )
{
	EventQueue* p = static_cast<EventQueue *>(lpParam);
	p->threadPoll();
	return 0;
}

void C4JThread::EventQueue::threadPoll()
{
	ShutdownManager::HasStarted(ShutdownManager::eEventQueueThreads, m_startEvent);

	if(m_threadInitFunc)
		m_threadInitFunc();

	while(ShutdownManager::ShouldRun(ShutdownManager::eEventQueueThreads))
	{

		DWORD err = m_startEvent->WaitForAny(INFINITE);
		if(err == WAIT_OBJECT_0)
		{
			bool bListEmpty = true;
			do 
			{
				EnterCriticalSection(&m_critSect);
				void* updateParam = m_queue.front();
				LeaveCriticalSection(&m_critSect);

				m_updateFunc(updateParam);

				EnterCriticalSection(&m_critSect);
				m_queue.pop();
				bListEmpty = m_queue.empty();
				if(bListEmpty)
				{
					m_finishedEvent->Set();
				}
				LeaveCriticalSection(&m_critSect);

			} while(!bListEmpty);
		}
	};

	ShutdownManager::HasFinished(ShutdownManager::eEventQueueThreads);
}


#ifdef __ORBIS__

void C4JThread::PushAffinityAllCores()
{
	assert(m_oldAffinityMask == 0);
	int err;
	ScePthread currThreadID = scePthreadSelf();
	err = scePthreadGetaffinity(currThreadID, &m_oldAffinityMask);
	assert(err == SCE_OK);
	err = scePthreadSetaffinity(currThreadID, 63);
	assert(err == SCE_OK);


}

void C4JThread::PopAffinity()
{
	int err;
	ScePthread currThreadID = scePthreadSelf();
	err = scePthreadSetaffinity(currThreadID, m_oldAffinityMask);
	m_oldAffinityMask = 0;
	assert(err == SCE_OK);
}

#endif // __ORBIS__