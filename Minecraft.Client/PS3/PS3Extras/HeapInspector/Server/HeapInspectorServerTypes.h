#ifndef _HEAPINSPECTORSERVER_TYPES_H_
#define _HEAPINSPECTORSERVER_TYPES_H_

#define BEGIN_NAMESPACE(x) namespace x {
#define END_NAMESPACE(x) }
#define BEGIN_UNNAMED_NAMESPACE() namespace {
#define END_UNNAMED_NAMESPACE() }

BEGIN_NAMESPACE(HeapInspectorServer)

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#if HEAPINSPECTOR_PS3
	typedef unsigned long long uint64;
	typedef long long int64;
#else
	typedef unsigned __int64 uint64;
	typedef __int64 int64;
#endif

typedef char int8;
typedef short int16;
typedef int int32;

typedef char char8;
typedef wchar_t char16;

END_NAMESPACE(HeapInspectorServer)

#endif //_HEAPINSPECTORSERVER_TYPES_H_
