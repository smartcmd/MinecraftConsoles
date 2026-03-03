#pragma once

#include <cstdint>
#include <stddef.h>

#define S_OK 0

typedef uint32_t DWORD;
typedef int32_t BOOL;
typedef uint8_t BYTE;
typedef uint16_t WORD;
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
typedef int *PLONG;
typedef int *LPLONG;
typedef DWORD *PDWORD;
typedef DWORD *LPDWORD;
typedef void *LPVOID;
typedef const void *LPCVOID;
typedef void *PVOID;
typedef unsigned int ULONG;

typedef unsigned char boolean;

typedef int INT;
typedef unsigned int UINT;
typedef unsigned int *PUINT;

typedef long __int64;
typedef unsigned long __uint64;
typedef unsigned int DWORD;
typedef int INT;
typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR SIZE_T, *PSIZE_T;

typedef long LONG64, *PLONG64;

#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef int LONG;
typedef long LONGLONG;
typedef unsigned long ULONGLONG;

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

typedef struct _FILETIME {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct {
  DWORD LowPart;
  LONG HighPart;
  long long QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER *PLARGE_INTEGER;

typedef struct {
  DWORD LowPart;
  LONG HighPart;
  unsigned long long QuadPart;
} ULARGE_INTEGER;

typedef ULARGE_INTEGER *PULARGE_INTEGER;

typedef int HRESULT;
typedef void *HANDLE;

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

#ifndef AUTO_VAR
#define AUTO_VAR(_var, _val) auto _var = _val
#endif