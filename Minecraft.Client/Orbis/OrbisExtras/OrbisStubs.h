#pragma once
#include <libdbg.h>

#include "TLSStorage.h"

class C4JThread;

//const char* getConsoleHomePath();
char* getUsrDirPath();

void OrbisInit();

DWORD TlsAlloc(VOID);
LPVOID TlsGetValue(DWORD dwTlsIndex);
BOOL TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue);

typedef struct _RECT
{
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
} RECT, *PRECT;

typedef void ID3D11Device;
typedef void ID3D11DeviceContext;
typedef void IDXGISwapChain;
typedef RECT D3D11_RECT;
typedef void ID3D11Buffer;
typedef DWORD (*PTHREAD_START_ROUTINE)(	LPVOID lpThreadParameter);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

typedef int errno_t;

// typedef struct _RTL_CRITICAL_SECTION {
// 	//
// 	//  The following field is used for blocking when there is contention for
// 	//  the resource
// 	//
//
// 	union {
// 		ULONG_PTR RawEvent[4];
// 	} Synchronization;
//
// 	//
// 	//  The following three fields control entering and exiting the critical
// 	//  section for the resource
// 	//
//
// 	LONG LockCount;
// 	LONG RecursionCount;
// 	HANDLE OwningThread;
// } RTL_CRITICAL_SECTION, *PRTL_CRITICAL_SECTION;

class OrbisCriticalSection
{
public:
	ScePthreadMutex mutex;
	int				m_cLock;
#ifdef _DEBUG
	C4JThread* m_pOwnerThread;
#endif
};

typedef OrbisCriticalSection RTL_CRITICAL_SECTION;
typedef OrbisCriticalSection* PRTL_CRITICAL_SECTION;

typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION PCRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION LPCRITICAL_SECTION;

void EnterCriticalSection(CRITICAL_SECTION* _c);
void LeaveCriticalSection(CRITICAL_SECTION* _c);
void InitializeCriticalSection(CRITICAL_SECTION* _c);
void DeleteCriticalSection(CRITICAL_SECTION* _c);
HANDLE CreateEvent(void* lpEventAttributes,	BOOL bManualReset,	BOOL bInitialState,	LPCSTR lpName);
VOID Sleep(DWORD dwMilliseconds);
BOOL SetThreadPriority(HANDLE hThread, int nPriority);
DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);

LONG InterlockedCompareExchangeRelease(LONG volatile *Destination, LONG Exchange,LONG Comperand	);

int32_t GetAudioBGMHandle();

VOID InitializeCriticalSection(PCRITICAL_SECTION CriticalSection);
VOID InitializeCriticalSectionAndSpinCount(PCRITICAL_SECTION CriticalSection, ULONG SpinCount);
VOID DeleteCriticalSection(PCRITICAL_SECTION CriticalSection);
VOID EnterCriticalSection(PCRITICAL_SECTION CriticalSection);
VOID LeaveCriticalSection(PCRITICAL_SECTION CriticalSection);
ULONG TryEnterCriticalSection(PCRITICAL_SECTION CriticalSection);
DWORD WaitForMultipleObjects(DWORD nCount, CONST HANDLE *lpHandles,BOOL bWaitAll,DWORD dwMilliseconds);

LONG64 InterlockedCompareExchangeRelease64(LONG64 volatile *Destination, LONG64 Exchange, LONG64 Comperand);

BOOL CloseHandle(HANDLE hObject);
BOOL SetEvent(HANDLE hEvent);

HMODULE GetModuleHandle(LPCSTR lpModuleName);

HANDLE CreateThread( void* lpThreadAttributes, DWORD dwStackSize, void* lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId );
DWORD ResumeThread( HANDLE hThread );
DWORD GetCurrentThreadId(VOID);
DWORD WaitForMultipleObjectsEx(DWORD nCount,CONST HANDLE *lpHandles,BOOL bWaitAll,DWORD dwMilliseconds,BOOL bAlertable	);
BOOL GetExitCodeThread(HANDLE hThread, LPDWORD lpExitCode);


LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
BOOL VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);

DWORD GetFileSize( HANDLE hFile, LPDWORD lpFileSizeHigh	);
BOOL GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize );
BOOL WriteFile(
HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped );
BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped );
#define INVALID_SET_FILE_POINTER false
BOOL SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);
HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
#define CreateFile CreateFileA
BOOL CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
#define CreateDirectory CreateDirectoryA
BOOL DeleteFileA(LPCSTR lpFileName);
#define DeleteFile DeleteFileA
DWORD GetFileAttributesA(LPCSTR lpFileName);
#define GetFileAttributes GetFileAttributesA
BOOL MoveFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName);
#define MoveFile MoveFileA

#define MAX_PATH          260

void __debugbreak();
VOID DebugBreak(VOID);


enum D3D11_BLEND
{
	D3D11_BLEND_ZERO	= 1,
	D3D11_BLEND_ONE	= 2,
	D3D11_BLEND_SRC_COLOR	= 3,
	D3D11_BLEND_INV_SRC_COLOR	= 4,
	D3D11_BLEND_SRC_ALPHA	= 5,
	D3D11_BLEND_INV_SRC_ALPHA	= 6,
	D3D11_BLEND_DEST_ALPHA	= 7,
	D3D11_BLEND_INV_DEST_ALPHA	= 8,
	D3D11_BLEND_DEST_COLOR	= 9,
	D3D11_BLEND_INV_DEST_COLOR	= 10,
	D3D11_BLEND_SRC_ALPHA_SAT	= 11,
	D3D11_BLEND_BLEND_FACTOR	= 14,
	D3D11_BLEND_INV_BLEND_FACTOR	= 15,
	D3D11_BLEND_SRC1_COLOR	= 16,
	D3D11_BLEND_INV_SRC1_COLOR	= 17,
	D3D11_BLEND_SRC1_ALPHA	= 18,
	D3D11_BLEND_INV_SRC1_ALPHA	= 19
};


enum D3D11_COMPARISON_FUNC
{
	D3D11_COMPARISON_NEVER	= 1,
	D3D11_COMPARISON_LESS	= 2,
	D3D11_COMPARISON_EQUAL	= 3,
	D3D11_COMPARISON_LESS_EQUAL	= 4,
	D3D11_COMPARISON_GREATER	= 5,
	D3D11_COMPARISON_NOT_EQUAL	= 6,
	D3D11_COMPARISON_GREATER_EQUAL	= 7,
	D3D11_COMPARISON_ALWAYS	= 8
};


typedef struct _SYSTEMTIME {
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

VOID GetSystemTime(	LPSYSTEMTIME lpSystemTime);
BOOL FileTimeToSystemTime(CONST FILETIME *lpFileTime, LPSYSTEMTIME lpSystemTime);
BOOL SystemTimeToFileTime(CONST SYSTEMTIME *lpSystemTime, LPFILETIME lpFileTime);
VOID GetLocalTime(LPSYSTEMTIME lpSystemTime);

typedef struct _MEMORYSTATUS {
	DWORD dwLength;
	DWORD dwMemoryLoad;
	SIZE_T dwTotalPhys;
	SIZE_T dwAvailPhys;
	SIZE_T dwTotalPageFile;
	SIZE_T dwAvailPageFile;
	SIZE_T dwTotalVirtual;
	SIZE_T dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;


#define WINAPI

#define CREATE_SUSPENDED				0x00000004

#define THREAD_BASE_PRIORITY_LOWRT  15  // value that gets a thread to LowRealtime-1
#define THREAD_BASE_PRIORITY_MAX    2   // maximum thread base priority boost
#define THREAD_BASE_PRIORITY_MIN    -2  // minimum thread base priority boost
#define THREAD_BASE_PRIORITY_IDLE   -15 // value that gets a thread to idle

#define THREAD_PRIORITY_LOWEST          THREAD_BASE_PRIORITY_MIN
#define THREAD_PRIORITY_BELOW_NORMAL    (THREAD_PRIORITY_LOWEST+1)
#define THREAD_PRIORITY_NORMAL          0
#define THREAD_PRIORITY_HIGHEST         THREAD_BASE_PRIORITY_MAX
#define THREAD_PRIORITY_ABOVE_NORMAL    (THREAD_PRIORITY_HIGHEST-1)
#define THREAD_PRIORITY_ERROR_RETURN    (MAXLONG)

#define THREAD_PRIORITY_TIME_CRITICAL   THREAD_BASE_PRIORITY_LOWRT
#define THREAD_PRIORITY_IDLE            THREAD_BASE_PRIORITY_IDLE

#define WAIT_TIMEOUT                     258L
#define STATUS_ABANDONED_WAIT_0          ((DWORD   )0x00000080L)
#define WAIT_ABANDONED         ((STATUS_ABANDONED_WAIT_0 ) + 0 )

#define MAXUINT_PTR  (~((UINT_PTR)0))
#define MAXINT_PTR   ((INT_PTR)(MAXUINT_PTR >> 1))
#define MININT_PTR   (~MAXINT_PTR)

#define MAXULONG_PTR (~((ULONG_PTR)0))
#define MAXLONG_PTR  ((LONG_PTR)(MAXULONG_PTR >> 1))
#define MINLONG_PTR  (~MAXLONG_PTR)

#define MAXUHALF_PTR ((UHALF_PTR)~0)
#define MAXHALF_PTR  ((HALF_PTR)(MAXUHALF_PTR >> 1))
#define MINHALF_PTR  (~MAXHALF_PTR)

#define INVALID_HANDLE_VALUE ((HANDLE)-1)
//
// Generic test for success on any status value (non-negative numbers
// indicate success).
//

//#define HRESULT_SUCCEEDED(Status) ((HRESULT)(Status) >= 0)

//
// and the inverse
//
#define _HRESULT_TYPEDEF_(_sc) _sc

#define FAILED(Status) ((HRESULT)(Status)<0)
#define MAKE_HRESULT(sev,fac,code) \
	((HRESULT) (((unsigned int)(sev)<<31) | ((unsigned int)(fac)<<16) | ((unsigned int)(code))) )
#define MAKE_SCODE(sev,fac,code) \
	((SCODE) (((unsigned int)(sev)<<31) | ((unsigned int)(fac)<<16) | ((unsigned int)(code))) )
#define E_FAIL                           _HRESULT_TYPEDEF_(0x80004005L)
#define E_ABORT                          _HRESULT_TYPEDEF_(0x80004004L)
#define E_NOINTERFACE                    _HRESULT_TYPEDEF_(0x80004002L)

#define GENERIC_READ                     (0x80000000L)
#define GENERIC_WRITE                    (0x40000000L)
#define GENERIC_EXECUTE                  (0x20000000L)
#define GENERIC_ALL                      (0x10000000L)

#define FILE_SHARE_READ                 0x00000001
#define FILE_SHARE_WRITE                0x00000002
#define FILE_SHARE_DELETE               0x00000004
#define FILE_ATTRIBUTE_READONLY             0x00000001
#define FILE_ATTRIBUTE_HIDDEN               0x00000002
#define FILE_ATTRIBUTE_SYSTEM               0x00000004
#define FILE_ATTRIBUTE_DIRECTORY            0x00000010
#define FILE_ATTRIBUTE_ARCHIVE              0x00000020
#define FILE_ATTRIBUTE_DEVICE               0x00000040
#define FILE_ATTRIBUTE_NORMAL               0x00000080
#define FILE_ATTRIBUTE_TEMPORARY            0x00000100

#define FILE_FLAG_WRITE_THROUGH         0x80000000
#define FILE_FLAG_OVERLAPPED            0x40000000
#define FILE_FLAG_NO_BUFFERING          0x20000000
#define FILE_FLAG_RANDOM_ACCESS         0x10000000
#define FILE_FLAG_SEQUENTIAL_SCAN       0x08000000
#define FILE_FLAG_DELETE_ON_CLOSE       0x04000000
#define FILE_FLAG_BACKUP_SEMANTICS      0x02000000

#define FILE_BEGIN           0
#define FILE_CURRENT         1
#define FILE_END             2

#define CREATE_NEW          1
#define CREATE_ALWAYS       2
#define OPEN_EXISTING       3
#define OPEN_ALWAYS         4
#define TRUNCATE_EXISTING   5

#define PAGE_NOACCESS          0x01
#define PAGE_READONLY          0x02
#define PAGE_READWRITE         0x04
#define PAGE_WRITECOPY         0x08
#define PAGE_EXECUTE           0x10
#define PAGE_EXECUTE_READ      0x20
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_EXECUTE_WRITECOPY 0x80
#define PAGE_GUARD            0x100
#define PAGE_NOCACHE          0x200
#define PAGE_WRITECOMBINE     0x400
#define PAGE_USER_READONLY   0x1000
#define PAGE_USER_READWRITE  0x2000
#define MEM_COMMIT           0x1000
#define MEM_RESERVE          0x2000
#define MEM_DECOMMIT         0x4000
#define MEM_RELEASE          0x8000
#define MEM_FREE            0x10000
#define MEM_PRIVATE         0x20000
#define MEM_RESET           0x80000
#define MEM_TOP_DOWN       0x100000
#define MEM_NOZERO         0x800000
#define MEM_LARGE_PAGES  0x20000000
#define MEM_HEAP         0x40000000
#define MEM_16MB_PAGES   0x80000000

#define IGNORE              0       // Ignore signal
#define INFINITE            0xFFFFFFFF  // Infinite timeout
#define WAIT_FAILED ((DWORD)0xFFFFFFFF)
#define STATUS_WAIT_0                    ((DWORD   )0x00000000L)
#define WAIT_OBJECT_0       ((STATUS_WAIT_0 ) + 0 )
#define STATUS_PENDING                   ((DWORD   )0x00000103L)
#define STILL_ACTIVE                        STATUS_PENDING

DWORD GetLastError(VOID);
VOID GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer);

DWORD GetTickCount();
BOOL QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency);
BOOL QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount);


#define ERROR_SUCCESS                    0L
#define ERROR_IO_PENDING                 997L    // dderror
#define ERROR_CANCELLED                  1223L
#define S_OK                                   ((HRESULT)0x00000000L)
#define S_FALSE                                ((HRESULT)0x00000001L)

#define RtlEqualMemory(Destination,Source,Length) (!memcmp((Destination),(Source),(Length)))
#define RtlMoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
#define RtlCopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
#define RtlFillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))

#define MoveMemory       RtlMoveMemory
#define CopyMemory       RtlCopyMemory
#define FillMemory       RtlFillMemory
#define ZeroMemory       RtlZeroMemory

#define CDECL
#define APIENTRY

#define VK_ESCAPE                       0x1B
#define VK_RETURN                       0x0D

VOID OutputDebugStringW(LPCWSTR lpOutputString);
VOID OutputDebugString(LPCSTR lpOutputString);
VOID OutputDebugStringA(LPCSTR lpOutputString);

errno_t _itoa_s(int _Value, char * _DstBuf, size_t _Size, int _Radix);
errno_t _i64toa_s(int64_t _Val, char * _DstBuf, size_t _Size, int _Radix);

#define __declspec(a)
extern "C" int _wcsicmp (const wchar_t * dst, const wchar_t * src);

size_t wcsnlen(const wchar_t *wcs, size_t maxsize);

typedef struct _WIN32_FIND_DATAA {
	DWORD dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	DWORD nFileSizeHigh;
	DWORD nFileSizeLow;
	DWORD dwReserved0;
	DWORD dwReserved1;
	CHAR cFileName[ MAX_PATH ];
	CHAR cAlternateFileName[ 14 ];
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;
typedef WIN32_FIND_DATAA WIN32_FIND_DATA;
typedef PWIN32_FIND_DATAA PWIN32_FIND_DATA;
typedef LPWIN32_FIND_DATAA LPWIN32_FIND_DATA;

typedef struct _WIN32_FILE_ATTRIBUTE_DATA {
	DWORD dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	DWORD nFileSizeHigh;
	DWORD nFileSizeLow;
} WIN32_FILE_ATTRIBUTE_DATA, *LPWIN32_FILE_ATTRIBUTE_DATA;


DWORD GetFileAttributesA(LPCSTR lpFileName);
#define GetFileAttributes GetFileAttributesA
typedef enum _GET_FILEEX_INFO_LEVELS {
	GetFileExInfoStandard,
	GetFileExMaxInfoLevel
} GET_FILEEX_INFO_LEVELS;

BOOL GetFileAttributesExA(LPCSTR lpFileName,GET_FILEEX_INFO_LEVELS fInfoLevelId,LPVOID lpFileInformation);
#define GetFileAttributesEx GetFileAttributesExA

BOOL DeleteFileA(LPCSTR lpFileName);
#define DeleteFile DeleteFileA


HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData);
#define FindFirstFile FindFirstFileA

BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData);
#define FindNextFile FindNextFileA
#define FindClose(hFindFile) CloseHandle(hFindFile)

DWORD XGetLanguage();
DWORD XGetLocale();
DWORD XEnableGuestSignin(BOOL fEnable);

#ifdef _CONTENT_PACKAGE
#define ORBIS_STUBBED { }
#else
#define ORBIS_STUBBED { static bool bSet = false; if(!bSet){printf("missing function on PS4 : %s\n Tell MarkH about this, then press f5 to continue.\n", __FUNCTION__); bSet = true; SCE_BREAK();} }
#endif

