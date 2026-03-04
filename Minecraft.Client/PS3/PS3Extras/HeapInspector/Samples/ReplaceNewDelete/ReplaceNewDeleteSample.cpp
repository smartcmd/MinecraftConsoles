// =================================================================================================================================
// This sample demonstrates how to replace operator new and delete and how to send these allocations to the HeapInspector client.
// Please note that the Hook sample captures allocations on a lower level and will also trace all new/delete allocations. 
//
// WARNING: Make sure that you replace both the array and non-array operators. If there are any pairing issues in your code 
// (allocating with new[] and deleting with delete), HeapInspector will miss those deallocations and that will be problematic 
// for a subsequent allocation on that address: it will then warn that it found a double allocation. That will actually be a 
// sign that the operators aren't properly matched.
// =================================================================================================================================

#include "../../Server/HeapInspectorServer.h"
#include <stdlib.h>
#include <new>

using namespace HeapInspectorServer;

void Wait(int a_MilliSeconds);

void* operator new(size_t a_Size)
{
	Mutation mutation = BeginAlloc();
	void* mem = malloc(a_Size);
	EndAlloc(mutation, 0, mem, a_Size, a_Size);
	return mem;
}

void operator delete(void* a_Pointer)
{
	Mutation mutation = BeginFree();
	free(a_Pointer);
	EndFree(mutation, 0, a_Pointer);
}

void* operator new[](size_t a_Size)
{
	Mutation mutation = BeginAlloc();
	void* mem = malloc(a_Size);
	EndAlloc(mutation, 0, mem, a_Size, a_Size);
	return mem;
}

void operator delete[](void* a_Pointer)
{
	Mutation mutation = BeginFree();
	free(a_Pointer);
	EndFree(mutation, 0, a_Pointer);
}

void RunHeapInspectorServer()
{
	Initialise(GetDefaultHeapInfo(), 3000, WaitForConnection_Enabled);

	while (1)
	{
		int* xArray = new int[100];
		float* y = new float;

		Wait(100);

		delete[] xArray;
		delete y;

		Wait(100);
	}

	Shutdown();
}
