/* SCE CONFIDENTIAL
 PlayStation(R)4 Programmer Tool Runtime Library Release 01.600.051
 * Copyright (C) 2013 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */
extern "C"
{

#include <stdlib.h>
#include <mspace.h>
#include <kernel.h>
#include "../Minecraft.World/System.h"

static size_t s_heapLength;
static size_t s_memLength;

static SceLibcMspace s_mspace;
static off_t s_memStart;
static size_t s_memAlign = 2 * 1024 * 1024;

int user_malloc_init(void);
int user_malloc_finalize(void);
void *user_malloc(size_t size);
void user_free(void *ptr);
void *user_calloc(size_t nelem, size_t size);
void *user_realloc(void *ptr, size_t size);
void *user_memalign(size_t boundary, size_t size);
int user_posix_memalign(void **ptr, size_t boundary, size_t size);
void *user_reallocalign(void *ptr, size_t size, size_t boundary);
int user_malloc_stats(SceLibcMallocManagedSize *mmsize);
int user_malloc_stats_fast(SceLibcMallocManagedSize *mmsize);
size_t user_malloc_usable_size(void *ptr);


//E Replace _malloc_init function.
int user_malloc_init(void)
{
	int res;
	void *addr;
	uint64_t dmemSize = SCE_KERNEL_MAIN_DMEM_SIZE;

	s_heapLength = ((size_t)4608) * 1024 * 1024;	// Initial allocation for the application
	s_heapLength -= ((size_t)4) * 1024 * 1024;		// Allocated for TLS
	s_heapLength -= ((size_t)2) * 1024 * 1024;		// 64K (sometimes?) allocated for razor - rounding up to 2MB here to match our alignment
	s_heapLength -= ((size_t)24) * 1024 * 1024;		// 24MB for save file, which is handled by virtual allocs
	s_heapLength -= ((size_t)512) * 1024 * 1024;	// 512MB allocated for GPU by renderer
	s_heapLength -= ((size_t)112) * 1024 * 1024;	// 112MB allocated for CPU by renderer
	s_memLength = s_heapLength;

	//E Allocate direct memory
	res = sceKernelAllocateDirectMemory(0, SCE_KERNEL_MAIN_DMEM_SIZE, s_memLength, s_memAlign, SCE_KERNEL_WB_ONION, &s_memStart);
	if (res < 0) {
		//E Error handling
		return 1;
	}

	addr = nullptr;
	//E Map direct memory to the process address space
	res = sceKernelMapDirectMemory(&addr, s_heapLength, SCE_KERNEL_PROT_CPU_READ | SCE_KERNEL_PROT_CPU_WRITE, 0, s_memStart, s_memAlign);
	if (res < 0) {
		//E Error handling
		return 1;
	}

	//E Generate mspace
	s_mspace = sceLibcMspaceCreate("User Malloc", addr, s_heapLength, 0);
	if (s_mspace == nullptr) {
		//E Error handling
		return 1;
	}

	return 0;
}

//E Replace _malloc_finalize function.
int user_malloc_finalize(void)
{
	int res;

	if (s_mspace != nullptr) {
		//E Free mspace
		//J mspace を解放する
		res = sceLibcMspaceDestroy(s_mspace);
		if (res != 0) {
			//E Error handling
			//J エラー処理
			return 1;
		}
	}

	//E Release direct memory
	//J ダイレクトメモリを解放する
	res = sceKernelReleaseDirectMemory(s_memStart, s_memLength);
	if (res < 0) {
		//E Error handling
		//J エラー処理
		return 1;
	}

	return 0;
}

//E Replace malloc function.
//J malloc 関数と置き換わる
void *user_malloc(size_t size)
{
#if 0
	static int throttle = 0;
	static int64_t lasttime = 0;
	if( ( throttle % 100 ) == 0 )
	{
		int64_t nowtime = System::currentTimeMillis();
		if( ( nowtime - lasttime ) > 20000 )
		{
			lasttime = nowtime;
			SceLibcMallocManagedSize mmsize;
			SCE_LIBC_INIT_MALLOC_MANAGED_SIZE(mmsize);
			int ret = sceLibcMspaceMallocStats( s_mspace, &mmsize);
			printf("%d max %d of %d\n",mmsize.currentInuseSize/(1024*1024),mmsize.maxInuseSize/(1024*1024),mmsize.currentSystemSize/(1024*1024));
		}
	}
	throttle++;
#endif

	return sceLibcMspaceMalloc(s_mspace, size);
}

//E Replace free function.
//J free 関数と置き換わる
void user_free(void *ptr)
{
	sceLibcMspaceFree(s_mspace, ptr);
}

//E Replace calloc function.
//J calloc 関数と置き換わる
void *user_calloc(size_t nelem, size_t size)
{
	return sceLibcMspaceCalloc(s_mspace, nelem, size);
}

//E Replace realloc function.
//J realloc 関数と置き換わる
void *user_realloc(void *ptr, size_t size)
{
	return sceLibcMspaceRealloc(s_mspace, ptr, size);
}

//E Replace memalign function.
//J memalign 関数と置き換わる
void *user_memalign(size_t boundary, size_t size)
{
	return sceLibcMspaceMemalign(s_mspace, boundary, size);
}

//E Replace posix_memalign function.
//J posix_memalign 関数と置き換わる
int user_posix_memalign(void **ptr, size_t boundary, size_t size)
{
	return sceLibcMspacePosixMemalign(s_mspace, ptr, boundary, size);
}

//E Replace reallocalign function.
//J reallocalign 関数と置き換わる
void *user_reallocalign(void *ptr, size_t size, size_t boundary)
{
	return sceLibcMspaceReallocalign(s_mspace, ptr, boundary, size);
}

//E Replace malloc_stats function.
//J malloc_stats 関数と置き換わる
int user_malloc_stats(SceLibcMallocManagedSize *mmsize)
{
	return sceLibcMspaceMallocStats(s_mspace, mmsize);
}

//E Replace malloc_stats_fast function.
//J malloc_stata_fast 関数と置き換わる
int user_malloc_stats_fast(SceLibcMallocManagedSize *mmsize)
{
	return sceLibcMspaceMallocStatsFast(s_mspace, mmsize);
}

//E Replace malloc_usable_size function.
//J malloc_usable_size 関数と置き換わる
size_t user_malloc_usable_size(void *ptr)
{
	return sceLibcMspaceMallocUsableSize(ptr);
}


} // extern "c"
