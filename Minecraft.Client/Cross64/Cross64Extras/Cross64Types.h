#pragma once

#include <cstdint>
#include <stddef.h>

#define S_OK 0
#define TRUE 1
#define FALSE 0

#ifndef AUTO_VAR
#define AUTO_VAR(_var, _val) auto _var = _val
#endif

#define VOID void
#define CONST const

#define __in_ecount(a)
#define __in_bcount(a)

// BASIC TYPES
typedef uint32_t DWORD;
typedef int32_t BOOL; // what were they thinking
typedef unsigned char boolean; // keeping as is, scary fucked up javaheads only.
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef float FLOAT;
typedef int32_t INT;
typedef uint32_t UINT;
typedef uint32_t ULONG;
typedef int8_t CHAR;
typedef int16_t SHORT;
typedef int32_t LONG;
typedef int64_t LONG64;
typedef int64_t LONGLONG;
typedef uint64_t ULONGLONG;
typedef int32_t __int32;
typedef int64_t __int64;
typedef uint64_t __uint64;

typedef uintptr_t ULONG_PTR;
typedef size_t SIZE_T;

// POINTERS
typedef BOOL *PBOOL, *LPBOOL;
typedef BYTE *PBYTE, *LPBYTE;
typedef WORD *PWORD, *LPWORD;
typedef INT *PINT, *LPINT, *PUINT;
typedef LONG *PLONG, *LPLONG;
typedef LONG64 *PLONG64;
typedef DWORD *PDWORD, *LPDWORD;
typedef FLOAT *PFLOAT;
typedef ULONG_PTR *PULONG_PTR;
typedef SIZE_T *PSIZE_T;

typedef void* PVOID;
typedef void* LPVOID;
typedef const void* LPCVOID;

typedef void *HANDLE;
typedef HANDLE HINSTANCE;
typedef HINSTANCE HMODULE; /* HMODULEs can be used in place of HINSTANCEs */
typedef int32_t HRESULT;

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

typedef struct _FILETIME {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef union _LARGE_INTEGER {
  struct {
    DWORD LowPart;
    LONG HighPart;
  };

  struct {
    DWORD LowPart;
    LONG HighPart;
  } u;
  LONGLONG QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER {
  struct {
    DWORD LowPart;
    DWORD HighPart;
  };

  struct {
    DWORD LowPart;
    DWORD HighPart;
  } u;
  ULONGLONG QuadPart;
} ULARGE_INTEGER;

typedef ULARGE_INTEGER *PULARGE_INTEGER;

typedef struct _OVERLAPPED {
  ULONG_PTR Internal;
  ULONG_PTR InternalHigh;
  DWORD Offset;
  DWORD OffsetHigh;
  HANDLE hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef LPVOID PSECURITY_ATTRIBUTES;
typedef LPVOID LPSECURITY_ATTRIBUTES;