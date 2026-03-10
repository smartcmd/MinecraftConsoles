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

#define HEAP_SIZE (4 * 1024 * 1024)

static SceLibcMspace s_mspace;
static off_t s_memStart;
static size_t s_memLength = 4 * 1024 * 1024;
static size_t s_memAlign = 2 * 1024 * 1024;

int user_malloc_init_for_tls(void);
int user_malloc_fini_for_tls(void);
void *user_malloc_for_tls(size_t size);
void user_free_for_tls(void *ptr);
int user_posix_memalign_for_tls(void **ptr, size_t boundary, size_t size);

//E Replace _malloc_init_for_tls function.
//J _malloc_init_for_tls 関数と置き換わる
int user_malloc_init_for_tls(void)
{
	int res;
	void *addr;
	
	//E Allocate direct memory
	//J ダイレクトメモリを割り当てる
	res = sceKernelAllocateDirectMemory(0, SCE_KERNEL_MAIN_DMEM_SIZE, s_memLength, s_memAlign, SCE_KERNEL_WB_ONION, &s_memStart);
	if (res < 0) {
		//E Error handling
		//J エラー処理
		return 1;
	}

	addr = nullptr;
	//E Map direct memory to the process address space
	//J ダイレクトメモリをプロセスアドレス空間にマップする
	res = sceKernelMapDirectMemory(&addr, HEAP_SIZE, SCE_KERNEL_PROT_CPU_READ | SCE_KERNEL_PROT_CPU_WRITE, 0, s_memStart, s_memAlign);
	if (res < 0) {
		//E Error handling
		//J エラー処理
		return 1;
	}

	//E Generate mspace
	//J mspace を生成する
	s_mspace = sceLibcMspaceCreate("User Malloc For TLS", addr, HEAP_SIZE, 0);
	if (s_mspace == nullptr) {
		//E Error handling
		//J エラー処理
		return 1;
	}

	return 0;
}

//E Replace _malloc_fini_for_tls function.
//J _malloc_fini_for_tls 関数と置き換わる
int user_malloc_fini_for_tls(void)
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

//E Replace _malloc_for_tls function.
//J _malloc_for_tls 関数と置き換わる
void *user_malloc_for_tls(size_t size)
{
	return sceLibcMspaceMalloc(s_mspace, size);
}

//E Replace _free_for_tls function.
//J _free_for_tls 関数と置き換わる
void user_free_for_tls(void *ptr)
{
	sceLibcMspaceFree(s_mspace, ptr);
}

//E Replace _posix_memalign_for_tls function.
//J _posix_memalign_for_tls 関数と置き換わる
int user_posix_memalign_for_tls(void **ptr, size_t boundary, size_t size)
{
	return sceLibcMspacePosixMemalign(s_mspace, ptr, boundary, size);
}

} // extern "c"
