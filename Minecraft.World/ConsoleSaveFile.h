#pragma once

#include "FileHeader.h"
#include "ConsoleSavePath.h"

class ConsoleSaveFile
{
public:
	virtual ~ConsoleSaveFile() {};

	virtual FileEntry *createFile( const ConsoleSavePath &fileName ) = 0;
	virtual void deleteFile( FileEntry *file ) = 0;
	virtual void setFilePointer( FileEntry *file,LONG lDistanceToMove, PLONG lpDistanceToMoveHigh,DWORD dwMoveMethod) = 0;
	virtual BOOL writeFile(	FileEntry *file, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten) = 0;
	virtual BOOL zeroFile(FileEntry *file, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten) = 0;
	virtual BOOL readFile( FileEntry *file, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead ) = 0;
	virtual BOOL closeHandle( FileEntry *file ) = 0;
	virtual void finalizeWrite() = 0;
	virtual void tick() {};

	virtual bool doesFileExist(ConsoleSavePath file) = 0;

	virtual void Flush(bool autosave, bool updateThumbnail = true) = 0;

#ifndef _CONTENT_PACKAGE
	virtual void DebugFlushToFile(void *compressedData = nullptr, unsigned int compressedDataSize = 0) = 0;
#endif
	virtual unsigned int getSizeOnDisk() = 0;
	virtual wstring getFilename() = 0;
	virtual vector<FileEntry *> *getFilesWithPrefix(const wstring &prefix) = 0;
	virtual vector<FileEntry *> *getRegionFilesByDimension(unsigned int dimensionIndex) = 0;

#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	virtual wstring getPlayerDataFilenameForLoad(const PlayerUID& pUID) = 0;
	virtual wstring getPlayerDataFilenameForSave(const PlayerUID& pUID) = 0;
	virtual vector<FileEntry *> *getValidPlayerDatFiles() = 0;
#endif //__PS3__

	virtual int getSaveVersion() = 0;
	virtual int getOriginalSaveVersion() = 0;

	virtual void LockSaveAccess() = 0;
	virtual void ReleaseSaveAccess() = 0;

	virtual ESavePlatform getSavePlatform() = 0;
	virtual bool isSaveEndianDifferent() = 0;
	virtual void setLocalPlatform() = 0;
	virtual void setPlatform(ESavePlatform plat) = 0;
	virtual ByteOrder getSaveEndian() = 0;
	virtual ByteOrder getLocalEndian() = 0;
	virtual void setEndian(ByteOrder endian) = 0;

	virtual void ConvertRegionFile(File sourceFile) = 0;
	virtual void ConvertToLocalPlatform() = 0;

	virtual void *getWritePointer(FileEntry *file) { return nullptr; }
};
