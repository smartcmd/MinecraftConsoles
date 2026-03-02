#pragma once

// 4J added - Storage for data (ie the extra per tile storage). Data is normally stored as 4-bits per tile, in a DataLayer class of 16384 bytes ( 128 x 16 x 16 x 0.5 )
// This class provides more economical storage for such data by taking into consideration that it is quite common for large parts of the data to be very compressible (ie zero).
// We are aiming here to balance performance (this data is accessed very frequently) against size.

// Details of storage method:

// 1. Data is split into horizontal planes, of which there are 128, and each taking up 128 bytes (16 x 16 x 0.5)
// 2. Each of these layers has a permanently allocated index in this class (planeIndices).
// 3. Data for allocatedPlaneCount planes worth of data is allocated in the data array ( allocatedPlaneCount * 128 bytes )
// 4. If a plane index for a layer is < 128, then the data for that layer is at data[ index * 128 ]
// 5. If a plane index for a layer is 128, then all values for that plane are 0

// This class needs to be thread safe as there are times where chunk data is shared between server & main threads. Data values are queried
// very regularly so this needs to be as light-weight as possible.

// To meet these requirements, this class is now implemented using a lock-free system, implemented using a read-copy-update (RCU) type algorithm. Some details...

// (1) The storage details for the class are now packed into a single __int64, which contains both a pointer to the data that is required and a count of how many planes worth
//     of storage are allocated. This allows the full storage to be updated atomically using compare and exchange operations (implemented with InterlockedCompareExchangeRelease64).
// (2) The data pointer referenced in this __int64 points to an area of memory which is 128 + 128 * plane_count bytes long, where the first 128 bytes stoere the plane indices, and
//     the rest of the data is variable in size to accomodate however many planes are required to be stored
// (3) The RCU bit of the algorithm means that any read operations don't need to do any checks or locks at all. When the data needs to be updated, a copy of it is made and updated,
//     then an attempt is made to swap the new data in - if this succeeds then the old data pointer is deleted later at some point where we know nothing will be reading from it anymore.
//     This is achieved by putting the delete request in a queue which means it won't actually get deleted until 2 game ticks after the last time its reference existed, which should give
//     us a large margin of safety. If the attempt to swap the new data in fails, then the whole write operation has to be attempted again - this is the only time there is really a
//     high cost for this algorithm and such write collisions should be rare.

//#define DATA_COMPRESSION_STATS

class SparseDataStorage_SPU
{
private:
//	unsigned char	planeIndices[128];
	unsigned char* m_pData;	

//	unsigned char	*data;
//	unsigned int	allocatedPlaneCount;

	static const int ALL_0_INDEX = 128;

public:
    SparseDataStorage_SPU(unsigned char* data) : m_pData(data) {}

	unsigned char* getDataPtr() { return m_pData; }

	int  get(int x, int y, int z)							// Get an individual data value
	{
		unsigned char *planeIndices, *data;
		getPlaneIndicesAndData(&planeIndices, &data);

		if( planeIndices[y] == ALL_0_INDEX )
		{
			return 0;
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

	void getPlaneIndicesAndData(unsigned char **planeIndices, unsigned char **data)
	{
		*planeIndices = m_pData;
		*data = m_pData + 128;
	}

};

