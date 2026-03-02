#pragma once
using namespace std;

#include "DirectoryLevelStorageSource.h"
#include "FileFilter.h"
#include "FilenameFilter.h"

class ProgressListener;
class LevelStorage;

class McRegionLevelStorageSource : public DirectoryLevelStorageSource
{
public:
	class ChunkFile;

	McRegionLevelStorageSource(File dir);
    virtual wstring getName();
    virtual vector<LevelSummary *> *getLevelList();
    virtual void clearAll();
    virtual shared_ptr<LevelStorage> selectLevel(ConsoleSaveFile *saveFile, const wstring& levelId, bool createPlayerDir);
    virtual bool isConvertible(ConsoleSaveFile *saveFile, const wstring& levelId);
    virtual bool requiresConversion(ConsoleSaveFile *saveFile, const wstring& levelId);
    virtual bool convertLevel(ConsoleSaveFile *saveFile, const wstring& levelId, ProgressListener *progress);

private:
#if 0
	// 4J - not required anymore
	void addRegions(File &baseFolder, vector<ChunkFile *> *dest, vector<File *> *firstLevelFolders);
#endif
    void convertRegions(File &baseFolder, vector<ChunkFile *> *chunkFiles, int currentCount, int totalCount, ProgressListener *progress);
    void eraseFolders(vector<File *> *folders, int currentCount, int totalCount, ProgressListener *progress);

public:
#if 0
	// 4J - not required anymore
    static class FolderFilter : public FileFilter 
	{
	public:
		static const std::tr1::wregex chunkFolderPattern; // was Pattern
		bool accept(File *file);
    };

    static class ChunkFilter : public FilenameFilter 
	{
	public:
		static const std::tr1::wregex chunkFilePattern; // was Pattern
        bool accept(File *dir, const wstring& name);
    };

    static class ChunkFile // implements Comparable<ChunkFile>
	{
	private:
		/* const */ File *file;
		/* const */ int x;
		/* const */ int z;

	public:
		ChunkFile(File *file);
		int compareTo(ChunkFile *rhs);
        File *getFile();
        int getX();
        int getZ();

		// a < b
		bool operator<( ChunkFile *b );
    };
#endif
};
