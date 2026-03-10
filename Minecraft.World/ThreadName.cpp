#include "stdafx.h"

// From Xbox documentation

typedef struct tagTHREADNAME_INFO {
    DWORD dwType;     // Must be 0x1000
    LPCSTR szName;    // Pointer to name (in user address space)
    DWORD dwThreadID; // Thread ID (-1 for caller thread)
    DWORD dwFlags;    // Reserved for future use; must be zero
} THREADNAME_INFO;

void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName )
{
#ifndef __PS3__
    THREADNAME_INFO info;
	
    info.dwType = 0x1000;
    info.szName = szThreadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
	
#if ( defined _WINDOWS64 | defined _DURANGO )
	__try
	{
        RaiseException(0x406D1388, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR *)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
#endif
#ifdef _XBOX
    __try
    {
        RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD *)&info );
    }
    __except( GetExceptionCode()==0x406D1388 ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_EXECUTE_HANDLER )
    {
    }
#endif
#endif // __PS3__
}
