#include "stdafx.h"
#include "SparseLightStorage.h"

// Note: See header for an overview of this class

int SparseLightStorage::deleteQueueIndex;
XLockFreeStack <unsigned char> SparseLightStorage::deleteQueue[3];

void SparseLightStorage::staticCtor()
{
	for( int i = 0; i < 3; i++ )
	{
		deleteQueue[i].Initialize();
	}
}

// Initialise lighting storage, with very limited compression - the very first plane is stored as either compressed to be "all 15" or "all 0" depending on whether this
// will store sky or not, and the rest of the planes aren't compressed at all. The reason behind this is to keep the total allocation as a round number of 4K (small) pages, ie 16K.
// By doing this, and doing this "special" allocation as a XPhysicalAlloc rather than a malloc, we can help ensure that this full allocation gets cleaned up properly when the first
// proper compression is done on this storage. If it were just allocated with malloc, then the memory management system would have a large number of 16512 allocations to free, and
// it seems from experimentation that these basically don't make it back to the system as free pages.
// Note - the other approach here would be to allocate *no* actual storage for the lights at the ctor stage. However, as chunks are created then this creates an awful lot of intermediate
// stages as each line of lighting is added, so it is actually much cleaner to just allocate almost fully here & then attempt to do a single compression pass over the data later on.
SparseLightStorage::SparseLightStorage(bool sky)
{
	// Allocate using physical alloc. As this will (by default) return memory from the pool of 4KB pages, the address will in the range of MM_PHYSICAL_4KB_BASE upwards. We can use
	// this fact to identify the allocation later, and so free it with the corresponding call to XPhysicalFree.
#ifdef _XBOX
	unsigned char *planeIndices = (unsigned char *)XPhysicalAlloc(128 * 128, MAXULONG_PTR, 4096, PAGE_READWRITE);
#else
	unsigned char *planeIndices = (unsigned char *)malloc(128 * 128);
#endif
	unsigned char *data = planeIndices + 128;
	planeIndices[127] = sky ? ALL_15_INDEX : ALL_0_INDEX;
	for( int i = 0; i < 127; i++ )
	{
		planeIndices[i] = i;
	}
	XMemSet(data, 0, 128 * 127);

	// Data and count packs together the pointer to our data and the count of planes allocated - 127 planes allocated in this case
#pragma warning ( disable : 4826 )
	dataAndCount = 0x007F000000000000L | (( (__int64) planeIndices ) & 0x0000ffffffffffffL);
#pragma warning ( default : 4826 )
#ifdef LIGHT_COMPRESSION_STATS
	count = 127;
#endif
}

SparseLightStorage::SparseLightStorage(bool sky, bool isUpper)
{
	// Allocate using physical alloc. As this will (by default) return memory from the pool of 4KB pages, the address will in the range of MM_PHYSICAL_4KB_BASE upwards. We can use
	// this fact to identify the allocation later, and so free it with the corresponding call to XPhysicalFree.
	unsigned char *planeIndices = (unsigned char *)malloc(128);
	for( int i = 0; i < 128; i++ )
	{
		planeIndices[i] = sky ? ALL_15_INDEX : ALL_0_INDEX;
	}

	// Data and count packs together the pointer to our data and the count of planes allocated - 0 planes allocated in this case
#pragma warning ( disable : 4826 )
	dataAndCount = 0x0000000000000000L | (( (__int64) planeIndices ) & 0x0000ffffffffffffL);
#pragma warning ( default : 4826 )
#ifdef LIGHT_COMPRESSION_STATS
	count = 0;
#endif
}

SparseLightStorage::~SparseLightStorage()
{
	unsigned char *indicesAndData = (unsigned char *)(dataAndCount & 0x0000ffffffffffff);
	// Determine correct means to free this data - could have been allocated either with XPhysicalAlloc or malloc
	
#ifdef _XBOX
	if( (unsigned int)indicesAndData >= MM_PHYSICAL_4KB_BASE )
	{
		XPhysicalFree(indicesAndData);
	}
	else
#endif
	{
		free(indicesAndData);
	}
//	printf("Free (in dtor) 0x%x\n", indicesAndData);
}

SparseLightStorage::SparseLightStorage(SparseLightStorage *copyFrom)
{
	// Extra details of source storage
	__int64 sourceDataAndCount = copyFrom->dataAndCount;
	unsigned char *sourceIndicesAndData = (unsigned char *)(sourceDataAndCount & 0x0000ffffffffffff);
	int sourceCount = (sourceDataAndCount >> 48 ) & 0xffff;

	// Allocate & copy indices ( 128 bytes ) and any allocated planes (128 * count)
	unsigned char *destIndicesAndData = (unsigned char *)malloc( sourceCount * 128 + 128 );

	// AP - I've moved this to be before the memcpy because of a very strange bug on vita. Sometimes dataAndCount wasn't valid in time when ::get was called.
	// This should never happen and this isn't a proper solution but fixes it for now.
#pragma warning ( disable : 4826 )
	dataAndCount = ( sourceDataAndCount & 0xffff000000000000L ) | ( ((__int64) destIndicesAndData ) & 0x0000ffffffffffffL );
#pragma warning ( default : 4826 )

	XMemCpy( destIndicesAndData, sourceIndicesAndData, sourceCount * 128 + 128 );

#ifdef LIGHT_COMPRESSION_STATS
	count = sourceCount;
#endif
}

// Set all lighting values from a data array of length 16384 (128 x 16 x 16 x 0.5). Source data must have same order as original java game
void SparseLightStorage::setData(byteArray dataIn, unsigned int inOffset)
{
	//  Original order is defined as:
	//  pos = (x << 11 | z << 7 | y);
	//  slot = pos >> 1;
	//  part = pos & 1;
	//  if ( part == 0 ) value = data[slot] & 0xf
	//  else value = (data[slot] >> 4) & 0xf

	// Two passed through the data. First pass sets up plane indices, and counts number of planes that we actually need to allocate
	int allocatedPlaneCount = 0;
	unsigned char _planeIndices[128];

	for( int y = 0; y < 128; y++ )
	{
		bool all0 = true;
		bool all15 = true;
			
		for( int xz = 0; xz < 256; xz++ )	// 256 in loop as 16 x 16 separate bytes need checked
		{
			int pos = ( xz << 7 ) | y;
			int slot = pos >> 1;
			int part = pos & 1;
			unsigned char value = ( dataIn[slot + inOffset] >> (part * 4) ) & 15;
			if( value != 0 ) all0 = false;
			if( value != 15 ) all15 = false;
		}
		if( all0 )
		{
			_planeIndices[y] = ALL_0_INDEX;
		}
		else if( all15 )
		{
			_planeIndices[y] = ALL_15_INDEX;
		}
		else
		{
			_planeIndices[y] = allocatedPlaneCount++;
		}
	}

	// Allocate required storage
	unsigned char *planeIndices = (unsigned char *)malloc(128 * allocatedPlaneCount + 128);
	unsigned char *data = planeIndices + 128;
	XMemCpy(planeIndices, _planeIndices, 128);

	// Second pass through to actually copy the data in to the storage allocated for the required planes
	unsigned char *pucOut = data;
	for( int y = 0; y < 128 ; y++ )
	{
		// Index will be < 128 if we allocated storage for it and it has a valid index. No need to actually check the index as
		// we know they were sequentially allocated above.
		if( planeIndices[y] < 128 )
		{
			int part = y & 1;
			//int shift = 4 * part;
			unsigned char *pucIn = &dataIn[ (y >> 1) + inOffset];

			for( int xz = 0; xz < 128; xz++ )	// 128 ( 16 x 16 x 0.5 ) in loop as packing 2 values into each destination byte
			{
				*pucOut = ( ( *pucIn ) >> ( part * 4 ) ) & 15;
				pucIn += 64;

				*pucOut |= ( ( ( *pucIn ) >> ( part * 4 ) ) & 15 ) << 4;
				pucIn += 64;
				pucOut++;
			}
		}
	}

	// Get new data and count packed info
#pragma warning ( disable : 4826 )
	__int64 newDataAndCount = ((__int64) planeIndices) & 0x0000ffffffffffffL;
#pragma warning ( default : 4826 )
	newDataAndCount |= ((__int64)allocatedPlaneCount) << 48;

	updateDataAndCount( newDataAndCount );
}

// Gets all lighting values into an array of length 16384. Destination data will have same order as original java game.
void SparseLightStorage::getData(byteArray retArray, unsigned int retOffset)
{
	XMemSet(retArray.data + retOffset, 0, 16384);
	unsigned char *planeIndices, *data;
	getPlaneIndicesAndData(&planeIndices, &data);

	//  Original order is defined as:
	//  pos = (x << 11 | z << 7 | y);
	//  slot = pos >> 1;
	//  part = pos & 1;
	//  if ( part == 0 ) value = data[slot] & 0xf
	//  else value = (data[slot] >> 4) & 0xf

	for( int y = 0; y < 128; y++ )
	{
		if( planeIndices[y] == ALL_0_INDEX )
		{
			// No need to do anything in this case as retArray is initialised to zero
		}
		else if( planeIndices[y] == ALL_15_INDEX )
		{
			int part = y & 1;
			unsigned char value = 15 << ( part * 4 );
			unsigned char *pucOut = &retArray.data[ (y >> 1) + retOffset];
			for( int xz = 0; xz < 256; xz++ )
			{
				*pucOut |= value;
				pucOut += 64;
			}
		}
		else
		{
			int part = y & 1;
			int shift = 4 * part;
			unsigned char *pucOut = &retArray.data[ (y >> 1) + retOffset];
			unsigned char *pucIn = &data[ planeIndices[ y ] * 128 ];
			for( int xz = 0; xz < 128; xz++ )		// 128 in loop (16 x 16 x 0.5) as input data is being treated in pairs of nybbles that are packed in the same byte
			{
				unsigned char value = (*pucIn) & 15;
				*pucOut |= ( value << shift );
				pucOut += 64;

				value = ((*pucIn) >> 4 ) & 15;
				*pucOut |= ( value << shift );
				pucOut += 64;

				pucIn++;
			}
		}
	}
}

// Get an individual lighting value
int SparseLightStorage::get(int x, int y, int z)
{
	unsigned char *planeIndices, *data;
	getPlaneIndicesAndData(&planeIndices, &data);
	
	if( planeIndices[y] == ALL_0_INDEX )
	{
		return 0;
	}
	else if ( planeIndices[y] == ALL_15_INDEX )
	{
		return 15;
	}
	else
	{
		int planeIndex = x * 16 + z;				// Index within this xz plane
		int byteIndex = planeIndex / 2;				// Byte index within the plane (2 tiles stored per byte)
		int shift = ( planeIndex & 1 ) * 4;			// Bit shift within the byte
		int retval = ( data[ planeIndices[y] * 128 + byteIndex ] >> shift ) & 15;

		return retval;
	}
}

// Set an individual lighting value
void SparseLightStorage::set(int x, int y, int z, int val)
{
	unsigned char *planeIndices, *data;
	getPlaneIndicesAndData(&planeIndices, &data);

	// If this plane isn't yet allocated, then we might have some extra work to do
	if( planeIndices[y] >= ALL_0_INDEX )
	{
		// No data allocated. Early out though if we are storing what is already represented by our special index.
		if( ( val == 0 ) && ( planeIndices[y] == ALL_0_INDEX ) )
		{
			return;
		}
		if( ( val == 15 ) && ( planeIndices[y] == ALL_15_INDEX ) )
		{
			return;
		}

		// Reallocate the storage for planes to accomodate one extra
		addNewPlane(y);

		// Get pointers again as these may have moved
		getPlaneIndicesAndData(&planeIndices, &data);
	}

	// Either data was already allocated, or we've just done that. Now store our value into the right place.

	int planeIndex = x * 16 + z;				// Index within this xz plane
	int byteIndex = planeIndex / 2;				// Byte index within the plane (2 tiles stored per byte)
	int shift = ( planeIndex & 1 ) * 4;			// Bit shift within the byte
	int mask = 0xf0 >> shift;

	int idx = planeIndices[y] * 128 + byteIndex;
	data[idx] = ( data[idx] & mask ) | ( val << shift );

}

void SparseLightStorage::setAllBright()
{
	unsigned char *planeIndices = (unsigned char *)malloc(128);
	for( int i = 0; i < 128; i++ )
	{
		planeIndices[i] = ALL_15_INDEX;
	}
	// Data and count packs together the pointer to our data and the count of planes allocated, which is currently zero
#pragma warning ( disable : 4826 )
	__int64 newDataAndCount = ( (__int64) planeIndices ) & 0x0000ffffffffffffL;
#pragma warning ( default : 4826 )

	updateDataAndCount( newDataAndCount );
}

// Sets a region of lighting values with the data at offset position in the array dataIn - external ordering compatible with java DataLayer
// Note - when data was extracted from the original data layers by LevelChunk::getBlocksAndData, y0 had to have even alignment and y1 - y0 also
// needed to be even as data was packed in nyblles in this dimension, and the code didn't make any attempt to unpack it. This behaviour is copied
// here for compatibility even though our source data isn't packed this way.
// Returns size of data copied.
int SparseLightStorage::setDataRegion(byteArray dataIn, int x0, int y0, int z0, int x1, int y1, int z1, int offset)
{
	// Actual setting of data happens when calling set method so no need to lock here
	unsigned char *pucIn = &dataIn.data[offset];
	for( int x = x0; x < x1; x++ )
	{
		for( int z = z0; z < z1; z++ )
		{
			// Emulate how data was extracted from DataLayer... see comment above
			int yy0 = y0 & 0xfffffffe;
			int len = ( y1 - y0 ) / 2;
			for( int i = 0; i < len; i++ )
			{
				int y = yy0 + ( i * 2 );

				set(x, y, z, (*pucIn) & 15 );
				set(x, y + 1, z, ((*pucIn) >> 4 ) & 15 );
				pucIn++;
			}
		}
	}
	ptrdiff_t count = pucIn - &dataIn.data[offset];

	return (int)count;
}

// Updates the data at offset position dataInOut with a region of lighting information - external ordering compatible with java DataLayer
// Note - when data was placed in the original data layers by LevelChunk::setBlocksAndData, y0 had to have even alignment and y1 - y0 also
// needed to be even as data was packed in nyblles in this dimension, and the code didn't make any attempt to unpack it. This behaviour is copied
// here for compatibility even though our source data isn't packed this way
// Returns size of data copied.
int SparseLightStorage::getDataRegion(byteArray dataInOut, int x0, int y0, int z0, int x1, int y1, int z1, int offset)
{
	unsigned char *pucOut = &dataInOut.data[offset];
	for( int x = x0; x < x1; x++ )
	{
		for( int z = z0; z < z1; z++ )
		{
			// Emulate how data was extracted from DataLayer... see comment above
			int yy0 = y0 & 0xfffffffe;
			int len = ( y1 - y0 ) / 2;
			for( int i = 0; i < len; i++ )
			{
				int y = yy0 + ( i * 2 );

				*pucOut = get( x, y, z);
				*pucOut |= get( x, y + 1, z) << 4;
				pucOut++;
			}
		}
	}
	ptrdiff_t count = pucOut - &dataInOut.data[offset];
	
	return (int)count;
}

void SparseLightStorage::addNewPlane(int y)
{
	bool success = false;
	do
	{
		// Get last packed data pointer & count
		__int64 lastDataAndCount = dataAndCount;

		// Unpack count & data pointer
		int lastLinesUsed = (int)(( lastDataAndCount >> 48 ) & 0xffff);
		unsigned char *lastDataPointer = (unsigned char *)(lastDataAndCount & 0x0000ffffffffffff);

		// Find out what to prefill the newly allocated line with
		unsigned char planeIndex = lastDataPointer[y];
		int prefill = 0;
		if( planeIndex < ALL_0_INDEX ) return;			// Something has already allocated this line - we're done
		else if( planeIndex == ALL_15_INDEX ) prefill = 255;

		int linesUsed = lastLinesUsed + 1;

		// Allocate new memory storage, copy over anything from old storage, and initialise remainder
		unsigned char *dataPointer = (unsigned char *)malloc(linesUsed * 128 + 128);
		XMemCpy( dataPointer, lastDataPointer, 128 * lastLinesUsed + 128);
		XMemSet( dataPointer + ( 128 * lastLinesUsed ) + 128, prefill, 128 );
		dataPointer[y] = lastLinesUsed;

		// Get new data and count packed info
#pragma warning ( disable : 4826 )
		__int64 newDataAndCount = ((__int64) dataPointer) & 0x0000ffffffffffffL;
#pragma warning ( default : 4826 )
		newDataAndCount |= ((__int64)linesUsed) << 48;

		// Attempt to update the data & count atomically. This command will Only succeed if the data stored at
		// dataAndCount is equal to lastDataAndCount, and will return the value present just before the write took place
		__int64 lastDataAndCount2 = InterlockedCompareExchangeRelease64( &dataAndCount, newDataAndCount, lastDataAndCount );
		
		if( lastDataAndCount2 == lastDataAndCount )
		{
			success = true;
			// Queue old data to be deleted
			queueForDelete( lastDataPointer );
//			printf("Marking for delete 0x%x\n", lastDataPointer);
#ifdef LIGHT_COMPRESSION_STATS
			count = linesUsed;
#endif
		}
		else
		{
			// If we didn't succeed, queue data that we made to be deleted, and try again
			queueForDelete( dataPointer );
//			printf("Marking for delete (fail) 0x%x\n", dataPointer);
		}
	} while( !success );
}

void SparseLightStorage::getPlaneIndicesAndData(unsigned char **planeIndices, unsigned char **data)
{
	unsigned char *indicesAndData = (unsigned char *)(dataAndCount & 0x0000ffffffffffff);

	*planeIndices = indicesAndData;
	*data = indicesAndData + 128;

}

void SparseLightStorage::queueForDelete(unsigned char *data)
{
	// Add this into a queue for deleting. This shouldn't be actually deleted until tick has been called twice from when
	// the data went into the queue.
	deleteQueue[deleteQueueIndex].Push( data );
}

void SparseLightStorage::tick()
{
	// We have 3 queues for deleting. Always delete from the next one after where we are writing to, so it should take 2 ticks
	// before we ever delete something, from when the request to delete it came in
	int freeIndex = ( deleteQueueIndex + 1 ) % 3;

//	printf("Free queue: %d, %d\n",deleteQueue[freeIndex].GetEntryCount(),deleteQueue[freeIndex].GetAllocated());
	unsigned char *toFree = NULL;
	do
	{
		toFree = deleteQueue[freeIndex].Pop();
//		if( toFree ) printf("Deleting 0x%x\n", toFree);
		// Determine correct means to free this data - could have been allocated either with XPhysicalAlloc or malloc
#ifdef _XBOX
		if( (unsigned int)toFree >= MM_PHYSICAL_4KB_BASE )
		{
			XPhysicalFree(toFree);
		}
		else
#endif
		{
			free(toFree);
		}
	} while( toFree );

	deleteQueueIndex = ( deleteQueueIndex + 1 ) % 3;
}

// Update storage with a new values for dataAndCount, repeating as necessary if other simultaneous writes happen.
void SparseLightStorage::updateDataAndCount(__int64 newDataAndCount)
{
	// Now actually assign this data to the storage. Just repeat until successful, there isn't any useful really that we can merge the results of this
	// with any other simultaneous writes that might be happening.
	bool success = false;
	do
	{
		__int64 lastDataAndCount = dataAndCount;
		unsigned char *lastDataPointer = (unsigned char *)(lastDataAndCount & 0x0000ffffffffffff);

		// Attempt to update the data & count atomically. This command will Only succeed if the data stored at
		// dataAndCount is equal to lastDataAndCount, and will return the value present just before the write took place
		__int64 lastDataAndCount2 = InterlockedCompareExchangeRelease64( &dataAndCount, newDataAndCount, lastDataAndCount );
		
		if( lastDataAndCount2 == lastDataAndCount )
		{
			success = true;
			// Queue old data to be deleted
//			printf("Marking for delete 0x%x (full replace)\n", lastDataPointer);
			queueForDelete( lastDataPointer );
		}
	} while( !success);

#ifdef LIGHT_COMPRESSION_STATS
	count = ( newDataAndCount >> 48 ) & 0xffff;
#endif

}

// Attempt to compress the stored data. This method makes no guarantee of success - if it fails due to something else writing to the storage whilst this is running, then it won't actually do anything.
int SparseLightStorage::compress()
{
	unsigned char _planeIndices[128];
	bool needsCompressed = false;

	__int64 lastDataAndCount = dataAndCount;

	unsigned char *planeIndices = (unsigned char *)(lastDataAndCount & 0x0000ffffffffffff);
	unsigned char *data = planeIndices + 128;

	int planesToAlloc = 0;
	for( int i = 0; i < 128; i++ )
	{
		if( planeIndices[i] == ALL_0_INDEX )
		{
			_planeIndices[i] = ALL_0_INDEX;
		}
		else if( planeIndices[i] == ALL_15_INDEX )
		{
			_planeIndices[i] = ALL_15_INDEX;
		}
		else
		{
			unsigned char *pucData = &data[ 128 * planeIndices[i] ];
			bool all0 = true;
			bool all15 = true;
			for( int j = 0; j < 128; j++ )	// 16 x 16 x 4-bits
			{
				if( *pucData != 0 ) all0 = false;
				if( *pucData != 255 ) all15 = false;
				pucData++;
			}
			if( all0 )
			{
				_planeIndices[i] = ALL_0_INDEX;
				needsCompressed = true;
			}
			else if ( all15 )
			{
				_planeIndices[i] = ALL_15_INDEX;
				needsCompressed = true;
			}
			else
			{
				_planeIndices[i] = planesToAlloc++;
			}
		}
	}

	if( needsCompressed )
	{
		unsigned char *newIndicesAndData = (unsigned char *)malloc( 128 + 128 * planesToAlloc );
		unsigned char *pucData = newIndicesAndData + 128;
		XMemCpy( newIndicesAndData, _planeIndices, 128 );

		for( int i = 0; i < 128; i++ )
		{
			if( newIndicesAndData[i] < ALL_0_INDEX )
			{
				XMemCpy( pucData, &data[ 128 * planeIndices[i] ], 128 );
				pucData += 128;
			}
		}

		// Get new data and count packed info
#pragma warning ( disable : 4826 )
		__int64 newDataAndCount = ((__int64) newIndicesAndData) & 0x0000ffffffffffffL;
#pragma warning ( default : 4826 )
		newDataAndCount |= ((__int64)planesToAlloc) << 48;

		// Attempt to update the data & count atomically. This command will Only succeed if the data stored at
		// dataAndCount is equal to lastDataAndCount, and will return the value present just before the write took place
		__int64 lastDataAndCount2 = InterlockedCompareExchangeRelease64( &dataAndCount, newDataAndCount, lastDataAndCount );

		if( lastDataAndCount2 != lastDataAndCount )
		{
			// Failed to write. Don't bother trying again... being very conservative here.
//			printf("Marking for delete 0x%x (compress fail)\n", newIndicesAndData);
			queueForDelete( newIndicesAndData );
		}
		else
		{
			// Success
			queueForDelete( planeIndices );
//			printf("Successfully compressed to %d planes, to delete 0x%x\n", planesToAlloc, planeIndices); 
#ifdef LIGHT_COMPRESSION_STATS
			count = planesToAlloc;
#endif
		}

		return planesToAlloc;
	}
	else
	{
		return (int)((lastDataAndCount >> 48 ) & 0xffff);
	}
}


bool SparseLightStorage::isCompressed()
{

	int	count = ( dataAndCount >> 48 ) & 0xffff;
	return (count < 127);


}

void SparseLightStorage::write(DataOutputStream *dos)
{
	int count = ( dataAndCount >> 48 ) & 0xffff;
	dos->writeInt(count);
	unsigned char *dataPointer = (unsigned char *)(dataAndCount & 0x0000ffffffffffff);
	byteArray wrapper(dataPointer, count * 128 + 128);
	dos->write(wrapper);
}

void SparseLightStorage::read(DataInputStream *dis)
{
	int count = dis->readInt();
	unsigned char *dataPointer = (unsigned char *)malloc(count * 128 + 128);
	byteArray wrapper(dataPointer, count * 128 + 128);
	dis->readFully(wrapper);

#pragma warning ( disable : 4826 )
	__int64 newDataAndCount = ((__int64) dataPointer) & 0x0000ffffffffffffL;
#pragma warning ( default : 4826 )
	newDataAndCount |= ((__int64)count) << 48;

	updateDataAndCount( newDataAndCount );
}
