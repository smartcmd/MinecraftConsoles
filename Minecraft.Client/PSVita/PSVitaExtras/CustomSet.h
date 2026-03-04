#ifndef CustomSet_H
#define CustomSet_H
// AP - This replaces the std::unordered_set used in Level.h
// The problem with the original system is that it calls malloc for every insert it does. Not only is that expensive in itself but it also
// clashes with any other mallocs on other threads (specifically the large amount of mallocing being done in MobSpawner for the std::unordered_map)
// causing huge stalls.
// This isn't really a univeral replacement for std::unordered_set and is quite specific to Level.h

#include "../../../Minecraft.World/ChunkPos.h"

typedef struct SCustomSetNode
{
	unsigned int Hash;
	ChunkPos key;
	struct SCustomSetNode *Next;
} SCustomSetNode;

class CustomSet
{
private:
	SCustomSetNode **m_NodePool;
	int m_NodePoolSize;
	int m_NodePoolIndex;

	int m_HashSize;
	SCustomSetNode **m_HashTable;

public:
	CustomSet();
	~CustomSet();

	void clear();
	SCustomSetNode* find(const ChunkPos &Key);
	int end();
	ChunkPos get(int index);
	void insert(const ChunkPos &Key);

private:
	void resize();
};

#endif // CustomSet_H