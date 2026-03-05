#include "stdafx.h"
#include "File.h"
#include "LevelData.h"
#include "LevelSummary.h"
#include "com.mojang.nbt.h"
#include "DirectoryLevelStorage.h"
#include "DirectoryLevelStorageSource.h"

#include "ConsoleSaveFileIO.h"
#include "ConsoleSaveFileOriginal.h"

class LevelStorage;

DirectoryLevelStorageSource::DirectoryLevelStorageSource(const File dir) : baseDir( dir )
{
	//if (!dir.exists()) dir.mkdirs(); // 4J Removed
	//this->baseDir = dir;
}

wstring DirectoryLevelStorageSource::getName()
{
	return L"Old Format";
}

vector<LevelSummary *> *DirectoryLevelStorageSource::getLevelList()
{
	// 4J Stu - We don't use directory list with the Xbox save locations
	vector<LevelSummary *> *levels = new vector<LevelSummary *>;
#if 0
	for (int i = 0; i < 5; i++)
	{
		wstring levelId = wstring(L"World").append( std::to_wstring( (i+1) ) );

		LevelData *levelData = getDataTagFor(saveFile, levelId);
		if (levelData != NULL)
		{
			levels->push_back(new LevelSummary(levelId, L"", levelData->getLastPlayed(), levelData->getSizeOnDisk(), levelData.getGameType(), false, levelData->isHardcore()));
		}
	}
#endif
	return levels;
}

void DirectoryLevelStorageSource::clearAll()
{
}

LevelData *DirectoryLevelStorageSource::getDataTagFor(ConsoleSaveFile *saveFile, const wstring& levelId)
{
	//File dataFile(dir, L"level.dat");
	ConsoleSavePath dataFile = ConsoleSavePath( wstring( L"level.dat" ) );
	if ( saveFile->doesFileExist( dataFile ) )
	{
		ConsoleSaveFileInputStream fis = ConsoleSaveFileInputStream(saveFile, dataFile);
		CompoundTag *root = NbtIo::readCompressed(&fis);
		CompoundTag *tag = root->getCompound(L"Data");
		LevelData *ret = new LevelData(tag);
		delete root;
		return ret;
	}

	return NULL;
}

void DirectoryLevelStorageSource::renameLevel(const wstring& levelId, const wstring& newLevelName)
{
	ConsoleSaveFileOriginal tempSave(levelId);

	//File dataFile = File(dir, L"level.dat");
	ConsoleSavePath dataFile = ConsoleSavePath( wstring( L"level.dat" ) );
	if ( tempSave.doesFileExist( dataFile ) )
	{
		ConsoleSaveFileInputStream fis = ConsoleSaveFileInputStream(&tempSave, dataFile);
		CompoundTag *root = NbtIo::readCompressed(&fis);
		CompoundTag *tag = root->getCompound(L"Data");
		tag->putString(L"LevelName", newLevelName);

		ConsoleSaveFileOutputStream fos = ConsoleSaveFileOutputStream(&tempSave, dataFile);
		NbtIo::writeCompressed(root, &fos);
	}
}

bool DirectoryLevelStorageSource::isNewLevelIdAcceptable(const wstring& levelId)
{
	// 4J Jev, removed try/catch.

	File levelFolder = File(baseDir, levelId);
	if (levelFolder.exists())
	{
		return false;
	}

	levelFolder.mkdir();

	return true;
}

void DirectoryLevelStorageSource::deleteLevel(const wstring& levelId)
{
	File dir = File(baseDir, levelId);
	if (!dir.exists()) return;

	deleteRecursive(dir.listFiles());
	dir._delete();
}

void DirectoryLevelStorageSource::deleteRecursive(vector<File *> *files)
{
	if ( files )
	{
		for (File* file : *files)
		{
			if (file->isDirectory())
			{
				deleteRecursive(file->listFiles());
			}
			file->_delete();
		}
	}
}

shared_ptr<LevelStorage> DirectoryLevelStorageSource::selectLevel(ConsoleSaveFile *saveFile, const wstring& levelId, bool createPlayerDir)
{
	return shared_ptr<LevelStorage> (new DirectoryLevelStorage(saveFile, baseDir, levelId, createPlayerDir));
}

bool DirectoryLevelStorageSource::isConvertible(ConsoleSaveFile *saveFile, const wstring& levelId)
{
	return false;
}

bool DirectoryLevelStorageSource::requiresConversion(ConsoleSaveFile *saveFile, const wstring& levelId)
{
	return false;
}

bool DirectoryLevelStorageSource::convertLevel(ConsoleSaveFile *saveFile, const wstring& levelId, ProgressListener *progress)
{
	return false;
}
