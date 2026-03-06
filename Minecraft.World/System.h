#pragma once
using namespace std;

#include "ArrayWithLength.h"

// 4J Jev, just thought it would be easier this way.
#define ArrayCopyFunctionDeclaration(x) static void arraycopy(arrayWithLength<x> src, unsigned int srcPos, arrayWithLength<x> *dst, unsigned int dstPos, unsigned int length);
#define ArrayCopyFunctionDefinition(x) void System::arraycopy(arrayWithLength<x> src, unsigned int srcPos, arrayWithLength<x> *dst, unsigned int dstPos, unsigned int length)\
{\
	arraycopy<x>(src,srcPos,dst,dstPos,length);\
}\

class System
{
	template <class T> static void arraycopy(arrayWithLength<T> src, unsigned int srcPos, arrayWithLength<T> *dst, unsigned int dstPos, unsigned int length);

public:
	ArrayCopyFunctionDeclaration(byte)
	ArrayCopyFunctionDeclaration(Node *)
	ArrayCopyFunctionDeclaration(Biome *)
	ArrayCopyFunctionDeclaration(int)

	static int64_t nanoTime();
	static int64_t currentTimeMillis();
	static int64_t currentRealTimeMillis(); // 4J Added to get real-world time for timestamps in saves

	static void ReverseUSHORT(unsigned short *pusVal);
	static void ReverseSHORT(short *psVal);
	static void ReverseULONG(unsigned long *pulVal);
	static void ReverseULONG(unsigned int *pulVal);
	static void ReverseINT(int *piVal);
	static void ReverseULONGLONG(int64_t *pullVal);
	static void ReverseWCHARA(WCHAR *pwch,int iLen);

};

#define MAKE_FOURCC(ch0, ch1, ch2, ch3)                              \
	((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |       \
	((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
