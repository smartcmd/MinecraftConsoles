#pragma once
using namespace std;

#include "System.h"

// The first 4 bytes is the location of the header (the header itself is at the end of the file)
// Then 4 bytes for the size of the header
// Then 2 bytes for the version number at which this save was first generated
// Then 2 bytes for the version number that the save should now be at
// ( the rest of the header is actually a footer )
#define SAVE_FILE_HEADER_SIZE 12

enum ESaveVersions
{
	// Pre-release version
	SAVE_FILE_VERSION_PRE_LAUNCH = 1,

	// This is the version at which we launched the Xbox360 version
	SAVE_FILE_VERSION_LAUNCH = 2,

	// This is the version at which we had made changes that broke older saves
	SAVE_FILE_VERSION_POST_LAUNCH = 3,

	// This is the version at which we introduced the End, and any saves older than this will have their End data deleted
	SAVE_FILE_VERSION_NEW_END = 4,

	// This is the version at which we change the stronghold generation, and any saves older than this should should the original version
	SAVE_FILE_VERSION_MOVED_STRONGHOLD = 5,

	// This is the version at which we changed the playeruid format for PS3
	SAVE_FILE_VERSION_CHANGE_MAP_DATA_MAPPING_SIZE = 6,

	// This is the version at which we changed the playeruid format for Xbox One
	SAVE_FILE_VERSION_DURANGO_CHANGE_MAP_DATA_MAPPING_SIZE = 7,

	// This is the version at which we changed the chunk format to directly save the compressed storage formats
	SAVE_FILE_VERSION_COMPRESSED_CHUNK_STORAGE,

	// This is the version at which we added inhabited time to chunk (1.6.4)
	SAVE_FILE_VERSION_CHUNK_INHABITED_TIME,


	// 4J Stu - If you add a new version here, the save conversion tool will also need updated to be able to read this new format

	SAVE_FILE_VERSION_NEXT,
};

// This is the version at which we changed the playeruid format for Xbox One
#define SAVE_FILE_VERSION_DURANGO_CHANGE_MAP_DATA_MAPPING_SIZE 7

enum ESavePlatform
{
	SAVE_FILE_PLATFORM_NONE		= MAKE_FOURCC('N', 'O', 'N', 'E') ,
	SAVE_FILE_PLATFORM_X360		= MAKE_FOURCC('X', '3', '6', '0') ,
	SAVE_FILE_PLATFORM_XBONE	= MAKE_FOURCC('X', 'B', '1', '_') ,
	SAVE_FILE_PLATFORM_PS3		= MAKE_FOURCC('P', 'S', '3', '_') ,
	SAVE_FILE_PLATFORM_PS4		= MAKE_FOURCC('P', 'S', '4', '_') ,
	SAVE_FILE_PLATFORM_PSVITA	= MAKE_FOURCC('P', 'S', 'V', '_') ,
	SAVE_FILE_PLATFORM_WIN64	= MAKE_FOURCC('W', 'I', 'N', '_') ,

#if defined _XBOX
	SAVE_FILE_PLATFORM_LOCAL = SAVE_FILE_PLATFORM_X360
#elif defined _DURANGO
	SAVE_FILE_PLATFORM_LOCAL = SAVE_FILE_PLATFORM_XBONE
#elif defined __PS3__
	SAVE_FILE_PLATFORM_LOCAL = SAVE_FILE_PLATFORM_PS3
#elif defined __ORBIS__
	SAVE_FILE_PLATFORM_LOCAL = SAVE_FILE_PLATFORM_PS4
#elif defined __PSVITA__
	SAVE_FILE_PLATFORM_LOCAL = SAVE_FILE_PLATFORM_PSVITA
#elif defined _WINDOWS64
	SAVE_FILE_PLATFORM_LOCAL = SAVE_FILE_PLATFORM_WIN64
#endif
};
#define SAVE_FILE_VERSION_NUMBER (SAVE_FILE_VERSION_NEXT - 1)

struct FileEntrySaveDataV1
{
public:
	wchar_t filename[64];				// 64 * 2B
	unsigned int length; // In bytes	// 4B

	// This is only valid once the save file has been written/loaded at least once
	unsigned int startOffset;			// 4B
};

// It's important that we keep the order and size of the data here to smooth updating
// 4J Stu - As of writing the tutorial level uses a V1 save file
struct FileEntrySaveDataV2
{
public:
	wchar_t filename[64];				// 64 * 2B
	unsigned int length; // In bytes	// 4B

	union
	{
		// This is only valid once the save file has been written/loaded at least once
		unsigned int startOffset;			// 4B
		// For region files stored via ConsolveSaveFileSplit, these aren't stored within the normal save file, identified by not having a name (filename[0] is 0).
		// Note: These won't be read or written as part of a file header, and should only exist wrapped up in a FileEntry class
		unsigned int regionIndex;			// 4B

	};

	int64_t lastModifiedTime;			// 8B
};

typedef FileEntrySaveDataV2 FileEntrySaveData;

class FileEntry
{
public:
	FileEntrySaveData data;

	unsigned int currentFilePointer;

	FileEntry() { ZeroMemory(&data, sizeof(FileEntrySaveData)); }

	FileEntry( wchar_t name[64], unsigned int length, unsigned int startOffset )
	{
		data.length = length;
		data.startOffset = startOffset;
		memset( &data.filename, 0, sizeof( wchar_t ) * 64 );
		memcpy( &data.filename, name, sizeof( wchar_t ) * 64 );

		data.lastModifiedTime = 0;

		currentFilePointer = data.startOffset;
	}

	unsigned int getFileSize() { return data.length; }
	bool isRegionFile() { return data.filename[0] == 0; }				// When using ConsoleSaveFileSplit only
	unsigned int getRegionFileIndex() { return data.regionIndex; }		// When using ConsoleSaveFileSplit only

	void updateLastModifiedTime() {	data.lastModifiedTime = System::currentRealTimeMillis(); }

	/*
	Comparison function object that returns true if the first argument goes before the second argument in the specific strict weak ordering it defines, and false otherwise.
	Used in a call to std::sort in DirectoryLevelStorage.cpp
	*/
	static bool newestFirst( FileEntry *a, FileEntry *b ) { return a->data.lastModifiedTime > b->data.lastModifiedTime; }
};

// A class the represents the header of the save file
class FileHeader
{
	friend class ConsoleSaveFileOriginal;
	friend class ConsoleSaveFileSplit;
private:
	vector<FileEntry *> fileTable;
	ESavePlatform	m_savePlatform;
	ByteOrder		m_saveEndian;
#if defined(__PS3__) || defined(_XBOX)
	static const ByteOrder m_localEndian = BIGENDIAN;
#else
	static const ByteOrder m_localEndian = LITTLEENDIAN;
#endif

	short m_saveVersion;
	short m_originalSaveVersion;

public:
	FileEntry *lastFile;

public:
	FileHeader();
	~FileHeader();

protected:
	FileEntry *AddFile( const wstring &name, unsigned int length = 0 );
	void RemoveFile( FileEntry * );
	void WriteHeader( LPVOID saveMem );
	void ReadHeader( LPVOID saveMem, ESavePlatform plat = SAVE_FILE_PLATFORM_LOCAL );

	unsigned int GetStartOfNextData();

	unsigned int GetFileSize();

	void AdjustStartOffsets(FileEntry *file, DWORD nNumberOfBytesToWrite, bool subtract = false);

	bool fileExists( const wstring &name );

	vector<FileEntry *> *getFilesWithPrefix(const wstring &prefix);

	vector<FileEntry *> *getValidPlayerDatFiles();

#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	wstring getPlayerDataFilenameForLoad(const PlayerUID& pUID);
	wstring getPlayerDataFilenameForSave(const PlayerUID& pUID);
	vector<FileEntry *> *getDatFilesWithOnlineID(const PlayerUID& pUID);
	vector<FileEntry *> *getDatFilesWithMacAndUserID(const PlayerUID& pUID);
	vector<FileEntry *> *getDatFilesWithPrimaryUser();
#endif

	void setSaveVersion(int version) { m_saveVersion = version; }
	int getSaveVersion() { return m_saveVersion; }
	void setOriginalSaveVersion(int version) { m_originalSaveVersion = version; }
	int getOriginalSaveVersion() { return m_originalSaveVersion; }
	ESavePlatform getSavePlatform() { return m_savePlatform; }
	void setPlatform(ESavePlatform plat) { m_savePlatform = plat; }
	bool isSaveEndianDifferent() { return m_saveEndian != m_localEndian; }
	void setLocalPlatform() { m_savePlatform = SAVE_FILE_PLATFORM_LOCAL; m_saveEndian = m_localEndian; }
	ByteOrder getSaveEndian() { return m_saveEndian; }
	static ByteOrder getLocalEndian() { return m_localEndian; }
	void setEndian(ByteOrder endian) { m_saveEndian = endian; }
	static ByteOrder getEndian(ESavePlatform plat);
	bool isLocalEndianDifferent(ESavePlatform plat){return m_localEndian != getEndian(plat); }

};
