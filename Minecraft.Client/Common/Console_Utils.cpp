#include "stdafx.h"
#if defined(_WINDOWS64) && defined(MINECRAFT_SERVER_BUILD)
#include "..\..\Minecraft.Server\ServerLogManager.h"
#endif

//--------------------------------------------------------------------------------------
// Name: DebugSpewV()
// Desc: Internal helper function
//--------------------------------------------------------------------------------------
#ifndef _CONTENT_PACKAGE
static VOID DebugSpewV( const CHAR* strFormat, va_list pArgList )
{
#if defined __PS3__ || defined __ORBIS__ || defined __PSVITA__
    assert(0);
#else
#if defined(_WINDOWS64) && defined(MINECRAFT_SERVER_BUILD)
    // Dedicated server routes legacy debug spew through ServerLogger to preserve CLI prompt handling.
    if (ServerRuntime::ServerLogManager::ShouldForwardClientDebugLogs())
    {
        ServerRuntime::ServerLogManager::ForwardClientDebugSpewLogV(strFormat, pArgList);
        return;
    }
#endif
    CHAR str[2048];
    // Use the secure CRT to avoid buffer overruns. Specify a count of
    // _TRUNCATE so that too long strings will be silently truncated
    // rather than triggering an error.
    _vsnprintf_s( str, _TRUNCATE, strFormat, pArgList );
    OutputDebugStringA( str );
#endif
}
#endif

//--------------------------------------------------------------------------------------
// Name: DebugSpew()
// Desc: Prints formatted debug spew
//--------------------------------------------------------------------------------------
#ifdef  _Printf_format_string_  // VC++ 2008 and later support this annotation
VOID CDECL DebugSpew( _In_z_ _Printf_format_string_ const CHAR* strFormat, ... )
#else
VOID CDECL DebugPrintf( const CHAR* strFormat, ... )
#endif
{
#ifndef _CONTENT_PACKAGE
    va_list pArgList;
    va_start( pArgList, strFormat );
    DebugSpewV( strFormat, pArgList );
    va_end( pArgList );
#endif
}
