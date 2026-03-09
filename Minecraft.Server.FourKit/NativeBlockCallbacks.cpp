#include "NativeBlockCallbacks.h"

void NativeBlockCallbacks::BreakNaturally(int x, int y, int z, int dimension)
{
	NativeCallback_BlockBreakNaturally(x, y, z, dimension);
}

int NativeBlockCallbacks::GetBlockType(int x, int y, int z, int dimension)
{
	return NativeCallback_GetBlockType(x, y, z, dimension);
}

void NativeBlockCallbacks::SetBlockType(int x, int y, int z, int dimension, int id)
{
	NativeCallback_SetBlockType(x, y, z, dimension, id);
}

int NativeBlockCallbacks::GetBlockData(int x, int y, int z, int dimension)
{
	return NativeCallback_GetBlockData(x, y, z, dimension);
}

void NativeBlockCallbacks::SetBlockData(int x, int y, int z, int dimension, int data)
{
	NativeCallback_SetBlockData(x, y, z, dimension, data);
}
