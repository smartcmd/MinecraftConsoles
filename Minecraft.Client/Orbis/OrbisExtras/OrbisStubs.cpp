#include "stdafx.h"
#include <sce_atomic.h>
#include <fios2.h>
#include <libsysmodule.h>
#include <perf.h>
#include <audioout.h>
#include <game_custom_data_dialog.h>
#include <system_service.h>
#include <error_dialog.h>
#include <message_dialog.h>


//#include <app_content.h>

// These are required so that the heap will automatically expand - default is limited to 256KB
size_t sceLibcHeapSize = SCE_LIBC_HEAP_SIZE_EXTENDED_ALLOC_NO_LIMIT;
unsigned int sceLibcHeapExtendedAlloc = 1;

//static char dirName[128];
//static char contentInfoPath[128];
static char usrdirPath[128];
int32_t hBGMAudio;

//static char sc_loadPath[] = {"/app0/"};
//const char* getConsoleHomePath() { return sc_loadPath; }

char* getUsrDirPath()
{
	return usrdirPath;
}


int _wcsicmp( const wchar_t * dst, const wchar_t * src )
{
	wchar_t f,l;

	// validation section 
	// 	_VALIDATE_RETURN(dst != NULL, EINVAL, _NLSCMPERROR);
	// 	_VALIDATE_RETURN(src != NULL, EINVAL, _NLSCMPERROR);

	do  {
		f = towlower(*dst);
		l = towlower(*src);
		dst++;
		src++;
	} while ( (f) && (f == l) );
	return (int)(f - l);
}

size_t wcsnlen(const wchar_t *wcs, size_t maxsize)
{
    size_t n;

//      Note that we do not check if s == nullptr, because we do not
//      return errno_t...

    for (n = 0; n < maxsize && *wcs; n++, wcs++)
        ;

    return n;
}


VOID GetSystemTime(	LPSYSTEMTIME lpSystemTime)
{
	SceRtcDateTime dateTime;
	int err = sceRtcGetCurrentClock(&dateTime, 0);
	assert(err == SCE_OK);

	lpSystemTime->wYear = sceRtcGetYear(&dateTime);
	lpSystemTime->wMonth = sceRtcGetMonth(&dateTime);
	lpSystemTime->wDay = sceRtcGetDay(&dateTime);
	lpSystemTime->wDayOfWeek = sceRtcGetDayOfWeek(lpSystemTime->wYear, lpSystemTime->wMonth, lpSystemTime->wDay);
	lpSystemTime->wHour = sceRtcGetHour(&dateTime);
	lpSystemTime->wMinute = sceRtcGetMinute(&dateTime);
	lpSystemTime->wSecond = sceRtcGetSecond(&dateTime);
	lpSystemTime->wMilliseconds = sceRtcGetMicrosecond(&dateTime)/1000;
}
BOOL FileTimeToSystemTime(CONST FILETIME *lpFileTime, LPSYSTEMTIME lpSystemTime) { ORBIS_STUBBED; return false; }
BOOL SystemTimeToFileTime(CONST SYSTEMTIME *lpSystemTime, LPFILETIME lpFileTime) { ORBIS_STUBBED; return false; }
VOID GetLocalTime(LPSYSTEMTIME lpSystemTime)
{
	SceRtcDateTime dateTime;
	int err = sceRtcGetCurrentClockLocalTime(&dateTime);
	assert(err == SCE_OK );

	lpSystemTime->wYear = sceRtcGetYear(&dateTime);
	lpSystemTime->wMonth = sceRtcGetMonth(&dateTime);
	lpSystemTime->wDay = sceRtcGetDay(&dateTime);
	lpSystemTime->wDayOfWeek = sceRtcGetDayOfWeek(lpSystemTime->wYear, lpSystemTime->wMonth, lpSystemTime->wDay);
	lpSystemTime->wHour = sceRtcGetHour(&dateTime);
	lpSystemTime->wMinute = sceRtcGetMinute(&dateTime);
	lpSystemTime->wSecond = sceRtcGetSecond(&dateTime);
	lpSystemTime->wMilliseconds = sceRtcGetMicrosecond(&dateTime)/1000;
}

HANDLE CreateEvent(void* lpEventAttributes,	BOOL bManualReset,	BOOL bInitialState,	LPCSTR lpName) { ORBIS_STUBBED; return NULL; }
VOID Sleep(DWORD dwMilliseconds) 
{ 
	C4JThread::Sleep(dwMilliseconds);
}

BOOL SetThreadPriority(HANDLE hThread, int nPriority) { ORBIS_STUBBED; return FALSE; }
DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) { ORBIS_STUBBED; return false; }

LONG InterlockedCompareExchangeRelease(LONG volatile *Destination, LONG Exchange,LONG Comperand	)
{
	return sceAtomicCompareAndSwap32((int32_t*)Destination, (int32_t)Comperand, (int32_t)Exchange);
}

LONG64 InterlockedCompareExchangeRelease64(LONG64 volatile *Destination, LONG64 Exchange, LONG64 Comperand)
{
	return sceAtomicCompareAndSwap64((int64_t*)Destination, (int64_t)Comperand, (int64_t)Exchange);
}


ScePthreadMutexattr mutexParams;
int64_t g_OpStorage[4*1024];    /* 32KiB */
int64_t g_ChunkStorage[8*1024]; /* 64KiB */
int64_t g_FHStorage[2*1024];    /* 16KiB */
int64_t g_DHStorage[512];       /* 4KiB */

VOID OrbisInit()
{
	static bool initialised = false;

	if( initialised ) return;
	initialised = true;

	sceSysmoduleLoadModule(SCE_SYSMODULE_PNG_ENC);
	sceSysmoduleLoadModule(SCE_SYSMODULE_PNG_DEC);
	sceSysmoduleLoadModule(SCE_SYSMODULE_APP_CONTENT);
	sceSysmoduleLoadModule(SCE_SYSMODULE_SAVE_DATA_DIALOG);
	sceSysmoduleLoadModule(SCE_SYSMODULE_IME_DIALOG);
	sceSysmoduleLoadModule(SCE_SYSMODULE_RUDP);
	sceSysmoduleLoadModule(SCE_SYSMODULE_NP_MATCHING2);
	sceSysmoduleLoadModule(SCE_SYSMODULE_INVITATION_DIALOG);
	sceSysmoduleLoadModule(SCE_SYSMODULE_NP_PARTY );
	sceSysmoduleLoadModule(SCE_SYSMODULE_GAME_CUSTOM_DATA_DIALOG );
	sceSysmoduleLoadModule(SCE_SYSMODULE_NP_SCORE_RANKING );
	sceSysmoduleLoadModule(SCE_SYSMODULE_NP_AUTH );
	sceSysmoduleLoadModule(SCE_SYSMODULE_NP_COMMERCE);
	sceSysmoduleLoadModule(SCE_SYSMODULE_REMOTE_PLAY);
	sceSysmoduleLoadModule(SCE_SYSMODULE_ERROR_DIALOG);
	sceSysmoduleLoadModule(SCE_SYSMODULE_MESSAGE_DIALOG);
	sceSysmoduleLoadModule(SCE_SYSMODULE_VOICE);
	sceSysmoduleLoadModule(SCE_SYSMODULE_GAME_LIVE_STREAMING);

	SceFiosParams fiosParams = SCE_FIOS_PARAMS_INITIALIZER;
	fiosParams.opStorage.pPtr = g_OpStorage;
	fiosParams.opStorage.length = sizeof(g_OpStorage);
	fiosParams.chunkStorage.pPtr = g_ChunkStorage;
	fiosParams.chunkStorage.length = sizeof(g_ChunkStorage);
	fiosParams.fhStorage.pPtr = g_FHStorage;
	fiosParams.fhStorage.length = sizeof(g_FHStorage);
	fiosParams.dhStorage.pPtr = g_DHStorage;
	fiosParams.dhStorage.length = sizeof(g_DHStorage);

	int err = sceFiosInitialize(&fiosParams);
	assert(err == SCE_FIOS_OK);

	scePthreadMutexattrInit(&mutexParams);
	scePthreadMutexattrSettype(&mutexParams, SCE_PTHREAD_MUTEX_ADAPTIVE);
#ifdef USE_RAZOR
	const int RAZOR_BUFFER_SIZE = 65536;
	void *razorMem = malloc(RAZOR_BUFFER_SIZE);
	err = sceRazorCpuInit(razorMem, RAZOR_BUFFER_SIZE);
	assert(err == 0 );
#endif
	scePthreadSetaffinity(scePthreadSelf(), 1);

	sceAudioOutInit();
	hBGMAudio=sceAudioOutOpen(
		SCE_USER_SERVICE_USER_ID_SYSTEM,
		SCE_AUDIO_OUT_PORT_TYPE_BGM,0,
		256,
		48000,
		2);

	err = sceCommonDialogInitialize();
	//err = sceNpCommerceDialogInitialize();
	assert(err == SCE_OK);

	// 4J-PB - Can't keep this initialised as we monitor and handle the finished status in the main menu for patches
// 	err = sceErrorDialogInitialize();
// 	assert(err == SCE_OK);

	// 4J-PB - can't init this here - it conflicts with the commerce dialog for dlc checkouts/PSPlus upsells. We need to init it when we use it, and then terminate it.
	//err = sceMsgDialogInitialize();
	assert(err == SCE_OK);
}

int32_t GetAudioBGMHandle()
{
	return hBGMAudio;
}

VOID InitializeCriticalSection(PCRITICAL_SECTION CriticalSection)
{
	char name[1] = {0};

	int err = scePthreadMutexInit(&CriticalSection->mutex, &mutexParams, name);
	CriticalSection->m_cLock = 0;
	assert(err == SCE_OK);
#ifdef _DEBUG
	CriticalSection->m_pOwnerThread = nullptr;
#endif

}


VOID InitializeCriticalSectionAndSpinCount(PCRITICAL_SECTION CriticalSection, ULONG SpinCount)
{
	InitializeCriticalSection(CriticalSection);
}

VOID DeleteCriticalSection(PCRITICAL_SECTION CriticalSection)
{
	int err = scePthreadMutexDestroy(&CriticalSection->mutex);
	assert(err == SCE_OK);
}

extern CRITICAL_SECTION g_singleThreadCS;

VOID EnterCriticalSection(PCRITICAL_SECTION CriticalSection)
{
	int err = scePthreadMutexLock(&CriticalSection->mutex);
	assert(err == SCE_OK || err == SCE_KERNEL_ERROR_EDEADLK );
	CriticalSection->m_cLock++;

#ifdef _DEBUG
	__thread static bool bRecursing = false;
	if(bRecursing == false)
	{
		bRecursing = true;
		CriticalSection->m_pOwnerThread = C4JThread::getCurrentThread();
		bRecursing = false;
	}
#endif
}


VOID LeaveCriticalSection(PCRITICAL_SECTION CriticalSection)
{
	if(--CriticalSection->m_cLock == 0 )
	{
		int err = scePthreadMutexUnlock(&CriticalSection->mutex);
		assert(err == SCE_OK );
#ifdef _DEBUG
		CriticalSection->m_pOwnerThread = nullptr;
#endif

	}
}

ULONG TryEnterCriticalSection(PCRITICAL_SECTION CriticalSection)
{
	int err = scePthreadMutexTrylock(&CriticalSection->mutex);
	if((err == SCE_OK || err == SCE_KERNEL_ERROR_EDEADLK ))
	{
		CriticalSection->m_cLock++;
		return true;
	}
	return false;
}

DWORD WaitForMultipleObjects(DWORD nCount, CONST HANDLE *lpHandles,BOOL bWaitAll,DWORD dwMilliseconds) { ORBIS_STUBBED; return 0; }

BOOL CloseHandle(HANDLE hObject) 
{ 
	sceFiosFHCloseSync(NULL,(SceFiosFH)((int64_t)hObject));
	return true;
// 	ORBIS_STUBBED;
// 	return false;
}

BOOL SetEvent(HANDLE hEvent) { ORBIS_STUBBED; return false; }

HMODULE GetModuleHandle(LPCSTR lpModuleName) { ORBIS_STUBBED; return 0; }

DWORD GetCurrentThreadId(VOID)
{
	return 0; // TODO
}
DWORD WaitForMultipleObjectsEx(DWORD nCount,CONST HANDLE *lpHandles,BOOL bWaitAll,DWORD dwMilliseconds,BOOL bAlertable	) { ORBIS_STUBBED; return 0; }
BOOL GetExitCodeThread(HANDLE hThread, LPDWORD lpExitCode) { ORBIS_STUBBED; return false;}

DWORD TlsAlloc(VOID) { return TLSStorageOrbis::Instance()->Alloc(); }
BOOL TlsFree(DWORD dwTlsIndex) { return TLSStorageOrbis::Instance()->Free(dwTlsIndex); }
LPVOID TlsGetValue(DWORD dwTlsIndex) { return TLSStorageOrbis::Instance()->GetValue(dwTlsIndex); }
BOOL TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue) { return TLSStorageOrbis::Instance()->SetValue(dwTlsIndex, lpTlsValue); }


// we have to manage our own virtual allocs here, so this class stores all the info for each of them
class OrbisVAlloc
{
public:
	class PageInfo
	{
	public:
		off_t				m_physAddr;
		void*				m_virtualAddr;
		uint64_t			m_size;

		PageInfo(off_t physAddr, void* virtualAddr, uint64_t size)
			: m_physAddr(physAddr)
			, m_virtualAddr(virtualAddr)
			, m_size(size)
		{}
	};
	void*					m_virtualAddr;
	uint64_t				m_virtualSize;
	std::vector<PageInfo>	m_pagesAllocated;
	uint64_t				m_allocatedSize;

	OrbisVAlloc(void* addr, uint64_t size)
		: m_virtualAddr(addr)
		, m_virtualSize(size)
		, m_allocatedSize(0)
	{
	}

	~OrbisVAlloc()
	{
		Decommit();
		int err = sceKernelMunmap(m_virtualAddr, m_virtualSize);
		assert( err == SCE_OK );
	}

	void* Commit(uint64_t size)
	{
		uint64_t sizeToAdd = size - m_allocatedSize; // the extra memory size that we have to add on
		assert(sizeToAdd >= 0);

		if(sizeToAdd == 0)
			return m_virtualAddr; // nothing to add


		off_t physAddr;
		// Allocate the physical memory here
		int err = sceKernelAllocateDirectMemory( 0, SCE_KERNEL_MAIN_DMEM_SIZE, sizeToAdd, 16*1024,	SCE_KERNEL_WB_ONION, &physAddr);
		if(err != SCE_OK)
		{
			assert(0);
			return nullptr;
		}
		// work out where the next page should be in virtual addr space, and pass that to the mapping function
		void* pageVirtualAddr = ((char*)m_virtualAddr) + m_allocatedSize;

		void* inAddr = pageVirtualAddr;
		err = sceKernelMapDirectMemory(
			&inAddr,
			sizeToAdd,
			SCE_KERNEL_PROT_CPU_READ | SCE_KERNEL_PROT_CPU_WRITE,
			SCE_KERNEL_MAP_FIXED,
			physAddr,
			16*1024	);

		if(inAddr != pageVirtualAddr) // make sure we actually get the virtual address that we requested
		{
			assert(0);
			return nullptr;
		}
		if(err != SCE_OK)
		{
			assert(0);
			return nullptr;
		}
		m_pagesAllocated.push_back(PageInfo(physAddr, pageVirtualAddr, sizeToAdd));
		m_allocatedSize += sizeToAdd;
		return m_virtualAddr;
	}



	void Decommit()
	{
		// spin round all the pages, unmapping and deallocating them
		for(int i=0;i<m_pagesAllocated.size();i++)
		{
			int err = sceKernelMunmap(m_pagesAllocated[i].m_virtualAddr, m_pagesAllocated[i].m_size);
			assert(err == SCE_OK);
			err = sceKernelReleaseDirectMemory(m_pagesAllocated[i].m_physAddr, m_pagesAllocated[i].m_size);
			assert(err == SCE_OK);
		}
		m_pagesAllocated.clear();
		m_allocatedSize = 0;
	}

};

// List of all virtual allocs that have been made
static std::vector<OrbisVAlloc*> s_orbisVAllocs;


LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect) 
{ 
	if(lpAddress == NULL)
	{
		void *pAddr = (void*)SCE_KERNEL_APP_MAP_AREA_START_ADDR;
		int err = sceKernelReserveVirtualRange(&pAddr, dwSize, 0, 16*1024);
		if( err != SCE_OK )
		{
			app.DebugPrintf("sceKernelReserveVirtualRange failed: 0x%08X\n", err);
			return nullptr;
		}
		s_orbisVAllocs.push_back(new OrbisVAlloc(pAddr, dwSize));
		return (LPVOID)pAddr;
	}
	else
	{
		if( flAllocationType & MEM_COMMIT )
		{
			for(int i=0;i<s_orbisVAllocs.size();i++)
			{
				if(s_orbisVAllocs[i]->m_virtualAddr == lpAddress)
				{
					return s_orbisVAllocs[i]->Commit(dwSize);
				}
			}
			assert(0); // failed to find the virtual alloc in our table
			return nullptr;
		}
	}
	return nullptr;
}

BOOL VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType)
{
	int idx = -1;
	for(int i=0;i<s_orbisVAllocs.size();i++)
	{
		if(s_orbisVAllocs[i]->m_virtualAddr == lpAddress)
		{
			idx = i;
		}
	}

	assert(idx >= 0);
	assert(dwSize == s_orbisVAllocs[idx]->m_virtualSize); // only supporting decommitting the entire memory size

	if(dwFreeType == MEM_DECOMMIT)
	{
		s_orbisVAllocs[idx]->Decommit();
	}
	else if(dwFreeType == MEM_RELEASE)
	{
		delete s_orbisVAllocs[idx];
		s_orbisVAllocs.erase(s_orbisVAllocs.begin()+idx);
	}

	return TRUE;
}

DWORD GetFileSize( HANDLE hFile, LPDWORD lpFileSizeHigh	)
{
	SceFiosSize FileSize;
	SceFiosFH fh = (SceFiosFH)((int64_t)hFile);
	//DWORD FileSizeLow;
	FileSize=sceFiosFHGetSize(fh);
	if(lpFileSizeHigh)
		*lpFileSizeHigh= (DWORD)(FileSize>>32);
	else
	{
		assert(FileSize>>32 == 0);
	}

	return (DWORD)FileSize;
}

BOOL GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize )
{
	SceFiosSize FileSize;
	SceFiosFH fh = (SceFiosFH)((int64_t)hFile);

	FileSize=sceFiosFHGetSize(fh);
	lpFileSize->QuadPart=FileSize;

	return true;
}
BOOL WriteFile(
	HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped )
{
	SceFiosFH fh = (SceFiosFH)((int64_t)hFile);
	// sceFiosFHReadSync - Non-negative values are the number of bytes read, 0 <= result <= length. Negative values are error codes.
	SceFiosSize bytesRead = sceFiosFHWriteSync(nullptr, fh, lpBuffer, (SceFiosSize)nNumberOfBytesToWrite);
	if(bytesRead < 0)
	{
		// error
		return FALSE;
	}
	else
	{
		*lpNumberOfBytesWritten = (DWORD)bytesRead;
		return TRUE;
	}
}

BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped )
{
	SceFiosFH fh = (SceFiosFH)((int64_t)hFile);
	// sceFiosFHReadSync - Non-negative values are the number of bytes read, 0 <= result <= length. Negative values are error codes.
	SceFiosSize bytesRead = sceFiosFHReadSync(nullptr, fh, lpBuffer, (SceFiosSize)nNumberOfBytesToRead);
	*lpNumberOfBytesRead = (DWORD)bytesRead;
	if(bytesRead < 0)
	{
		// error
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
	SceFiosFH fd = (SceFiosFH)((int64_t)hFile);

	uint64_t bitsToMove = (int64_t) lDistanceToMove;
	SceFiosOffset pos = 0;

	if (lpDistanceToMoveHigh != nullptr)
		bitsToMove |= ((uint64_t) (*lpDistanceToMoveHigh)) << 32;

	SceFiosWhence whence = SCE_FIOS_SEEK_SET;
	switch(dwMoveMethod)
	{
	case FILE_BEGIN:	whence = SCE_FIOS_SEEK_SET; break;
	case FILE_CURRENT:	whence = SCE_FIOS_SEEK_CUR; break;
	case FILE_END:		whence = SCE_FIOS_SEEK_END; break;
	};

	pos = sceFiosFHSeek(fd, (int64_t) lDistanceToMove, whence);

	return (pos != -1);
}


HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	char filePath[256];
	std::string mountedPath = StorageManager.GetMountedPath(lpFileName);
	if(mountedPath.length() > 0)
	{
		strcpy(filePath, mountedPath.c_str());
	}
	else if(lpFileName[0] == '/') // already fully qualified path
		strcpy(filePath, lpFileName );
	else
		sprintf(filePath,"%s/%s",getUsrDirPath(), lpFileName );

#ifndef _CONTENT_PACKAGE
	app.DebugPrintf("*** Opening %s\n",filePath);
#endif

	SceFiosFH fh;
	SceFiosOpenParams openParams;
	ZeroMemory(&openParams, sizeof(SceFiosOpenParams));

	switch(dwDesiredAccess)
	{
	case GENERIC_READ:
		openParams.openFlags = SCE_FIOS_O_RDONLY;	break;
	case GENERIC_WRITE:
		openParams.openFlags = SCE_FIOS_O_WRONLY;	break;
	default:
		openParams.openFlags = SCE_FIOS_O_READ | SCE_FIOS_O_WRITE;		break;
	}

	switch(dwCreationDisposition)
	{
	case CREATE_ALWAYS:
		openParams.openFlags |= SCE_FIOS_O_CREAT;	break;
	case CREATE_NEW:
		openParams.openFlags |= SCE_FIOS_O_CREAT;	break;
	case OPEN_ALWAYS:
		openParams.openFlags |= SCE_FIOS_O_CREAT;	break;
	case OPEN_EXISTING:
		break;
	case TRUNCATE_EXISTING:
		break;
	}
	int err = sceFiosFHOpenSync(nullptr, &fh, filePath, &openParams);

	if(err != SCE_FIOS_OK)
	{
		return INVALID_HANDLE_VALUE;
	}
	//assert( err == SCE_FIOS_OK );

	return (void*)fh;
}

BOOL CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes){ ORBIS_STUBBED; return false; }
BOOL DeleteFileA(LPCSTR lpFileName) { ORBIS_STUBBED; return false; }

// BOOL XCloseHandle(HANDLE a)
// {
// 	sceFiosFHCloseSync(nullptr,(SceFiosFH)((int64_t)a));
// 	return true;
// }


DWORD GetFileAttributesA(LPCSTR lpFileName)
{
	char filePath[256];
	std::string mountedPath = StorageManager.GetMountedPath(lpFileName);
	if(mountedPath.length() > 0)
	{
		strcpy(filePath, mountedPath.c_str());
	}
	else if(lpFileName[0] == '/') // already fully qualified path
		strcpy(filePath, lpFileName );
	else
		sprintf(filePath,"%s/%s",getUsrDirPath(), lpFileName );	// set to load from host

	// check if the file exists first
	SceFiosStat  statData;
	if(sceFiosStatSync(nullptr, filePath, &statData) != SCE_FIOS_OK)
	{
		app.DebugPrintf("*** sceFiosStatSync Failed\n");
		return -1;
	}
	if(statData.statFlags & SCE_FIOS_STATUS_DIRECTORY )
		return FILE_ATTRIBUTE_DIRECTORY;
	else
		return FILE_ATTRIBUTE_NORMAL;
}


BOOL MoveFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName) { ORBIS_STUBBED; return false; }


DWORD GetLastError(VOID) { ORBIS_STUBBED; return 0; }
VOID GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer)
{
	SceLibcMallocManagedSize stat;
	int err = malloc_stats(&stat);
	if(err != 0)
	{
		app.DebugPrintf("Failed to get mem stats\n");
	}

	lpBuffer->dwTotalPhys = stat.maxSystemSize;
	lpBuffer->dwAvailPhys = stat.maxSystemSize - stat.currentSystemSize;
	lpBuffer->dwAvailVirtual = stat.maxSystemSize - stat.currentInuseSize;
}

DWORD GetTickCount()
{
	// This function returns the current system time at this function is called.
	// The system time is represented the time elapsed since the system starts up in microseconds.
	uint64_t sysTime = sceKernelGetProcessTime();
	return (DWORD)(sysTime / 1000);
}

// we should really use libperf for this kind of thing, but this will do for now.
BOOL QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency)
{
	// microseconds
	lpFrequency->QuadPart =  (1000 * 1000);
	return false;
}
BOOL QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
{
	// microseconds
	lpPerformanceCount->QuadPart = sceKernelGetProcessTime();
	return true;
}

#ifndef _FINAL_BUILD

VOID OutputDebugStringW(LPCWSTR lpOutputString)
{
	wprintf(lpOutputString);
}

VOID OutputDebugStringA(LPCSTR lpOutputString)
{
	printf(lpOutputString);
}

VOID OutputDebugString(LPCSTR lpOutputString)
{
	printf(lpOutputString);
}
#endif // _CONTENT_PACKAGE

BOOL GetFileAttributesExA(LPCSTR lpFileName,GET_FILEEX_INFO_LEVELS fInfoLevelId,LPVOID lpFileInformation)
{
	ORBIS_STUBBED;
	return false;
}
HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData) { ORBIS_STUBBED; return 0;}
BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData) { ORBIS_STUBBED; return false;}

errno_t _itoa_s(int _Value, char * _DstBuf, size_t _Size, int _Radix) { if(_Radix==10) sprintf(_DstBuf,"%d",_Value); else if(_Radix==16) sprintf(_DstBuf,"%lx",_Value); else return -1; return 0; }
errno_t _i64toa_s(int64_t _Val, char * _DstBuf, size_t _Size, int _Radix) { if(_Radix==10) sprintf(_DstBuf,"%lld",_Val); else return -1; return 0; }

DWORD XGetLanguage()
{
	unsigned char ucLang = app.GetMinecraftLanguage(0);
	int iLang;

	// check if we should override the system language or not
	if(ucLang==MINECRAFT_LANGUAGE_DEFAULT)
	{
		sceSystemServiceParamGetInt(SCE_SYSTEM_SERVICE_PARAM_ID_LANG,&iLang);
	}
	else
	{
		return (DWORD)ucLang;
	}

	switch(iLang)
	{
	case SCE_SYSTEM_PARAM_LANG_JAPANESE			: return XC_LANGUAGE_JAPANESE;
	case SCE_SYSTEM_PARAM_LANG_ENGLISH_US		: return XC_LANGUAGE_ENGLISH;
	case SCE_SYSTEM_PARAM_LANG_FRENCH			: return XC_LANGUAGE_FRENCH;

	case SCE_SYSTEM_PARAM_LANG_SPANISH			: return XC_LANGUAGE_SPANISH;
	case SCE_SYSTEM_PARAM_LANG_SPANISH_LA		: return XC_LANGUAGE_LATINAMERICANSPANISH;

	case SCE_SYSTEM_PARAM_LANG_GERMAN			: return XC_LANGUAGE_GERMAN;
	case SCE_SYSTEM_PARAM_LANG_ITALIAN			: return XC_LANGUAGE_ITALIAN;
	case SCE_SYSTEM_PARAM_LANG_PORTUGUESE_PT	: return XC_LANGUAGE_PORTUGUESE;

	case SCE_SYSTEM_PARAM_LANG_RUSSIAN			: return XC_LANGUAGE_RUSSIAN;
	case SCE_SYSTEM_PARAM_LANG_KOREAN			: return XC_LANGUAGE_KOREAN;
	case SCE_SYSTEM_PARAM_LANG_CHINESE_T		: return XC_LANGUAGE_TCHINESE;
	case SCE_SYSTEM_PARAM_LANG_PORTUGUESE_BR	: return XC_LANGUAGE_PORTUGUESE;
	case SCE_SYSTEM_PARAM_LANG_ENGLISH_GB		: return XC_LANGUAGE_ENGLISH;

	case SCE_SYSTEM_PARAM_LANG_DUTCH			: return XC_LANGUAGE_DUTCH;
	case SCE_SYSTEM_PARAM_LANG_FINNISH			: return XC_LANGUAGE_FINISH;
	case SCE_SYSTEM_PARAM_LANG_SWEDISH 			: return XC_LANGUAGE_SWEDISH;
	case SCE_SYSTEM_PARAM_LANG_DANISH 			: return XC_LANGUAGE_DANISH;
	case SCE_SYSTEM_PARAM_LANG_NORWEGIAN  		: return XC_LANGUAGE_BNORWEGIAN;
	case SCE_SYSTEM_PARAM_LANG_POLISH  			: return XC_LANGUAGE_POLISH;
	case SCE_SYSTEM_PARAM_LANG_TURKISH			: return XC_LANGUAGE_TURKISH;


	case SCE_SYSTEM_PARAM_LANG_CHINESE_S		: return XC_LANGUAGE_SCHINESE;

	default										: return XC_LANGUAGE_ENGLISH;
	}

}
DWORD XGetLocale()
{
	int iLang;
	sceSystemServiceParamGetInt(SCE_SYSTEM_SERVICE_PARAM_ID_LANG,&iLang);
	switch(iLang)
	{
	case SCE_SYSTEM_PARAM_LANG_JAPANESE			: return XC_LOCALE_JAPAN;
	case SCE_SYSTEM_PARAM_LANG_ENGLISH_US		: return XC_LOCALE_UNITED_STATES;
	case SCE_SYSTEM_PARAM_LANG_FRENCH			: return XC_LOCALE_FRANCE;

	case SCE_SYSTEM_PARAM_LANG_SPANISH			: return XC_LOCALE_SPAIN;
	case SCE_SYSTEM_PARAM_LANG_SPANISH_LA		: return XC_LOCALE_LATIN_AMERICA;

	case SCE_SYSTEM_PARAM_LANG_GERMAN			: return XC_LOCALE_GERMANY;
	case SCE_SYSTEM_PARAM_LANG_ITALIAN			: return XC_LOCALE_ITALY;
	case SCE_SYSTEM_PARAM_LANG_PORTUGUESE_PT	: return XC_LOCALE_PORTUGAL;

	case SCE_SYSTEM_PARAM_LANG_RUSSIAN			: return XC_LOCALE_RUSSIAN_FEDERATION;
	case SCE_SYSTEM_PARAM_LANG_KOREAN			: return XC_LOCALE_KOREA;
	case SCE_SYSTEM_PARAM_LANG_CHINESE_T		: return XC_LOCALE_CHINA;
	case SCE_SYSTEM_PARAM_LANG_PORTUGUESE_BR	: return XC_LOCALE_BRAZIL;
	case SCE_SYSTEM_PARAM_LANG_ENGLISH_GB		: return XC_LOCALE_GREAT_BRITAIN;

	case SCE_SYSTEM_PARAM_LANG_DUTCH			: return XC_LOCALE_NETHERLANDS;
	case SCE_SYSTEM_PARAM_LANG_FINNISH			: return XC_LOCALE_FINLAND;
	case SCE_SYSTEM_PARAM_LANG_SWEDISH 			: return XC_LOCALE_SWEDEN;
	case SCE_SYSTEM_PARAM_LANG_DANISH 			: return XC_LOCALE_DENMARK;
	case SCE_SYSTEM_PARAM_LANG_NORWEGIAN  		: return XC_LOCALE_NORWAY;
	case SCE_SYSTEM_PARAM_LANG_POLISH  			: return XC_LOCALE_POLAND;
	case SCE_SYSTEM_PARAM_LANG_TURKISH			: return XC_LOCALE_TURKEY;


	case SCE_SYSTEM_PARAM_LANG_CHINESE_S		: return XC_LOCALE_CHINA;
	default										: return XC_LOCALE_UNITED_STATES;
	}
}

DWORD XEnableGuestSignin(BOOL fEnable)
{
	return 0;
}