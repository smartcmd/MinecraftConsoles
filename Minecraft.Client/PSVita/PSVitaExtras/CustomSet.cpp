#include "stdafx.h"
#include "CustomSet.h"


CustomSet::CustomSet()
{
	m_NodePool = nullptr;
	m_NodePoolSize = 0;
	m_NodePoolIndex = 0;

	m_HashSize = 1024;
	m_HashTable = static_cast<SCustomSetNode **>(malloc(m_HashSize * sizeof(SCustomSetNode)));

	clear();
}

CustomSet::~CustomSet()
{
	for( int i = 0;i < m_NodePoolSize; i += 1 )
	{
		free(m_NodePool[i]);
	}
	free(m_NodePool);

	free(m_HashTable);
}

void CustomSet::clear()
{
	// reset the pool index
	m_NodePoolIndex = 0;

	// clear the hash table
	memset(m_HashTable, 0, m_HashSize * sizeof(SCustomSetNode));
}

SCustomSetNode* CustomSet::find(const ChunkPos &Key)
{
	unsigned int Hash = (Key.x & 0x00000001f) | (Key.z << 5);	// hopefully this will produce a good hash for a 1024 entry table
	unsigned int Index = Hash & (m_HashSize-1);

	SCustomSetNode* Node = m_HashTable[Index];
	while( Node && Node->Hash != Hash )
	{
		Node = Node->Next;
	}

	return Node;
}

int CustomSet::end()
{
	return m_NodePoolIndex;
}

ChunkPos CustomSet::get(int index)
{
	return m_NodePool[index]->key;
}

void CustomSet::insert(const ChunkPos &Key)
{
	// see if this key already exists
	SCustomSetNode* Node = find(Key);

	if( !Node )
	{
		// do we have any space in the pool
		if( m_NodePoolIndex >= m_NodePoolSize )
		{
			resize();
		}

		// grab the next node from the pool
		Node = m_NodePool[m_NodePoolIndex];
		m_NodePoolIndex++;
	}
	else
	{
		return;
	}

	// create the new node;
	unsigned int Hash = (Key.x & 0x00000001f) | (Key.z << 5);	// hopefully this will produce a good hash for a 1024 entry table
	unsigned int Index = Hash & (m_HashSize-1);
	Node->Hash = Hash;
	Node->key = Key;
	Node->Next = nullptr;

	// are any nodes in this hash index
	if( !m_HashTable[Index] )
	{
		m_HashTable[Index] = Node;
	}
	else
	{
		// loop to the last node in the hash list
		SCustomSetNode* OldNode = m_HashTable[Index];
		while( OldNode->Next )
		{
			OldNode = OldNode->Next;
		}

		// link the old last node to the new one
		OldNode->Next = Node;
	}
}

void CustomSet::resize()
{
	int OldPoolSize = m_NodePoolSize;
	m_NodePoolSize += 512;
	SCustomSetNode **NodePool;
	if( m_NodePool )
	{
		NodePool = static_cast<SCustomSetNode **>(realloc(m_NodePool, m_NodePoolSize * sizeof(SCustomSetNode)));
	}
	else
	{
		NodePool = static_cast<SCustomSetNode **>(malloc(m_NodePoolSize * sizeof(SCustomSetNode)));
	}

	for( int i = 0;i < m_NodePoolSize - OldPoolSize;i += 1 )
	{
		NodePool[i + OldPoolSize] = static_cast<SCustomSetNode *>(malloc(sizeof(SCustomSetNode)));
	}

	m_NodePool = NodePool;
}

