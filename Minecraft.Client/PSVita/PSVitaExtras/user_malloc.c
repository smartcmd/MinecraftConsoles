/* SCE CONFIDENTIAL
 PlayStation(R)Vita Programmer Tool Runtime Library Release 03.000.061
 * Copyright (C) 2012 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

// AP - this is a modified version of the basic sony memory overide functions. 
// It was found from profiling that threads were stalling far too much waiting for their turn to malloc/free
// This manager will keep memory bound to a thread once allocated so it can be reused by the same thread later.
// It's only interested in chunks up to 1036 bytes in size, anything greater is not retained
// Todo : I may need to put in a memory flusher if I find threads holding onto memory too much.

#include <stdlib.h>
#include <mspace.h>
#include <kernel.h>
//#include <libdbg.h>
#include <assert.h>
#define HEAP_SIZE (164 * 1024 * 1024)
#define HEAP_ERROR1 1
#define HEAP_ERROR2 2
#define HEAP_ERROR3 3

static SceUID s_heapUid;
static mspace s_mspace;

void user_malloc_init(void);
void user_malloc_finalize(void);
void *user_malloc(size_t size);
void user_free(void *ptr);
void *user_calloc(size_t nelem, size_t size);
void *user_realloc(void *ptr, size_t size);
void *user_memalign(size_t boundary, size_t size);
void *user_reallocalign(void *ptr, size_t size, size_t boundary);
int user_malloc_stats(struct malloc_managed_size *mmsize);
int user_malloc_stats_fast(struct malloc_managed_size *mmsize);
size_t user_malloc_usable_size(void *ptr);

// this is our basic node for managing stacks
typedef struct Block
{
	void* Memory;
	struct Block *Next;
} Block;

#define MaxRetainedBytes 1036
#define Malloc_BlocksMemorySize 1024
typedef struct
{
	Block **Malloc_MemoryPool;		// this is an array of available memory allocations up to 1036 bytes in size
	Block *Malloc_Blocks;			// this is a stack of available block nodes used to store memory chunks
	Block *Malloc_BlocksMemory[Malloc_BlocksMemorySize];		// this is a pool of block nodes allocated in large chunks (256 blocks at a time)
	int Malloc_BlocksAlloced;			// this shows how many block node chunks have been allocated in Malloc_BlocksMemory
} SThreadStorage;

__thread SThreadStorage *Malloc_ThreadStorage = nullptr;

/**E Replace _malloc_init function. */
/**J _malloc_init 関数と置き換わる */
void user_malloc_init(void)
{
	int res;
	void *base = nullptr;

	/**E Allocate a memory block from the kernel */
	/**J カーネルからメモリブロックを確保する */
	s_heapUid = sceKernelAllocMemBlock("UserAllocator", SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA, HEAP_SIZE, SCE_NULL);
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

/**E Replace _malloc_finalize function. */
/**J _malloc_finalize 関数と置き換わる */
void user_malloc_finalize(void)
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
		s_mspace = nullptr;
	}

	if (SCE_OK <= s_heapUid) {
		/**E Free the memory block */
		/**J メモリブロックを解放する */
#if 0
		res = sceKernelFreeMemBlock(s_heapUid);
		if (res < SCE_OK) {
			/**E Error handling */
			/**J エラー処理 */
			__breakpoint(0);
		}
#endif
	}
}

// before a thread can use the memory system it should register itself
void user_registerthread()
{
	Malloc_ThreadStorage = mspace_malloc(s_mspace, sizeof(SThreadStorage));
	Malloc_ThreadStorage->Malloc_Blocks = nullptr;
	Malloc_ThreadStorage->Malloc_BlocksAlloced = 0;
	Malloc_ThreadStorage->Malloc_MemoryPool = nullptr;
}

// before a thread is destroyed make sure we free any space it might be holding on to
void user_removethread()
{
	SThreadStorage *psStorage = Malloc_ThreadStorage;
	if( psStorage )
	{
		if( psStorage->Malloc_MemoryPool )
		{
			for( int j = 0;j < 1037;j += 1 )
			{
				Block *OldBlock = psStorage->Malloc_MemoryPool[j];
				while( OldBlock )
				{
					mspace_free(s_mspace, OldBlock->Memory);
					OldBlock = OldBlock->Next;
				}
			}

			mspace_free(s_mspace, psStorage->Malloc_MemoryPool);
		}

		for( int j = 0;j < psStorage->Malloc_BlocksAlloced;j += 1 )
		{
			free(psStorage->Malloc_BlocksMemory[j]);
		}

		mspace_free(s_mspace, psStorage);
		Malloc_ThreadStorage = nullptr;
	}
}

/**E Replace malloc function. */
/**J malloc 関数と置き換わる */
void *user_malloc(size_t size)
{
	void *p = nullptr;

	SThreadStorage *psStorage = Malloc_ThreadStorage;
	if( psStorage )
	{
		// is this the first time we've malloced
		if( psStorage->Malloc_MemoryPool == nullptr )
		{
			// create an array of pointers to Block nodes, one pointer for each memory bytes size up to 1036
			psStorage->Malloc_MemoryPool = mspace_malloc(s_mspace, (MaxRetainedBytes+1) * 4);
			for( int i = 0;i < (MaxRetainedBytes+1);i += 1 )
			{
				psStorage->Malloc_MemoryPool[i] = nullptr;
			}
		}

		// are we interested in retaining this size of memory (less 4 bytes to store the associated thread ID)
		if( size < (MaxRetainedBytes-4) )
		{
			// add on space for the thread ID
			size += 4;

			// round to the nearest malloc boundary. This is what happens internally in the malloc library
			if( size <= 12 )
				size = 12;
			else
				size = ((size - 12) & 0xfffffff0) + 16 + 12;

			// do we have any memory of this size retained
			if( psStorage->Malloc_MemoryPool[size] )
			{
				// pop it from the retained pool
				Block * OldBlock = psStorage->Malloc_MemoryPool[size];
				psStorage->Malloc_MemoryPool[size] = OldBlock->Next;

				p = OldBlock->Memory;

				// push the block storage onto the stack
				OldBlock->Next = psStorage->Malloc_Blocks;
				psStorage->Malloc_Blocks = OldBlock;
			}
			else
			{
				// create some new memory
				p = mspace_malloc(s_mspace, size);

				// store the thread ID in the last 4 bytes
				unsigned int threadID = sceKernelGetThreadId();
				unsigned int *ThreadAddr = (unsigned int *) ((unsigned char*) p + (size - 4));
				ThreadAddr[0] = threadID;
			}
		}
		else
		{
			p = mspace_malloc(s_mspace, size);
		}
	}
	else
	{
		p = mspace_malloc(s_mspace, size);
	}

//	SCE_DBG_LOG_TRACE("Called malloc(%u)", size);
	return p;
}

/**E Replace free function. */
/**J free 関数と置き換わる */
void user_free(void *ptr)
{
	if( !ptr || !s_mspace )
	{
		return;
	}

	SThreadStorage *psStorage = Malloc_ThreadStorage;
	if( psStorage )
	{
		// calc the size of this chunk rounding to a valid size
		unsigned int size = ((unsigned int*)ptr)[-1] - 7;
		unsigned int RoundedSize = (size & 0xfffffff0) + 12;
	
		if( RoundedSize < (MaxRetainedBytes+1) )
		{
			// grab the thread ID from the last 4 bytes
			unsigned int *ThreadAddr = (unsigned int *) ((unsigned char*) ptr + (RoundedSize - 4));

			// did this thread create this memory
			if( sceKernelGetThreadId() != ThreadAddr[0] )
			{
				// don't worry about retaining memory allocated from a different thread. too much mucking about
				mspace_free(s_mspace, ptr);
				return;
			}

			// we need a block node to retain the memory on our stack. do we have any block nodes available
			if( psStorage->Malloc_Blocks == nullptr )
			{
				if( psStorage->Malloc_BlocksAlloced == Malloc_BlocksMemorySize ) 
				{
					// Max blocks allocated
					// The worst case for this running out is when the player has explored a large amount of the map in a single session and then quits. All the allocations
					// are suddenly freed up so we need lots of space to store them.
					printf("oh nos, max blocks allocated. increase Malloc_BlocksMemorySize");
					assert(0);
				}

				// allocate some more block space in a large chunk
				int chunkSize = 256;
				psStorage->Malloc_BlocksMemory[psStorage->Malloc_BlocksAlloced] = mspace_malloc(s_mspace, chunkSize * sizeof(Block));

				// push all the new blocks onto the pool
				for(int i = 0;i < chunkSize;i += 1 )
				{
					Block *NewBlock = &psStorage->Malloc_BlocksMemory[psStorage->Malloc_BlocksAlloced][i];
					NewBlock->Next = psStorage->Malloc_Blocks;
					psStorage->Malloc_Blocks = NewBlock;
				}

				psStorage->Malloc_BlocksAlloced++;
			}

			// pop a block node off the stack
			Block *NewBlock = psStorage->Malloc_Blocks;
			psStorage->Malloc_Blocks = NewBlock->Next;

			// set up the block data and retain it to the correct slot. We can now reuse this memory on the next malloc
			NewBlock->Memory = ptr;
			NewBlock->Next = psStorage->Malloc_MemoryPool[RoundedSize];
			psStorage->Malloc_MemoryPool[RoundedSize] = NewBlock;
		}
		else
		{
			mspace_free(s_mspace, ptr);
		}
	}
	else
	{
		mspace_free(s_mspace, ptr);
	}
}

/**E Replace calloc function. */
/**J calloc 関数と置き換わる */
void *user_calloc(size_t nelem, size_t size)
{
//	SCE_DBG_LOG_TRACE("Called calloc(%u, %u)", nelem, size);
	return mspace_calloc(s_mspace, nelem, size);
}

/**E Replace realloc function. */
/**J realloc 関数と置き換わる */
void *user_realloc(void *ptr, size_t size)
{
	void* p = nullptr;

	if( Malloc_ThreadStorage )
	{
		// make sure we use malloc/memcpy/free instead of realloc
		p = user_malloc(size);

		unsigned int OldSize;
		unsigned int OldRoundedSize;
		if( ptr )
		{
			// calc the size of this chunk rounding to a valid size
			OldSize = ((unsigned int*)ptr)[-1] - 7;
			OldRoundedSize = (OldSize & 0xfffffff0) + 12;
			memcpy(p, ptr, OldRoundedSize);
			user_free(ptr);
		}
	}
	else
	{
		p = mspace_realloc(s_mspace, ptr, size);
	}

	return p;

//	SCE_DBG_LOG_TRACE("Called realloc(%p, %u)", ptr, size);
//	return mspace_realloc(s_mspace, ptr, size);
}

/**E Replace memalign function. */
/**J memalign 関数と置き換わる */
void *user_memalign(size_t boundary, size_t size)
{
//	SCE_DBG_LOG_TRACE("Called memalign(%u, %u)", boundary, size);
	return mspace_memalign(s_mspace, boundary, size);
}

/**E Replace reallocalign function. */
/**J reallocalign 関数と置き換わる */
void *user_reallocalign(void *ptr, size_t size, size_t boundary)
{
//	SCE_DBG_LOG_TRACE("Called reallocalign(%p, %u, %u)", ptr, size, boundary);
	return mspace_reallocalign(s_mspace, ptr, size, boundary);
}

/**E Replace malloc_stats function. */
/**J malloc_stats 関数と置き換わる */
int user_malloc_stats(struct malloc_managed_size *mmsize)
{
//	SCE_DBG_LOG_TRACE("Called malloc_stats");
	return mspace_malloc_stats(s_mspace, mmsize);
}

/**E Replace malloc_stats_fast function. */
/**J malloc_stata_fast 関数と置き換わる */
int user_malloc_stats_fast(struct malloc_managed_size *mmsize)
{
//	SCE_DBG_LOG_TRACE("Called malloc_stats_fast");
	return mspace_malloc_stats_fast(s_mspace, mmsize);
}

/**E Replace malloc_usable_size function. */
/**J malloc_usable_size 関数と置き換わる */
size_t user_malloc_usable_size(void *ptr)
{
//	SCE_DBG_LOG_TRACE("Called malloc_usable_size");
	return mspace_malloc_usable_size(ptr);
}

