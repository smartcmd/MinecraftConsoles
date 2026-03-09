#pragma once

#include "FileHeader.h"
#include "ConsoleSavePath.h"
#include "ConsoleSaveFile.h"

class ConsoleSaveFileOriginal : public ConsoleSaveFile
{
private:
	FileHeader header;

	wstring m_fileName;

//	HANDLE hHeap;
	static void *pvHeap;
	static unsigned int pagesCommitted;
#ifdef _LARGE_WORLDS
	static const unsigned int CSF_PAGE_SIZE = 64 * 1024;
	static const unsigned int MAX_PAGE_COUNT = 32 * 1024; // 2GB virtual allocation
#elif defined(__PS3__)
	static const unsigned int CSF_PAGE_SIZE = 1024 * 1024;
	static const unsigned int MAX_PAGE_COUNT = 64;
#else
	static const unsigned int CSF_PAGE_SIZE = 64 * 1024;
	static const unsigned int MAX_PAGE_COUNT = 1024;
#endif
	LPVOID pvSaveMem;

	CRITICAL_SECTION m_lock;

	void PrepareForWrite( FileEntry *file, DWORD nNumberOfBytesToWrite );
	void MoveDataBeyond(FileEntry *file, DWORD nNumberOfBytesToWrite);

public:
#if (defined __PS3__ || defined __ORBIS__ || defined __PSVITA__ || defined _DURANGO || defined _WINDOWS64)
	static int SaveSaveDataCallback(LPVOID lpParam,bool bRes);
#endif
	ConsoleSaveFileOriginal(const wstring &fileName, LPVOID pvSaveData = nullptr, DWORD fileSize = 0, bool forceCleanSave = false, ESavePlatform plat = SAVE_FILE_PLATFORM_LOCAL);
	virtual ~ConsoleSaveFileOriginal();

	// 4J Stu - Initial implementation is intended to have a similar interface to the standard Xbox file access functions

	virtual FileEntry *createFile( const ConsoleSavePath &fileName );
	virtual void deleteFile( FileEntry *file );

	virtual void setFilePointer(FileEntry *file,LONG lDistanceToMove,PLONG lpDistanceToMoveHigh,DWORD dwMoveMethod);
	virtual BOOL writeFile(	FileEntry *file, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten );
	virtual BOOL zeroFile(FileEntry *file, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten);
	virtual BOOL readFile( FileEntry *file, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead );
	virtual BOOL closeHandle( FileEntry *file );

	virtual void finalizeWrite();

	virtual bool doesFileExist(ConsoleSavePath file);

	virtual void Flush(bool autosave, bool updateThumbnail = true);

#ifndef _CONTENT_PACKAGE
	virtual void DebugFlushToFile(void *compressedData = nullptr, unsigned int compressedDataSize = 0);
#endif
	virtual unsigned int getSizeOnDisk();

	virtual wstring getFilename();

	virtual vector<FileEntry *> *getFilesWithPrefix(const wstring &prefix);
	virtual vector<FileEntry *> *getRegionFilesByDimension(unsigned int dimensionIndex);

#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	virtual wstring getPlayerDataFilenameForLoad(const PlayerUID& pUID);
	virtual wstring getPlayerDataFilenameForSave(const PlayerUID& pUID);
	virtual vector<FileEntry *> *getValidPlayerDatFiles();
#endif //__PS3__

	virtual int getSaveVersion();
	virtual int getOriginalSaveVersion();

	virtual void LockSaveAccess();
	virtual void ReleaseSaveAccess();

	virtual ESavePlatform getSavePlatform();
	virtual bool isSaveEndianDifferent();
	virtual void setLocalPlatform();
	virtual void setPlatform(ESavePlatform plat);
	virtual ByteOrder getSaveEndian();
	virtual ByteOrder getLocalEndian();
	virtual void setEndian(ByteOrder endian);
	virtual	bool isLocalEndianDifferent(ESavePlatform plat);

	virtual void ConvertRegionFile(File sourceFile);
	virtual void ConvertToLocalPlatform();

protected:
	virtual void *getWritePointer(FileEntry *file);
};