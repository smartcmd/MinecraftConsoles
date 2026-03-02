#pragma once

#include <assert.h>

//Note - this is meant to be a really simple wrapper round a pointer just to be able to add a length value to arrays.
// As such, it shouldn't delete its data in a destructor as shallow copies will be made of this and we don't want to
// free the data just because one of those has gone out of scope
template <class T> class arrayWithLength
{
public:
	T *data;
	unsigned int length;
	arrayWithLength() { data = NULL; length = 0; }
	arrayWithLength(unsigned int elements, bool bClearArray=true) { assert(elements!=0); data = new T[elements];  if(bClearArray){ memset( data,0,sizeof(T)*elements); }  this->length = elements; }

	// 4J Stu Added this ctor so I static init arrays in the Item derivation tree
	arrayWithLength( T data[], unsigned int elements) { this->data = data; this->length = elements; }

	//~arrayWithLength() { delete[] data; }

	void resize( unsigned int elements )
	{
		assert( elements > length );
		T *temp = new T[elements];
		memset( temp,0,sizeof(T)*elements);

		if( data != NULL )
		{
			std::copy( data, data+length, temp );

			delete[] data;
		}
		data = temp;
		length = elements;
	}

	T& operator[](unsigned int i) { return data[i]; }
	T  operator[](unsigned int i) const { return data[i]; }
};

// TODO 4J Stu - This looks right, but is it?
template <class T> class array2DWithLength
{
	typedef arrayWithLength< T >* _parrayWithLength;
public:
	_parrayWithLength *data;
	unsigned int length;
	array2DWithLength() { data = NULL; length = 0; }
	array2DWithLength(unsigned int dimA, unsigned int dimB)
	{
		data = new _parrayWithLength[dimA];
		this->length = dimA;

		for( unsigned int i = 0; i < length; i++ )
			data[i] = new arrayWithLength<T>(dimB);
	}

	_parrayWithLength& operator[](unsigned int i) { return data[i]; }
	_parrayWithLength  operator[](unsigned int i) const { return data[i]; }
};


class Biome;
class LevelChunk;
class Node;
class Item;
class Tile;
class Stat;
class MobCategory;
class File;
class Vertex;
class _Polygon;		// Renaming as have conflict with Windows Polygon fn
class ServerLevel;
class MultiPlayerLevel;
class Level;
class LevelRenderer;
class WeighedRandomItem;
class WeighedTreasure;
class Layer;
//class Cube;
class ModelPart;
class Enchantment;
class ClipChunk;

typedef arrayWithLength<double> doubleArray;
typedef arrayWithLength<byte> byteArray;
typedef arrayWithLength<char> charArray;
typedef arrayWithLength<short> shortArray;
typedef arrayWithLength<int> intArray;
typedef arrayWithLength<float> floatArray;
typedef arrayWithLength<Biome *> BiomeArray;
typedef arrayWithLength<LevelChunk *> LevelChunkArray;
typedef array2DWithLength<LevelChunk *> LevelChunk2DArray;
typedef arrayWithLength<Node *> NodeArray;
typedef arrayWithLength<Item *> ItemArray;
typedef arrayWithLength<Tile *> TileArray;
typedef arrayWithLength<Stat *> StatArray;
typedef arrayWithLength<MobCategory *> MobCategoryArray;
typedef arrayWithLength<File *> FileArray;
typedef arrayWithLength<Vertex *> VertexArray;
typedef arrayWithLength<_Polygon *> PolygonArray;
typedef arrayWithLength<ServerLevel *> ServerLevelArray;
typedef arrayWithLength<MultiPlayerLevel *> MultiPlayerLevelArray;
typedef arrayWithLength<Level *> LevelArray;
typedef arrayWithLength<LevelRenderer *> LevelRendererArray;
typedef arrayWithLength<WeighedRandomItem *> WeighedRandomItemArray;
typedef arrayWithLength<WeighedTreasure *> WeighedTreasureArray;
typedef arrayWithLength< shared_ptr<Layer> > LayerArray;
//typedef arrayWithLength<Cube *> CubeArray;
typedef arrayWithLength<ModelPart *> ModelPartArray;
typedef arrayWithLength<Enchantment *> EnchantmentArray;
typedef arrayWithLength<ClipChunk> ClipChunkArray;

#include "ItemInstance.h"
typedef arrayWithLength<shared_ptr<ItemInstance> > ItemInstanceArray;
