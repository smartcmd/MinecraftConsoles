#pragma once
#include "compression.h"
#include "InputOutputStream.h"
using namespace std;

class FileEntry;
class ConsoleSaveFile;

class RegionFile
{
// 4J Stu TEMP FOR TESTING
private:
	FileEntry *fileEntry;

private:
	static const int VERSION_GZIP = 1;
    static const int VERSION_DEFLATE = 2;
	static const int VERSION_XBOX = 3;
	
    static const int SECTOR_BYTES = 4096;
    static const int SECTOR_INTS = SECTOR_BYTES / 4;

    static const int CHUNK_HEADER_SIZE = 8;
	static byteArray emptySector;

	File *fileName;
	//HANDLE file;
	ConsoleSaveFile *m_saveFile;

	int *offsets;
	int *chunkTimestamps;
	vector<bool> *sectorFree;
	int sizeDelta;
	__int64 _lastModified;
	bool m_bIsEmpty;			// 4J added

public:
    RegionFile(ConsoleSaveFile *saveFile, File *path);
	~RegionFile();

    /* the modification date of the region file when it was first opened */
    __int64 lastModified();

    /* gets how much the region file has grown since it was last checked */
    int getSizeDelta();

    /*
     * gets an (uncompressed) stream representing the chunk data returns null if
     * the chunk is not found or an error occurs
     */
    DataInputStream *getChunkDataInputStream(int x, int z);
    DataOutputStream *getChunkDataOutputStream(int x, int z);

	class ChunkBuffer : public ByteArrayOutputStream
	{
	private:
		RegionFile *rf;
		int x, z;
	public:
		ChunkBuffer( RegionFile *rf, int x, int z ) : ByteArrayOutputStream(8096)
		{
			this->rf = rf;
			this->x = x;
			this->z = z;
		}
		void close()
		{
			rf->write(x,z,buf.data,count);
		}
	};

    /* write a chunk at (x,z) with length bytes of data to disk */
protected:
    void write(int x, int z, byte *data, int length);

    /* write a chunk data to the region file at specified sector number */
private:
    void write(int sectorNumber, byte *data, int length, unsigned int compLength);
	void zero(int sectorNumber, int length);	// 4J added

    /* is this an invalid chunk coordinate? */
    bool outOfBounds(int x, int z);

    int getOffset(int x, int z);

public:
    bool hasChunk(int x, int z);

private:
	void insertInitialSectors();		// 4J added
    void setOffset(int x, int z, int offset);
    void setTimestamp(int x, int z, int value);

public:
	void writeAllOffsets();
    void close();
};
