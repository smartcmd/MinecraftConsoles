#include "stdafx.h"
#ifdef __PS3__
#include <sys/sys_time.h>
#endif
#include "System.h"

template <class T> void System::arraycopy(arrayWithLength<T> src, unsigned int srcPos, arrayWithLength<T> *dst, unsigned int dstPos, unsigned int length)
{
	assert( srcPos >=0 && srcPos <= src.length);
	assert( srcPos + length <= src.length );
	assert( dstPos + length <= dst->length );

	std::copy( src.data + srcPos, src.data + srcPos + length, dst->data + dstPos );
}

ArrayCopyFunctionDefinition(Node *)
ArrayCopyFunctionDefinition(Biome *)

void System::arraycopy(arrayWithLength<BYTE> src, unsigned int srcPos, arrayWithLength<BYTE> *dst, unsigned int dstPos, unsigned int length)
{
	assert( srcPos >=0 && srcPos <= src.length);
	assert( srcPos + length <= src.length );
	assert( dstPos + length <= dst->length );

	memcpy( dst->data + dstPos, src.data + srcPos, length);
}

void System::arraycopy(arrayWithLength<int> src, unsigned int srcPos, arrayWithLength<int> *dst, unsigned int dstPos, unsigned int length)
{
	assert( srcPos >=0 && srcPos <= src.length);
	assert( srcPos + length <= src.length );
	assert( dstPos + length <= dst->length );

	memcpy( dst->data + dstPos, src.data + srcPos, length * sizeof(int) );
}

// TODO 4J Stu - These time functions may suffer from accuracy and we might have to use a high-resolution timer
//Returns the current value of the most precise available system timer, in nanoseconds.
//This method can only be used to measure elapsed time and is not related to any other notion of system or wall-clock time.
//The value returned represents nanoseconds since some fixed but arbitrary time (perhaps in the future, so values may be negative).
//This method provides nanosecond precision, but not necessarily nanosecond accuracy. No guarantees are made about how
//frequently values change. Differences in successive calls that span greater than approximately 292 years (263 nanoseconds)
//will not accurately compute elapsed time due to numerical overflow.
//
//For example, to measure how long some code takes to execute:
//
//   long startTime = System.nanoTime();
//   // ... the code being measured ...
//   long estimatedTime = System.nanoTime() - startTime;
//
//Returns:
//The current value of the system timer, in nanoseconds.
int64_t System::nanoTime()
{
#if defined _WINDOWS64 || defined _XBOX || defined _WIN32
	static LARGE_INTEGER s_frequency = { 0 };
	if (s_frequency.QuadPart == 0)
	{
		QueryPerformanceFrequency(&s_frequency);
	}

	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);

	// Using double to avoid 64-bit overflow during multiplication for long uptime
	// Precision is sufficient for ~100 days of uptime.
	return static_cast<int64_t>(static_cast<double>(counter.QuadPart) * 1000000000.0 / static_cast<double>(s_frequency.QuadPart));
#else
	return GetTickCount() * 1000000LL;
#endif
}

//Returns the current time in milliseconds. Note that while the unit of time of the return value is a millisecond,
//the granularity of the value depends on the underlying operating system and may be larger. For example,
//many operating systems measure time in units of tens of milliseconds.
//See the description of the class Date for a discussion of slight discrepancies that may arise between "computer time"
//and coordinated universal time (UTC).
//
//Returns:
//the difference, measured in milliseconds, between the current time and midnight, January 1, 1970 UTC.
int64_t System::currentTimeMillis()
{
#ifdef __PS3__
//	sys_time_get_current_time() obtains the elapsed time since Epoch (1970/01/01 00:00:00 UTC).
//	The value is separated into two parts: sec stores the elapsed time in seconds, and nsec
//  stores the value that is smaller than a second in nanoseconds.
	sys_time_sec_t sec;
	sys_time_nsec_t nsec;
	sys_time_get_current_time(&sec, &nsec);
	int64_t msec = (sec * 1000) + (nsec / (1000*1000));
	return msec;

#elif defined __ORBIS__
	SceRtcTick tick;
	int err = sceRtcGetCurrentTick(&tick);

	return (int64_t)(tick.tick / 1000);
#elif defined __PSVITA__
	// AP - TRC states we can't use the RTC for measuring elapsed game time
	return sceKernelGetProcessTimeWide() / 1000;
/*	SceDateTime Time;
	sceRtcGetCurrentClockLocalTime(&Time);
	int64_t systTime = (((((((Time.day * 24) + Time.hour) * 60) + Time.minute) * 60) + Time.second) * 1000) + (Time.microsecond / 1000);
	return systTime;*/

#else

	SYSTEMTIME UTCSysTime;
    GetSystemTime( &UTCSysTime );

	//Represents as a 64-bit value the number of 100-nanosecond intervals since January 1, 1601
    FILETIME UTCFileTime;
    SystemTimeToFileTime( &UTCSysTime, &UTCFileTime );

	LARGE_INTEGER li;
	li.HighPart = UTCFileTime.dwHighDateTime;
	li.LowPart = UTCFileTime.dwLowDateTime;

	return li.QuadPart/10000;
#endif // __PS3__
}

// 4J Stu - Added this so that we can use real-world timestamps in PSVita saves. Particularly required for the save transfers to be smooth
int64_t System::currentRealTimeMillis()
{
#ifdef __PSVITA__
	SceFiosDate fileTime = sceFiosDateGetCurrent();

	return fileTime/1000000;
#else
	return currentTimeMillis();
#endif
}


void System::ReverseUSHORT(unsigned short *pusVal)
{
	unsigned short usValue=*pusVal;
	unsigned char *pchVal1=(unsigned char *)pusVal;
	unsigned char *pchVal2=(unsigned char *)&usValue;

	pchVal1[0]=pchVal2[1];
	pchVal1[1]=pchVal2[0];
}

void System::ReverseSHORT(short *pusVal)
{
	short usValue=*pusVal;
	unsigned char *pchVal1=(unsigned char *)pusVal;
	unsigned char *pchVal2=(unsigned char *)&usValue;

	pchVal1[0]=pchVal2[1];
	pchVal1[1]=pchVal2[0];
}

void System::ReverseULONG(unsigned long *pulVal)
{
	unsigned long ulValue=*pulVal;
	unsigned char *pchVal1=(unsigned char *)pulVal;
	unsigned char *pchVal2=(unsigned char *)&ulValue;

	pchVal1[0]=pchVal2[3];
	pchVal1[1]=pchVal2[2];
	pchVal1[2]=pchVal2[1];
	pchVal1[3]=pchVal2[0];
}

void System::ReverseULONG(unsigned int *pulVal)
{
	unsigned int ulValue=*pulVal;
	unsigned char *pchVal1=(unsigned char *)pulVal;
	unsigned char *pchVal2=(unsigned char *)&ulValue;

	pchVal1[0]=pchVal2[3];
	pchVal1[1]=pchVal2[2];
	pchVal1[2]=pchVal2[1];
	pchVal1[3]=pchVal2[0];
}

void System::ReverseINT(int *piVal)
{
	int ulValue=*piVal;
	unsigned char *pchVal1=(unsigned char *)piVal;
	unsigned char *pchVal2=(unsigned char *)&ulValue;

	pchVal1[0]=pchVal2[3];
	pchVal1[1]=pchVal2[2];
	pchVal1[2]=pchVal2[1];
	pchVal1[3]=pchVal2[0];
}

void System::ReverseULONGLONG(int64_t *pullVal)
{
	int64_t ullValue=*pullVal;
	unsigned char *pchVal1=(unsigned char *)pullVal;
	unsigned char *pchVal2=(unsigned char *)&ullValue;

	pchVal1[0]=pchVal2[7];
	pchVal1[1]=pchVal2[6];
	pchVal1[2]=pchVal2[5];
	pchVal1[3]=pchVal2[4];
	pchVal1[4]=pchVal2[3];
	pchVal1[5]=pchVal2[2];
	pchVal1[6]=pchVal2[1];
	pchVal1[7]=pchVal2[0];
}

void System::ReverseWCHARA(WCHAR *pwch,int iLen)
{
	for(int i=0;i<iLen;i++)
	{
		ReverseUSHORT((unsigned short *)&pwch[i]);
	}
}
