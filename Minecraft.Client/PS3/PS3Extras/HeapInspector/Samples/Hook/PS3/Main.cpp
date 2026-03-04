#include "../../../Server/HeapInspectorServer.h"
#include "../../../Server/PS3/HeapHooks.hpp"

void RunHeapInspectorServer();

extern "C" void* __real__malloc_init();
extern "C" void* __wrap__malloc_init()
{
	void* result = __real__malloc_init();
	Initialise(HeapInspectorServer::GetDefaultHeapInfo(), 3000, HeapInspectorServer::WaitForConnection_Enabled);
	return result;
}

int main(int /*a_ArgC*/, const char* /*a_ArgV[]*/)
{
	RunHeapInspectorServer();
	HeapInspectorServer::Shutdown();
}
