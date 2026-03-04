// =================================================================================================================================
// This sample demonstrates how to use the Server API to send allocs, reallocs and frees directly to the client. It also shows 
// how to configure multiple heaps.
// =================================================================================================================================

#include "../../Server/HeapInspectorServer.h"
#include <stdlib.h>

using namespace HeapInspectorServer;

void Wait(int a_MilliSeconds);

std::vector<HeapInfo> GetHeapInfo()
{
	std::vector<HeapInfo> result;
	HeapInfo heapInfo;
	heapInfo.m_Description = "Heap1";
	heapInfo.m_Range.m_Min = 0;
	heapInfo.m_Range.m_Max = 0x80000000 - 1;
	result.push_back(heapInfo);

	heapInfo.m_Description = "Heap2";
	heapInfo.m_Range.m_Min = 0;
	heapInfo.m_Range.m_Max = 0x80000000 - 1;
	result.push_back(heapInfo);

	return result;
}

void* Alloc(uint32 a_Size, int a_Heap)
{
	Mutation mutation = BeginAlloc();
	void* mem = malloc(a_Size);
	EndAlloc(mutation, a_Heap, mem, a_Size, a_Size);
	return mem;
}

void Free(void* a_Block, int a_Heap)
{
	Mutation mutation = BeginFree();
	free(a_Block);
	EndFree(mutation, a_Heap, a_Block);
}

void* ReAlloc(void* a_OldBlock, uint32 a_Size, int a_Heap)
{
	Mutation mutation = BeginReAlloc();
	void* mem = realloc(a_OldBlock, a_Size);
	EndReAlloc(mutation, a_Heap, a_OldBlock, mem, a_Size, a_Size);
	return mem;
}

void RunHeapInspectorServer()
{
	Initialise(GetHeapInfo(), 3000, WaitForConnection_Enabled);

	while (1)
	{
		void* mem1;
		void* memB;
		
		mem1 = Alloc(16, 0);
		memB = Alloc(1024, 1);

		Wait(100);

		void* mem2 = ReAlloc(mem1, 32, 0);

		Wait(100);

		Free(mem2, 0);
		Free(memB, 1);

		Wait(100);
	}

	Shutdown();
}
