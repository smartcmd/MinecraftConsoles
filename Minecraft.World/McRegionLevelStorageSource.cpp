#include "stdafx.h"
#include "JavaMath.h"
#include "BasicTypeContainers.h"
#if 0
// 4J - not required anymore
#include "Matcher.h"
#endif
#include "ProgressListener.h"
#include "net.minecraft.world.level.chunk.storage.h"
#include "net.minecraft.world.level.chunk.h"
#include "LevelSummary.h"
#include "McRegionLevelStorage.h"
#include "File.h"
#include "LevelData.h"
#include "McRegionLevelStorageSource.h"

#include "ConsoleSaveFileIO.h"

#if 0
// 4J - not required anymore
// These were Pattern class objects, using the c++0x regex class instead
const std::tr1::wregex McRegionLevelStorageSource::FolderFilter::chunkFolderPattern = std::tr1::wregex(L"[0-9a-z]|([0-9a-z][0-9a-z])");
const std::tr1::wregex McRegionLevelStorageSource::ChunkFilter::chunkFilePattern = std::tr1::wregex(L"c\\.(-?[0-9a-z]+)\\.(-?[0-9a-z]+)\\.dat");
#endif

McRegionLevelStorageSource::McRegionLevelStorageSource(File dir) : DirectoryLevelStorageSource(dir)
{
}

wstring McRegionLevelStorageSource::getName()
{
	return L"Scaevolus' McRegion";
}

vector<LevelSummary *> *McRegionLevelStorageSource::getLevelList()
{
	// 4J Stu - We don't need to do directory lookups with the xbox save files
	vector<LevelSummary *> *levels = new vector<LevelSummary *>;
#if 0
	vector<File *> *subFolders = baseDir.listFiles();
	File *file;
	for (auto& subFolder : *subFolders)
	{
		file = subFolder; //subFolders->at(i);

		if (file->isDirectory())
		{
			continue;
		}

		wstring levelId = file->getName();

		LevelData *levelData = getDataTagFor(levelId);
		if (levelData != nullptr)
		{
			bool requiresConversion = levelData->getVersion() != McRegionLevelStorage::MCREGION_VERSION_ID;
			wstring levelName = levelData->getLevelName();

			if (levelName.empty()) // 4J Jev TODO: levelName can't be nullptr? if (levelName == nullptr || isEmpty(levelName))
			{
				levelName = levelId;
			}
			// long size = getLevelSize(folder);
            long size = 0;
			levels->push_back(new LevelSummary(levelId, levelName, levelData->getLastPlayed(), size, requiresConversion, levelData->isHardcore()));
		}
	}
#endif
	return levels;
}

void McRegionLevelStorageSource::clearAll()
{
}

shared_ptr<LevelStorage> McRegionLevelStorageSource::selectLevel(ConsoleSaveFile *saveFile, const wstring& levelId, bool createPlayerDir)
{
	//        return new LevelStorageProfilerDecorator(new McRegionLevelStorage(baseDir, levelId, createPlayerDir));
	return std::make_shared<McRegionLevelStorage>(saveFile, baseDir, levelId, createPlayerDir);
}

bool McRegionLevelStorageSource::isConvertible(ConsoleSaveFile *saveFile, const wstring& levelId)
{
	// check if there is old file format level data
	LevelData *levelData = getDataTagFor(saveFile, levelId);
	if (levelData == nullptr || levelData->getVersion() != 0)
	{
		delete levelData;
		return false;
	}
	delete levelData;

	return true;
}

bool McRegionLevelStorageSource::requiresConversion(ConsoleSaveFile *saveFile, const wstring& levelId)
{
	LevelData *levelData = getDataTagFor(saveFile, levelId);
	if (levelData == nullptr || levelData->getVersion() != 0)
	{
		delete levelData;
		return false;
	}
	delete levelData;

	return true;
}

bool McRegionLevelStorageSource::convertLevel(ConsoleSaveFile *saveFile, const wstring& levelId, ProgressListener *progress)
{
	assert(false);
		// I removed this while updating the saves to use the single save file
		// Will we ever use this convertLevel function anyway? The main issue is the check
		// for the hellFolder.exists() which would require a slight change to the way our
		// save files are structured
#if 0
	progress->progressStagePercentage(0);

	vector<ChunkFile *> *normalRegions = new vector<ChunkFile *>;
	vector<File *> *normalBaseFolders = new vector<File *>;
	vector<ChunkFile *> *netherRegions = new vector<ChunkFile *>;
	vector<File *> *netherBaseFolders = new vector<File *>;
    ArrayList<ChunkFile> enderRegions = new ArrayList<ChunkFile>();
    ArrayList<File> enderBaseFolders = new ArrayList<File>();

	//File baseFolder = File(baseDir, levelId);
	//File netherFolder = File(baseFolder, LevelStorage::HELL_FOLDER);
	//File enderFolder = new File(baseFolder, LevelStorage.ENDER_FOLDER);
	ConsoleSaveFile saveFile = ConsoleSaveFile( levelId );

	// System.out.println("Scanning folders..."); 4J Jev, TODO how do we println ?

	// find normal world
	addRegions(baseFolder, normalRegions, normalBaseFolders);

	// find hell world
	if (netherFolder.exists())
	{
		addRegions(netherFolder, netherRegions, netherBaseFolders);
	}
	if (enderFolder.exists())
	{
        addRegions(enderFolder, enderRegions, enderBaseFolders);
    }

	int totalCount = normalRegions->size() + netherRegions->size() + enderRegions.size() + normalBaseFolders->size() + netherBaseFolders->size() + enderBaseFolders.size();

	// System.out.println("Total conversion count is " + totalCount); 4J Jev, TODO

	// convert normal world
	convertRegions(baseFolder, normalRegions, 0, totalCount, progress);
	// convert hell world
	convertRegions(netherFolder, netherRegions, normalRegions->size(), totalCount, progress);
    // convert hell world
    convertRegions(enderFolder, enderRegions, normalRegions.size() + netherRegions.size(), totalCount, progress);

	LevelData *levelData = getDataTagFor(levelId);
	levelData->setVersion(McRegionLevelStorage::MCREGION_VERSION_ID);

	LevelStorage *levelStorage = selectLevel(levelId, false);
	levelStorage->saveLevelData(levelData);

	// erase old files
	eraseFolders(normalBaseFolders, normalRegions->size() + netherRegions->size(), totalCount, progress);
	if (netherFolder.exists())
	{
		eraseFolders(netherBaseFolders, normalRegions->size() + netherRegions->size() + normalBaseFolders->size(), totalCount, progress);
	}
#endif
	return true;
}

#if 0
// 4J - not required anymore
void McRegionLevelStorageSource::addRegions(File &baseFolder, vector<ChunkFile *> *dest, vector<File *> *firstLevelFolders)
{
	FolderFilter folderFilter;
	ChunkFilter chunkFilter;

	File *folder1;
	vector<File *> *folderLevel1 = baseFolder.listFiles((FileFilter *) &folderFilter);
    for (auto it = folderLevel1->begin(); it != folderLevel1->end(); it++)
    {
		folder1 = *it; //folderLevel1->at(i1);

		// keep this for the clean-up process later on
		firstLevelFolders->push_back(folder1);

		File *folder2;
		vector<File *> *folderLevel2 = folder1->listFiles(&folderFilter);
        auto itEnd2 = folderLevel2->end();
        for ( auto it2 = folderLevel2->begin(); it2 != itEnd2; it2++)
		{
			folder2 = *it2; //folderLevel2->at(i2);

			vector<File *> *chunkFiles = folder2->listFiles((FileFilter *) &chunkFilter);

			File *chunk;
            auto itEndFile = chunkFiles->end();
            for (auto itFile = chunkFiles->begin(); itFile != itEndFile; itFile++)
            {
				chunk = *itFile; //chunkFiles->at(i3);

				dest->push_back(new ChunkFile(chunk));
			}
		}
	}
}
#endif

void McRegionLevelStorageSource::convertRegions(File &baseFolder, vector<ChunkFile *> *chunkFiles, int currentCount, int totalCount, ProgressListener *progress)
{
	assert( false );

	// 4J Stu - Removed, see comment in convertLevel above
#if 0
	//Collections::sort(chunkFiles);
	std::sort( chunkFiles->begin(), chunkFiles->end() );

	byteArray buffer = byteArray(4096);

	ChunkFile *chunkFile;
	for (auto& it : *chunkFiles)
	{
		chunkFile = it; //chunkFiles->at(i1);

		//            Matcher matcher = ChunkFilter.chunkFilePattern.matcher(chunkFile.getName());
		//            if (!matcher.matches()) {
		//                continue;
		//            }
		//            int x = Integer.parseInt(matcher.group(1), 36);
		//            int z = Integer.parseInt(matcher.group(2), 36);

		int x = chunkFile->getX();
		int z = chunkFile->getZ();

		RegionFile *region = RegionFileCache::getRegionFile(baseFolder, x, z);
		if (!region->hasChunk(x & 31, z & 31))
		{
			FileInputStream fis = new BufferedInputStream(FileInputStream(*chunkFile->getFile()));
			DataInputStream istream = DataInputStream(&fis); // 4J - was new GZIPInputStream as well

			DataOutputStream *out = region->getChunkDataOutputStream(x & 31, z & 31);

			int length = 0;
			while ( (length = istream.read(buffer)) != -1 )
			{
				out->write(buffer, 0, length);
			}

			out->close();
			istream.close();

			// 4J Stu - getChunkDataOutputStream makes a new DataOutputStream that points to a new ChunkBuffer( ByteArrayOutputStream )
			// We should clean these up when we are done
			out->deleteChildStream();
			delete out;
		}

		currentCount++;
		int percent = (int) Math::round(100.0 * (double) currentCount / (double) totalCount);
		progress->progressStagePercentage(percent);
	}
	RegionFileCache::clear();
#endif

}

void McRegionLevelStorageSource::eraseFolders(vector<File *> *folders, int currentCount, int totalCount, ProgressListener *progress)
{
	for (File *folder : *folders)
	{
		vector<File *> *files = folder->listFiles();
		deleteRecursive(files);
		folder->_delete();

		currentCount++;
		int percent = static_cast<int>(Math::round(100.0 * (double)currentCount / (double)totalCount));
		progress->progressStagePercentage(percent);
	}
}

#if 0
// 4J - not required anymore
bool McRegionLevelStorageSource::FolderFilter::accept(File *file)
{
	if (file->isDirectory())
	{
		Matcher matcher( chunkFolderPattern, file->getName() );
		return matcher.matches();
	}
	return false;
}


bool McRegionLevelStorageSource::ChunkFilter::accept(File *dir, const wstring& name)
{
	Matcher matcher( chunkFilePattern, name );
	return matcher.matches();
}


McRegionLevelStorageSource::ChunkFile::ChunkFile(File *file)
{
	this->file = file;

	Matcher matcher( ChunkFilter::chunkFilePattern, file->getName() );
	if (matcher.matches())
	{
		x = Integer::parseInt(matcher.group(1), 36);
		z = Integer::parseInt(matcher.group(2), 36);
	}
	else
	{
		x = 0;
		z = 0;
	}
}

//Returns a negative integer, zero, or a positive integer as this object is less than, equal to, or greater than the specified object.
int McRegionLevelStorageSource::ChunkFile::compareTo(ChunkFile *rhs)
{
	// sort chunk files so that they are placed according to their
	// region position
	int rx = x >> 5;
	int rhsrx = rhs->x >> 5;
	if (rx == rhsrx)
	{
		int rz = z >> 5;
		int rhsrz = rhs->z >> 5;
		return rz - rhsrz;
	}

	return rx - rhsrx;
}

// 4J Stu Added so we can use std::sort instead of the java Collections::sort
// a < b
bool McRegionLevelStorageSource::ChunkFile::operator<( ChunkFile *b )
{
	return compareTo( b ) < 0;
}

File *McRegionLevelStorageSource::ChunkFile::getFile()
{
	return (File *) file;
}

int McRegionLevelStorageSource::ChunkFile::getX()
{
	return x;
}

int McRegionLevelStorageSource::ChunkFile::getZ()
{
	return z;
}
#endif