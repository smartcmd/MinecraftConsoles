#include "stdafx.h"
#include "StringHelpers.h"
#include "ConsoleSaveFileSplit.h"
#include "ConsoleSaveFileConverter.h"
#include "File.h"
#include <xuiapp.h>
#include "compression.h"
#include "..\Minecraft.Client\Minecraft.h"
#include "..\Minecraft.Client\MinecraftServer.h"
#include "..\Minecraft.Client\ServerLevel.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\LevelData.h"
#include "..\Minecraft.Client\Common\GameRules\LevelGenerationOptions.h"
#include "..\Minecraft.World\net.minecraft.world.level.chunk.storage.h"

#define RESERVE_ALLOCATION  MEM_RESERVE
#define COMMIT_ALLOCATION  MEM_COMMIT

unsigned int ConsoleSaveFileSplit::pagesCommitted = 0;
void *ConsoleSaveFileSplit::pvHeap = NULL;

ConsoleSaveFileSplit::RegionFileReference::RegionFileReference(int index, unsigned int regionIndex,  unsigned int length/*=0*/, unsigned char *data/*=NULL*/)
{
	fileEntry = new FileEntry();
	fileEntry->currentFilePointer = 0;
	fileEntry->data.length = 0;
	fileEntry->data.regionIndex = regionIndex;
	this->data	= 0;
	this->index = index;
	this->dirty = false;
	this->dataCompressed = data;
	this->dataCompressedSize = length;
	this->lastWritten = 0;
}

ConsoleSaveFileSplit::RegionFileReference::~RegionFileReference()
{
	free(data);
	delete fileEntry;
}

// Compress from data to dataCompressed. Uses a special compression method that is designed just to efficiently store runs of zeros, with little overhead on other stuff.
// Compresed format is a 4 byte uncompressed size, followed by data as follows:
// 
// Byte value												Meaning
//
// 1 - 255													Normal data
// 0 followed by 1 - 255									Run of 1 - 255 0s
// 0 followed by 0, followed by 256 to 65791 (as 2 bytes)	Run of 256 to 65791 zeros

void ConsoleSaveFileSplit::RegionFileReference::Compress()
{
	unsigned char *dataIn = data;
	unsigned char *dataInLast = data + fileEntry->data.length;

//	int64_t startTime = System::currentTimeMillis();

	// One pass through to work out storage space required for compressed data
	unsigned int outputSize = 4;		// 4 bytes required to store the uncompressed size for faster decompression
	unsigned int runLength = 0;
	while( dataIn != dataInLast )
	{
		unsigned char thisByte = *dataIn++;
		if( ( thisByte != 0 ) || ( runLength == ( 65535 + 256 ) ) )
		{
			// We've got a non-zero value, or we've hit our maximum run length.
			// If there was a preceeding run of zeros, encode that nwo
			if( runLength != 0 )
			{
				if( runLength < 256 )
				{
					// Runs of 1 to 255 encoded as 0 followed by one byte of run length
					outputSize += 2;
				}
				else
				{
					// Runs of 256 to 65791 encoded as two 0s followed by two bytes of run length - 256
					outputSize += 4;
				}
				// Run is now processed
				runLength = 0;
			}
			// Now handle the current byte
			if( thisByte == 0 )
			{
				runLength++;
			}
			else
			{
				// Non-zero, just copy over to output
				outputSize++;
			}
		}
		else
		{
			// It's a zero - keep counting size of the run
			runLength++;
		}
	}
	// Handle any outstanding run
	if ( runLength != 0 )
	{
		if( runLength < 256 )
		{
			// Runs of 1 to 255 encoded as 0 followed by one byte of run length
			outputSize += 2;
		}
		else
		{
			// Runs of 256 to 65791 encoded as two 0s followed by two bytes of run length - 256
			outputSize += 4;
		}
		// Run is now processed
		runLength = 0;
	}

	// Now actually allocate & write the compress data. First 4 bytes store the uncompressed size
	dataCompressed = (unsigned char *)malloc(outputSize);
	*((unsigned int *)dataCompressed) = fileEntry->data.length;
	unsigned char *dataOut = dataCompressed + 4;
	dataIn = data;

	// Now same process as before, but actually writing
	while( dataIn != dataInLast )
	{
		unsigned char thisByte = *dataIn++;
		if( ( thisByte != 0 ) || ( runLength == ( 65535 + 256 ) ) )
		{
			// We've got a non-zero value, or we've hit our maximum run length.
			// If there was a preceeding run of zeros, encode that nwo
			if( runLength != 0 )
			{
				if( runLength < 256 )
				{
					// Runs of 1 to 255 encoded as 0 followed by one byte of run length
					*dataOut++ = 0;
					*dataOut++ = runLength;
				}
				else
				{
					// Runs of 256 to 65791 encoded as two 0s followed by two bytes of run length - 256
					*dataOut++ = 0;
					*dataOut++ = 0;
					unsigned int largeRunLength = runLength - 256;
					*dataOut++ = ( largeRunLength >> 8 ) & 0xff;
					*dataOut++ = ( largeRunLength ) & 0xff;
				}
				// Run is now processed
				runLength = 0;
			}
			// Now handle the current byte
			if( thisByte == 0 )
			{
				runLength++;
			}
			else
			{
				// Non-zero, just copy over to output
				*dataOut++ = thisByte;
			}
		}
		else
		{
			// It's a zero - keep counting size of the run
			runLength++;
		}
	}
	// Handle any outstanding run
	if( runLength != 0 )
	{
		if( runLength < 256 )
		{
			// Runs of 1 to 255 encoded as 0 followed by one byte of run length
			*dataOut++ = 0;
			*dataOut++ = runLength;
		}
		else
		{
			// Runs of 256 to 65791 encoded as two 0s followed by two bytes of run length - 256
			*dataOut++ = 0;
			*dataOut++ = 0;
			unsigned int largeRunLength = runLength - 256;
			*dataOut++ = ( largeRunLength >> 8 ) & 0xff;
			*dataOut++ = ( largeRunLength ) & 0xff;
		}
		// Run is now processed
		runLength = 0;
	}
	assert(( dataOut - dataCompressed ) == outputSize );
	dataCompressedSize = outputSize;
//	int64_t endTime = System::currentTimeMillis();
//	app.DebugPrintf("Compressing region file 0x%.8x from %d to %d bytes - %dms\n", fileEntry->data.regionIndex, fileEntry->data.length, dataCompressedSize, endTime - startTime);
}

// Decompress from dataCompressed -> data. See comment in Compress method for format
void ConsoleSaveFileSplit::RegionFileReference::Decompress()
{
//	int64_t startTime = System::currentTimeMillis();
	fileEntry->data.length = *((unsigned int *)dataCompressed);

	// If this is unusually large, then test how big it would be when expanded before trying to allocate. Matching the expanded size
	// is (currently) our means of knowing that this file is ok
	if( fileEntry->data.length > 1 * 1024 * 1024 )
	{
		unsigned int uncompressedSize = 0;
		unsigned char *dataIn = dataCompressed + 4;
		unsigned char *dataInLast = dataCompressed + dataCompressedSize;

		while (dataIn != dataInLast)
		{
			unsigned char thisByte = *dataIn++;
			if( thisByte == 0 )
			{
				thisByte = *dataIn++;
				if( thisByte == 0 )
				{
					unsigned int runLength = (*dataIn++) << 8;
					runLength |= (*dataIn++);
					runLength += 256;
					uncompressedSize += runLength;
				}
				else
				{
					unsigned int runLength = thisByte;
					uncompressedSize += runLength;
				}
			}
			else
			{
				uncompressedSize++;
			}
		}

		if( fileEntry->data.length != uncompressedSize )
		{
			// Treat as if it was an empty region file
			fileEntry->data.length = 0;
			assert(0);
			return;
		}
	}


	data = (unsigned char *)malloc(fileEntry->data.length);
	unsigned char *dataIn = dataCompressed + 4;
	unsigned char *dataInLast = dataCompressed + dataCompressedSize;
	unsigned char *dataOut = data;

	while (dataIn != dataInLast)
	{
		unsigned char thisByte = *dataIn++;
		if( thisByte == 0 )
		{
			thisByte = *dataIn++;
			if( thisByte == 0 )
			{
				unsigned int runLength = (*dataIn++) << 8;
				runLength |= (*dataIn++);
				runLength += 256;
				for( unsigned int i = 0; i < runLength; i++ )
				{
					*dataOut++ = 0;
				}
			}
			else
			{
				unsigned int runLength = thisByte;
				for( unsigned int i = 0; i < runLength; i++ )
				{
					*dataOut++ = 0;
				}
			}
		}
		else
		{
			*dataOut++ = thisByte;
		}
	}
	// If we failed to correctly decompress, then treat as if it was an empty region file
	if( ( dataOut - data ) != fileEntry->data.length )
	{
		free(data);
		fileEntry->data.length = 0;
		data = NULL;
		assert(0);
	}
//	int64_t endTime = System::currentTimeMillis();
//	app.DebugPrintf("Decompressing region file from 0x%.8x %d to %d bytes - %dms\n", fileEntry->data.regionIndex, dataCompressedSize, fileEntry->data.length, endTime - startTime);//
}

unsigned int ConsoleSaveFileSplit::RegionFileReference::GetCompressedSize()
{
	unsigned char *dataIn = data;
	unsigned char *dataInLast = data + fileEntry->data.length;

	unsigned int outputSize = 4;		// 4 bytes required to store the uncompressed size for faster decompression
	unsigned int runLength = 0;
	while( dataIn != dataInLast )
	{
		unsigned char thisByte = *dataIn++;
		if( ( thisByte != 0 ) || ( runLength == ( 65535 + 256 ) ) )
		{
			// We've got a non-zero value, or we've hit our maximum run length.
			// If there was a preceeding run of zeros, encode that nwo
			if( runLength != 0 )
			{
				if( runLength < 256 )
				{
					// Runs of 1 to 255 encoded as 0 followed by one byte of run length
					outputSize += 2;
				}
				else
				{
					// Runs of 256 to 65791 encoded as two 0s followed by two bytes of run length - 256
					outputSize += 4;
				}
				// Run is now processed
				runLength = 0;
			}
			// Now handle the current byte
			if( thisByte == 0 )
			{
				runLength++;
			}
			else
			{
				// Non-zero, just copy over to output
				outputSize++;
			}
		}
		else
		{
			// It's a zero - keep counting size of the run
			runLength++;
		}
	}
	// Handle any outstanding run
	if ( runLength != 0 )
	{
		if( runLength < 256 )
		{
			// Runs of 1 to 255 encoded as 0 followed by one byte of run length
			outputSize += 2;
		}
		else
		{
			// Runs of 256 to 65791 encoded as two 0s followed by two bytes of run length - 256
			outputSize += 4;
		}
		// Run is now processed
		runLength = 0;
	}
	return outputSize;
}

// Release dataCompressed
void ConsoleSaveFileSplit::RegionFileReference::ReleaseCompressed()
{
//	app.DebugPrintf("Releasing compressed data for region file from 0x%.8x\n", fileEntry->data.regionIndex );
	free(dataCompressed);
	dataCompressed = NULL;
	dataCompressedSize = NULL;
}

FileEntry *ConsoleSaveFileSplit::GetRegionFileEntry(unsigned int regionIndex)
{
	// Is a region file - determine if we've got it as a separate file
	AUTO_VAR(it, regionFiles.find(regionIndex) );
	if( it != regionFiles.end() )
	{
		// Already got it
		return it->second->fileEntry;
	}

	int index = StorageManager.AddSubfile(regionIndex);
	RegionFileReference *newRef = new RegionFileReference(index, regionIndex);
	regionFiles[regionIndex] = newRef;
	
	return newRef->fileEntry;
}

ConsoleSaveFileSplit::ConsoleSaveFileSplit(const wstring &fileName, LPVOID pvSaveData /*= NULL*/, DWORD dFileSize /*= 0*/, bool forceCleanSave /*= false*/, ESavePlatform plat /*= SAVE_FILE_PLATFORM_LOCAL*/)
{
	DWORD fileSize = dFileSize;

	// Load a save from the game rules
	bool bLevelGenBaseSave = false;
	LevelGenerationOptions *levelGen = app.getLevelGenerationOptions();
	if( pvSaveData == NULL && levelGen != NULL && levelGen->requiresBaseSave())
	{
		pvSaveData = levelGen->getBaseSaveData(fileSize);
		if(pvSaveData && fileSize != 0) bLevelGenBaseSave = true;
	}

	if( pvSaveData == NULL || fileSize == 0)
		fileSize = StorageManager.GetSaveSize();

	if( forceCleanSave )
		fileSize = 0;

	_init(fileName, pvSaveData, fileSize, plat);

	if(bLevelGenBaseSave)
	{
		levelGen->deleteBaseSaveData();
	}
}

ConsoleSaveFileSplit::ConsoleSaveFileSplit(ConsoleSaveFile *sourceSave, bool alreadySmallRegions, ProgressListener *progress)
{
	_init(sourceSave->getFilename(), NULL, 0, sourceSave->getSavePlatform());

	header.setOriginalSaveVersion(sourceSave->getOriginalSaveVersion());
	header.setSaveVersion(sourceSave->getSaveVersion());

	if(alreadySmallRegions)
	{

		vector<FileEntry *> *sourceFiles = sourceSave->getFilesWithPrefix(L"");

		DWORD bytesWritten;
		for(AUTO_VAR(it, sourceFiles->begin()); it != sourceFiles->end(); ++it)
		{
			FileEntry *sourceEntry = *it;
			sourceSave->setFilePointer(sourceEntry,0,NULL,FILE_BEGIN);

			FileEntry *targetEntry = createFile(ConsoleSavePath(sourceEntry->data.filename));

			writeFile(targetEntry, sourceSave->getWritePointer(sourceEntry), sourceEntry->getFileSize(), &bytesWritten);
		}

		delete sourceFiles;
	}
	else
	{
		ConsoleSaveFileConverter::ConvertSave(sourceSave, this, progress);
	}
}

void ConsoleSaveFileSplit::_init(const wstring &fileName, LPVOID pvSaveData, DWORD fileSize, ESavePlatform plat)
{
	InitializeCriticalSectionAndSpinCount(&m_lock,5120);

	m_lastTickTime = 0;

	// One time initialise of static stuff required for our storage
	if( pvHeap == NULL )
	{
		// Reserve a chunk of 64MB of virtual address space for our saves, using 64KB pages.
		// We'll only be committing these as required to grow the storage we need, which will
		// the storage to grow without having to use realloc.
		pvHeap = VirtualAlloc(NULL, MAX_PAGE_COUNT * CSF_PAGE_SIZE, RESERVE_ALLOCATION, PAGE_READWRITE );
	}

	pvSaveMem = pvHeap;
	m_fileName = fileName;

	// Get details of region files. From this point on we are responsible for the memory that the storage manager initially allocated for them
	unsigned int regionCount = StorageManager.GetSubfileCount();
	for( unsigned int i = 0; i < regionCount; i++ )
	{
		unsigned int regionIndex;
		unsigned char *regionDataCompressed;
		unsigned int regionSizeCompressed;

		StorageManager.GetSubfileDetails(i, &regionIndex, &regionDataCompressed, &regionSizeCompressed);

		RegionFileReference *regionFileRef = new RegionFileReference(i, regionIndex, regionSizeCompressed, regionDataCompressed);
		if( regionSizeCompressed > 0 )
		{
			regionFileRef->Decompress();
		}
		else
		{
			regionFileRef->fileEntry->data.length = 0;
		}
		regionFileRef->ReleaseCompressed();
		regionFiles[regionIndex] = regionFileRef;
	}

	DWORD heapSize = max( fileSize, (DWORD)(1024 * 1024 * 2)); // 4J Stu - Our files are going to be bigger than 2MB so allocate high to start with

	// Initially committ enough room to store headSize bytes (using CSF_PAGE_SIZE pages, so rounding up here). We should only ever have one save file at a time,
	// and the pages should be decommitted in the dtor, so pages committed should always be zero at this point.
	if( pagesCommitted != 0 )
	{
#ifndef _CONTENT_PACKAGE
		__debugbreak();
#endif
	}

	unsigned int pagesRequired = ( heapSize + (CSF_PAGE_SIZE - 1 ) ) / CSF_PAGE_SIZE;

	void *pvRet = VirtualAlloc(pvHeap, pagesRequired * CSF_PAGE_SIZE, COMMIT_ALLOCATION, PAGE_READWRITE);
	if( pvRet == NULL )
	{
#ifndef _CONTENT_PACKAGE
		// Out of physical memory
		__debugbreak();
#endif
	}
	pagesCommitted = pagesRequired;

	if( fileSize > 0)
	{
		if(pvSaveData != NULL)
		{
			memcpy(pvSaveMem, pvSaveData, fileSize);
		}
		else
		{
			unsigned int storageLength;
			StorageManager.GetSaveData( pvSaveMem, &storageLength );
			app.DebugPrintf("Filesize - %d, Adjusted size - %d\n",fileSize,storageLength);
			fileSize = storageLength;
		} 

		int compressed = *(int*)pvSaveMem;
		if( compressed == 0 )
		{
			unsigned int decompSize = *( (int*)pvSaveMem+1 );

			// An invalid save, so clear the memory and start from scratch
			if(decompSize == 0)
			{
				// 4J Stu - Saves created between 2/12/2011 and 7/12/2011 will have this problem
				app.DebugPrintf("Invalid save data format\n");
				ZeroMemory( pvSaveMem, fileSize );
				// Clear the first 8 bytes that reference the header
				header.WriteHeader( pvSaveMem );
			}
			else
			{
				unsigned char *buf = new unsigned char[decompSize];
	
				if( Compression::getCompression()->Decompress(buf, &decompSize, (unsigned char *)pvSaveMem+8, fileSize-8 ) == S_OK)
				{

					// Only ReAlloc if we need to (we might already have enough) and align to 512 byte boundaries
					DWORD currentHeapSize = pagesCommitted * CSF_PAGE_SIZE;

					DWORD desiredSize = decompSize;

					if( desiredSize > currentHeapSize )
					{
						unsigned int pagesRequired = ( desiredSize + (CSF_PAGE_SIZE - 1 ) ) / CSF_PAGE_SIZE;
						void *pvRet = VirtualAlloc(pvHeap, pagesRequired * CSF_PAGE_SIZE, COMMIT_ALLOCATION, PAGE_READWRITE);
						if( pvRet == NULL )
						{
							// Out of physical memory
							__debugbreak();
						}
						pagesCommitted = pagesRequired;
					}

					memcpy(pvSaveMem, buf, decompSize);
				}
				else
				{
					// Corrupt save, although most of the terrain should actually be ok
					app.DebugPrintf("Failed to decompress save data!\n");
#ifndef _CONTENT_PACKAGE
					__debugbreak();
#endif
					ZeroMemory( pvSaveMem, fileSize );
					// Clear the first 8 bytes that reference the header
					header.WriteHeader( pvSaveMem );
				}

				delete[] buf;
			}
		}

		header.ReadHeader( pvSaveMem, plat );

	}
	else
	{	
		// Clear the first 8 bytes that reference the header
		header.WriteHeader( pvSaveMem );
	}
}

ConsoleSaveFileSplit::~ConsoleSaveFileSplit()
{
	VirtualFree( pvHeap, MAX_PAGE_COUNT * CSF_PAGE_SIZE, MEM_DECOMMIT );
	pagesCommitted = 0;
	// Make sure we don't have any thumbnail data still waiting round - we can't need it now we've destroyed the save file anyway
#if defined _XBOX 
	app.GetSaveThumbnail(NULL,NULL);
#elif defined __PS3__
	app.GetSaveThumbnail(NULL,NULL, NULL,NULL);
#endif

	for(AUTO_VAR(it,regionFiles.begin()); it != regionFiles.end(); it++ )
	{
		delete it->second;
	}

	StorageManager.ResetSubfiles();
	DeleteCriticalSection(&m_lock);
}

// Add the file to our table of internal files if not already there
// Open our actual save file ready for reading/writing, and the set the file pointer to the start of this file
FileEntry *ConsoleSaveFileSplit::createFile( const ConsoleSavePath &fileName )
{
	LockSaveAccess();
	
	// Determine if the file is a region file that should be split off into its own file
	unsigned int regionFileIndex;
	bool isRegionFile = GetNumericIdentifierFromName(fileName.getName(), &regionFileIndex);
	if( isRegionFile )
	{
		// First, for backwards compatibility, check if it is already in the main file - will just use that if so
		if( !header.fileExists( fileName.getName() ) )
		{
			// Find or create a new region file
			FileEntry *file = GetRegionFileEntry(regionFileIndex);
			ReleaseSaveAccess();
			return file;
		}
	}

	FileEntry *file = header.AddFile( fileName.getName() );
	ReleaseSaveAccess();

	return file;
}

void ConsoleSaveFileSplit::deleteFile( FileEntry *file )
{
	if( file == NULL ) return;

	assert( file->isRegionFile() == false );

	LockSaveAccess();

	DWORD numberOfBytesRead = 0;
	DWORD numberOfBytesWritten = 0;

	const int bufferSize = 4096;
	int amountToRead = bufferSize;
	byte buffer[bufferSize];
	DWORD bufferDataSize = 0;


	char *readStartOffset = (char *)pvSaveMem + file->data.startOffset + file->getFileSize();

	char *writeStartOffset = (char *)pvSaveMem + file->data.startOffset;

	char *endOfDataOffset = (char *)pvSaveMem + header.GetStartOfNextData();

	while(true)
	{
		// Fill buffer from file
		if( readStartOffset + bufferSize > endOfDataOffset )
		{
			amountToRead = (int)(endOfDataOffset - readStartOffset);
		}
		else
		{
			amountToRead = bufferSize;
		}

		if( amountToRead == 0 )
			break;

		memcpy( buffer, readStartOffset, amountToRead );
		numberOfBytesRead = amountToRead;

		bufferDataSize = amountToRead;
		readStartOffset += numberOfBytesRead;

		// Write buffer to file
		memcpy( (void *)writeStartOffset, buffer, bufferDataSize );
		numberOfBytesWritten = bufferDataSize;

		writeStartOffset += numberOfBytesWritten;
	}

	header.RemoveFile( file );

	finalizeWrite();

	ReleaseSaveAccess();
}

void ConsoleSaveFileSplit::setFilePointer(FileEntry *file,LONG lDistanceToMove,PLONG lpDistanceToMoveHigh,DWORD dwMoveMethod)
{
	LockSaveAccess();

	if( file->isRegionFile() )
	{
		file->currentFilePointer = lDistanceToMove;
	}
	else
	{
		file->currentFilePointer = file->data.startOffset + lDistanceToMove;
	}

	if( dwMoveMethod == FILE_END)
	{
		file->currentFilePointer += file->getFileSize();
	}

	ReleaseSaveAccess();
}

// If this file needs to grow, move the data after along
void ConsoleSaveFileSplit::PrepareForWrite( FileEntry *file, DWORD nNumberOfBytesToWrite )
{
	int bytesToGrowBy = ( (file->currentFilePointer - file->data.startOffset) + nNumberOfBytesToWrite) - file->getFileSize();
	if( bytesToGrowBy <= 0 )
		return;

	// 4J Stu - Not forcing a minimum size, it is up to the caller to write data in sensible amounts
	// This lets us keep some of the smaller files small
	//if( bytesToGrowBy < 1024 )
	//	bytesToGrowBy = 1024;

	// Move all the data beyond us
	PIXBeginNamedEvent(0,"Growing file by %d bytes", bytesToGrowBy);
	MoveDataBeyond(file, bytesToGrowBy);
	PIXEndNamedEvent();

	// Update our length
	if( file->data.length < 0 )
		file->data.length = 0;
	file->data.length += bytesToGrowBy;

	// Write the header with the updated data
	finalizeWrite();
}

BOOL ConsoleSaveFileSplit::writeFile(FileEntry *file,LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten)
{
	assert( pvSaveMem != NULL );
	if( pvSaveMem == NULL )
	{
		return 0;
	}

	LockSaveAccess();

	if( file->isRegionFile() )
	{
		unsigned int sizeRequired = file->currentFilePointer + nNumberOfBytesToWrite;
		RegionFileReference *fileRef = regionFiles[file->data.regionIndex];
		if( sizeRequired > file->getFileSize() )
		{
			fileRef->data = (unsigned char *)realloc(fileRef->data, sizeRequired);
			file->data.length = sizeRequired;
		}

		memcpy( fileRef->data + file->currentFilePointer, lpBuffer, nNumberOfBytesToWrite );

//		app.DebugPrintf(">>>>>>>>>>>>>> writing a region file's data 0x%.8x, 0x%x offset %d of %d bytes (writing %d bytes)\n",file->data.regionIndex,fileRef->data,file->currentFilePointer, file->getFileSize(), nNumberOfBytesToWrite);

		file->currentFilePointer += nNumberOfBytesToWrite;
		file->updateLastModifiedTime();
		fileRef->dirty = true;
	}
	else
	{
		PrepareForWrite( file, nNumberOfBytesToWrite );

		char *writeStartOffset = (char *)pvSaveMem + file->currentFilePointer;
		//printf("Write: pvSaveMem = %0xd, currentFilePointer = %d, writeStartOffset = %0xd\n", pvSaveMem, file->currentFilePointer, writeStartOffset);

		memcpy( (void *)writeStartOffset, lpBuffer, nNumberOfBytesToWrite );
		*lpNumberOfBytesWritten = nNumberOfBytesToWrite;

		if(file->data.length < 0)
			file->data.length = 0;

		file->currentFilePointer += *lpNumberOfBytesWritten;

		//wprintf(L"Wrote %d bytes to %s, new file pointer is %I64d\n", *lpNumberOfBytesWritten, file->data.filename, file->currentFilePointer);

		file->updateLastModifiedTime();
	}

	ReleaseSaveAccess();

	return 1;
}

BOOL ConsoleSaveFileSplit::zeroFile(FileEntry *file, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten)
{
	assert( pvSaveMem != NULL );
	if( pvSaveMem == NULL )
	{
		return 0;
	}

	LockSaveAccess();

	if( file->isRegionFile() )
	{
		unsigned int sizeRequired = file->currentFilePointer + nNumberOfBytesToWrite;
		RegionFileReference *fileRef = regionFiles[file->data.regionIndex];
		if( sizeRequired > file->getFileSize() )
		{
			fileRef->data = (unsigned char *)realloc(fileRef->data, sizeRequired);
			file->data.length = sizeRequired;
		}

		memset( fileRef->data + file->currentFilePointer, 0, nNumberOfBytesToWrite );

//		app.DebugPrintf(">>>>>>>>>>>>>> writing a region file's data 0x%.8x, 0x%x offset %d of %d bytes (writing %d bytes)\n",file->data.regionIndex,fileRef->data,file->currentFilePointer, file->getFileSize(), nNumberOfBytesToWrite);

		file->currentFilePointer += nNumberOfBytesToWrite;
		file->updateLastModifiedTime();
		fileRef->dirty = true;
	}
	else
	{
		PrepareForWrite( file, nNumberOfBytesToWrite );

		char *writeStartOffset = (char *)pvSaveMem + file->currentFilePointer;
		//printf("Write: pvSaveMem = %0xd, currentFilePointer = %d, writeStartOffset = %0xd\n", pvSaveMem, file->currentFilePointer, writeStartOffset);

		memset( (void *)writeStartOffset, 0, nNumberOfBytesToWrite );
		*lpNumberOfBytesWritten = nNumberOfBytesToWrite;

		if(file->data.length < 0)
			file->data.length = 0;

		file->currentFilePointer += *lpNumberOfBytesWritten;

		//wprintf(L"Wrote %d bytes to %s, new file pointer is %I64d\n", *lpNumberOfBytesWritten, file->data.filename, file->currentFilePointer);

		file->updateLastModifiedTime();
	}

	ReleaseSaveAccess();

	return 1;
}

BOOL ConsoleSaveFileSplit::readFile( FileEntry *file, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead)
{
	DWORD actualBytesToRead;
	assert( pvSaveMem != NULL );
	if( pvSaveMem == NULL )
	{
		return 0;
	}

	LockSaveAccess();

	if( file->isRegionFile() )
	{
		actualBytesToRead = nNumberOfBytesToRead;
		if( file->currentFilePointer + nNumberOfBytesToRead > file->data.length )
		{
			actualBytesToRead = file->data.length - file->currentFilePointer;
		}
		RegionFileReference *fileRef = regionFiles[file->data.regionIndex];
		memcpy( lpBuffer, fileRef->data + file->currentFilePointer, actualBytesToRead );
		*lpNumberOfBytesRead = actualBytesToRead;

		file->currentFilePointer += actualBytesToRead;
	}
	else
	{
		char *readStartOffset = (char *)pvSaveMem + file->currentFilePointer;
		//printf("Read: pvSaveMem = %0xd, currentFilePointer = %d, readStartOffset = %0xd\n", pvSaveMem, file->currentFilePointer, readStartOffset);

		assert( nNumberOfBytesToRead <= file->getFileSize() );

		actualBytesToRead = nNumberOfBytesToRead;
		if( file->currentFilePointer + nNumberOfBytesToRead > file->data.startOffset + file->data.length )
		{
			actualBytesToRead = (file->data.startOffset + file->data.length) - file->currentFilePointer;
		}

		memcpy( lpBuffer, readStartOffset, actualBytesToRead );
		*lpNumberOfBytesRead = actualBytesToRead;

		file->currentFilePointer += *lpNumberOfBytesRead;

		//wprintf(L"Read %d bytes from %s, new file pointer is %I64d\n", *lpNumberOfBytesRead, file->data.filename, file->currentFilePointer);
	}



	ReleaseSaveAccess();

	return 1;
}

BOOL ConsoleSaveFileSplit::closeHandle( FileEntry *file )
{
	LockSaveAccess();
	finalizeWrite();
	ReleaseSaveAccess();

	return TRUE;
}

// In this method, attempt to write any dirty region files, subject to maintaining a maximum write output rate. Writing is prioritised by time since the region was last written.
void ConsoleSaveFileSplit::tick()
{
	int64_t currentTime = System::currentTimeMillis();

	// Don't do anything if the save system is up to something...
	if( StorageManager.GetSaveState() != C4JStorage::ESaveGame_Idle )
	{
		return;
	}

	// ...or we shouldn't be saving...
	if( StorageManager.GetSaveDisabled() )
	{
		return;
	}

	// ... or we haven't passed the required time since last assessing what to do
	if( ( currentTime - m_lastTickTime ) < WRITE_TICK_RATE_MS )
	{
		return;
	}

	LockSaveAccess();

	m_lastTickTime = currentTime;

	// Get total amount of data written over the time period we are interested in averaging over. Remove any older data.
	unsigned int bytesWritten = 0;
	for( AUTO_VAR(it, writeHistory.begin()); it != writeHistory.end(); )
	{
		if( ( currentTime - it->writeTime ) > ( WRITE_BANDWIDTH_MEASUREMENT_PERIOD_SECONDS * 1000 ) )
		{
			it = writeHistory.erase(it);
		}
		else
		{
			bytesWritten += it->writeSize;
			it++;
		}
	}

	// Compile a vector of dirty regions. 
	vector<DirtyRegionFile> dirtyRegions;
	for( AUTO_VAR(it, regionFiles.begin()); it != regionFiles.end(); it++ )
	{
		DirtyRegionFile dirtyRegion;

		if( it->second->dirty )
		{
			dirtyRegion.fileRef = it->second->fileEntry->getRegionFileIndex();
			dirtyRegion.lastWritten = it->second->lastWritten;
			dirtyRegions.push_back( dirtyRegion );
		}
	}

	// Sort into ascending order, by lastWritten time. First elements will therefore be the ones least recently saved
	std::sort( dirtyRegions.begin(), dirtyRegions.end() );

	bool writeRequired = false;
	unsigned int bytesInTimePeriod = bytesWritten;
	unsigned int bytesAddedThisTick = 0;
	for( int i = 0; i < dirtyRegions.size(); i++ )
	{
		RegionFileReference *regionRef = regionFiles[dirtyRegions[i].fileRef];
		unsigned int compressedSize = regionRef->GetCompressedSize();
		bytesInTimePeriod += compressedSize;
		bytesAddedThisTick += compressedSize;

		// Always consider at least one item for writing, even if it breaks the rule on the maximum number of bytes we would like to send per tick
		if( ( i > 0 ) && ( bytesAddedThisTick > WRITE_MAX_WRITE_PER_TICK ) )
		{
			break;
		}

		// Could we add this without breaking our bytes per second cap?
		if ( ( bytesInTimePeriod / WRITE_BANDWIDTH_MEASUREMENT_PERIOD_SECONDS ) > WRITE_BANDWIDTH_BYTESPERSECOND )
		{
			break;
		}

		// Can add for writing
		WriteHistory writeEvent;
		writeEvent.writeSize = compressedSize;
		writeEvent.writeTime = System::currentTimeMillis();
		writeHistory.push_back(writeEvent);

		regionRef->Compress();
//		app.DebugPrintf("Tick: Writing region 0x%.8x, compressed as %d bytes\n",regionRef->fileEntry->getRegionFileIndex(), regionRef->dataCompressedSize);
		StorageManager.UpdateSubfile(regionRef->index, regionRef->dataCompressed, regionRef->dataCompressedSize);
		regionRef->dirty = false;
		regionRef->lastWritten = System::currentTimeMillis();

		writeRequired = true;
	}
#ifndef _CONTENT_PACKAGE
	{
		unsigned int totalDirty = 0;
		unsigned int totalDirtyBytes = 0;
		__int64 oldestDirty = currentTime;
		for( AUTO_VAR(it, regionFiles.begin()); it != regionFiles.end(); it++ )
		{
			if( it->second->dirty )
			{
				if( it->second->lastWritten < oldestDirty )
				{
					oldestDirty = it->second->lastWritten;
				}
				totalDirty++;
				totalDirtyBytes += it->second->fileEntry->getFileSize();
			}
		}
#ifdef _DURANGO
		PIXReportCounter(L"Dirty regions", (float)totalDirty);
		PIXReportCounter(L"Dirty MB", (float)totalDirtyBytes / ( 1024 * 1024) );
		PIXReportCounter(L"Dirty oldest age", ((float) currentTime - oldestDirty ) );
		PIXReportCounter(L"Region writing bandwidth",((float)bytesInTimePeriod/ WRITE_BANDWIDTH_MEASUREMENT_PERIOD_SECONDS) / ( 1024 * 1024));
#endif
	}
#endif

	if( writeRequired )
	{
		StorageManager.SaveSubfiles(SaveRegionFilesCallback, this);
	}

	ReleaseSaveAccess();
}

void ConsoleSaveFileSplit::finalizeWrite()
{
	LockSaveAccess();
	header.WriteHeader( pvSaveMem );
	ReleaseSaveAccess();
}

void ConsoleSaveFileSplit::MoveDataBeyond(FileEntry *file, DWORD nNumberOfBytesToWrite)
{
	DWORD numberOfBytesRead = 0;
	DWORD numberOfBytesWritten = 0;

	const DWORD bufferSize = 4096;
	DWORD amountToRead = bufferSize;
	//assert( nNumberOfBytesToWrite <= bufferSize );
	static byte buffer1[bufferSize];
	static byte buffer2[bufferSize];
	DWORD buffer1Size = 0;
	DWORD buffer2Size = 0;

	// Only ReAlloc if we need to (we might already have enough) and align to 512 byte boundaries
	DWORD currentHeapSize = pagesCommitted * CSF_PAGE_SIZE;
	
	DWORD desiredSize = header.GetFileSize() + nNumberOfBytesToWrite;

	if( desiredSize > currentHeapSize )
	{
		unsigned int pagesRequired = ( desiredSize + (CSF_PAGE_SIZE - 1 ) ) / CSF_PAGE_SIZE;
		void *pvRet = VirtualAlloc(pvHeap, pagesRequired * CSF_PAGE_SIZE, COMMIT_ALLOCATION, PAGE_READWRITE);
		if( pvRet == NULL )
		{
			// Out of physical memory
			__debugbreak();
		}
		pagesCommitted = pagesRequired;
	}

	// This is the start of where we want the space to be, and the start of the data that we need to move
	char *spaceStartOffset = (char *)pvSaveMem + file->data.startOffset + file->getFileSize();

	// This is the end of where we want the space to be
	char *spaceEndOffset = spaceStartOffset + nNumberOfBytesToWrite;

	// This is the current end of the data that we want to move
	char *beginEndOfDataOffset = (char *)pvSaveMem + header.GetStartOfNextData();

	// This is where the end of the data is going to be
	char *finishEndOfDataOffset = beginEndOfDataOffset + nNumberOfBytesToWrite;

	// This is where we are going to read from (with the amount we want to read subtracted before we read)
	char *readStartOffset = beginEndOfDataOffset;

	// This is where we can safely write to (with the amount we want write subtracted before we write)
	char *writeStartOffset = finishEndOfDataOffset;

	//printf("\n******* MOVEDATABEYOND *******\n");
	//printf("Space start: %d, space end: %d\n", spaceStartOffset - (char *)pvSaveMem, spaceEndOffset - (char *)pvSaveMem);
	//printf("Current end of data: %d, new end of data: %d\n", beginEndOfDataOffset - (char *)pvSaveMem, finishEndOfDataOffset - (char *)pvSaveMem);

	// Optimisation for things that are being moved in whole region file sector (4K chunks). We could generalise this a bit more but seems safest at the moment to identify this particular type
	// of move and code explicitly for this situation
	if( ( nNumberOfBytesToWrite & 4095 ) == 0 )
	{
		if( nNumberOfBytesToWrite > 0 )
		{
			// Get addresses for start & end of the region we are copying from as uintptr_t, for easier maths
			uintptr_t uiFromStart = (uintptr_t)spaceStartOffset;
			uintptr_t uiFromEnd = (uintptr_t)beginEndOfDataOffset;

			// Round both of these values to get 4096 byte chunks that we will need to at least partially move
			uintptr_t uiFromStartChunk = uiFromStart & ~((uintptr_t)4095);
			uintptr_t uiFromEndChunk = (uiFromEnd - 1 ) & ~((uintptr_t)4095);

			// Loop through all the affected source 4096 chunks, going backwards so we don't overwrite anything we'll need in the future
			for( uintptr_t uiCurrentChunk = uiFromEndChunk; uiCurrentChunk >= uiFromStartChunk; uiCurrentChunk -= 4096 )
			{
				// Establish chunk we'll need to copy
				uintptr_t uiCopyStart = uiCurrentChunk;
				uintptr_t uiCopyEnd = uiCurrentChunk + 4096;
				// Clamp chunk to the bounds of the full region we are trying to copy
				if( uiCopyStart < uiFromStart )
				{
					// Needs to be clampged against the start of our region
					uiCopyStart = uiFromStart;
				}
				if ( uiCopyEnd > uiFromEnd )
				{
					// Needs to be clamped to the end of our region
					uiCopyEnd = uiFromEnd;					
				}
				XMemCpy( (void *)(uiCopyStart + nNumberOfBytesToWrite), ( void *)uiCopyStart, uiCopyEnd - uiCopyStart );
			}
		}
	}
	else
	{
		while(true)
		{
			// Copy buffer 1 to buffer 2
			memcpy( buffer2, buffer1, buffer1Size);
			buffer2Size = buffer1Size;

			// Fill buffer 1 from file
			if( (readStartOffset - bufferSize) < spaceStartOffset )
			{
				amountToRead = (DWORD)(readStartOffset - spaceStartOffset);
			}
			else
			{
				amountToRead = bufferSize;
			}

			// Push the read point back by the amount of bytes that we are going to read
			readStartOffset -= amountToRead;

			//printf("About to read %u from %d\n", amountToRead, readStartOffset - (char *)pvSaveMem );

			memcpy( buffer1, readStartOffset, amountToRead );
			numberOfBytesRead = amountToRead;

			buffer1Size = amountToRead;

			// Move back the write pointer by the amount of bytes we are going to write
			writeStartOffset -= buffer2Size;

			// Write buffer 2 to file
			if( (writeStartOffset + buffer2Size) <= finishEndOfDataOffset)
			{
				//printf("About to write %u to %d\n", buffer2Size, writeStartOffset - (char *)pvSaveMem );
				memcpy( (void *)writeStartOffset, buffer2, buffer2Size );
				numberOfBytesWritten = buffer2Size;
			}
			else
			{
				assert((writeStartOffset + buffer2Size) <= finishEndOfDataOffset);
				numberOfBytesWritten = 0;
			}

			if( numberOfBytesRead == 0 )
			{
				//printf("\n************** MOVE COMPLETED *************** \n\n");
				assert( writeStartOffset == spaceEndOffset );
				break;
			}
		}
	}

	header.AdjustStartOffsets( file, nNumberOfBytesToWrite );
}

// Attempt to convert a filename into a numeric identifier, which we use for region files. File names supported are of the form:
//
// Filename				Encoded as
//
// r.x.z.mcr			00 00 xx zz
// DIM-1r.x.z.mcr		00 01 xx zz
// DIM1/r.x.z.mcr		00 02 xx zz

bool ConsoleSaveFileSplit::GetNumericIdentifierFromName(const wstring &fileName, unsigned int *idOut)
{
	// Determine whether it is one of our region file names if the file extension is ".mbr"
	if( fileName.length() < 4 ) return false;
	wstring extension = fileName.substr(fileName.length()-4,4);
	if( extension != wstring(L".mcr") ) return false;

	unsigned int id = 0;
	int x, z;

	const wchar_t *cstr = fileName.c_str();
	const wchar_t *body = cstr + 2;

	// If this filename starts with a "r" then assume it is of the format "r.x.z.mcr" - don't do anything as default value we've set are correct
	if( cstr[0] != L'r' )
	{
		// Must be prefixed by "DIM-1r." or "DIM1/r."
		body = cstr + 7;
		// Differentiate between these 2 options
		if( cstr[3] == L'-' )
		{
			// "DIM-1r."
			id = 0x00010000;
		}
		else
		{
			// "DIM/1r."
			id = 0x00020000;
		}
	}
	// Get x/z coords
	swscanf_s(body, L"%d.%d.mcr", &x, &z );

	// Pack full id
	id |= ( ( x << 8 ) & 0x0000ff00 );
	id |= ( z & 0x000000ff );

	*idOut = id;

	return true;
}

// Convert a numeric file identifier (for region files) back into a normal filename. See comment above.

wstring ConsoleSaveFileSplit::GetNameFromNumericIdentifier(unsigned int idIn)
{
	wstring prefix;

	switch(idIn & 0x00ff0000 )
	{
		case 0:
			prefix = L"";
			break;
		case 1:
			prefix = L"DIM-1";
			break;
		case 2:
			prefix = L"DIM1/";
			break;
	}
	signed char regionX = ( idIn >> 8 ) & 255;
	signed char regionZ = idIn & 255;
	wstring region = ( prefix + wstring(L"r.") + _toString(regionX) + L"." + _toString(regionZ) + L".mcr" );

	return region;
}

// Compress any dirty region files, and tell the storage manager about them so that it will process them when we ask it to save sub files
void ConsoleSaveFileSplit::processSubfilesForWrite()
{
#if 0
	// 4J Stu - There are debug reasons where we want to force a save of all regions
	StorageManager.ResetSubfiles();
	for(AUTO_VAR(it,regionFiles.begin()); it != regionFiles.end(); it++ )
	{
		RegionFileReference* region = it->second;
		int index = StorageManager.AddSubfile(region->fileEntry->data.regionIndex);
		//if( region->dirty )
		{
			region->Compress();
			StorageManager.UpdateSubfile(index, region->dataCompressed, region->dataCompressedSize);
			region->dirty = false;
			region->lastWritten = System::currentTimeMillis();
		}
	}
#else
	for(AUTO_VAR(it,regionFiles.begin()); it != regionFiles.end(); it++ )
	{
		RegionFileReference* region = it->second;
		if( region->dirty )
		{
			region->Compress();
			StorageManager.UpdateSubfile(region->index, region->dataCompressed, region->dataCompressedSize);
			region->dirty = false;
			region->lastWritten = System::currentTimeMillis();
		}
	}
#endif
}

// Clean up any memory allocated for compressed data when we have finished writing
void ConsoleSaveFileSplit::processSubfilesAfterWrite()
{
	// This is called from the StorageManager.Tick() which should always be on the main thread
	for(AUTO_VAR(it,regionFiles.begin()); it != regionFiles.end(); it++ )
	{
		RegionFileReference* region = it->second;
		region->ReleaseCompressed();
	}
}

bool ConsoleSaveFileSplit::doesFileExist(ConsoleSavePath file)
{
	LockSaveAccess();
	bool exists = header.fileExists( file.getName() );
	ReleaseSaveAccess();

	return exists;
}

void ConsoleSaveFileSplit::Flush(bool autosave, bool updateThumbnail)
{
	LockSaveAccess();

#ifdef _XBOX_ONE
	MinecraftServer *server = MinecraftServer::getInstance();
#endif

	// The storage manage might potentially be busy doing a sub-file write initiated from the tick. Wait until this is totally processed.
	while( StorageManager.GetSaveState() != C4JStorage::ESaveGame_Idle )
	{
#ifdef _XBOX_ONE
		if (server && server->IsSuspending())
		{
			// If the server is mid-suspend we need to tick the storage manager ourselves
			StorageManager.Tick();
		}
#endif

		app.DebugPrintf("Flush wait\n");
		Sleep(10);
	}

	finalizeWrite();

	m_autosave = autosave;
	if(!m_autosave) processSubfilesForWrite();

	 // Get the frequency of the timer
	LARGE_INTEGER qwTicksPerSec, qwTime, qwNewTime, qwDeltaTime;
	float fElapsedTime = 0.0f;
	QueryPerformanceFrequency( &qwTicksPerSec );
	float fSecsPerTick = 1.0f / (float)qwTicksPerSec.QuadPart;

	unsigned int fileSize = header.GetFileSize();

	// Assume that the compression will make it smaller so initially attempt to allocate the current file size
	// We add 4 bytes to the start so that we can signal compressed data
	// And another 4 bytes to store the decompressed data size
	unsigned int compLength = fileSize+8;

	// 4J Stu - Added TU-1 interim

	// Attempt to allocate the required memory
	// We do not own this, it belongs to the StorageManager
	byte *compData = (byte *)StorageManager.AllocateSaveData( compLength );

	// If we failed to allocate then compData will be NULL
	// Pre-calculate the compressed data size so that we can attempt to allocate a smaller buffer
	if(compData == NULL)
	{
		// Length should be 0 here so that the compression call knows that we want to know the length back
		compLength = 0;

		// Pre-calculate the buffer size required for the compressed data
		PIXBeginNamedEvent(0,"Pre-calc save compression");
		// Save the start time
		QueryPerformanceCounter( &qwTime );
		Compression::getCompression()->Compress(NULL,&compLength,pvSaveMem,fileSize);
		QueryPerformanceCounter( &qwNewTime );

		qwDeltaTime.QuadPart = qwNewTime.QuadPart - qwTime.QuadPart;
		fElapsedTime = fSecsPerTick * ((FLOAT)(qwDeltaTime.QuadPart));

		app.DebugPrintf("Check buffer size: Elapsed time %f\n", fElapsedTime);
		PIXEndNamedEvent();

		// We add 4 bytes to the start so that we can signal compressed data
		// And another 4 bytes to store the decompressed data size
		compLength = compLength+8;

		// Attempt to allocate the required memory
		compData = (byte *)StorageManager.AllocateSaveData( compLength );
	}
	
	if(compData != NULL)
	{
		// Re-compress all save data before we save it to disk
		PIXBeginNamedEvent(0,"Actual save compression");
		// Save the start time
		QueryPerformanceCounter( &qwTime );
		Compression::getCompression()->Compress(compData+8,&compLength,pvSaveMem,fileSize);
		QueryPerformanceCounter( &qwNewTime );

		qwDeltaTime.QuadPart = qwNewTime.QuadPart - qwTime.QuadPart;
		fElapsedTime = fSecsPerTick * ((FLOAT)(qwDeltaTime.QuadPart));

		app.DebugPrintf("Compress: Elapsed time %f\n", fElapsedTime);
		PIXEndNamedEvent();

		ZeroMemory(compData,8);
		int saveVer = 0;
		memcpy( compData, &saveVer, sizeof(int) );
		memcpy( compData+4, &fileSize, sizeof(int) );

		app.DebugPrintf("Save data compressed from %d to %d\n", fileSize, compLength);

		if(updateThumbnail)
		{
			PBYTE pbThumbnailData=NULL;
			DWORD dwThumbnailDataSize=0;

			PBYTE pbDataSaveImage=NULL;
			DWORD dwDataSizeSaveImage=0;

#if ( defined _XBOX || defined _DURANGO )
			app.GetSaveThumbnail(&pbThumbnailData,&dwThumbnailDataSize);
#elif ( defined __PS3__ || defined __ORBIS__ )
			app.GetSaveThumbnail(&pbThumbnailData,&dwThumbnailDataSize,&pbDataSaveImage,&dwDataSizeSaveImage);
#endif

			BYTE bTextMetadata[88];
			ZeroMemory(bTextMetadata,88);

			__int64 seed = 0;
			bool hasSeed = false;
			if(MinecraftServer::getInstance()!= NULL && MinecraftServer::getInstance()->levels[0]!=NULL)
			{
				seed = MinecraftServer::getInstance()->levels[0]->getLevelData()->getSeed();
				hasSeed = true;
			}

			int iTextMetadataBytes = app.CreateImageTextData(bTextMetadata, seed, hasSeed, app.GetGameHostOption(eGameHostOption_All), Minecraft::GetInstance()->getCurrentTexturePackId());

			// set the icon and save image
			StorageManager.SetSaveImages(pbThumbnailData,dwThumbnailDataSize,pbDataSaveImage,dwDataSizeSaveImage,bTextMetadata,iTextMetadataBytes);
			app.DebugPrintf("Save thumbnail size %d\n",dwThumbnailDataSize);

		}
		
		INT saveOrCheckpointId = 0;
		bool validSave = StorageManager.GetSaveUniqueNumber(&saveOrCheckpointId);
		TelemetryManager->RecordLevelSaveOrCheckpoint(ProfileManager.GetPrimaryPad(), saveOrCheckpointId, compLength+8);

		// save the data
		StorageManager.SaveSaveData( &ConsoleSaveFileSplit::SaveSaveDataCallback, this );
#ifndef _CONTENT_PACKAGE
		if( app.DebugSettingsOn())
		{
			if(app.GetWriteSavesToFolderEnabled() )
			{
				DebugFlushToFile(compData, compLength+8);
			}
		}
#endif
		ReleaseSaveAccess();
	}
}

int ConsoleSaveFileSplit::SaveSaveDataCallback(LPVOID lpParam,bool bRes)
{
	ConsoleSaveFileSplit *pClass=(ConsoleSaveFileSplit *)lpParam;
	
	// Don't save sub files on autosave (their always being saved anyway)
	if (!pClass->m_autosave)
	{
		// This is called from the StorageManager.Tick() which should always be on the main thread
		StorageManager.SaveSubfiles(SaveRegionFilesCallback, pClass);
	}
	return 0;
}

int ConsoleSaveFileSplit::SaveRegionFilesCallback(LPVOID lpParam,bool bRes)
{
	ConsoleSaveFileSplit *pClass=(ConsoleSaveFileSplit *)lpParam;
	
	// This is called from the StorageManager.Tick() which should always be on the main thread
	pClass->processSubfilesAfterWrite();

	return 0;
}

#ifndef _CONTENT_PACKAGE
void ConsoleSaveFileSplit::DebugFlushToFile(void *compressedData /*= NULL*/, unsigned int compressedDataSize /*= 0*/)
{
	LockSaveAccess();

	finalizeWrite();

	unsigned int fileSize = header.GetFileSize();

	DWORD numberOfBytesWritten = 0;

	File targetFileDir(L"Saves");

	if(!targetFileDir.exists())
		targetFileDir.mkdir();

	wchar_t *fileName = new wchar_t[XCONTENT_MAX_FILENAME_LENGTH+1];

	SYSTEMTIME t;
    GetSystemTime( &t );

	//14 chars for the digits
	//11 chars for the separators + suffix
	//25 chars total
	wstring cutFileName = m_fileName;
	if(m_fileName.length() > XCONTENT_MAX_FILENAME_LENGTH - 25)
	{
		cutFileName = m_fileName.substr(0, XCONTENT_MAX_FILENAME_LENGTH - 25);
	}
	swprintf(fileName, XCONTENT_MAX_FILENAME_LENGTH+1, L"\\v%04d-%ls%02d.%02d.%02d.%02d.%02d.mcs",VER_PRODUCTBUILD,cutFileName.c_str(), t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);

#ifdef _UNICODE
	wstring wtemp = targetFileDir.getPath() + wstring(fileName);
	LPCWSTR lpFileName =  wtemp.c_str();
#else
	LPCSTR lpFileName = wstringtofilename( targetFileDir.getPath() + wstring(fileName) );
#endif

	HANDLE hSaveFile = CreateFile( lpFileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS, NULL);

	if(compressedData != NULL && compressedDataSize > 0)
	{
		WriteFile(hSaveFile,compressedData,compressedDataSize,&numberOfBytesWritten,NULL);
		assert(numberOfBytesWritten == compressedDataSize);
	}
	else
	{
		WriteFile(hSaveFile,pvSaveMem,fileSize,&numberOfBytesWritten,NULL);
		assert(numberOfBytesWritten == fileSize);
	}
	CloseHandle( hSaveFile );

	delete[] fileName;

	ReleaseSaveAccess();
}
#endif

unsigned int ConsoleSaveFileSplit::getSizeOnDisk()
{
	return header.GetFileSize();
}

wstring ConsoleSaveFileSplit::getFilename()
{
	return m_fileName;
}

vector<FileEntry *> *ConsoleSaveFileSplit::getFilesWithPrefix(const wstring &prefix)
{
	return header.getFilesWithPrefix( prefix );
}

vector<FileEntry *> *ConsoleSaveFileSplit::getRegionFilesByDimension(unsigned int dimensionIndex)
{
	vector<FileEntry *> *files = NULL;

	for( AUTO_VAR(it,regionFiles.begin()); it != regionFiles.end(); ++it )
	{
		unsigned int entryDimension = ( (it->first) >> 16) & 0xFF;

		if(entryDimension == dimensionIndex)
		{
			if( files == NULL )
			{
				files = new vector<FileEntry *>();
			}

			files->push_back(it->second->fileEntry);
		}
	}

	return files;
}

#if defined(__PS3__) || defined(__ORBIS__)
wstring ConsoleSaveFileSplit::getPlayerDataFilenameForLoad(const PlayerUID& pUID)
{
	return header.getPlayerDataFilenameForLoad( pUID );
}
wstring ConsoleSaveFileSplit::getPlayerDataFilenameForSave(const PlayerUID& pUID)
{
	return header.getPlayerDataFilenameForSave( pUID );
}
vector<FileEntry *> *ConsoleSaveFileSplit::getValidPlayerDatFiles()
{
	return header.getValidPlayerDatFiles();
}
#endif

int ConsoleSaveFileSplit::getSaveVersion()
{
	return header.getSaveVersion();
}

int ConsoleSaveFileSplit::getOriginalSaveVersion()
{
	return header.getOriginalSaveVersion();
}

void ConsoleSaveFileSplit::LockSaveAccess()
{
	EnterCriticalSection(&m_lock);
}

void ConsoleSaveFileSplit::ReleaseSaveAccess()
{
	LeaveCriticalSection(&m_lock);
}

ESavePlatform ConsoleSaveFileSplit::getSavePlatform()
{
	return header.getSavePlatform();
}

bool ConsoleSaveFileSplit::isSaveEndianDifferent()
{
	return header.isSaveEndianDifferent();
}

void ConsoleSaveFileSplit::setLocalPlatform()
{
	header.setLocalPlatform();
}

void ConsoleSaveFileSplit::setPlatform(ESavePlatform plat)
{
	header.setPlatform(plat);
}

ByteOrder ConsoleSaveFileSplit::getSaveEndian()
{
	return header.getSaveEndian();
}

ByteOrder ConsoleSaveFileSplit::getLocalEndian()
{
	return header.getLocalEndian();
}

void ConsoleSaveFileSplit::setEndian(ByteOrder endian)
{
	header.setEndian(endian);
}

void ConsoleSaveFileSplit::ConvertRegionFile(File sourceFile)
{
	DWORD numberOfBytesWritten = 0;
	DWORD numberOfBytesRead = 0;

	RegionFile sourceRegionFile(this, &sourceFile);

	for(unsigned int x = 0; x < 32; ++x)
	{
		for(unsigned int z = 0; z < 32; ++z)
		{
			DataInputStream *dis = sourceRegionFile.getChunkDataInputStream(x,z);

			if(dis)
			{
				byteArray inData(1024*1024);
				int read = dis->read(inData);
				dis->close();
				dis->deleteChildStream();
				delete dis;

				DataOutputStream *dos = sourceRegionFile.getChunkDataOutputStream(x,z);
				dos->write(inData, 0, read);


				dos->close();
				dos->deleteChildStream();
				delete dos;
				delete inData.data;

			}

		}
	}
	sourceRegionFile.writeAllOffsets(); // saves all the endian swapped offsets back out to the file (not all of these are written in the above processing).

}

void ConsoleSaveFileSplit::ConvertToLocalPlatform()
{
	if(getSavePlatform() == SAVE_FILE_PLATFORM_LOCAL)
	{
		// already in the correct format
		return;
	}
	// convert each of the region files to the local platform
	vector<FileEntry *> *allFilesInSave = getFilesWithPrefix(wstring(L""));
	for(AUTO_VAR(it, allFilesInSave->begin()); it < allFilesInSave->end(); ++it)
	{
		FileEntry *fe = *it;
		wstring fName( fe->data.filename );
		wstring suffix(L".mcr");
		if( fName.compare(fName.length() - suffix.length(), suffix.length(), suffix) == 0 )
		{
			app.DebugPrintf("Processing a region file: %ls\n",fName.c_str());
			ConvertRegionFile(File(fe->data.filename) );
		}
		else
		{
			app.DebugPrintf("%ls is not a region file, ignoring\n", fName.c_str());
		}
	}

 	setLocalPlatform(); // set the platform of this save to the local platform, now that it's been coverted
}
