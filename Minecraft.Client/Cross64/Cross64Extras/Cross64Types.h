#pragma once

#include <stddef.h>
#include <typeinfo>
#include <cstdint>
#include <cstddef>
#include <cwchar>

#define S_OK 0

typedef unsigned long DWORD;
typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef float FLOAT;

typedef int __int32;

typedef FLOAT *PFLOAT;
typedef BOOL *PBOOL;
typedef BOOL *LPBOOL;
typedef BYTE *PBYTE;
typedef BYTE *LPBYTE;
typedef int *PINT;
typedef int *LPINT;
typedef WORD *PWORD;
typedef WORD *LPWORD;
typedef long *PLONG;
typedef long *LPLONG;
typedef DWORD *PDWORD;
typedef DWORD *LPDWORD;
typedef void *LPVOID;
typedef const void *LPCVOID;
typedef void *PVOID;
typedef unsigned long ULONG;

typedef unsigned char boolean;

typedef int INT;
typedef unsigned int UINT;
typedef unsigned int *PUINT;

typedef unsigned char byte;
typedef long long __int64;
typedef unsigned long long __uint64;
typedef unsigned long DWORD;
typedef int INT;
typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR SIZE_T, *PSIZE_T;

typedef __int64 LONG64, *PLONG64;

#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef __int64 LONGLONG;
typedef __uint64 ULONGLONG;

#define CONST const
typedef wchar_t WCHAR; // wc,   16-bit UNICODE character
typedef WCHAR *PWCHAR;
typedef WCHAR *LPWCH, *PWCH;
typedef CONST WCHAR *LPCWCH, *PCWCH;
typedef WCHAR *NWPSTR;
typedef WCHAR *LPWSTR, *PWSTR;
typedef CONST WCHAR *LPCWSTR, *PCWSTR;

//
// ANSI (Multi-byte Character) types
//
typedef CHAR *PCHAR;
typedef CHAR *LPCH, *PCH;
typedef CONST CHAR *LPCCH, *PCCH;
typedef CHAR *NPSTR;
typedef CHAR *LPSTR, *PSTR;
typedef CONST CHAR *LPCSTR, *PCSTR;

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

typedef void *HANDLE;
typedef void *HMODULE;
typedef void *HINSTANCE;
typedef void *HWND;
typedef int errno_t;

typedef struct _FILETIME {
#ifdef _M_PPCBE
  DWORD dwHighDateTime;
  DWORD dwLowDateTime;
#else
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
#endif
} FILETIME, *PFILETIME, *LPFILETIME;

#if defined(MIDL_PASS)
typedef struct _LARGE_INTEGER {
#else // MIDL_PASS
typedef union _LARGE_INTEGER {
  struct {
#if defined(_M_PPCBE)
    LONG HighPart;
    DWORD LowPart;
#else
    DWORD LowPart;
    LONG HighPart;
#endif
  };
  struct {
#if defined(_M_PPCBE)
    LONG HighPart;
    DWORD LowPart;
#else
    DWORD LowPart;
    LONG HighPart;
#endif
  } u;
#endif // MIDL_PASS
  LONGLONG QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER *PLARGE_INTEGER;

#if defined(MIDL_PASS)
typedef struct _ULARGE_INTEGER {
#else // MIDL_PASS
typedef union _ULARGE_INTEGER {
  struct {
#if defined(_M_PPCBE)
    DWORD HighPart;
    DWORD LowPart;
#else
    DWORD LowPart;
    DWORD HighPart;
#endif
  };
  struct {
#if defined(_M_PPCBE)
    DWORD HighPart;
    DWORD LowPart;
#else
    DWORD LowPart;
    DWORD HighPart;
#endif
  } u;
#endif // MIDL_PASS
  ULONGLONG QuadPart;
} ULARGE_INTEGER;

typedef ULARGE_INTEGER *PULARGE_INTEGER;

#define DECLARE_HANDLE(name) typedef HANDLE name
DECLARE_HANDLE(HINSTANCE);

typedef HINSTANCE HMODULE; /* HMODULEs can be used in place of HINSTANCEs */

typedef struct _OVERLAPPED {
  ULONG_PTR Internal;
  ULONG_PTR InternalHigh;
  DWORD Offset;
  DWORD OffsetHigh;
  HANDLE hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef LPVOID PSECURITY_ATTRIBUTES;
typedef LPVOID LPSECURITY_ATTRIBUTES;

#define __in_ecount(a)
#define __in_bcount(a)