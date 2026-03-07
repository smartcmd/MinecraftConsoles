
#pragma once
 

#include <stdlib.h>

class C4JMemoryPool
{
public:
	unsigned int Align(unsigned int val, unsigned int align) { 		return int((val+(align-1))/align) * align; }
	virtual void* Alloc(size_t size) = 0;
	virtual void Free(void* ptr) = 0;
};



// Fast Efficient Fixed-Size Memory Pool : No Loops and No Overhead
// http://www.alogicalmind.com/memory_pools/index.htm
class C4JMemoryPoolFixed : public C4JMemoryPool
{ 
	// Basic type define
	typedef unsigned int uint;
	typedef unsigned char uchar;
	uint m_numOfBlocks; // Num of blocks
	uint m_sizeOfEachBlock; // Size of each block
	uint m_numFreeBlocks; // Num of remaining blocks
	uint m_numInitialized; // Num of initialized blocks
	uchar* m_memStart; // Beginning of memory pool
	uchar* m_memEnd; // End of memory pool
	uchar* m_next; // Num of next free block
// 	CRITICAL_SECTION m_CS;
public:
	C4JMemoryPoolFixed()
	{
		m_numOfBlocks = 0;
		m_sizeOfEachBlock = 0;
		m_numFreeBlocks = 0;
		m_numInitialized = 0;
		m_memStart = NULL;
		m_memEnd = NULL;
		m_next = 0;
	}

	C4JMemoryPoolFixed(uint sizeOfEachBlock,	uint numOfBlocks)
	{
		CreatePool(sizeOfEachBlock, numOfBlocks);
	}

	~C4JMemoryPoolFixed() { DestroyPool(); }

	void CreatePool(uint sizeOfEachBlock,	uint numOfBlocks)
	{
		assert(sizeOfEachBlock >= 4); // has to be at least the size of an int, for book keeping
		m_numOfBlocks = numOfBlocks;
		m_sizeOfEachBlock = sizeOfEachBlock;
		m_numFreeBlocks = numOfBlocks;
		m_numInitialized = 0;
		m_memStart = new uchar[ m_sizeOfEachBlock *
			m_numOfBlocks ];
		m_memEnd = m_memStart + (m_sizeOfEachBlock * m_numOfBlocks);
		m_next = m_memStart;
// 		InitializeCriticalSection(&m_CS);
	}

	void DestroyPool()
	{
		delete[] m_memStart;
		m_memStart = NULL;
	}

	uchar* AddrFromIndex(uint i) const
	{
		return m_memStart + ( i * m_sizeOfEachBlock );
	}

	uint IndexFromAddr(const uchar* p) const
	{
		return (((uint)(p - m_memStart)) / m_sizeOfEachBlock);
	}

	virtual void* Alloc(size_t size)
	{
		if(size > m_sizeOfEachBlock)
			return ::malloc(size);
// 		EnterCriticalSection(&m_CS);
		if (m_numInitialized < m_numOfBlocks )
		{
			uint* p = (uint*)AddrFromIndex( m_numInitialized );
			*p = m_numInitialized + 1;
			m_numInitialized++;
		}
		void* ret = NULL;
		if ( m_numFreeBlocks > 0 )
		{
			ret = (void*)m_next;
			--m_numFreeBlocks;
			if (m_numFreeBlocks!=0)
			{
				m_next = AddrFromIndex( *((uint*)m_next) );
			}
			else
			{
				m_next = NULL;
			}
		}
// 		LeaveCriticalSection(&m_CS);
		return ret;
	}

	virtual void Free(void* ptr)
	{
		if(ptr < m_memStart || ptr > m_memEnd)
		{
			::free(ptr);
			return;
		}
// 		EnterCriticalSection(&m_CS);
		if (m_next != NULL)
		{
			(*(uint*)ptr) = IndexFromAddr( m_next );
			m_next = (uchar*)ptr;
		}
		else
		{
			*((uint*)ptr) = m_numOfBlocks;
			m_next = (uchar*)ptr;
		}
		++m_numFreeBlocks;
// 		LeaveCriticalSection(&m_CS);
	}
}; // End pool class


// this pool will constantly grow until it is reset (automatically when all allocs have been "freed")
class C4JMemoryPoolGrow : public C4JMemoryPool
{
	uint32_t	m_totalSize;
	uint32_t	m_memUsed;
	uint32_t	m_numAllocations;
	uint8_t*	m_pMemory;
	uint32_t	m_currentOffset;

public:
	C4JMemoryPoolGrow(uint32_t size = 64*1024)
	{
		size = Align(size, 4);
		m_totalSize = size;
		m_pMemory = new uint8_t[size];
		m_currentOffset = 0;
		m_memUsed = 0;
		m_numAllocations = 0;
	}

	virtual void* Alloc(size_t size)
	{
		size = Align(size, 4);								// 4 uint8_t align the memory
		assert((m_currentOffset + size) < m_totalSize);		// make sure we haven't ran out of space
		void* returnMem = &m_pMemory[m_currentOffset];		// grab the return memory
		m_currentOffset += size;							
		m_numAllocations++;
		return returnMem;
	}
	virtual void Free(void* ptr)
	{
		m_numAllocations--;
		if(m_numAllocations == 0)
			m_currentOffset = 0;		// reset the pool when we reach zero allocations
	}
};







