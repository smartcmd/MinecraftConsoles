#include "stdafx.h"
#include "SparseDataStorage.h"

// Note: See header for an overview of this class

int SparseDataStorage::deleteQueueIndex;
XLockFreeStack <unsigned char> SparseDataStorage::deleteQueue[3];

void SparseDataStorage::staticCtor()
{
	for( int i = 0; i < 3; i++ )
	{
		deleteQueue[i].Initialize();
	}
}

// Initialise data storage, with very limited compression - the very first plane is stored as either compressed to be "all 0", and the rest of the planes aren't compressed at all.
// The reason behind this is to keep the total allocation as a round number of 4K (small) pages, ie 16K.
// By doing this, and doing this "special" allocation as a XPhysicalAlloc rather than a malloc, we can help ensure that this full allocation gets cleaned up properly when the first
// proper compression is done on this storage. If it were just allocated with malloc, then the memory management system would have a large number of 16512 allocations to free, and
// it seems from experimentation that these basically don't make it back to the system as free pages.
// Note - the other approach here would be to allocate *no* actual storage for the data at the ctor stage. However, as chunks are created then this creates an awful lot of intermediate
// stages as each line of data is added, so it is actually much cleaner to just allocate almost fully here & then attempt to do a single compression pass over the data later on.
SparseDataStorage::SparseDataStorage()
{
	// Allocate using physical alloc. As this will (by default) return memory from the pool of 4KB pages, the address will in the range of MM_PHYSICAL_4KB_BASE upwards. We can use
	// this fact to identify the allocation later, and so free it with the corresponding call to XPhysicalFree.
#ifdef _XBOX
	unsigned char *planeIndices = (unsigned char *)XPhysicalAlloc(128 * 128, MAXULONG_PTR, 4096, PAGE_READWRITE);
#else
	unsigned char *planeIndices = static_cast<unsigned char *>(malloc(128 * 128));
#endif
	unsigned char *data = planeIndices + 128;
	planeIndices[0] = ALL_0_INDEX;
	for( int i = 1; i < 128; i++ )
	{
		planeIndices[i] = i - 1;
	}
	XMemSet(data, 0, 128 * 127);

	// Data and count packs together the pointer to our data and the count of planes allocated - 127 planes allocated in this case
#pragma warning ( disable : 4826 )
	dataAndCount = 0x007F000000000000L | (( (int64_t) planeIndices ) & 0x0000ffffffffffffL);
#pragma warning ( default : 4826 )
#ifdef DATA_COMPRESSION_STATS
	count = 128;
#endif
}

SparseDataStorage::SparseDataStorage(bool isUpper)
{
	// Allocate using physical alloc. As this will (by default) return memory from the pool of 4KB pages, the address will in the range of MM_PHYSICAL_4KB_BASE upwards. We can use
	// this fact to identify the allocation later, and so free it with the corresponding call to XPhysicalFree.
	unsigned char *planeIndices = static_cast<unsigned char *>(malloc(128));
	for( int i = 0; i < 128; i++ )
	{
		planeIndices[i] = ALL_0_INDEX;
	}

	// Data and count packs together the pointer to our data and the count of planes allocated - 127 planes allocated in this case
#pragma warning ( disable : 4826 )
	dataAndCount = 0x0000000000000000L | (( (int64_t) planeIndices ) & 0x0000ffffffffffffL);
#pragma warning ( default : 4826 )
#ifdef DATA_COMPRESSION_STATS
	count = 128;
#endif
}

SparseDataStorage::~SparseDataStorage()
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

SparseDataStorage::SparseDataStorage(SparseDataStorage *copyFrom)
{
	// Extra details of source storage
	const int64_t sourceDataAndCount = copyFrom->dataAndCount;
	const unsigned char *sourceIndicesAndData = (unsigned char *)(sourceDataAndCount & 0x0000ffffffffffff);
	int sourceCount = (sourceDataAndCount >> 48 ) & 0xffff;

	// Allocate & copy indices ( 128 bytes ) and any allocated planes (128 * count)
	unsigned char *destIndicesAndData = static_cast<unsigned char *>(malloc(sourceCount * 128 + 128));

	// AP - I've moved this to be before the memcpy because of a very strange bug on vita. Sometimes dataAndCount wasn't valid in time when ::get was called.
	// This should never happen and this isn't a proper solution but fixes it for now.
#pragma warning ( disable : 4826 )
	dataAndCount = ( sourceDataAndCount & 0xffff000000000000L ) | ( ((int64_t) destIndicesAndData ) & 0x0000ffffffffffffL );
#pragma warning ( default : 4826 )

	XMemCpy( destIndicesAndData, sourceIndicesAndData, sourceCount * 128 + 128 );

#ifdef DATA_COMPRESSION_STATS
	count = sourceCount;
#endif
}

// Set all data values from a data array of length 16384 (128 x 16 x 16 x 0.5). Source data must have same order as original java game
void SparseDataStorage::setData(byteArray dataIn, unsigned int inOffset)
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

	//unsigned char *lastDataPointer = (unsigned char *)(dataAndCount & 0x0000ffffffffffff);

	for( int y = 0; y < 128; y++ )
	{
		bool all0 = true;

		for( int xz = 0; xz < 256; xz++ )	// 256 in loop as 16 x 16 separate bytes need checked
		{
			const int pos = ( xz << 7 ) | y;
			const int slot = pos >> 1;
			const int part = pos & 1;
			const unsigned char value = ( dataIn[slot + inOffset] >> (part * 4) ) & 15;
			if( value != 0 ) all0 = false;
		}
		if( all0 )
		{
			_planeIndices[y] = ALL_0_INDEX;
		}
		else
		{
			_planeIndices[y] = allocatedPlaneCount++;
		}
	}

	// Allocate required storage
	unsigned char *planeIndices = static_cast<unsigned char *>(malloc(128 * allocatedPlaneCount + 128));
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
			const int part = y & 1;
			//int shift = 4 * part;
			const unsigned char *pucIn = &dataIn[ (y >> 1) + inOffset];

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
	int64_t newDataAndCount = reinterpret_cast<int64_t>(planeIndices) & 0x0000ffffffffffffL;
#pragma warning ( default : 4826 )
	newDataAndCount |= static_cast<int64_t>(allocatedPlaneCount) << 48;

	updateDataAndCount( newDataAndCount );
}

// Gets all data values into an array of length 16384. Destination data will have same order as original java game.
void SparseDataStorage::getData(byteArray retArray, unsigned int retOffset)
{
	XMemSet(retArray.data +  + retOffset, 0, 16384);
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
		else
		{
			const int part = y & 1;
			const int shift = 4 * part;
			unsigned char *pucOut = &retArray.data[ (y >> 1) +  + retOffset];
			const unsigned char *pucIn = &data[ planeIndices[ y ] * 128 ];
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

// Get an individual data value
int SparseDataStorage::get(int x, int y, int z)
{
	unsigned char *planeIndices, *data;
	getPlaneIndicesAndData(&planeIndices, &data);

	if( planeIndices[y] == ALL_0_INDEX )
	{
		return 0;
	}
	else
	{
		const int planeIndex = x * 16 + z;				// Index within this xz plane
		const int byteIndex = planeIndex / 2;				// Byte index within the plane (2 tiles stored per byte)
		const int shift = ( planeIndex & 1 ) * 4;			// Bit shift within the byte
		const int retval = ( data[ planeIndices[y] * 128 + byteIndex ] >> shift ) & 15;

		return retval;
	}
}

// Set an individual data value
void SparseDataStorage::set(int x, int y, int z, int val)
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

		// Reallocate the storage for planes to accomodate one extra
		addNewPlane(y);

		// Get pointers again as these may have moved
		getPlaneIndicesAndData(&planeIndices, &data);
	}

	// Either data was already allocated, or we've just done that. Now store our value into the right place.

	const int planeIndex = x * 16 + z;				// Index within this xz plane
	const int byteIndex = planeIndex / 2;				// Byte index within the plane (2 tiles stored per byte)
	const int shift = ( planeIndex & 1 ) * 4;			// Bit shift within the byte
	const int mask = 0xf0 >> shift;

	const int idx = planeIndices[y] * 128 + byteIndex;
	data[idx] = ( data[idx] & mask ) | ( val << shift );

}

// Sets a region of data values with the data at offset position in the array dataIn - external ordering compatible with java DataLayer
// Note - when data was extracted from the original data layers by LevelChunk::getBlocksAndData, y0 had to have even alignment and y1 - y0 also
// needed to be even as data was packed in nyblles in this dimension, and the code didn't make any attempt to unpack it. This behaviour is copied
// here for compatibility even though our source data isn't packed this way.
// Returns size of data copied.
int SparseDataStorage::setDataRegion(byteArray dataIn, int x0, int y0, int z0, int x1, int y1, int z1, int offset, tileUpdatedCallback callback, void *param, int yparam)
{
	// Actual setting of data happens when calling set method so no need to lock here
	const unsigned char *pucIn = &dataIn.data[offset];
	if( callback )
	{
		for( int x = x0; x < x1; x++ )
		{
			for( int z = z0; z < z1; z++ )
			{
				// Emulate how data was extracted from DataLayer... see comment above
				const int yy0 = y0 & 0xfffffffe;
				const int len = ( y1 - y0 ) / 2;
				for( int i = 0; i < len; i++ )
				{
					const int y = yy0 + ( i * 2 );

					int toSet = (*pucIn) & 15;
					if( get(x, y, z) != toSet )
					{
						set(x, y, z, toSet );
						callback(x, y, z, param, yparam);
					}
					toSet = ((*pucIn) >> 4 ) & 15;
					if( get(x, y + 1, z) != toSet )
					{
						set(x, y + 1, z, toSet );
						callback(x, y + 1, z, param, yparam);
					}
					pucIn++;
				}
			}
		}
	}
	else
	{
		for( int x = x0; x < x1; x++ )
		{
			for( int z = z0; z < z1; z++ )
			{
				// Emulate how data was extracted from DataLayer... see comment above
				const int yy0 = y0 & 0xfffffffe;
				const int len = ( y1 - y0 ) / 2;
				for( int i = 0; i < len; i++ )
				{
					const int y = yy0 + ( i * 2 );

					set(x, y, z, (*pucIn) & 15 );
					set(x, y + 1, z, ((*pucIn) >> 4 ) & 15 );
					pucIn++;
				}
			}
		}
	}
	const ptrdiff_t count = pucIn - &dataIn.data[offset];

	return static_cast<int>(count);
}

// Updates the data at offset position dataInOut with a region of data information - external ordering compatible with java DataLayer
// Note - when data was placed in the original data layers by LevelChunk::setBlocksAndData, y0 had to have even alignment and y1 - y0 also
// needed to be even as data was packed in nyblles in this dimension, and the code didn't make any attempt to unpack it. This behaviour is copied
// here for compatibility even though our source data isn't packed this way
// Returns size of data copied.
int SparseDataStorage::getDataRegion(byteArray dataInOut, int x0, int y0, int z0, int x1, int y1, int z1, int offset)
{
	unsigned char *pucOut = &dataInOut.data[offset];
	for( int x = x0; x < x1; x++ )
	{
		for( int z = z0; z < z1; z++ )
		{
			// Emulate how data was extracted from DataLayer... see comment above
			const int yy0 = y0 & 0xfffffffe;
			const int len = ( y1 - y0 ) / 2;
			for( int i = 0; i < len; i++ )
			{
				const int y = yy0 + ( i * 2 );

				*pucOut = get( x, y, z);
				*pucOut |= get( x, y + 1, z) << 4;
				pucOut++;
			}
		}
	}
	const ptrdiff_t count = pucOut - &dataInOut.data[offset];

	return static_cast<int>(count);
}

void SparseDataStorage::addNewPlane(int y)
{
	bool success = false;
	do
	{
		// Get last packed data pointer & count
		const int64_t lastDataAndCount = dataAndCount;

		// Unpack count & data pointer
		const int lastLinesUsed = static_cast<int>((lastDataAndCount >> 48) & 0xffff);
		unsigned char *lastDataPointer = (unsigned char *)(lastDataAndCount & 0x0000ffffffffffff);

		// Find out what to prefill the newly allocated line with
		const unsigned char planeIndex = lastDataPointer[y];

		if( planeIndex < ALL_0_INDEX ) return;			// Something has already allocated this line - we're done

		int linesUsed = lastLinesUsed + 1;

		// Allocate new memory storage, copy over anything from old storage, and initialise remainder
		auto dataPointer = static_cast<unsigned char *>(malloc(linesUsed * 128 + 128));
		XMemCpy( dataPointer, lastDataPointer, 128 * lastLinesUsed + 128);
		XMemSet( dataPointer + ( 128 * lastLinesUsed ) + 128, 0, 128 );
		dataPointer[y] = lastLinesUsed;

		// Get new data and count packed info
#pragma warning ( disable : 4826 )
		int64_t newDataAndCount = reinterpret_cast<int64_t>(dataPointer) & 0x0000ffffffffffffL;
#pragma warning ( default : 4826 )
		newDataAndCount |= static_cast<int64_t>(linesUsed) << 48;

		// Attempt to update the data & count atomically. This command will Only succeed if the data stored at
		// dataAndCount is equal to lastDataAndCount, and will return the value present just before the write took place
		const int64_t lastDataAndCount2 = InterlockedCompareExchangeRelease64( &dataAndCount, newDataAndCount, lastDataAndCount );

		if( lastDataAndCount2 == lastDataAndCount )
		{
			success = true;
			// Queue old data to be deleted
			queueForDelete( lastDataPointer );
//			printf("Marking for delete 0x%x\n", lastDataPointer);
#ifdef DATA_COMPRESSION_STATS
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

void SparseDataStorage::getPlaneIndicesAndData(unsigned char **planeIndices, unsigned char **data)
{
	unsigned char *indicesAndData = reinterpret_cast<unsigned char *>(dataAndCount & 0x0000ffffffffffff);

	*planeIndices = indicesAndData;
	*data = indicesAndData + 128;

}

void SparseDataStorage::queueForDelete(unsigned char *data)
{
	// Add this into a queue for deleting. This shouldn't be actually deleted until tick has been called twice from when
	// the data went into the queue.
	deleteQueue[deleteQueueIndex].Push( data );
}

void SparseDataStorage::tick()
{
	// We have 3 queues for deleting. Always delete from the next one after where we are writing to, so it should take 2 ticks
	// before we ever delete something, from when the request to delete it came in
	const int freeIndex = ( deleteQueueIndex + 1 ) % 3;

//	printf("Free queue: %d, %d\n",deleteQueue[freeIndex].GetEntryCount(),deleteQueue[freeIndex].GetAllocated());
	unsigned char *toFree = nullptr;
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
void SparseDataStorage::updateDataAndCount(int64_t newDataAndCount)
{
	// Now actually assign this data to the storage. Just repeat until successful, there isn't any useful really that we can merge the results of this
	// with any other simultaneous writes that might be happening.
	bool success = false;
	do
	{
		const int64_t lastDataAndCount = dataAndCount;
		unsigned char *lastDataPointer = reinterpret_cast<unsigned char *>(lastDataAndCount & 0x0000ffffffffffff);

		// Attempt to update the data & count atomically. This command will Only succeed if the data stored at
		// dataAndCount is equal to lastDataAndCount, and will return the value present just before the write took place
		const int64_t lastDataAndCount2 = InterlockedCompareExchangeRelease64( &dataAndCount, newDataAndCount, lastDataAndCount );

		if( lastDataAndCount2 == lastDataAndCount )
		{
			success = true;
			// Queue old data to be deleted
//			printf("Marking for delete 0x%x (full replace)\n", lastDataPointer);
			queueForDelete( lastDataPointer );
		}
	} while( !success);

#ifdef DATA_COMPRESSION_STATS
	count = ( newDataAndCount >> 48 ) & 0xffff;
#endif

}

// Attempt to compress the stored data. This method makes no guarantee of success - if it fails due to something else writing to the storage whilst this is running, then it won't actually do anything.
int SparseDataStorage::compress()
{
	unsigned char _planeIndices[128];
	bool needsCompressed = false;

	const int64_t lastDataAndCount = dataAndCount;

	unsigned char *planeIndices = (unsigned char *)(lastDataAndCount & 0x0000ffffffffffff);
	unsigned char *data = planeIndices + 128;

	int planesToAlloc = 0;
	for( int i = 0; i < 128; i++ )
	{
		if( planeIndices[i] == ALL_0_INDEX )
		{
			_planeIndices[i] = ALL_0_INDEX;
		}
		else
		{
			const unsigned char *pucData = &data[ 128 * planeIndices[i] ];
			bool all0 = true;
			for( int j = 0; j < 128; j++ )	// 16 x 16 x 4-bits
			{
				if( *pucData != 0 ) all0 = false;
				pucData++;
			}
			if( all0 )
			{
				_planeIndices[i] = ALL_0_INDEX;
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
		unsigned char *newIndicesAndData = static_cast<unsigned char *>(malloc(128 + 128 * planesToAlloc));
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
		int64_t newDataAndCount = ((int64_t) newIndicesAndData) & 0x0000ffffffffffffL;
#pragma warning ( default : 4826 )
		newDataAndCount |= static_cast<int64_t>(planesToAlloc) << 48;

		// Attempt to update the data & count atomically. This command will Only succeed if the data stored at
		// dataAndCount is equal to lastDataAndCount, and will return the value present just before the write took place
		const int64_t lastDataAndCount2 = InterlockedCompareExchangeRelease64( &dataAndCount, newDataAndCount, lastDataAndCount );

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
#ifdef DATA_COMPRESSION_STATS
			count = planesToAlloc;
#endif
		}

		return planesToAlloc;
	}
	else
	{
		return static_cast<int>((lastDataAndCount >> 48) & 0xffff);
	}
}


bool SparseDataStorage::isCompressed()
{

	const int	count = ( dataAndCount >> 48 ) & 0xffff;
	return (count < 127);


}

void SparseDataStorage::write(DataOutputStream *dos)
{
	const int count = ( dataAndCount >> 48 ) & 0xffff;
	dos->writeInt(count);
	unsigned char *dataPointer = (unsigned char *)(dataAndCount & 0x0000ffffffffffff);
	const byteArray wrapper(dataPointer, count * 128 + 128);
	dos->write(wrapper);
}

void SparseDataStorage::read(DataInputStream *dis)
{
	const int count = dis->readInt();
	unsigned char *dataPointer = static_cast<unsigned char *>(malloc(count * 128 + 128));
	const byteArray wrapper(dataPointer, count * 128 + 128);
	dis->readFully(wrapper);

#pragma warning ( disable : 4826 )
	int64_t newDataAndCount = reinterpret_cast<int64_t>(dataPointer) & 0x0000ffffffffffffL;
#pragma warning ( default : 4826 )
	newDataAndCount |= static_cast<int64_t>(count) << 48;

	updateDataAndCount(newDataAndCount);
}
