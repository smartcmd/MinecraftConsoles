#include "stdafx.h"
#include "System.h"
#include "InputOutputStream.h"
#include "File.h"
#include "RegionFile.h"

#include "ConsoleSaveFile.h"

byteArray RegionFile::emptySector(SECTOR_BYTES);

RegionFile::RegionFile(ConsoleSaveFile *saveFile, File *path)
{
	_lastModified = 0;

	m_saveFile = saveFile;

	offsets = new int[SECTOR_INTS];
	memset(offsets,0,SECTOR_BYTES);
	chunkTimestamps = new int[SECTOR_INTS];
	memset(chunkTimestamps,0,SECTOR_BYTES);

	/*  4J Jev, using files instead of strings:
	strncpy(fileName,path,MAX_PATH_SIZE); */

	fileName = path;

//    debugln("REGION LOAD " + fileName);

	sizeDelta = 0;

	// 4J - removed try/catch
//    try {

	/* 4J - Removed as _lastModifed not used and this is always failing as checking wrong thing
	if(  path->exists() )
	{
		_lastModified = path->lastModified();
	}
	*/

	fileEntry = m_saveFile->createFile( fileName->getName() );
	m_saveFile->setFilePointer( fileEntry, 0, NULL, FILE_END );

	if ( fileEntry->getFileSize() < SECTOR_BYTES)
	{
		// 4J altered - the original code used to write out 2 empty sectors here, which we don't want to do as we might be at a point where we shouldn't be touching the save file.
		// This now happens in insertInitialSectors when we do a first write to the region
		m_bIsEmpty = true;

		sizeDelta += SECTOR_BYTES * 2;
	}
	else
	{
		m_bIsEmpty = false;
	}

	//if ((GetFileSize(file,NULL) & 0xfff) != 0)
	if ((fileEntry->getFileSize() & 0xfff) != 0)
	{
		//byte zero = 0;
		DWORD numberOfBytesWritten = 0;
		DWORD bytesToWrite = 0x1000 - (fileEntry->getFileSize() & 0xfff);
		byte *zeroBytes = new byte[ bytesToWrite ];
		ZeroMemory(zeroBytes, bytesToWrite);

		/* the file size is not a multiple of 4KB, grow it */
		m_saveFile->writeFile(fileEntry,zeroBytes,bytesToWrite,&numberOfBytesWritten);

		delete [] zeroBytes;
	}

	/* set up the available sector map */

	int nSectors;
	if( m_bIsEmpty )		// 4J - added this case for our empty files that we now don't create
	{
		nSectors = 2;
	}
	else
	{
		nSectors = (int) fileEntry->getFileSize() / SECTOR_BYTES;
	}
	sectorFree = new vector<bool>;
	sectorFree->reserve(nSectors);

	for (int i = 0; i < nSectors; ++i)
	{
		sectorFree->push_back(true);
	}

	sectorFree->at(0) = false; // chunk offset table
	sectorFree->at(1) = false; // for the last modified info

	m_saveFile->setFilePointer( fileEntry, 0, NULL, FILE_BEGIN );
	for (int i = 0; i < SECTOR_INTS; ++i)
	{
		unsigned int offset = 0;
		DWORD numberOfBytesRead = 0;
		if( !m_bIsEmpty )		// 4J added condition, don't read back if we've just created an empty file as we don't immediately write this anymore
		{
			m_saveFile->readFile(fileEntry, &offset, 4, &numberOfBytesRead);

			if(saveFile->isSaveEndianDifferent()) System::ReverseULONG(&offset);

		}
		offsets[i] = offset;
		if (offset != 0 && (offset >> 8) + (offset & 0xFF) <= sectorFree->size())
		{
			for (unsigned int sectorNum = 0; sectorNum < (offset & 0xFF); ++sectorNum)
			{
				sectorFree->at((offset >> 8) + sectorNum) = false;
			}
		}
	}
	for (int i = 0; i < SECTOR_INTS; ++i)
	{
		int lastModValue = 0;
		DWORD numberOfBytesRead = 0;
		if( !m_bIsEmpty )		// 4J added condition, don't read back if we've just created an empty file as we don't immediately write this anymore
		{
			m_saveFile->readFile(fileEntry, &lastModValue, 4, &numberOfBytesRead);

			if(saveFile->isSaveEndianDifferent()) System::ReverseINT(&lastModValue);
		}
		chunkTimestamps[i] = lastModValue;
	}


//    } catch (IOException e) {
//        e.printStackTrace();
//    }
}

void RegionFile::writeAllOffsets() // used for the file ConsoleSaveFile conversion between platforms
{
	if(m_bIsEmpty == false)
	{
		// save all the offsets and timestamps
		m_saveFile->LockSaveAccess();

		DWORD numberOfBytesWritten = 0;
		m_saveFile->setFilePointer( fileEntry, 0, NULL, FILE_BEGIN );
		m_saveFile->writeFile(fileEntry,offsets, SECTOR_BYTES ,&numberOfBytesWritten);

		numberOfBytesWritten = 0;
		m_saveFile->setFilePointer( fileEntry, SECTOR_BYTES, NULL, FILE_BEGIN );
		m_saveFile->writeFile(fileEntry, chunkTimestamps, SECTOR_BYTES, &numberOfBytesWritten);

		m_saveFile->ReleaseSaveAccess();
	}

}
RegionFile::~RegionFile()
{
	delete[] offsets;
	delete[] chunkTimestamps;
	delete sectorFree;
	m_saveFile->closeHandle( fileEntry );
}

int64_t RegionFile::lastModified()
{
	return _lastModified;
}

int RegionFile::getSizeDelta()   // TODO - was synchronized
{
	int ret = sizeDelta;
	sizeDelta = 0;
	return ret;
}

DataInputStream *RegionFile::getChunkDataInputStream(int x, int z) // TODO - was synchronized
{
	if (outOfBounds(x, z))
	{
//        debugln("READ", x, z, "out of bounds");
		return NULL;
	}

	// 4J - removed try/catch
//    try {
	int offset = getOffset(x, z);
	if (offset == 0)
	{
		// debugln("READ", x, z, "miss");
		return NULL;
	}

	unsigned int sectorNumber = offset >> 8;
	unsigned int numSectors = offset & 0xFF;

	if (sectorNumber + numSectors > sectorFree->size())
	{
//        debugln("READ", x, z, "invalid sector");
		return NULL;
	}

	m_saveFile->LockSaveAccess();

	//SetFilePointer(file,sectorNumber * SECTOR_BYTES,0,FILE_BEGIN);
	m_saveFile->setFilePointer( fileEntry, sectorNumber * SECTOR_BYTES, NULL, FILE_BEGIN);

	unsigned int length;
	unsigned int decompLength;
	unsigned int readDecompLength;

	DWORD numberOfBytesRead = 0;

	// 4J - this differs a bit from the java file format. Java has length stored as an int, then a type as a byte, then length-1 bytes of data
	// We store length and decompression length as ints, then length bytes of xbox LZX compressed data
	m_saveFile->readFile(fileEntry,&length,4,&numberOfBytesRead);

	if(m_saveFile->isSaveEndianDifferent()) System::ReverseULONG(&length);

	// Using to bit of length to signify that this data was compressed with RLE method
	bool useRLE = false;
	if( length & 0x80000000 )
	{
		useRLE = true;
		length &= 0x7fffffff;
	}
	m_saveFile->readFile(fileEntry,&decompLength,4,&numberOfBytesRead);

	if(m_saveFile->isSaveEndianDifferent()) System::ReverseULONG(&decompLength);

	if (length > SECTOR_BYTES * numSectors)
	{
//        debugln("READ", x, z, "invalid length: " + length + " > 4096 * " + numSectors);

		m_saveFile->ReleaseSaveAccess();
		return NULL;
	}

	MemSect(50);
	byte *data = new byte[length];
	byte *decomp = new byte[decompLength];
	MemSect(0);
	readDecompLength = decompLength;
	m_saveFile->readFile(fileEntry,data,length,&numberOfBytesRead);

	m_saveFile->ReleaseSaveAccess();

	Compression::getCompression()->SetDecompressionType(m_saveFile->getSavePlatform()); // if this save is from another platform, set the correct decompression type

	if( useRLE )
	{
		Compression::getCompression()->DecompressLZXRLE(decomp, &readDecompLength, data, length );
	}
	else
	{
		Compression::getCompression()->Decompress(decomp, &readDecompLength, data, length );
	}

	Compression::getCompression()->SetDecompressionType(SAVE_FILE_PLATFORM_LOCAL); // and then set the decompression back to the local machine's standard type

	delete [] data;

	// 4J - was InflaterInputStream in here too, but we've already decompressed
	DataInputStream *ret = new DataInputStream(new ByteArrayInputStream( byteArray( decomp, readDecompLength) ));
	return ret;

//    } catch (IOException e) {
//        debugln("READ", x, z, "exception");
//        return null;
//    }
}

DataOutputStream *RegionFile::getChunkDataOutputStream(int x, int z)
{
	// 4J - was DeflatorOutputStream in here too, but we've already compressed
	return new DataOutputStream( new ChunkBuffer(this, x, z));
}

/* write a chunk at (x,z) with length bytes of data to disk */
void RegionFile::write(int x, int z, byte *data, int length)		// TODO - was synchronized
{
	// 4J Stu - Do the compression here so that we know how much space we need to store the compressed data
	byte *compData = new byte[length + 2048];	// presuming compression is going to make this smaller...	UPDATE - for some really small things this isn't the case. Added 2K on here to cover those.
	unsigned int compLength = length;
	Compression::getCompression()->CompressLZXRLE(compData,&compLength,data,length);

	int sectorsNeeded = (compLength + CHUNK_HEADER_SIZE) / SECTOR_BYTES + 1;

//	app.DebugPrintf(">>>>>>>>>>>>>> writing compressed data for 0x%.8x, %d %d\n",fileEntry->data.regionIndex,x,z);

	// maximum chunk size is 1MB
	if (sectorsNeeded >= 256)
	{
		return;
	}

	m_saveFile->LockSaveAccess();
	{
		int offset = getOffset(x, z);
		int sectorNumber = offset >> 8;
		int sectorsAllocated = offset & 0xFF;

#ifndef _CONTENT_PACKAGE
		if(sectorNumber < 0)
		{
			__debugbreak();
		}
#endif

		if (sectorNumber != 0 && sectorsAllocated == sectorsNeeded)
		{
			/* we can simply overwrite the old sectors */
	//        debug("SAVE", x, z, length, "rewrite");
	#ifndef _CONTENT_PACKAGE
			//wprintf(L"Writing chunk (%d,%d) in %ls from current sector %d to %d\n", x,z, fileEntry->data.filename, sectorNumber, sectorNumber + sectorsNeeded - 1);
	#endif
			write(sectorNumber, compData, length, compLength);
		}
		else
		{
			/* we need to allocate new sectors */

			/* mark the sectors previously used for this chunk as free */
			for (int i = 0; i < sectorsAllocated; ++i)
			{
				sectorFree->at(sectorNumber + i) = true;
			}
			// 4J added - zero this now unused region of the file, so it can be better compressed until it is reused
			zero(sectorNumber, SECTOR_BYTES * sectorsAllocated);

			PIXBeginNamedEvent(0,"Scanning for free space\n");
			/* scan for a free space large enough to store this chunk */
			int runStart = (int)(find(sectorFree->begin(),sectorFree->end(),true) - sectorFree->begin());  // 4J - was sectorFree.indexOf(true)
			int runLength = 0;
			if (runStart != -1)
			{
				for (unsigned int i = runStart; i < sectorFree->size(); ++i)
				{
					if (runLength != 0)
					{
						if (sectorFree->at(i)) runLength++;
						else runLength = 0;
					} else if (sectorFree->at(i))
					{
						runStart = i;
						runLength = 1;
					}
					if (runLength >= sectorsNeeded)
					{
						break;
					}
				}
			}
			PIXEndNamedEvent();

			if (runLength >= sectorsNeeded)
			{
				/* we found a free space large enough */
	//            debug("SAVE", x, z, length, "reuse");
				sectorNumber = runStart;
				setOffset(x, z, (sectorNumber << 8) | sectorsNeeded);
	#ifndef _CONTENT_PACKAGE
				//wprintf(L"Writing chunk (%d,%d) in %ls from old sector %d to %d\n", x,z, fileEntry->data.filename, sectorNumber, sectorNumber + sectorsNeeded - 1);
	#endif
				for (int i = 0; i < sectorsNeeded; ++i)
				{
					sectorFree->at(sectorNumber + i) = false;
				}
				write(sectorNumber, compData, length, compLength);
			}
			else
			{
				PIXBeginNamedEvent(0,"Expanding storage for %d sectors\n", sectorsNeeded);
				/*
					* no free space large enough found -- we need to grow the
					* file
					*/
	//            debug("SAVE", x, z, length, "grow");
				//SetFilePointer(file,0,0,FILE_END);
				m_saveFile->setFilePointer( fileEntry, 0, NULL, FILE_END );

				sectorNumber = (int)sectorFree->size();
	#ifndef _CONTENT_PACAKGE
				//wprintf(L"Writing chunk (%d,%d) in %ls from new sector %d to %d\n", x,z, fileEntry->data.filename, sectorNumber, sectorNumber + sectorsNeeded - 1);
	#endif
				DWORD numberOfBytesWritten = 0;
				for (int i = 0; i < sectorsNeeded; ++i)
				{
					//WriteFile(file,emptySector.data,SECTOR_BYTES,&numberOfBytesWritten,NULL);
					m_saveFile->writeFile(fileEntry,emptySector.data,SECTOR_BYTES,&numberOfBytesWritten);
					sectorFree->push_back(false);
				}
				sizeDelta += SECTOR_BYTES * sectorsNeeded;

				write(sectorNumber, compData, length, compLength);
				setOffset(x, z, (sectorNumber << 8) | sectorsNeeded);
				PIXEndNamedEvent();
			}
		}
		setTimestamp(x, z, (int) (System::currentTimeMillis() / 1000L));
	}
	m_saveFile->ReleaseSaveAccess();

//    } catch (IOException e) {
//        e.printStackTrace();
//    }
}

/* write a chunk data to the region file at specified sector number */
void RegionFile::write(int sectorNumber, byte *data, int length, unsigned int compLength)
{
	DWORD numberOfBytesWritten = 0;
	//SetFilePointer(file,sectorNumber * SECTOR_BYTES,0,FILE_BEGIN);
	m_saveFile->setFilePointer( fileEntry, sectorNumber * SECTOR_BYTES, NULL, FILE_BEGIN );

	// 4J - this differs a bit from the java file format. Java has length stored as an int, then a type as a byte, then length-1 bytes of data
	// We store length and decompression length as ints, then length bytes of xbox LZX compressed data

	// 4J Stu - We need to do the compression at a level above this, where it is checking for free space

	compLength |= 0x80000000;	// 4J - signify that this has been encoded with RLE method ( see code in getChunkDataInputStream() for matching detection of this)
	m_saveFile->writeFile(fileEntry,&compLength,4,&numberOfBytesWritten);
	compLength &= 0x7fffffff;
	m_saveFile->writeFile(fileEntry,&length,4,&numberOfBytesWritten);
	m_saveFile->writeFile(fileEntry,data,compLength,&numberOfBytesWritten);
	delete[] data;
}

void RegionFile::zero(int sectorNumber, int length)
{
	DWORD numberOfBytesWritten = 0;
	//SetFilePointer(file,sectorNumber * SECTOR_BYTES,0,FILE_BEGIN);
	m_saveFile->setFilePointer( fileEntry, sectorNumber * SECTOR_BYTES, NULL, FILE_BEGIN );
	m_saveFile->zeroFile( fileEntry, length, &numberOfBytesWritten );
}

/* is this an invalid chunk coordinate? */
bool RegionFile::outOfBounds(int x, int z)
{
	return x < 0 || x >= 32 || z < 0 || z >= 32;
}

int RegionFile::getOffset(int x, int z)
{
	return offsets[x + z * 32];
}

bool RegionFile::hasChunk(int x, int z)
{
	return getOffset(x, z) != 0;
}

// 4J added - write the initial two sectors that used to be written in the ctor when the file was empty
void RegionFile::insertInitialSectors()
{
	m_saveFile->setFilePointer( fileEntry, 0, NULL, FILE_BEGIN );
	DWORD numberOfBytesWritten = 0;
	byte zeroBytes[ SECTOR_BYTES ];
	ZeroMemory(zeroBytes, SECTOR_BYTES);

	/* we need to write the chunk offset table */
	m_saveFile->writeFile(fileEntry,zeroBytes,SECTOR_BYTES,&numberOfBytesWritten);

	// write another sector for the timestamp info
	m_saveFile->writeFile(fileEntry,zeroBytes,SECTOR_BYTES,&numberOfBytesWritten);

	m_bIsEmpty = false;
}

void RegionFile::setOffset(int x, int z, int offset)
{
	if( m_bIsEmpty )
	{
		insertInitialSectors();		// 4J added
	}

	DWORD numberOfBytesWritten = 0;
	offsets[x + z * 32] = offset;
	m_saveFile->setFilePointer( fileEntry, (x + z * 32) * 4, NULL, FILE_BEGIN );

	m_saveFile->writeFile(fileEntry,&offset,4,&numberOfBytesWritten);
}

void RegionFile::setTimestamp(int x, int z, int value)
{
	if( m_bIsEmpty )
	{
		insertInitialSectors();		// 4J added
	}

	DWORD numberOfBytesWritten = 0;
	chunkTimestamps[x + z * 32] = value;
	m_saveFile->setFilePointer( fileEntry, SECTOR_BYTES + (x + z * 32) * 4, NULL, FILE_BEGIN );

	m_saveFile->writeFile(fileEntry,&value,4,&numberOfBytesWritten);
}

void RegionFile::close()
{
	m_saveFile->closeHandle( fileEntry );
}
