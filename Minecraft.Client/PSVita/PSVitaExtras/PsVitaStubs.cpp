#include "stdafx.h"
#include <rtc.h>
#include <sce_atomic.h>
#include <fios2.h>
#include <apputil.h>
#include "PSVitaTLSStorage.h"

#ifdef _CONTENT_PACKAGE
#define PSVITA_ASSERT_SCE_ERROR(errVal) {}
#else
#define PSVITA_ASSERT_SCE_ERROR(errVal) if(errVal != SCE_OK) { printf("----------------------\n    %s failed with error %d [0x%08x]\n----------------------\n", __FUNCTION__, errVal, errVal); assert(0); }
#endif

#define MAX_PATH_LENGTH 291	// TODO - check this is correct for our usage here

static char driveRoot[MAX_PATH_LENGTH] = "app0:";
static char dirName[MAX_PATH_LENGTH];
static char contentInfoPath[MAX_PATH_LENGTH];
static char usrdirPath[MAX_PATH_LENGTH] = "app0:";
static char contentInfoPathBDPatch[MAX_PATH_LENGTH];
static char usrdirPathBDPatch[MAX_PATH_LENGTH];

/*E The FIOS2 default maximum path is 1024, games can normally use a much smaller value. */

/*E Buffers for FIOS2 initialization.
 * These are typical values that a game might use, but adjust them as needed. They are
 * of type int64_t to avoid alignment issues. */

/* 64 ops: */
int64_t g_OpStorage[SCE_FIOS_OP_STORAGE_SIZE(64, MAX_PATH_LENGTH) / sizeof(int64_t) + 1];
/* 1024 chunks, 64KiB: */
int64_t g_ChunkStorage[SCE_FIOS_CHUNK_STORAGE_SIZE(1024) / sizeof(int64_t) + 1];
/* 16 file handles: */
int64_t g_FHStorage[SCE_FIOS_FH_STORAGE_SIZE(16, MAX_PATH_LENGTH) / sizeof(int64_t) + 1];
/* 4 directory handles: */
int64_t g_DHStorage[SCE_FIOS_DH_STORAGE_SIZE(4, MAX_PATH_LENGTH) / sizeof(int64_t) + 1];

void PSVitaInit()
{
	SceFiosParams params = SCE_FIOS_PARAMS_INITIALIZER;

	/*E Provide required storage buffers. */
	params.opStorage.pPtr = g_OpStorage;
	params.opStorage.length = sizeof(g_OpStorage);
	params.chunkStorage.pPtr = g_ChunkStorage;
	params.chunkStorage.length = sizeof(g_ChunkStorage);
	params.fhStorage.pPtr = g_FHStorage;
	params.fhStorage.length = sizeof(g_FHStorage);
	params.dhStorage.pPtr = g_DHStorage;
	params.dhStorage.length = sizeof(g_DHStorage);

	params.pathMax = MAX_PATH_LENGTH;

	params.pMemcpy = memcpy;

	int err = sceFiosInitialize(&params);
	assert(err == SCE_FIOS_OK);
}
char* getConsoleHomePath()
{
	return contentInfoPath;
}

char* getUsrDirRoot()
{
	return driveRoot;
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

VOID GetSystemTime(	LPSYSTEMTIME lpSystemTime)
{
	SceDateTime dateTime;
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
BOOL FileTimeToSystemTime(CONST FILETIME *lpFileTime, LPSYSTEMTIME lpSystemTime) { PSVITA_STUBBED; return false; }
BOOL SystemTimeToFileTime(CONST SYSTEMTIME *lpSystemTime, LPFILETIME lpFileTime)
{
	SceUInt64 diffHundredNanos;
	SceDateTime dateTime;
	int err = sceRtcGetCurrentClock(&dateTime, 0);
	sceRtcGetTime64_t(&dateTime, &diffHundredNanos);
	diffHundredNanos *= 10;

	lpFileTime->dwHighDateTime = diffHundredNanos >> 32;
	lpFileTime->dwLowDateTime = diffHundredNanos & 0xffffffff;
	return true;
}

VOID GetLocalTime(LPSYSTEMTIME lpSystemTime)
{
	SceDateTime dateTime;
	int err = sceRtcGetCurrentClockLocalTime(&dateTime);
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

HANDLE CreateEvent(void* lpEventAttributes,	BOOL bManualReset,	BOOL bInitialState,	LPCSTR lpName) { PSVITA_STUBBED; return NULL; }
VOID Sleep(DWORD dwMilliseconds)
{
	C4JThread::Sleep(dwMilliseconds);
}

BOOL SetThreadPriority(HANDLE hThread, int nPriority) { PSVITA_STUBBED; return FALSE; }
DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) { PSVITA_STUBBED; return false; }

LONG InterlockedCompareExchangeRelease(LONG volatile *Destination, LONG Exchange,LONG Comperand	)
{
	return sceAtomicCompareAndSwap32((int32_t*)Destination, (int32_t)Comperand, (int32_t)Exchange);
}

LONG64 InterlockedCompareExchangeRelease64(LONG64 volatile *Destination, LONG64 Exchange, LONG64 Comperand)
{
	return sceAtomicCompareAndSwap64((int64_t*)Destination, (int64_t)Comperand, (int64_t)Exchange);
}


VOID InitializeCriticalSection(PCRITICAL_SECTION CriticalSection)
{
	char name[1] = {0};

	int err = sceKernelCreateLwMutex((SceKernelLwMutexWork *)(&CriticalSection->mutex), name, SCE_KERNEL_LW_MUTEX_ATTR_TH_PRIO | SCE_KERNEL_LW_MUTEX_ATTR_RECURSIVE, 0, NULL);
	PSVITA_ASSERT_SCE_ERROR(err);
}


VOID InitializeCriticalSectionAndSpinCount(PCRITICAL_SECTION CriticalSection, ULONG SpinCount)
{
	// no spin count on PSVita
	InitializeCriticalSection(CriticalSection);
}

VOID DeleteCriticalSection(PCRITICAL_SECTION CriticalSection)
{
	int err = sceKernelDeleteLwMutex((SceKernelLwMutexWork *)(&CriticalSection->mutex));
	PSVITA_ASSERT_SCE_ERROR(err);
}

extern CRITICAL_SECTION g_singleThreadCS;

VOID EnterCriticalSection(PCRITICAL_SECTION CriticalSection)
{
	int err = sceKernelLockLwMutex ((SceKernelLwMutexWork *)(&CriticalSection->mutex), 1, NULL);
	PSVITA_ASSERT_SCE_ERROR(err);
}


VOID LeaveCriticalSection(PCRITICAL_SECTION CriticalSection)
{
	int err =  sceKernelUnlockLwMutex ((SceKernelLwMutexWork *)(&CriticalSection->mutex), 1);
	PSVITA_ASSERT_SCE_ERROR(err);
}

ULONG TryEnterCriticalSection(PCRITICAL_SECTION CriticalSection)
{
	int err =  sceKernelTryLockLwMutex ((SceKernelLwMutexWork *)(&CriticalSection->mutex), 1);
	if(err == SCE_OK)
		return true;
	return false;
}
DWORD WaitForMultipleObjects(DWORD nCount, CONST HANDLE *lpHandles,BOOL bWaitAll,DWORD dwMilliseconds) { PSVITA_STUBBED; return 0; }



VOID InitializeCriticalRWSection(PCRITICAL_RW_SECTION CriticalSection)
{
	char name[1] = {0};

	CriticalSection->RWLock = sceKernelCreateRWLock(name, SCE_KERNEL_RW_LOCK_ATTR_TH_PRIO | SCE_KERNEL_RW_LOCK_ATTR_RECURSIVE, NULL);
}

VOID DeleteCriticalRWSection(PCRITICAL_RW_SECTION CriticalSection)
{
	int err = sceKernelDeleteRWLock(CriticalSection->RWLock);
	PSVITA_ASSERT_SCE_ERROR(err);
}

VOID EnterCriticalRWSection(PCRITICAL_RW_SECTION CriticalSection, bool Write)
{
	int err;
	if( Write )
	{
		err =  sceKernelLockWriteRWLock(CriticalSection->RWLock, 0);
	}
	else
	{
		err =  sceKernelLockReadRWLock(CriticalSection->RWLock, 0);
	}
	PSVITA_ASSERT_SCE_ERROR(err);
}

VOID LeaveCriticalRWSection(PCRITICAL_RW_SECTION CriticalSection, bool Write)
{
	int err;
	if( Write )
	{
		err =  sceKernelUnlockWriteRWLock(CriticalSection->RWLock);
	}
	else
	{
		err =  sceKernelUnlockReadRWLock(CriticalSection->RWLock);
	}
	PSVITA_ASSERT_SCE_ERROR(err);
}



BOOL CloseHandle(HANDLE hObject)
{
	sceFiosFHCloseSync(NULL,(SceFiosFH)((int32_t)hObject));
	return true;
}

BOOL SetEvent(HANDLE hEvent) { PSVITA_STUBBED; return false; }

HMODULE GetModuleHandle(LPCSTR lpModuleName) { PSVITA_STUBBED; return 0; }

DWORD TlsAlloc(VOID) { return PSVitaTLSStorage::Instance()->Alloc(); }
BOOL TlsFree(DWORD dwTlsIndex) { return PSVitaTLSStorage::Instance()->Free(dwTlsIndex); }
LPVOID TlsGetValue(DWORD dwTlsIndex) { return PSVitaTLSStorage::Instance()->GetValue(dwTlsIndex); }
BOOL TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue) { return PSVitaTLSStorage::Instance()->SetValue(dwTlsIndex, lpTlsValue); }

// AP - all this virtual stuff has been added because Vita doesn't have a virtual memory system so we allocate 1MB real memory chunks instead
// and access memory reads and writes via VirtualCopyTo and VirtualCopyFrom which divides memcpys across multiple 1MB chunks if required
static void* VirtualAllocs[1000];	// a list of 1MB allocations
static int VirtualNumAllocs = 0;	// how many 1MB chunks have been allocated

LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect)
{
	if( flAllocationType == MEM_COMMIT )
	{
		// how many pages do we need
		int NumPagesRequired = dwSize / VIRTUAL_PAGE_SIZE;
		int BytesLeftOver = dwSize % VIRTUAL_PAGE_SIZE;
		if( BytesLeftOver )
		{
			NumPagesRequired += 1;
		}

		// allocate pages until we reach the required number of pages
		while( VirtualNumAllocs < NumPagesRequired )
		{
			// allocate a new page
			void* NewAlloc = malloc(VIRTUAL_PAGE_SIZE);

			// add it to the list
			VirtualAllocs[VirtualNumAllocs] = NewAlloc;
			VirtualNumAllocs += 1;
		}
	}

	return (void*) VIRTUAL_OFFSET;
}

BOOL VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType)
{
	while( VirtualNumAllocs )
	{
		// free and remove a page
		VirtualNumAllocs -= 1;
		free(VirtualAllocs[VirtualNumAllocs]);
	}

	return TRUE;
}


// memset a section of the virtual chunks
VOID VirtualMemset(LPVOID lpDestOffset, int val, SIZE_T dwSize)
{
	int DestOffset = ((int)(lpDestOffset) - VIRTUAL_OFFSET);	// convert the pointer back into a virtual offset
	int StartPage = DestOffset / VIRTUAL_PAGE_SIZE;				// which 1MB page do we start on
	int EndPage = (DestOffset + dwSize) / VIRTUAL_PAGE_SIZE;	// which 1MB page do we end on
	int Offset = DestOffset % VIRTUAL_PAGE_SIZE;				// what is the byte offset within the current 1MB page
	if( StartPage == EndPage )									// early out if we're on the same page
	{
		uint8_t* Dest = (uint8_t*)VirtualAllocs[StartPage] + Offset;
		memset(Dest, val, dwSize);
	}
	else
	{
		while( dwSize )
		{
			// how many bytes do we copy in this chunk
			int BytesToSet = dwSize;
			if( StartPage != EndPage )
			{
				BytesToSet = VIRTUAL_PAGE_SIZE - Offset;
			}

			// get final point to real memory
			uint8_t* Dest = (uint8_t*)VirtualAllocs[StartPage] + Offset;

			// copy the required bytes
			memset(Dest, val, BytesToSet);

			// move to the next chunk
			dwSize -= BytesToSet;
			StartPage += 1;
			Offset = 0;
		}
	}
}


// copy a block of memory to the virtual chunks
VOID VirtualCopyTo(LPVOID lpDestOffset, LPVOID lpSrc, SIZE_T dwSize)
{
	int DestOffset = ((int)(lpDestOffset) - VIRTUAL_OFFSET);	// convert the pointer back into a virtual offset
	int StartPage = DestOffset / VIRTUAL_PAGE_SIZE;				// which 1MB page do we start on
	int EndPage = (DestOffset + dwSize) / VIRTUAL_PAGE_SIZE;	// which 1MB page do we end on
	int Offset = DestOffset % VIRTUAL_PAGE_SIZE;				// what is the byte offset within the current 1MB page
	if( StartPage == EndPage )									// early out if we're on the same page
	{
		uint8_t* Dest = (uint8_t*)VirtualAllocs[StartPage] + Offset;
		memcpy(Dest, lpSrc, dwSize);
	}
	else
	{
		uint8_t *Src = (uint8_t*) lpSrc;
		while( dwSize )
		{
			// how many bytes do we copy in this chunk
			int BytesToCopy = dwSize;
			if( StartPage != EndPage )
			{
				BytesToCopy = VIRTUAL_PAGE_SIZE - Offset;
			}

			// get final point to real memory
			uint8_t* Dest = (uint8_t*)VirtualAllocs[StartPage] + Offset;

			// copy the required bytes
			memcpy(Dest, Src, BytesToCopy);

			// move to the next chunk
			dwSize -= BytesToCopy;
			Src += BytesToCopy;
			StartPage += 1;
			Offset = 0;
		}
	}
}

// copy a block of memory from the virtual chunks
VOID VirtualCopyFrom(LPVOID lpDest, LPVOID lpSrcOffset, SIZE_T dwSize)
{
	int SrcOffset = ((int)(lpSrcOffset) - VIRTUAL_OFFSET);	// convert the pointer back into a virtual offset
	int StartPage = SrcOffset / VIRTUAL_PAGE_SIZE;			// which 1MB page do we start on
	int EndPage = (SrcOffset + dwSize) / VIRTUAL_PAGE_SIZE;	// which 1MB page do we end on
	int Offset = SrcOffset % VIRTUAL_PAGE_SIZE;				// what is the byte offset within the current 1MB page
	if( StartPage == EndPage )								// early out if we're on the same page
	{
		uint8_t* Src = (uint8_t*)VirtualAllocs[StartPage] + Offset;
		memcpy(lpDest, Src, dwSize);
	}
	else
	{
		uint8_t *Dest = (uint8_t*) lpDest;
		while( dwSize )
		{
			// how many bytes do we copy in this chunk
			int BytesToCopy = dwSize;
			if( StartPage != EndPage )
			{
				BytesToCopy = VIRTUAL_PAGE_SIZE - Offset;
			}

			// get final point to real memory
			uint8_t* Src = (uint8_t*)VirtualAllocs[StartPage] + Offset;

			// copy the required bytes
			memcpy(Dest, Src, BytesToCopy);

			// move to the next chunk
			dwSize -= BytesToCopy;
			Dest += BytesToCopy;
			StartPage += 1;
			Offset = 0;
		}
	}
}

// copy a block of memory between the virtual chunks
VOID VirtualMove(LPVOID lpDestOffset, LPVOID lpSrcOffset, SIZE_T dwSize)
{
	int DestOffset = ((int)(lpDestOffset) - VIRTUAL_OFFSET);	// convert the pointer back into a virtual offset
	int DestChunkOffset = DestOffset % VIRTUAL_PAGE_SIZE;		// what is the byte offset within the current 1MB page
	int DestPage = DestOffset / VIRTUAL_PAGE_SIZE;				// which 1MB page do we start on

	int SrcOffset = ((int)(lpSrcOffset) - VIRTUAL_OFFSET);		// convert the pointer back into a virtual offset
	int SrcChunkOffset = SrcOffset % VIRTUAL_PAGE_SIZE;			// what is the byte offset within the current 1MB page
	int SrcPage = SrcOffset / VIRTUAL_PAGE_SIZE;				// which 1MB page do we start on
	while( dwSize )
	{
		// how many bytes do we copy in this chunk
		int BytesToCopy = dwSize;
		// does the dest straddle 2 chunks
		if( DestChunkOffset + BytesToCopy > VIRTUAL_PAGE_SIZE )
		{
			BytesToCopy = VIRTUAL_PAGE_SIZE - DestChunkOffset;
		}
		// does the src straddle 2 chunks
		if( SrcChunkOffset + BytesToCopy > VIRTUAL_PAGE_SIZE )
		{
			BytesToCopy = VIRTUAL_PAGE_SIZE - SrcChunkOffset;
		}

		// get final point to real memory
		uint8_t* Dest = (uint8_t*)VirtualAllocs[DestPage] + DestChunkOffset;
		uint8_t* Src = (uint8_t*)VirtualAllocs[SrcPage] + SrcChunkOffset;

		// copy the required bytes
		memcpy(Dest, Src, BytesToCopy);

		// move to the next chunk
		DestChunkOffset += BytesToCopy;
		if( DestChunkOffset >= VIRTUAL_PAGE_SIZE )
		{
			DestChunkOffset = 0;
			DestPage += 1;
		}
		SrcChunkOffset += BytesToCopy;
		if( SrcChunkOffset >= VIRTUAL_PAGE_SIZE )
		{
			SrcChunkOffset = 0;
			SrcPage += 1;
		}

		dwSize -= BytesToCopy;
	}
}

// write the chunks to a file given a handle
BOOL VirtualWriteFile(LPCSTR lpFileName, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped )
{
	*lpNumberOfBytesWritten = 0;
	int Page = 0;
	while( nNumberOfBytesToWrite )
	{
		int BytesToWrite = nNumberOfBytesToWrite;
		if( BytesToWrite > VIRTUAL_PAGE_SIZE )
		{
			BytesToWrite = VIRTUAL_PAGE_SIZE;
		}

		void* Data = VirtualAllocs[Page];

		DWORD numberOfBytesWritten=0;
		WriteFileWithName(lpFileName, Data, BytesToWrite, &numberOfBytesWritten,NULL);
		*lpNumberOfBytesWritten += numberOfBytesWritten;

		nNumberOfBytesToWrite -= BytesToWrite;
		Page += 1;
	}

	return true;
}

// The data is mostly zlib compressed. Only blank areas are not so this will RLE just the zeros
// Yields about 2:1 compression
VOID VirtualCompress(LPVOID lpDest,LPDWORD lpNewSize, LPVOID lpAddress, SIZE_T dwSize)
{
	uint8_t *pDest = (uint8_t *) lpDest;
	int Offset = 0;
	int Page = 0;
	int NewSize = 0;
	int CountingZeros = 0;
	uint8_t* Data = (uint8_t*) VirtualAllocs[Page];
	while( dwSize )
	{
		// if this has a value just add it
		if( Data[Offset] )
		{
			// just store the value
			if( pDest )
			{
				pDest[NewSize] = Data[Offset];
			}
			NewSize += 1;
			CountingZeros = 0;
		}
		else
		{
			// is this the first zero (also we can only count to 255 zeros)
			if( !CountingZeros || CountingZeros == 255 )
			{
				CountingZeros = 0;

				// create space for the zero counter
				if( pDest )
				{
					pDest[NewSize] = 0;			// this indicates a zero
					pDest[NewSize + 1] = 0;		// this is how many zeros we counted
				}
				NewSize += 2;
			}

			if( pDest )
			{
				pDest[NewSize - 1] += 1;		// increment the number of zeros
			}

			CountingZeros += 1;
		}

		dwSize -= 1;
		Offset += 1;
		if( Offset == VIRTUAL_PAGE_SIZE )
		{
			Offset = 0;
			Page += 1;
			Data = (uint8_t*) VirtualAllocs[Page];
		}
	}

	*lpNewSize = NewSize;
}

// The data in is mostly zlib compressed. Only blank areas are not so this will RLE just the zeros
VOID VirtualDecompress(LPVOID buf, SIZE_T dwSize)
{
	uint8_t *pSrc = (uint8_t *)buf;
	int Offset = 0;
	int Page = 0;
	int Index = 0;
	uint8_t* Data = (uint8_t*) VirtualAllocs[Page];
	while( Index != dwSize )
	{
		// is this a normal value
		if( pSrc[Index] )
		{
			// just copy it across
			Data[Offset] = pSrc[Index];
			Offset += 1;
			if( Offset == VIRTUAL_PAGE_SIZE )
			{
				Offset = 0;
				Page += 1;
				if( Page == VirtualNumAllocs )
				{
					// allocate a new page
					void* NewAlloc = malloc(VIRTUAL_PAGE_SIZE);

					// add it to the list
					VirtualAllocs[VirtualNumAllocs] = NewAlloc;
					VirtualNumAllocs += 1;
				}
				Data = (uint8_t*) VirtualAllocs[Page];
			}
		}
		else
		{
			// how many zeros do we have
			Index += 1;
			int Count = pSrc[Index];
			// to do : this should really be a sequence of memsets
			for( int i = 0;i < Count;i += 1 )
			{
				Data[Offset] = 0;
				Offset += 1;
				if( Offset == VIRTUAL_PAGE_SIZE )
				{
					Offset = 0;
					Page += 1;
					if( Page == VirtualNumAllocs )
					{
						// allocate a new page
						void* NewAlloc = malloc(VIRTUAL_PAGE_SIZE);

						// add it to the list
						VirtualAllocs[VirtualNumAllocs] = NewAlloc;
						VirtualNumAllocs += 1;
					}
					Data = (uint8_t*) VirtualAllocs[Page];
				}
			}
		}

		Index += 1;
	}
}

DWORD GetFileSize( HANDLE hFile, LPDWORD lpFileSizeHigh	)
{
	SceFiosFH fh = (SceFiosFH)(hFile);

	// 4J Stu - sceFiosFHGetSize didn't seem to work...so doing this for now
	//SceFiosSize FileSize;
	//FileSize=sceFiosFHGetSize(fh);
	SceFiosStat statData;
	int err = sceFiosFHStatSync(NULL,fh,&statData);
	SceFiosOffset FileSize = statData.fileSize;

	if(lpFileSizeHigh)
		*lpFileSizeHigh= (DWORD)(FileSize>>32);
	else
	{
		assert(FileSize>>32 == 0);
	}

	return (DWORD)(FileSize&0xffffffff);
}
BOOL GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize ) { PSVITA_STUBBED; return false; }


BOOL WriteFileWithName(LPCSTR lpFileName,  LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped )
{
	char filePath[256];
	sprintf(filePath,"%s/%s",getUsrDirPath(), lpFileName );
	SceFiosSize bytesWritten = sceFiosFileWriteSync( NULL, filePath, lpBuffer, nNumberOfBytesToWrite, 0 );
	if(bytesWritten != nNumberOfBytesToWrite)
	{
		// error
		app.DebugPrintf("WriteFile error %x%08x\n",bytesWritten);
		return FALSE;
	}
	*lpNumberOfBytesWritten = (DWORD)bytesWritten;
	return TRUE;
}

BOOL WriteFile(	HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped )
{
	//CD - Use 'WriteFileWithName' instead
	//CD - it won't write via handle, for some unknown reason...
	PSVITA_STUBBED;
	return FALSE;
}

BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped )
{
	SceFiosFH fh = (SceFiosFH)((int64_t)hFile);
	// sceFiosFHReadSync - Non-negative values are the number of bytes read, 0 <= result <= length. Negative values are error codes.
	SceFiosSize bytesRead = sceFiosFHReadSync(NULL, fh, lpBuffer, (SceFiosSize)nNumberOfBytesToRead);
	if(bytesRead < 0)
	{
		// error
		return FALSE;
	}
	else
	{
		*lpNumberOfBytesRead = (DWORD)bytesRead;
		return TRUE;
	}
}

BOOL SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
	SceFiosFH fd = (SceFiosFH)((int64_t)hFile);

	uint64_t bitsToMove = (int64_t) lDistanceToMove;
	SceFiosOffset pos = 0;

	if (lpDistanceToMoveHigh != NULL)
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
	std::string mountedPath = StorageManager.GetMountedPath(lpFileName);
	if(mountedPath.length() > 0)
	{
		strcpy(filePath, mountedPath.c_str());
	}
	else if(strstr(lpFileName,":") != 0) // already fully qualified path
		strcpy(filePath, lpFileName );
	else
		sprintf(filePath,"%s/%s",getUsrDirPath(), lpFileName );
	//		sprintf(filePath,"%s/%s", driveRoot, lpFileName );

	//CD - Does the file need created?
	if( dwDesiredAccess == GENERIC_WRITE )
	{
		//CD - Create a blank file
		int err = sceFiosFileWriteSync( NULL, filePath, NULL, 0, 0 );
		assert( err == SCE_FIOS_OK );
	}

#ifndef _CONTENT_PACKAGE
	printf("*** Opening %s\n",filePath);
#endif

	SceFiosFH fh;
	int err = sceFiosFHOpenSync(NULL, &fh, filePath, NULL);
	assert( err == SCE_FIOS_OK );

	return (void*)fh;
}

BOOL CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
#ifndef _CONTENT_PACKAGE
	char filePath[256];
	sprintf(filePath,"%s/%s",usrdirPath, lpPathName );
	int ret = sceIoMkdir( filePath, SCE_STM_RWU );
	if( ret != SCE_OK )
	{
		printf("*** CreateDirectory %s FAILED\n",filePath);
		return false;
	}
	return true;
#endif
	return false;
}

BOOL DeleteFileA(LPCSTR lpFileName) { PSVITA_STUBBED; return false; }

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
	else if(strstr(lpFileName,":") != 0) // colon in the filename, so it's fully qualified
		strcpy(filePath, lpFileName);
	else
		sprintf(filePath,"%s/%s",getUsrDirPath(), lpFileName );

	//	sprintf(filePath,"%s/%s", driveRoot, lpFileName );

	// check if the file exists first
	SceFiosStat  statData;
	if(sceFiosStatSync(NULL, filePath, &statData) != SCE_FIOS_OK)
	{
		app.DebugPrintf("*** sceFiosStatSync Failed\n");
		return -1;
	}
	if(statData.statFlags & SCE_FIOS_STATUS_DIRECTORY )
		return FILE_ATTRIBUTE_DIRECTORY;
	else
		return FILE_ATTRIBUTE_NORMAL;
}


BOOL MoveFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName) { PSVITA_STUBBED; return false; }

void __debugbreak() { SCE_BREAK(); }
VOID DebugBreak(VOID) { SCE_BREAK(); }


DWORD GetLastError(VOID) { PSVITA_STUBBED; return 0; }
VOID GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer)
{
	PSVITA_STUBBED;
	/*	malloc_managed_size stat;
	int err = malloc_stats(&stat);
	if(err != 0)
	{
	//printf("Failed to get mem stats\n");
	}

	lpBuffer->dwTotalPhys = stat.max_system_size;
	lpBuffer->dwAvailPhys = stat.max_system_size - stat.current_system_size;
	lpBuffer->dwAvailVirtual = stat.max_system_size - stat.current_inuse_size;*/
}

DWORD GetTickCount()
{
	// This function returns the current system time at this function is called.
	// The system time is represented the time elapsed since the system starts up in microseconds.

	uint64_t sysTime = sceKernelGetProcessTimeWide();

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
	// microseconds
	lpPerformanceCount->QuadPart = sceKernelGetProcessTimeWide();
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
	WIN32_FILE_ATTRIBUTE_DATA *fileInfoBuffer = (WIN32_FILE_ATTRIBUTE_DATA*) lpFileInformation;

	char filePath[256];
	if(strstr(lpFileName,":") != 0) // colon in the filename, so it's fully qualified
		strcpy(filePath, lpFileName);
	else
		sprintf(filePath,"%s/%s",getUsrDirPath(), lpFileName );
	//	sprintf(filePath,"%s/%s", driveRoot, lpFileName );

	// check if the file exists first
	SceFiosStat  statData;
	if(sceFiosStatSync(NULL, filePath, &statData) != SCE_FIOS_OK)
	{
		app.DebugPrintf("*** sceFiosStatSync Failed\n");
		return false;
	}
	if(statData.statFlags & SCE_FIOS_STATUS_DIRECTORY )
		fileInfoBuffer->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	else
		fileInfoBuffer->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

	fileInfoBuffer->nFileSizeHigh = statData.fileSize >> 32;
	fileInfoBuffer->nFileSizeLow = statData.fileSize;

	return true;
}

HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData)
{
	PSVITA_STUBBED;
	return 0;
}

BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
{
	PSVITA_STUBBED;
	return false;
}

errno_t _itoa_s(int _Value, char * _DstBuf, size_t _Size, int _Radix) { if(_Radix==10) sprintf(_DstBuf,"%d",_Value); else if(_Radix==16) sprintf(_DstBuf,"%lx",_Value); else return -1; return 0; }
errno_t _i64toa_s(long long _Val, char * _DstBuf, size_t _Size, int _Radix) { if(_Radix==10) sprintf(_DstBuf,"%lld",_Val); else return -1; return 0; }

int _wtoi(const wchar_t *_Str)
{
	return wcstol(_Str, NULL, 10);
}

DWORD XGetLanguage()
{
	// check if we should override the system language or not
	unsigned char ucLang = app.GetMinecraftLanguage(0);
	if (ucLang != MINECRAFT_LANGUAGE_DEFAULT) return ucLang;

	SceInt32 iLang;
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG,&iLang);
	switch(iLang)
	{
	case SCE_SYSTEM_PARAM_LANG_JAPANESE			: return XC_LANGUAGE_JAPANESE;
	case SCE_SYSTEM_PARAM_LANG_ENGLISH_US		: return XC_LANGUAGE_ENGLISH;
	case SCE_SYSTEM_PARAM_LANG_FRENCH			: return XC_LANGUAGE_FRENCH;
	case SCE_SYSTEM_PARAM_LANG_SPANISH			: return XC_LANGUAGE_SPANISH;
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
	// check if we should override the system locale or not
	unsigned char ucLocale = app.GetMinecraftLocale(0);
	if (ucLocale != MINECRAFT_LANGUAGE_DEFAULT) return ucLocale;

	SceInt32 iLang;
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG,&iLang);
	switch(iLang)
	{
	case SCE_SYSTEM_PARAM_LANG_JAPANESE			: return XC_LOCALE_JAPAN;
	case SCE_SYSTEM_PARAM_LANG_ENGLISH_US		: return XC_LOCALE_UNITED_STATES;
	case SCE_SYSTEM_PARAM_LANG_FRENCH			: return XC_LOCALE_FRANCE;

	case SCE_SYSTEM_PARAM_LANG_SPANISH			:
		if(app.IsAmericanSKU())
		{
			return XC_LOCALE_LATIN_AMERICA;
		}
		else
		{
			return XC_LOCALE_SPAIN;
		}

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
