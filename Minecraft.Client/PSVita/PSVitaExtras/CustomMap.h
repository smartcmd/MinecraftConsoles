#ifndef CustomMap_H
#define CustomMap_H
// AP - This replaces the std::unordered_map used in MobSpawner.h
// The problem with the original system is that it calls malloc for every insert it does. Not only is that expensive in itself but it also
// clashes with any other mallocs on other threads (specifically the large amount of mallocing being done in Level.h for the std::unordered_set)
// causing huge stalls.
// This isn't really a univeral replacement for std::unordered_map and is quite specific to MobSpawner.h

#include "../../../Minecraft.World/ChunkPos.h"

typedef struct SCustomMapNode
{
	unsigned int Hash;
	ChunkPos first;
	bool second;
	struct SCustomMapNode *Next;
} SCustomMapNode;

class CustomMap
{
private:
	SCustomMapNode **m_NodePool;
	int m_NodePoolSize;
	int m_NodePoolIndex;

	int m_HashSize;
	SCustomMapNode **m_HashTable;

public:
	CustomMap();
	~CustomMap();

	void clear();
	SCustomMapNode* find(const ChunkPos &Key);
	int end();
	SCustomMapNode* get(int index);
	void insert(const ChunkPos &Key, bool Value);

private:
	void resize();
};

#endif // CustomMap_H