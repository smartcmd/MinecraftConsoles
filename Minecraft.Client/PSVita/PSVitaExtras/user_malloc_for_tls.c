/* SCE CONFIDENTIAL
 PlayStation(R)Vita Programmer Tool Runtime Library Release 03.000.061
 * Copyright (C) 2012 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <stdlib.h>
#include <mspace.h>
#include <kernel.h>
//#include <libdbg.h>

#define HEAP_SIZE (1024 * 1024)
#define HEAP_ERROR1 1
#define HEAP_ERROR2 2
#define HEAP_ERROR3 3

static SceUID s_heapUid;
static mspace s_mspace;

void user_malloc_for_tls_init(void);
void user_malloc_for_tls_finalize(void);
void *user_malloc_for_tls(size_t size);
void user_free_for_tls(void *ptr);

/**E Replace _malloc_for_tls_init function. */
/**J _malloc_for_tls_init 関数と置き換わる */
void user_malloc_for_tls_init(void)
{
	int res;
	void *base = nullptr;

	/**E Allocate a memory block from the kernel */
	/**J カーネルからメモリブロックを確保する */
	s_heapUid = sceKernelAllocMemBlock("UserAllocatorForTLS", SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA, HEAP_SIZE, SCE_NULL);
	if (s_heapUid < SCE_OK) {
		/**E Error handling */
		/**J エラー処理 */
		sceLibcSetHeapInitError(HEAP_ERROR1);
	} else {
		/**E Obtain the address of the allocated memory block */
		/**J 確保したメモリブロックのアドレスを取得する */
		res = sceKernelGetMemBlockBase(s_heapUid, &base);
		if (res < SCE_OK) {
			/**E Error handling */
			/**J エラー処理 */
			sceLibcSetHeapInitError(HEAP_ERROR2);
		} else {
			/**E Generate mspace */
			/**J mspace を生成する */
			s_mspace = mspace_create(base, HEAP_SIZE);
			if (s_mspace == nullptr) {
				/**E Error handling */
				/**J エラー処理 */
				sceLibcSetHeapInitError(HEAP_ERROR3);
			}
		}
	}
}

/**E Replace _malloc_for_tls_finalize function. */
/**J _malloc_for_tls_finalize 関数と置き換わる */
void user_malloc_for_tls_finalize(void)
{
	int res;

	if (s_mspace != nullptr) {
		/**E Free mspace */
		/**J mspace を解放する */
		res = mspace_destroy(s_mspace);
		if (res != 0) {
			/**E Error handling */
			/**J エラー処理 */
			__breakpoint(0);
		}
	}

	if (SCE_OK <= s_heapUid) {
		/**E Free the memory block */
		/**J メモリブロックを解放する */
		res = sceKernelFreeMemBlock(s_heapUid);
		if (res < SCE_OK) {
			/**E Error handling */
			/**J エラー処理 */
			__breakpoint(0);
		}
	}
}

/**E Replace _malloc_for_tls function. */
/**J _malloc_for_tls 関数と置き換わる */
void *user_malloc_for_tls(size_t size)
{
//	SCE_DBG_LOG_TRACE("Called malloc_for_tls(%u)", size);
	return mspace_malloc(s_mspace, size);
}

/**E Replace _free_for_tls function. */
/**J _free_for_tls 関数と置き換わる */
void user_free_for_tls(void *ptr)
{
//	SCE_DBG_LOG_TRACE("Called free_for_tls(%p)", ptr);
	mspace_free(s_mspace, ptr);
}
