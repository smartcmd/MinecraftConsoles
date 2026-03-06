#include "stdafx.h"
#include "StringHelpers.h"
#include "ConsoleSaveFileOriginal.h"
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


#ifdef _XBOX
#define RESERVE_ALLOCATION  MEM_RESERVE | MEM_LARGE_PAGES
#define COMMIT_ALLOCATION  MEM_COMMIT | MEM_LARGE_PAGES
#else
#define RESERVE_ALLOCATION  MEM_RESERVE
#define COMMIT_ALLOCATION  MEM_COMMIT
#endif

unsigned int ConsoleSaveFileOriginal::pagesCommitted = 0;
void *ConsoleSaveFileOriginal::pvHeap = NULL;

ConsoleSaveFileOriginal::ConsoleSaveFileOriginal(const wstring &fileName, LPVOID pvSaveData /*= NULL*/, DWORD dFileSize /*= 0*/, bool forceCleanSave /*= false*/, ESavePlatform plat /*= SAVE_FILE_PLATFORM_LOCAL*/)
{
	InitializeCriticalSectionAndSpinCount(&m_lock,5120);

	// One time initialise of static stuff required for our storage
	if( pvHeap == NULL )
	{
		// Reserve a chunk of 64MB of virtual address space for our saves, using 64KB pages.
		// We'll only be committing these as required to grow the storage we need, which will
		// the storage to grow without having to use realloc.

		// AP - The Vita doesn't have virtual memory so a pretend system has been implemented in PSVitaStubs.cpp. 
		// All access to the memory must be done via the access function as the pointer returned from VirtualAlloc
		// can't be used directly.
		pvHeap = VirtualAlloc(NULL, MAX_PAGE_COUNT * CSF_PAGE_SIZE, RESERVE_ALLOCATION, PAGE_READWRITE );
	}

	pvSaveMem = pvHeap;
	m_fileName = fileName;

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
		bool AllocData = false;
		if(pvSaveData != NULL)
		{
#ifdef __PSVITA__
			// AP - use this to access the virtual memory
			VirtualCopyTo(pvSaveMem, pvSaveData, fileSize);
#else
			memcpy(pvSaveMem, pvSaveData, fileSize);
			if(bLevelGenBaseSave)
			{
				levelGen->deleteBaseSaveData();
			}
#endif
		}
		else
		{
			unsigned int storageLength;
#ifdef __PSVITA__
			// create a buffer to hold the compressed data
			pvSaveData = malloc(fileSize);
			AllocData = true;
			StorageManager.GetSaveData( pvSaveData, &storageLength );
#else
			StorageManager.GetSaveData( pvSaveMem, &storageLength );
#endif
#ifdef __PS3__
			StorageManager.FreeSaveData();
#endif
			app.DebugPrintf("Filesize - %d, Adjusted size - %d\n",fileSize,storageLength);
			fileSize = storageLength;
		} 

#ifdef __PSVITA__
		if(plat == SAVE_FILE_PLATFORM_PSVITA)
		{
			// AP - decompress via the access function. This uses a special RLE format
			VirtualDecompress((unsigned char *)pvSaveData+8, fileSize-8 );
			if( AllocData )
			{
				// free the compressed data buffer if required
				free( pvSaveData );
			}
			else if(bLevelGenBaseSave)
			{
				levelGen->deleteBaseSaveData();
			}
		}
		else
#endif
		{
#ifdef __PSVITA__
			void* pvSourceData = pvSaveData;
#else
			void* pvSourceData = pvSaveMem;
#endif
			int compressed = *(int*)pvSourceData;
			if( compressed == 0 )
			{
				unsigned int decompSize = *( (int*)pvSourceData+1 );
				if(isLocalEndianDifferent(plat)) System::ReverseULONG(&decompSize);

				// An invalid save, so clear the memory and start from scratch
				if(decompSize == 0)
				{
					// 4J Stu - Saves created between 2/12/2011 and 7/12/2011 will have this problem
					app.DebugPrintf("Invalid save data format\n");
					ZeroMemory( pvSourceData, fileSize );
					// Clear the first 8 bytes that reference the header
					header.WriteHeader( pvSourceData );
				}
				else
				{
					unsigned char *buf = new unsigned char[decompSize];
#ifndef _XBOX
					if(plat == SAVE_FILE_PLATFORM_PSVITA)
					{
						Compression::VitaVirtualDecompress(buf, &decompSize, (unsigned char *)pvSourceData+8, fileSize-8 );
					}
					else
#endif
					{
						Compression::getCompression()->SetDecompressionType(plat); // if this save is from another platform, set the correct decompression type
						Compression::getCompression()->Decompress(buf, &decompSize, (unsigned char *)pvSourceData+8, fileSize-8 );
						Compression::getCompression()->SetDecompressionType(SAVE_FILE_PLATFORM_LOCAL); // and then set the decompression back to the local machine's standard type
					}

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
#ifdef __PSVITA__
					VirtualCopyTo(pvSaveMem, buf, decompSize);
#else
					memcpy(pvSaveMem, buf, decompSize);
#endif
					delete[] buf;
				}
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

ConsoleSaveFileOriginal::~ConsoleSaveFileOriginal()
{
	VirtualFree( pvHeap, MAX_PAGE_COUNT * CSF_PAGE_SIZE, MEM_DECOMMIT );
	pagesCommitted = 0;
	// Make sure we don't have any thumbnail data still waiting round - we can't need it now we've destroyed the save file anyway
#if defined _XBOX 
	app.GetSaveThumbnail(NULL,NULL);
#elif defined __PS3__
	app.GetSaveThumbnail(NULL,NULL, NULL,NULL);
#endif

	DeleteCriticalSection(&m_lock);
}

// Add the file to our table of internal files if not already there
// Open our actual save file ready for reading/writing, and the set the file pointer to the start of this file
FileEntry *ConsoleSaveFileOriginal::createFile( const ConsoleSavePath &fileName )
{
	LockSaveAccess();
	FileEntry *file = header.AddFile( fileName.getName() );
	ReleaseSaveAccess();

	return file;
}

void ConsoleSaveFileOriginal::deleteFile( FileEntry *file )
{
	if( file == NULL ) return;

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

#ifdef __PSVITA__
		// AP - use this to access the virtual memory
		VirtualCopyFrom( buffer, readStartOffset, amountToRead );
#else
		memcpy( buffer, readStartOffset, amountToRead );
#endif
		numberOfBytesRead = amountToRead;

		bufferDataSize = amountToRead;
		readStartOffset += numberOfBytesRead;

		// Write buffer to file
#ifdef __PSVITA__
		// AP - use this to access the virtual memory
		VirtualCopyTo( (void *)writeStartOffset, buffer, bufferDataSize );
#else
		memcpy( (void *)writeStartOffset, buffer, bufferDataSize );
#endif
		numberOfBytesWritten = bufferDataSize;

		writeStartOffset += numberOfBytesWritten;
	}

	header.RemoveFile( file );

	finalizeWrite();

	ReleaseSaveAccess();
}

void ConsoleSaveFileOriginal::setFilePointer(FileEntry *file,LONG lDistanceToMove,PLONG lpDistanceToMoveHigh,DWORD dwMoveMethod)
{
	LockSaveAccess();

	file->currentFilePointer = file->data.startOffset + lDistanceToMove;

	if( dwMoveMethod == FILE_END)
	{
		file->currentFilePointer += file->getFileSize();
	}

	ReleaseSaveAccess();
}

// If this file needs to grow, move the data after along
void ConsoleSaveFileOriginal::PrepareForWrite( FileEntry *file, DWORD nNumberOfBytesToWrite )
{
	int bytesToGrowBy = ( (file->currentFilePointer - file->data.startOffset) + nNumberOfBytesToWrite) - file->getFileSize();
	if( bytesToGrowBy <= 0 )
		return;

	// 4J Stu - Not forcing a minimum size, it is up to the caller to write data in sensible amounts
	// This lets us keep some of the smaller files small
	//if( bytesToGrowBy < 1024 )
	//	bytesToGrowBy = 1024;

	// Move all the data beyond us
	MoveDataBeyond(file, bytesToGrowBy);

	// Update our length
	if( file->data.length < 0 )
		file->data.length = 0;
	file->data.length += bytesToGrowBy;

	// Write the header with the updated data
	finalizeWrite();
}

BOOL ConsoleSaveFileOriginal::writeFile(FileEntry *file,LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten)
{
	assert( pvSaveMem != NULL );
	if( pvSaveMem == NULL )
	{
		return 0;
	}

	LockSaveAccess();

	PrepareForWrite( file, nNumberOfBytesToWrite );

	char *writeStartOffset = (char *)pvSaveMem + file->currentFilePointer;
	//printf("Write: pvSaveMem = %0xd, currentFilePointer = %d, writeStartOffset = %0xd\n", pvSaveMem, file->currentFilePointer, writeStartOffset);

#ifdef __PSVITA__
	// AP - use this to access the virtual memory
	VirtualCopyTo((void *)writeStartOffset, (void*)lpBuffer, nNumberOfBytesToWrite);
#else
	memcpy( (void *)writeStartOffset, lpBuffer, nNumberOfBytesToWrite );
#endif
	*lpNumberOfBytesWritten = nNumberOfBytesToWrite;

	if(file->data.length < 0)
		file->data.length = 0;

	file->currentFilePointer += *lpNumberOfBytesWritten;

	//wprintf(L"Wrote %d bytes to %s, new file pointer is %I64d\n", *lpNumberOfBytesWritten, file->data.filename, file->currentFilePointer);

	file->updateLastModifiedTime();

	ReleaseSaveAccess();

	return 1;
}

BOOL ConsoleSaveFileOriginal::zeroFile(FileEntry *file, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten)
{
	assert( pvSaveMem != NULL );
	if( pvSaveMem == NULL )
	{
		return 0;
	}

	LockSaveAccess();

	PrepareForWrite( file, nNumberOfBytesToWrite );

	char *writeStartOffset = (char *)pvSaveMem + file->currentFilePointer;
	//printf("Write: pvSaveMem = %0xd, currentFilePointer = %d, writeStartOffset = %0xd\n", pvSaveMem, file->currentFilePointer, writeStartOffset);

#ifdef __PSVITA__
	// AP - use this to access the virtual memory
	VirtualMemset( (void *)writeStartOffset, 0, nNumberOfBytesToWrite );
#else
	memset( (void *)writeStartOffset, 0, nNumberOfBytesToWrite );
#endif
	*lpNumberOfBytesWritten = nNumberOfBytesToWrite;

	if(file->data.length < 0)
		file->data.length = 0;

	file->currentFilePointer += *lpNumberOfBytesWritten;

	//wprintf(L"Wrote %d bytes to %s, new file pointer is %I64d\n", *lpNumberOfBytesWritten, file->data.filename, file->currentFilePointer);

	file->updateLastModifiedTime();

	ReleaseSaveAccess();

	return 1;
}

BOOL ConsoleSaveFileOriginal::readFile( FileEntry *file, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead)
{
	DWORD actualBytesToRead;
	assert( pvSaveMem != NULL );
	if( pvSaveMem == NULL )
	{
		return 0;
	}

	LockSaveAccess();

	char *readStartOffset = (char *)pvSaveMem + file->currentFilePointer;
	//printf("Read: pvSaveMem = %0xd, currentFilePointer = %d, readStartOffset = %0xd\n", pvSaveMem, file->currentFilePointer, readStartOffset);

	assert( nNumberOfBytesToRead <= file->getFileSize() );

	actualBytesToRead = nNumberOfBytesToRead;
	if( file->currentFilePointer + nNumberOfBytesToRead > file->data.startOffset + file->data.length )
	{
		actualBytesToRead = (file->data.startOffset + file->data.length) - file->currentFilePointer;
	}

#ifdef __PSVITA__
	// AP - use this to access the virtual memory
	VirtualCopyFrom( lpBuffer, readStartOffset, actualBytesToRead );
#else
	memcpy( lpBuffer, readStartOffset, actualBytesToRead );
#endif

	*lpNumberOfBytesRead = actualBytesToRead;

	file->currentFilePointer += *lpNumberOfBytesRead;

	//wprintf(L"Read %d bytes from %s, new file pointer is %I64d\n", *lpNumberOfBytesRead, file->data.filename, file->currentFilePointer);

	ReleaseSaveAccess();

	return 1;
}

BOOL ConsoleSaveFileOriginal::closeHandle( FileEntry *file )
{
	LockSaveAccess();
	finalizeWrite();
	ReleaseSaveAccess();

	return TRUE;
}

void ConsoleSaveFileOriginal::finalizeWrite()
{
	LockSaveAccess();
	header.WriteHeader( pvSaveMem );
	ReleaseSaveAccess();
}

void ConsoleSaveFileOriginal::MoveDataBeyond(FileEntry *file, DWORD nNumberOfBytesToWrite)
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
#ifdef __PSVITA__
				// AP - use this to access the virtual memory
				VirtualMove( (void *)(uiCopyStart + nNumberOfBytesToWrite), ( void *)uiCopyStart, uiCopyEnd - uiCopyStart);
#else
				XMemCpy( (void *)(uiCopyStart + nNumberOfBytesToWrite), ( void *)uiCopyStart, uiCopyEnd - uiCopyStart );
#endif
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
#ifdef __PSVITA__
			// AP - use this to access the virtual memory
			VirtualCopyFrom(buffer1, readStartOffset, amountToRead);
#else
			memcpy( buffer1, readStartOffset, amountToRead );
#endif
			numberOfBytesRead = amountToRead;

			buffer1Size = amountToRead;

			// Move back the write pointer by the amount of bytes we are going to write
			writeStartOffset -= buffer2Size;

			// Write buffer 2 to file
			if( (writeStartOffset + buffer2Size) <= finishEndOfDataOffset)
			{
				//printf("About to write %u to %d\n", buffer2Size, writeStartOffset - (char *)pvSaveMem );
#ifdef __PSVITA__
				// AP - use this to access the virtual memory
				VirtualCopyTo((void *)writeStartOffset, buffer2, buffer2Size);
#else
				memcpy( (void *)writeStartOffset, buffer2, buffer2Size );
#endif
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

bool ConsoleSaveFileOriginal::doesFileExist(ConsoleSavePath file)
{
	LockSaveAccess();
	bool exists = header.fileExists( file.getName() );
	ReleaseSaveAccess();

	return exists;
}

void ConsoleSaveFileOriginal::Flush(bool autosave, bool updateThumbnail )
{
	LockSaveAccess();

#ifdef __PSVITA__
	// On Vita we've had problems with saves being corrupted on rapid save/save-exiting so seems prudent to wait for idle
	while( StorageManager.GetSaveState() != C4JStorage::ESaveGame_Idle )
	{
		app.DebugPrintf("Flush wait\n");
		Sleep(10);
	}
#endif

	finalizeWrite();

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

#ifdef __PS3__
	// On PS3, don't compress the data as we can't really afford the extra memory this requires for the output buffer. Instead we'll be writing
	// directly from the save data.
	StorageManager.SetSaveData(pvSaveMem,fileSize);
	byte *compData = (byte *)pvSaveMem;
#else
	// Attempt to allocate the required memory
	// We do not own this, it belongs to the StorageManager
	byte *compData = (byte *)StorageManager.AllocateSaveData( compLength );

#ifdef __PSVITA__
	// AP - make sure we always allocate just what is needed so it will only SAVE what is needed.
	// If we don't do this the StorageManager will save a file of uncompressed size unnecessarily.
	compData = NULL;
#endif

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
#ifdef __PSVITA__
		// AP - get the compressed size via the access function. This uses a special RLE format
		VirtualCompress(NULL,&compLength,pvSaveMem,fileSize);
#else
		Compression::getCompression()->Compress(NULL,&compLength,pvSaveMem,fileSize);
#endif
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
#endif

	if(compData != NULL)
	{
		// No compression on PS3 - see comment above
#ifndef __PS3__
		// Re-compress all save data before we save it to disk
		PIXBeginNamedEvent(0,"Actual save compression");
		// Save the start time
		QueryPerformanceCounter( &qwTime );
#ifdef __PSVITA__
		// AP - compress via the access function. This uses a special RLE format
		VirtualCompress(compData+8,&compLength,pvSaveMem,fileSize);
#else
		Compression::getCompression()->Compress(compData+8,&compLength,pvSaveMem,fileSize);
#endif
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
#endif

		PBYTE pbThumbnailData=NULL;
		DWORD dwThumbnailDataSize=0;

		PBYTE pbDataSaveImage=NULL;
		DWORD dwDataSizeSaveImage=0;

#if ( defined _XBOX || defined _DURANGO || defined _WINDOWS64 )
		app.GetSaveThumbnail(&pbThumbnailData,&dwThumbnailDataSize);
#elif ( defined __PS3__ || defined __ORBIS__ || defined __PSVITA__ )
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

		INT saveOrCheckpointId = 0;
		bool validSave = StorageManager.GetSaveUniqueNumber(&saveOrCheckpointId);
		TelemetryManager->RecordLevelSaveOrCheckpoint(ProfileManager.GetPrimaryPad(), saveOrCheckpointId, compLength+8);

#ifdef _XBOX
		StorageManager.SaveSaveData( compLength+8,pbThumbnailData,dwThumbnailDataSize,bTextMetadata,iTextMetadataBytes );
		delete [] pbThumbnailData;	
#ifndef _CONTENT_PACKAGE
		if( app.DebugSettingsOn())
		{
			if(app.GetWriteSavesToFolderEnabled() )
			{
				DebugFlushToFile(compData, compLength+8);
			}
		}
#endif
	}
	else
	{
		// We have failed to allocate the memory required to save this file. Now what?
	}

	ReleaseSaveAccess();
#elif (defined __PS3__ || defined __ORBIS__ || defined __PSVITA__ || defined _DURANGO || defined _WINDOWS64)
		// set the icon and save image
		StorageManager.SetSaveImages(pbThumbnailData,dwThumbnailDataSize,pbDataSaveImage,dwDataSizeSaveImage,bTextMetadata,iTextMetadataBytes);
		app.DebugPrintf("Save thumbnail size %d\n",dwThumbnailDataSize);

		// save the data
		StorageManager.SaveSaveData( &ConsoleSaveFileOriginal::SaveSaveDataCallback, this );
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
#else
	}
	ReleaseSaveAccess();
#endif
}

#if (defined __PS3__ || defined __ORBIS__ || defined __PSVITA__ || defined _DURANGO || defined _WINDOWS64)

int ConsoleSaveFileOriginal::SaveSaveDataCallback(LPVOID lpParam,bool bRes)
{
	ConsoleSaveFile *pClass=(ConsoleSaveFile *)lpParam;

	return 0;
}

#endif

#ifndef _CONTENT_PACKAGE
void ConsoleSaveFileOriginal::DebugFlushToFile(void *compressedData /*= NULL*/, unsigned int compressedDataSize /*= 0*/)
{
	LockSaveAccess();

	finalizeWrite();

	unsigned int fileSize = header.GetFileSize();

	DWORD numberOfBytesWritten = 0;
#ifdef _XBOX
	File targetFileDir(L"GAME:\\Saves");
#else
	File targetFileDir(L"Saves");
#endif // _XBOX

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
#ifndef __PSVITA__
	HANDLE hSaveFile = CreateFile( lpFileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS, NULL);
#endif

	if(compressedData != NULL && compressedDataSize > 0)
	{
#ifdef __PSVITA__
		// AP - Use the access function to save
		VirtualWriteFile( lpFileName, compressedData, compressedDataSize, &numberOfBytesWritten, NULL);
#else
		WriteFile( hSaveFile,compressedData,compressedDataSize,&numberOfBytesWritten,NULL);
#endif
		assert(numberOfBytesWritten == compressedDataSize);
	}
	else
	{
#ifdef __PSVITA__
		// AP - Use the access function to save
		VirtualWriteFile( lpFileName, compressedData, compressedDataSize, &numberOfBytesWritten, NULL);
#else
		WriteFile(hSaveFile,pvSaveMem,fileSize,&numberOfBytesWritten,NULL);
#endif
		assert(numberOfBytesWritten == fileSize);
	}
#ifndef __PSVITA__
	CloseHandle( hSaveFile );
#endif

	delete[] fileName;

	ReleaseSaveAccess();
}
#endif

unsigned int ConsoleSaveFileOriginal::getSizeOnDisk()
{
	return header.GetFileSize();
}

wstring ConsoleSaveFileOriginal::getFilename()
{
	return m_fileName;
}

vector<FileEntry *> *ConsoleSaveFileOriginal::getFilesWithPrefix(const wstring &prefix)
{
	return header.getFilesWithPrefix( prefix );
}

vector<FileEntry *> *ConsoleSaveFileOriginal::getRegionFilesByDimension(unsigned int dimensionIndex)
{
	return NULL;
}

#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
wstring ConsoleSaveFileOriginal::getPlayerDataFilenameForLoad(const PlayerUID& pUID)
{
	return header.getPlayerDataFilenameForLoad( pUID );
}
wstring ConsoleSaveFileOriginal::getPlayerDataFilenameForSave(const PlayerUID& pUID)
{
	return header.getPlayerDataFilenameForSave( pUID );
}
vector<FileEntry *> *ConsoleSaveFileOriginal::getValidPlayerDatFiles()
{
	return header.getValidPlayerDatFiles();
}
#endif

int ConsoleSaveFileOriginal::getSaveVersion()
{
	return header.getSaveVersion();
}

int ConsoleSaveFileOriginal::getOriginalSaveVersion()
{
	return header.getOriginalSaveVersion();
}

void ConsoleSaveFileOriginal::LockSaveAccess()
{
	EnterCriticalSection(&m_lock);
}

void ConsoleSaveFileOriginal::ReleaseSaveAccess()
{
	LeaveCriticalSection(&m_lock);
}

ESavePlatform ConsoleSaveFileOriginal::getSavePlatform()
{
	return header.getSavePlatform();
}

bool ConsoleSaveFileOriginal::isSaveEndianDifferent()
{
	return header.isSaveEndianDifferent();
}

void ConsoleSaveFileOriginal::setLocalPlatform()
{
	header.setLocalPlatform();
}

void ConsoleSaveFileOriginal::setPlatform(ESavePlatform plat)
{
	header.setPlatform(plat);
}

ByteOrder ConsoleSaveFileOriginal::getSaveEndian()
{
	return header.getSaveEndian();
}

ByteOrder ConsoleSaveFileOriginal::getLocalEndian()
{
	return header.getLocalEndian();
}

void ConsoleSaveFileOriginal::setEndian(ByteOrder endian)
{
	header.setEndian(endian);
}

bool ConsoleSaveFileOriginal::isLocalEndianDifferent( ESavePlatform plat )
{
	return getLocalEndian() != header.getEndian(plat);
}


void ConsoleSaveFileOriginal::ConvertRegionFile(File sourceFile)
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

void ConsoleSaveFileOriginal::ConvertToLocalPlatform()
{
	if(getSavePlatform() == SAVE_FILE_PLATFORM_LOCAL)
	{
		// already in the correct format
		return;
	}
	// convert each of the region files to the local platform
	vector<FileEntry *> *allFilesInSave = getFilesWithPrefix(wstring(L""));
	if ( allFilesInSave )
	{
		for (FileEntry* fe : *allFilesInSave)
		{
			wstring fName(fe->data.filename);
			wstring suffix(L".mcr");
			if (fName.compare(fName.length() - suffix.length(), suffix.length(), suffix) == 0)
			{
				app.DebugPrintf("Processing a region file: %ls\n", fName.c_str());
				ConvertRegionFile(File(fe->data.filename));
			}
			else
			{
				app.DebugPrintf("%ls is not a region file, ignoring\n", fName.c_str());
			}
		}
	}

	setLocalPlatform(); // set the platform of this save to the local platform, now that it's been coverted
}

void *ConsoleSaveFileOriginal::getWritePointer(FileEntry *file)
{
	return (char *)pvSaveMem + file->currentFilePointer;;
}
