

#pragma once


#define BOOST_NO_CXX11_NULLPTR
#define BOOST_ENABLE_ASSERT_HANDLER
#include <typeinfo>

#include <stddef.h>
#include <boost/typeof/typeof.hpp>
#include <boost/tr1/memory.hpp>
#include "boost/tr1/unordered_map.hpp"
#include "boost/tr1/unordered_set.hpp"
#include "boost/pool/pool_alloc.hpp"
#include <boost/mpl/equal_to.hpp>


#define __forceinline inline

//#include "winerror.h"

using std::tr1::bad_weak_ptr;
using std::tr1::const_pointer_cast;
using std::tr1::dynamic_pointer_cast;
using std::tr1::enable_shared_from_this;
using std::tr1::get_deleter;
using std::tr1::shared_ptr;
using std::tr1::static_pointer_cast;
using std::tr1::swap;
using std::tr1::weak_ptr;
using std::tr1::unordered_map;
using std::tr1::unordered_set;
using boost::hash;


// user the pool_allocator for all unordered_set and unordered_map instances
// template <	class T, class H = hash<T>, class P = std::equal_to<T>, class A = boost::pool_allocator<T> >
// 	class unordered_set : public std::tr1::unordered_set<T, H, P, A >
// {};
//
// template <class K, class T, class H = hash<K>, class P = std::equal_to<K>, class A = boost::pool_allocator<std::pair<const K,T> > >
// 	class unordered_map : public std::tr1::unordered_map<K, T, H, P, A >
// {};



// template <	class T, class H = hash<T>, class P = std::equal_to<T>, class A = C4JPoolAllocator<T> >
// class unordered_set : public std::tr1::unordered_set<T, H, P, A >
// {};
//
// template <class K, class T, class H = hash<K>, class P = std::equal_to<K>, class A = C4JPoolAllocator<std::pair<const K,T> > >
// class unordered_map : public std::tr1::unordered_map<K, T, H, P, A >
// {};


// using boost::ublas::vector;

#define static_assert(a,b) BOOST_STATIC_ASSERT(a)

class Cnullptr{
public:
	template<typename T>
	operator shared_ptr<T>() { return shared_ptr<T>(); }
};

extern Cnullptr nullptr;




// #include <math.h>
// #include "DirectX\DirectXMath.h"
// using namespace DirectX;

#define S_OK 0


typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;

typedef int					__int32;

typedef FLOAT               *PFLOAT;
typedef BOOL           *PBOOL;
typedef BOOL            *LPBOOL;
typedef BYTE           *PBYTE;
typedef BYTE            *LPBYTE;
typedef int            *PINT;
typedef int             *LPINT;
typedef WORD           *PWORD;
typedef WORD            *LPWORD;
typedef long            *PLONG;
typedef long            *LPLONG;
typedef DWORD          *PDWORD;
typedef DWORD           *LPDWORD;
typedef void            *LPVOID;
typedef const void      *LPCVOID;
typedef void *PVOID;
typedef unsigned long ULONG;

typedef unsigned char boolean;

typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned int        *PUINT;


typedef unsigned char		byte;
typedef long long				__int64;
typedef unsigned long long		__uint64;
typedef unsigned long		DWORD;
typedef int                 INT;
typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR SIZE_T, *PSIZE_T;

typedef __int64 LONG64, *PLONG64;

#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef __int64 LONGLONG;
typedef __uint64 ULONGLONG;


#define CONST               const
typedef wchar_t WCHAR;    // wc,   16-bit UNICODE character
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
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif


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
#endif //MIDL_PASS
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
#endif //MIDL_PASS
	ULONGLONG QuadPart;
} ULARGE_INTEGER;

typedef ULARGE_INTEGER *PULARGE_INTEGER;

// 360 specifics
typedef long				HRESULT;
typedef void*				HANDLE;

#define DECLARE_HANDLE(name) typedef HANDLE name
DECLARE_HANDLE(HINSTANCE);

typedef HINSTANCE HMODULE;      /* HMODULEs can be used in place of HINSTANCEs */


typedef struct _OVERLAPPED {
	ULONG_PTR Internal;
	ULONG_PTR InternalHigh;
	DWORD   Offset;
	DWORD   OffsetHigh;
	HANDLE  hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef LPVOID PSECURITY_ATTRIBUTES;
typedef LPVOID LPSECURITY_ATTRIBUTES;



#define __in_ecount(a)
#define __in_bcount(a)





// const int XUSER_MAX_COUNT = 4;

