#pragma once
#include "xmcore.h"

// 4J added - Storage for block & sky light data. Lighting data is normally stored as 4-bits per tile, in a DataLayer class of 16384 bytes ( 128 x 16 x 16 x 0.5 )
// This class provides more economical storage for such data by taking into consideration that it is quite common for large parts of the lighting data in a level to
// be very compressible (large amounts of 0 for block lights, 0 and 15 for sky lights).
// We are aiming here to balance performance (lighting data is accessed very frequently) against size.

// Details of storage method:

// 1. Lighting is split into horizontal planes, of which there are 128, and each taking up 128 bytes (16 x 16 x 0.5)
// 2. Each of these layers has a permanently allocated index in this class (planeIndices).
// 3. Data for allocatedPlaneCount planes worth of data is allocated in the data array ( allocatedPlaneCount * 128 bytes )
// 4. If a plane index for a layer is < 128, then the data for that layer is at data[ index * 128 ]
// 5. If a plane index for a layer is 128, then all values for that plane are 0
// 6. If a plane index for a layer is 129, then all values for that plane are 15

// This class needs to be thread safe as there are times where chunk (and light) data are shared between server & main threads. Light values are queried
// very regularly so this needs to be as light-weight as possible.

// To meet these requirements, this class is now implemented using a lock-free system, implemented using a read-copy-update (RCU) type algorithm. Some details...

// (1) The storage details for the class are now packed into a single int64_t, which contains both a pointer to the data that is required and a count of how many planes worth
//     of storage are allocated. This allows the full storage to be updated atomically using compare and exchange operations (implemented with InterlockedCompareExchangeRelease64).
// (2) The data pointer referenced in this int64_t points to an area of memory which is 128 + 128 * plane_count bytes long, where the first 128 bytes stoere the plane indices, and
//     the rest of the data is variable in size to accomodate however many planes are required to be stored
// (3) The RCU bit of the algorithm means that any read operations don't need to do any checks or locks at all. When the data needs to be updated, a copy of it is made and updated,
//     then an attempt is made to swap the new data in - if this succeeds then the old data pointer is deleted later at some point where we know nothing will be reading from it anymore.
//     This is achieved by putting the delete request in a queue which means it won't actually get deleted until 2 game ticks after the last time its reference existed, which should give
//     us a large margin of safety. If the attempt to swap the new data in fails, then the whole write operation has to be attempted again - this is the only time there is really a
//     high cost for this algorithm and such write collisions should be rare.

//#define LIGHT_COMPRESSION_STATS
class TileCompressData_SPU;

class SparseLightStorage
{
	friend class TileCompressData_SPU;
private:
//	unsigned char	planeIndices[128];
	int64_t			dataAndCount;							// Contains packed-together data pointer (lower 48-bits), and count of lines used (upper 16-bits)

//	unsigned char	*data;
//	unsigned int	allocatedPlaneCount;

	static const int ALL_0_INDEX = 128;
	static const int ALL_15_INDEX = 129;
#ifdef _XBOX
	static const unsigned int MM_PHYSICAL_4KB_BASE = 0xE0000000;		// Start of where 4KB page sized physical allocations are made
#endif
public:
    SparseLightStorage(bool sky);
	SparseLightStorage(bool sky, bool isUpper);
	SparseLightStorage(SparseLightStorage *copyFrom);		// ctor with deep copy
	~SparseLightStorage();

	void setData(byteArray dataIn, unsigned int inOffset);		// Set all lighting values from a data array of length 16384 (128 x 16 x 16 x 0.5).
	void getData(byteArray retArray, unsigned int retOffset);	// Gets all lighting values into an array of length 16384.
    int  get(int x, int y, int z);							// Get an individual lighting value
    void set(int x, int y, int z, int val);					// Set an individual lighting value
	void setAllBright();									// Set all lighting values to fully bright
	int  setDataRegion(byteArray dataIn, int x0, int y0, int z0, int x1, int y1, int z1, int offset);		// Sets a region of lighting values with the data at offset position in the array dataIn - external ordering compatible with java DataLayer
	int  getDataRegion(byteArray dataInOut, int x0, int y0, int z0, int x1, int y1, int z1, int offset);	// Updates the data at offset position dataInOut with a region of lighting information - external ordering compatible with java DataLayer

	static void staticCtor();

	void addNewPlane(int y);
	void getPlaneIndicesAndData(unsigned char **planeIndices, unsigned char **data);
	void updateDataAndCount(int64_t newDataAndCount);
	int  compress();

	bool isCompressed();
	void queueForDelete(unsigned char *data);

	static void tick();
	static int deleteQueueIndex;
	static XLockFreeStack <unsigned char> deleteQueue[3];


#ifdef LIGHT_COMPRESSION_STATS
	int count;
#endif

	void write(DataOutputStream *dos);
	void read(DataInputStream *dis);
};

