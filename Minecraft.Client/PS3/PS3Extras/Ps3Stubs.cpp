#ifdef __PS3__


#include "stdafx.h"
#include <libsn.h>
#include <cell/cell_fs.h>
#include <sys/vm.h>
#include <sys/memory.h> 
#undef __in
#undef __out
#include <cell/atomic.h>
#include <sysutil/sysutil_gamecontent.h>
#include <np.h>
#include <np/drm.h>
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_sysparam.h>

#include "ShutdownManager.h"


static char dirName[CELL_GAME_PATH_MAX];
static char contentInfoPath[CELL_GAME_PATH_MAX];
static char usrdirPath[CELL_GAME_PATH_MAX];
static char contentInfoPathBDPatch[CELL_GAME_PATH_MAX];
static char usrdirPathBDPatch[CELL_GAME_PATH_MAX];
//static char sc_loadPath[] = {"/app_home/"};
//static char sc_loadPath[CELL_GAME_PATH_MAX];
static int iFilesOpen=0;
vector<int> vOpenFileHandles;

namespace boost
{
	void assertion_failed(char const * expr,
		char const * function, char const * file, long line) 
	{ 
#ifndef _CONTENT_PACKAGE
		printf("Assert failed!!!\n"); 
		printf(expr); 
		printf("\n"); 
		printf("----------------------\n    %s failed. File %s at line %d \n----------------------\n", function, file, line); 
		snPause();
#endif
	} // user defined
} // namespace boost

 char* getConsoleHomePath() 
 { 
 	return contentInfoPath; 
 }

char* getUsrDirPath() 
{ 
	return usrdirPath; 
}

char* getConsoleHomePathBDPatch() 
{ 
	return contentInfoPathBDPatch; 
}

char* getUsrDirPathBDPatch() 
{ 
	return usrdirPathBDPatch; 
}


char* getDirName() 
{ 
	return dirName; 
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

//      Note that we do not check if s == NULL, because we do not
//      return errno_t...

    for (n = 0; n < maxsize && *wcs; n++, wcs++)
        ;

    return n;
}

Cnullptr nullptr;

VOID GetSystemTime(	LPSYSTEMTIME lpSystemTime) 
{
	CellRtcDateTime dateTime;
	int err = cellRtcGetCurrentClock(&dateTime, 0);
	assert(err == CELL_OK);

	lpSystemTime->wYear = cellRtcGetYear(&dateTime);
	lpSystemTime->wMonth = cellRtcGetMonth(&dateTime);
	lpSystemTime->wDay = cellRtcGetDay(&dateTime);
	lpSystemTime->wDayOfWeek = cellRtcGetDayOfWeek(lpSystemTime->wYear, lpSystemTime->wMonth, lpSystemTime->wDay);
	lpSystemTime->wHour = cellRtcGetHour(&dateTime);
	lpSystemTime->wMinute = cellRtcGetMinute(&dateTime);
	lpSystemTime->wSecond = cellRtcGetSecond(&dateTime);
	lpSystemTime->wMilliseconds = cellRtcGetMicrosecond(&dateTime)/1000;
}
BOOL FileTimeToSystemTime(CONST FILETIME *lpFileTime, LPSYSTEMTIME lpSystemTime) { PS3_STUBBED; return false; }
BOOL SystemTimeToFileTime(CONST SYSTEMTIME *lpSystemTime, LPFILETIME lpFileTime) { PS3_STUBBED; return false; }
VOID GetLocalTime(LPSYSTEMTIME lpSystemTime) 
{ 
	CellRtcDateTime dateTime;
	int err = cellRtcGetCurrentClockLocalTime(&dateTime);
	assert(err == CELL_OK);

	lpSystemTime->wYear = cellRtcGetYear(&dateTime);
	lpSystemTime->wMonth = cellRtcGetMonth(&dateTime);
	lpSystemTime->wDay = cellRtcGetDay(&dateTime);
	lpSystemTime->wDayOfWeek = cellRtcGetDayOfWeek(lpSystemTime->wYear, lpSystemTime->wMonth, lpSystemTime->wDay);
	lpSystemTime->wHour = cellRtcGetHour(&dateTime);
	lpSystemTime->wMinute = cellRtcGetMinute(&dateTime);
	lpSystemTime->wSecond = cellRtcGetSecond(&dateTime);
	lpSystemTime->wMilliseconds = cellRtcGetMicrosecond(&dateTime)/1000;
}

HANDLE CreateEvent(void* lpEventAttributes,	BOOL bManualReset,	BOOL bInitialState,	LPCSTR lpName) { PS3_STUBBED; return NULL; }
VOID Sleep(DWORD dwMilliseconds) 
{ 
	C4JThread::Sleep(dwMilliseconds);
}

BOOL SetThreadPriority(HANDLE hThread, int nPriority) { PS3_STUBBED; return FALSE; }
DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) { PS3_STUBBED; return false; }

LONG InterlockedCompareExchangeRelease(LONG volatile *Destination, LONG Exchange,LONG Comperand	) 
{ 
	return cellAtomicCompareAndSwap32((uint32_t*)Destination, (uint32_t)Comperand, (uint32_t)Exchange);
}

LONG64 InterlockedCompareExchangeRelease64(LONG64 volatile *Destination, LONG64 Exchange, LONG64 Comperand) 
{ 
	return cellAtomicCompareAndSwap64((uint64_t*)Destination, (uint64_t)Comperand, (uint64_t)Exchange);
}


VOID InitializeCriticalSection(PCRITICAL_SECTION CriticalSection) 
{
	sys_lwmutex_attribute_t attr;
	// from the defaults in sys_lwmutex_attribute_initialize
	attr.attr_protocol = SYS_SYNC_PRIORITY;
	attr.attr_recursive =  SYS_SYNC_RECURSIVE;
	attr.name[0] = '\0';

	int err = sys_lwmutex_create(CriticalSection, &attr);
	PS3_ASSERT_CELL_ERROR(err);
}


VOID InitializeCriticalSectionAndSpinCount(PCRITICAL_SECTION CriticalSection, ULONG SpinCount) 
{
	// no spin count on PS3
	InitializeCriticalSection(CriticalSection);
}

VOID DeleteCriticalSection(PCRITICAL_SECTION CriticalSection)
{
	int err = sys_lwmutex_destroy(CriticalSection);
	PS3_ASSERT_CELL_ERROR(err);
}

extern CRITICAL_SECTION g_singleThreadCS; 

VOID EnterCriticalSection(PCRITICAL_SECTION CriticalSection) 
{
// 	if(CriticalSection != &g_singleThreadCS &&(C4JThread::isMainThread() == false) )
// 		LeaveCriticalSection(&g_singleThreadCS);
	int err = sys_lwmutex_lock(CriticalSection, 0); // timeout, in microsecs - 0 here
	PS3_ASSERT_CELL_ERROR(err);
// 	if(CriticalSection != &g_singleThreadCS && !C4JThread::isMainThread())
// 		EnterCriticalSection(&g_singleThreadCS);
}


VOID LeaveCriticalSection(PCRITICAL_SECTION CriticalSection) 
{
	int err =  sys_lwmutex_unlock(CriticalSection);
	PS3_ASSERT_CELL_ERROR(err);
}

ULONG TryEnterCriticalSection(PCRITICAL_SECTION CriticalSection)
{
	int err = sys_lwmutex_trylock(CriticalSection); 
	if(err == CELL_OK)
		return true;
	return false;
}
DWORD WaitForMultipleObjects(DWORD nCount, CONST HANDLE *lpHandles,BOOL bWaitAll,DWORD dwMilliseconds) { PS3_STUBBED; return 0; }



BOOL CloseHandle(HANDLE hObject) 
{ 
	if(hObject==INVALID_HANDLE_VALUE)
	{
		//printf("\n\nTRYING TO CLOSE AN INVALID FILE HANDLE\n\n");
		return false;
	}
	else
	{
		CellFsErrno err;
		err=cellFsClose(int(hObject)); 
		if(err==CELL_FS_SUCCEEDED)
		{
			iFilesOpen--;

			AUTO_VAR(itEnd, vOpenFileHandles.end());
			for (AUTO_VAR(it, vOpenFileHandles.begin()); it != itEnd; it++)
			{
				int iFH=(int)*it;
				if(iFH==(int)hObject)
				{
					vOpenFileHandles.erase(it);
					break;
				}
			}
			//printf("\n\nFiles Open - %d\n\n",iFilesOpen);
			return true; 
		}
		else
		{
			return false;
		}
	}

}

BOOL SetEvent(HANDLE hEvent) { PS3_STUBBED; return false; }

HMODULE GetModuleHandle(LPCSTR lpModuleName) { PS3_STUBBED; return 0; }

sys_ppu_thread_t GetCurrentThreadId(VOID) 
{ 
	sys_ppu_thread_t id;
	int err = sys_ppu_thread_get_id(&id);
	PS3_ASSERT_CELL_ERROR(err);
	return id;
}
DWORD WaitForMultipleObjectsEx(DWORD nCount,CONST HANDLE *lpHandles,BOOL bWaitAll,DWORD dwMilliseconds,BOOL bAlertable	) { PS3_STUBBED; return 0; }
BOOL GetExitCodeThread(HANDLE hThread, LPDWORD lpExitCode) { PS3_STUBBED; return FALSE; }

DWORD TlsAlloc(VOID) { return TLSStoragePS3::Instance()->Alloc(); }
BOOL TlsFree(DWORD dwTlsIndex) { return TLSStoragePS3::Instance()->Free(dwTlsIndex); }
LPVOID TlsGetValue(DWORD dwTlsIndex) { return TLSStoragePS3::Instance()->GetValue(dwTlsIndex); }
BOOL TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue) { return TLSStoragePS3::Instance()->SetValue(dwTlsIndex, lpTlsValue); }

LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect) 
{ 
	int err;
	sys_addr_t newAddress = NULL;
	if(lpAddress == NULL)
	{
		// reserve, and possibly commit also
		int commitSize = 0;
		if(flAllocationType == MEM_RESERVE)
			commitSize = 1024*1024;	// has to be a min of 1mb
		else if(flAllocationType == MEM_COMMIT)
			commitSize = dwSize;
		else
		{
			PS3_ASSERT(0);
		}
		err = sys_vm_memory_map(dwSize, commitSize, SYS_MEMORY_CONTAINER_ID_INVALID,
			SYS_MEMORY_PAGE_SIZE_64K, SYS_VM_POLICY_AUTO_RECOMMENDED, &newAddress);
		PS3_ASSERT(err == CELL_OK);
	}
	else
	{
		// reserved already, so just commit
		sys_vm_statistics_t stat;
		err = sys_vm_get_statistics((sys_addr_t)lpAddress, &stat);
		PS3_ASSERT(err == CELL_OK);
		int memToAdd = dwSize - stat.pmem_total;

		err = sys_vm_append_memory((sys_addr_t)lpAddress, memToAdd);
		PS3_ASSERT(err == CELL_OK);
		newAddress = (sys_addr_t)lpAddress;
	}
	return (LPVOID)newAddress;
}

BOOL VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType) 
{
	if(dwFreeType == MEM_DECOMMIT)
	{
		int err;
		sys_vm_statistics_t stat;
		err = sys_vm_get_statistics((sys_addr_t)lpAddress, &stat);
		PS3_ASSERT(err == CELL_OK);
		
		// 4J Stu - We can only return what we have actually committed on PS3
		// From PS3 Docs:
		// The maximum amount of memory that can be returned is the difference of the total amount of physical memory used by the virtual memory area minus 1MB. When an amount exceeding this value is specified, EBUSY will return.
		SIZE_T memToFree = stat.pmem_total - (1024 * 1024);
		if(dwSize < memToFree) 
			memToFree = dwSize;

		app.DebugPrintf("VirtualFree: Requested size - %d, Actual size - %d\n", dwSize, memToFree);
		err = sys_vm_return_memory((sys_addr_t)lpAddress, memToFree);
		PS3_ASSERT_CELL_ERROR(err);
		return FALSE;
	}
	else
	{
		assert(dwFreeType == MEM_RELEASE);
		assert(0); // not implemented yet
	}
	return TRUE;
}

DWORD GetFileSize( HANDLE hFile, LPDWORD lpFileSizeHigh	)
{
	// check if the file exists first
	//printf("GetFileSize\n");

	CellFsStat statData;
	if(cellFsFstat((int)hFile, &statData) != CELL_FS_SUCCEEDED)
		return 0;
	if(statData.st_mode == CELL_FS_S_IFDIR)
		return 0;
	//printf("+++GetFileSize - %ll\n",statData.st_size);

	return statData.st_size;
}
BOOL GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize ) { PS3_STUBBED; return false; }
BOOL WriteFile(	HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped )
{
	int fd = (int)hFile;
	uint64_t bytesWritten;
	CellFsErrno err = cellFsWrite(fd, lpBuffer, nNumberOfBytesToWrite, &bytesWritten);
	*lpNumberOfBytesWritten = (DWORD)bytesWritten;
	return (err == CELL_FS_OK); 
}

BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped ) 
{
	int fd = (int)hFile;
	uint64_t bytesRead;
	CellFsErrno err = cellFsRead(fd, lpBuffer, nNumberOfBytesToRead, &bytesRead);
	*lpNumberOfBytesRead = (DWORD)bytesRead;

	// fake a read error due to disc eject

	//err=CELL_FS_EBADF;

	switch(err)
	{
	case CELL_FS_EBADF: // disc eject
		CloseHandle((HANDLE)fd);
		// sys message will come in and flag to start a shutdown
		//ShutdownManager::StartShutdown();
		break;
	default:
		break;
	}

	return (err==CELL_FS_SUCCEEDED); 
}

BOOL SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
	int fd = (int) hFile;
	uint64_t pos = 0, bitsToMove = (int64_t) lDistanceToMove;

	if (lpDistanceToMoveHigh != NULL)
		bitsToMove |= ((uint64_t) (*lpDistanceToMoveHigh)) << 32;

	int whence = 0;
	switch(dwMoveMethod)
	{
	case FILE_BEGIN:	whence = CELL_FS_SEEK_SET; break;
	case FILE_CURRENT:	whence = CELL_FS_SEEK_CUR; break;
	case FILE_END:		whence = CELL_FS_SEEK_END; break;
	};

	CellFsErrno err = cellFsLseek(fd, (int64_t) lDistanceToMove, whence, &pos);

	return (err == CELL_FS_SUCCEEDED);
}
void replaceBackslashes(char* szFilename)
{
	int len = strlen(szFilename);
	for(int i=0;i<len;i++)
	{
		if(szFilename[i] == '\\')
			szFilename[i] = '/';
	}
}

HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) 
{
	char filePath[256];
	std::string mountedPath;

	// 4J-PB - If we are running from the blu ray patch , then we need to check if we should load a file from the blu ray or the patch dir

	if(app.GetBootedFromDiscPatch() && app.IsFileInPatchList(lpFileName))
	{
		// does this file need to be loaded from the patch directory?
#ifndef _CONTENT_PACKAGE
		printf("*** Found a patched file %s\n",lpFileName);
#endif
		char *pchPatchPath = app.GetDiscPatchUsrDir();
		if(lpFileName[0] == '/') // already fully qualified path
			strcpy(filePath, lpFileName );
		else
			sprintf(filePath,"%s/%s",pchPatchPath, lpFileName );
	}
	else
	{
		mountedPath = StorageManager.GetMountedPath(lpFileName);
		if(mountedPath.length() > 0)
		{
			strcpy(filePath, mountedPath.c_str());
		}
		else if(lpFileName[0] == '/') // already fully qualified path
			strcpy(filePath, lpFileName );
		else
			sprintf(filePath,"%s/%s",usrdirPath, lpFileName );	// set to load from host
	}


	replaceBackslashes(filePath);

	int fd = 0;
	int flags = 0;
	switch(dwDesiredAccess)
	{ 
	case GENERIC_READ:
		flags = CELL_FS_O_RDONLY;	break;
	case GENERIC_WRITE:
		flags = CELL_FS_O_WRONLY;	break;
	default:
		flags = CELL_FS_O_RDWR;		break;
	}

	switch(dwCreationDisposition)
	{
	case CREATE_ALWAYS:
		flags |= CELL_FS_O_CREAT;	break;
	case CREATE_NEW:
		flags |= CELL_FS_O_EXCL;	break;
	case OPEN_ALWAYS:
		flags |= CELL_FS_O_CREAT;	break;
	case OPEN_EXISTING:
		break;
	case TRUNCATE_EXISTING:
		break;
	}

	CellFsErrno err;

#ifndef _CONTENT_PACKAGE
	printf("*** Opening %s\n",filePath);
#endif

	// check if it's a DRM edat file
	int filePathLen = strlen(filePath);
	if( (filePath[filePathLen-5] == '.' || filePath[filePathLen-4] == '.') &&
		(filePath[filePathLen-4] == 'e' || filePath[filePathLen-4] == 'E') &&
		(filePath[filePathLen-3] == 'd' || filePath[filePathLen-3] == 'D') &&
		(filePath[filePathLen-2] == 'a' || filePath[filePathLen-2] == 'A') &&
		(filePath[filePathLen-1] == 't' || filePath[filePathLen-1] == 'T') )
	{
		SceNpDrmOpenArg arg;
		arg.flag = SCE_NP_DRM_OPEN_FLAG;
		err = cellFsOpen(filePath, flags, &fd, &arg, sizeof(arg));
		iFilesOpen++;
		//printf("\n\nNPDRM Files Open - %d\n\n",iFilesOpen);
	}
	else
	{
		err = cellFsOpen(filePath, flags, &fd ,NULL, 0);
		iFilesOpen++;
		//printf("\n\nFiles Open - %d\n\n",iFilesOpen);
	}
	//PS3_ASSERT(err == CELL_FS_OK);

	CellFsStat statData;

	err = cellFsFstat(fd,&statData);

	// fake a disc eject
	//err=CELL_FS_EBADF;
#ifndef _CONTENT_PACKAGE
	printf("*** Open error %d\n",err);
#endif
	switch(err)
	{
	case CELL_FS_SUCCEEDED:
		{		
			app.DebugPrintf("CELL_FS_SUCCEEDED\n");
			DWORD dwFileSize = (DWORD)statData.st_size;

			app.DebugPrintf("Filesize is %d\n", dwFileSize);

			// 4J-PB - push the file handle on to a vector
			vOpenFileHandles.push_back(fd);
		}
		break;
	case CELL_FS_EBADF:
		// fd is invalid or the file system on a removable media has been unmounted

		// When cellFsFstat() returns CELL_FS_EBADF, it can be deduced that the above error occurred because a disc was ejected.
		// Explicitly call cellFsClose() and close the applicable file. When using stream supporting APIs, call cellFsStReadFinish() before calling cellFsClose(). 

		app.DebugPrintf("CELL_FS_EBADF\n");
		CloseHandle((HANDLE)fd);

		// sys message will come in to start shutdown
		//ShutdownManager::StartShutdown();

		return INVALID_HANDLE_VALUE;
		
	case CELL_FS_EIO:
		app.DebugPrintf("CELL_FS_EIO\n");
		break;
	case CELL_FS_ENOMEM:
		app.DebugPrintf("CELL_FS_ENOMEM\n");
		break;
	case CELL_FS_EFSSPECIFIC:
		app.DebugPrintf("CELL_FS_EFSSPECIFIC\n");
		break;
	case CELL_FS_EFAULT:
		app.DebugPrintf("CELL_FS_EFAULT\n");
		break;
	}


	return (void*)fd;
}

BOOL CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) 
{ 
	char filePath[256];
	sprintf(filePath,"%s/%s",usrdirPath, lpPathName );
	CellFsErrno err = cellFsMkdir(filePath, CELL_FS_DEFAULT_CREATE_MODE_1);
	if(err == CELL_FS_SUCCEEDED)
		return true;
	//printf("cellFsMkdir failed with error 0x%08x\n", err);
	return false;
}

BOOL DeleteFileA(LPCSTR lpFileName) { PS3_STUBBED; return false; }

// BOOL XCloseHandle(HANDLE a) 
// { 
// 	cellFsClose(int(a)); 
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
		sprintf(filePath,"%s/%s",usrdirPath, lpFileName );	// set to load from host

	//strcat(filePath,".edat");
			
	//printf("GetFileAttributesA - %s\n",filePath);

	// check if the file exists first
	CellFsStat statData;
	if(cellFsStat(filePath, &statData) != CELL_FS_SUCCEEDED)
	{
		//printf("GetFileAttributesA FAIL\n");
		return -1;
	}
	if( statData.st_mode & CELL_FS_S_IFDIR )
		return FILE_ATTRIBUTE_DIRECTORY;
	else
		return FILE_ATTRIBUTE_NORMAL;
}


BOOL MoveFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName) { PS3_STUBBED; return false; }

void __debugbreak() { snPause(); }
VOID DebugBreak(VOID) { snPause(); }


DWORD GetLastError(VOID) { PS3_STUBBED; return 0; }
VOID GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer) 
{
	malloc_managed_size stat;
	int err = malloc_stats(&stat);
	if(err != 0)
	{
		//printf("Failed to get mem stats\n");
	}

	lpBuffer->dwTotalPhys = stat.max_system_size;
	lpBuffer->dwAvailPhys = stat.max_system_size - stat.current_system_size;
	lpBuffer->dwAvailVirtual = stat.max_system_size - stat.current_inuse_size;
}

DWORD GetTickCount() 
{
	// This function returns the current system time at this function is called. 
	// The system time is represented the time elapsed since the system starts up in microseconds.
	system_time_t sysTime = sys_time_get_system_time();
	return sysTime / 1000; 
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
	sys_time_sec_t sec;
	sys_time_nsec_t nsec;
	sys_time_get_current_time(&sec, &nsec);
	// microseconds
	lpPerformanceCount->QuadPart = (sec * (1000 * 1000)) + (nsec / 1000);
	return true;
}

#ifndef _FINAL_BUILD
VOID OutputDebugStringW(LPCWSTR lpOutputString) 
{ 
	wprintf(lpOutputString); 
}

VOID OutputDebugString(LPCSTR lpOutputString) 
{ 
	printf(lpOutputString); 
}

VOID OutputDebugStringA(LPCSTR lpOutputString) 
{ 
	printf(lpOutputString); 
}
#endif // _CONTENT_PACKAGE

BOOL GetFileAttributesExA(LPCSTR lpFileName,GET_FILEEX_INFO_LEVELS fInfoLevelId,LPVOID lpFileInformation)
{ 
	PS3_STUBBED;
	return false;
}
HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData) { PS3_STUBBED; return 0;}
BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData) { PS3_STUBBED; return false;}

errno_t _itoa_s(int _Value, char * _DstBuf, size_t _Size, int _Radix) { if(_Radix==10) sprintf(_DstBuf,"%d",_Value); else if(_Radix==16) sprintf(_DstBuf,"%lx",_Value); else return -1; return 0; }
errno_t _i64toa_s(__int64 _Val, char * _DstBuf, size_t _Size, int _Radix) { if(_Radix==10) sprintf(_DstBuf,"%lld",_Val); else return -1; return 0; }

int _wtoi(const wchar_t *_Str)
{
	return wcstol(_Str, NULL, 10);
}



DWORD XGetLanguage() 
{ 
	unsigned char ucLang = app.GetMinecraftLanguage(0);
	int iLang;

	// check if we should override the system language or not
	if(ucLang==MINECRAFT_LANGUAGE_DEFAULT)	
	{
		cellSysutilGetSystemParamInt(CELL_SYSUTIL_SYSTEMPARAM_ID_LANG,&iLang);
	}
	else
	{
		return (DWORD)ucLang;
	}

	switch(iLang)
	{
	case CELL_SYSUTIL_LANG_JAPANESE			: return XC_LANGUAGE_JAPANESE;
	case CELL_SYSUTIL_LANG_ENGLISH_US		: return XC_LANGUAGE_ENGLISH;
	case CELL_SYSUTIL_LANG_FRENCH			: return XC_LANGUAGE_FRENCH;

	case CELL_SYSUTIL_LANG_SPANISH			: 
		if(app.IsAmericanSKU())
		{
			return XC_LANGUAGE_LATINAMERICANSPANISH;
		}
		else
		{
			return XC_LANGUAGE_SPANISH;
		}

	case CELL_SYSUTIL_LANG_GERMAN			: return XC_LANGUAGE_GERMAN;
	case CELL_SYSUTIL_LANG_ITALIAN			: return XC_LANGUAGE_ITALIAN;
	case CELL_SYSUTIL_LANG_PORTUGUESE_PT	: return XC_LANGUAGE_PORTUGUESE;
	
	case CELL_SYSUTIL_LANG_RUSSIAN			: return XC_LANGUAGE_RUSSIAN;
	case CELL_SYSUTIL_LANG_KOREAN			: return XC_LANGUAGE_KOREAN;
	case CELL_SYSUTIL_LANG_CHINESE_T		: return XC_LANGUAGE_TCHINESE;
	case CELL_SYSUTIL_LANG_PORTUGUESE_BR	: return XC_LANGUAGE_PORTUGUESE;
	case CELL_SYSUTIL_LANG_ENGLISH_GB		: return XC_LANGUAGE_ENGLISH;

	case CELL_SYSUTIL_LANG_DUTCH			: return XC_LANGUAGE_DUTCH;
	case CELL_SYSUTIL_LANG_FINNISH			: return XC_LANGUAGE_FINISH;
	case CELL_SYSUTIL_LANG_SWEDISH 			: return XC_LANGUAGE_SWEDISH;
	case CELL_SYSUTIL_LANG_DANISH 			: return XC_LANGUAGE_DANISH;
	case CELL_SYSUTIL_LANG_NORWEGIAN  		: return XC_LANGUAGE_BNORWEGIAN;
	case CELL_SYSUTIL_LANG_POLISH  			: return XC_LANGUAGE_POLISH;
	case CELL_SYSUTIL_LANG_TURKISH			: return XC_LANGUAGE_TURKISH;


	case CELL_SYSUTIL_LANG_CHINESE_S		: return XC_LANGUAGE_SCHINESE;
	default									: return XC_LANGUAGE_ENGLISH;
	}

}
DWORD XGetLocale() 
{ 
	int iLang;
	cellSysutilGetSystemParamInt(CELL_SYSUTIL_SYSTEMPARAM_ID_LANG,&iLang);
	switch(iLang)
	{
	case CELL_SYSUTIL_LANG_JAPANESE			: return XC_LOCALE_JAPAN;
	case CELL_SYSUTIL_LANG_ENGLISH_US		: return XC_LOCALE_UNITED_STATES;
	case CELL_SYSUTIL_LANG_FRENCH			: return XC_LOCALE_FRANCE;

	case CELL_SYSUTIL_LANG_SPANISH			: 
		if(app.IsAmericanSKU())
		{
			return XC_LOCALE_LATIN_AMERICA;
		}
		else
		{
			return XC_LOCALE_SPAIN;
		}		
		return XC_LOCALE_SPAIN;

	case CELL_SYSUTIL_LANG_GERMAN			: return XC_LOCALE_GERMANY;
	case CELL_SYSUTIL_LANG_ITALIAN			: return XC_LOCALE_ITALY;
	case CELL_SYSUTIL_LANG_PORTUGUESE_PT	: return XC_LOCALE_PORTUGAL;

	case CELL_SYSUTIL_LANG_RUSSIAN			: return XC_LOCALE_RUSSIAN_FEDERATION;
	case CELL_SYSUTIL_LANG_KOREAN			: return XC_LOCALE_KOREA;
	case CELL_SYSUTIL_LANG_CHINESE_T		: return XC_LOCALE_CHINA;
	case CELL_SYSUTIL_LANG_PORTUGUESE_BR	: return XC_LOCALE_BRAZIL;
	case CELL_SYSUTIL_LANG_ENGLISH_GB		: return XC_LOCALE_GREAT_BRITAIN;

	case CELL_SYSUTIL_LANG_DUTCH			: return XC_LOCALE_NETHERLANDS;
	case CELL_SYSUTIL_LANG_FINNISH			: return XC_LOCALE_FINLAND;
	case CELL_SYSUTIL_LANG_SWEDISH 			: return XC_LOCALE_SWEDEN;
	case CELL_SYSUTIL_LANG_DANISH 			: return XC_LOCALE_DENMARK;
	case CELL_SYSUTIL_LANG_NORWEGIAN  		: return XC_LOCALE_NORWAY;
	case CELL_SYSUTIL_LANG_POLISH  			: return XC_LOCALE_POLAND;
	case CELL_SYSUTIL_LANG_TURKISH			: return XC_LOCALE_TURKEY;


	case CELL_SYSUTIL_LANG_CHINESE_S		: return XC_LOCALE_CHINA;
	default									: return XC_LOCALE_UNITED_STATES;
	} 
}
DWORD XEnableGuestSignin(BOOL fEnable) 
{ 
	return 0; 
}

#endif   // __PS3__

