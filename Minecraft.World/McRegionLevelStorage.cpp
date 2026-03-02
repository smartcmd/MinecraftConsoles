#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.chunk.storage.h"
#include "net.minecraft.world.level.dimension.h"
#include "ConsoleSaveFileIO.h"
#include "LevelData.h"
#include "File.h"
#include "McRegionLevelStorage.h"

McRegionLevelStorage::McRegionLevelStorage(ConsoleSaveFile *saveFile, File dir, const wstring& levelName, bool createPlayerDir)
	: DirectoryLevelStorage(saveFile, dir, levelName, createPlayerDir)
{
	RegionFileCache::clear();
}

McRegionLevelStorage::~McRegionLevelStorage()
{
	// Make sure cache is clear, as the DirectoryLevelStorage destructor is going to be deleting the underlying ConsoleSaveFile
	// reference so we don't want the RegionFileCache to still be referencing it either
	RegionFileCache::clear();
}

ChunkStorage *McRegionLevelStorage::createChunkStorage(Dimension *dimension) 
{
    //File folder = getFolder();

    if (dynamic_cast<HellDimension *>(dimension) != NULL)
	{
 
		if(app.GetResetNether())
		{
#ifdef SPLIT_SAVES
			vector<FileEntry *> *netherFiles = m_saveFile->getRegionFilesByDimension(1);
			if(netherFiles!=NULL)
			{
				DWORD bytesWritten = 0;
				for(AUTO_VAR(it, netherFiles->begin()); it != netherFiles->end(); ++it)
				{
					m_saveFile->zeroFile(*it, (*it)->getFileSize(), &bytesWritten);
				}
				delete netherFiles;
			}
#else
			vector<FileEntry *> *netherFiles = m_saveFile->getFilesWithPrefix(LevelStorage::NETHER_FOLDER);
			if(netherFiles!=NULL)
			{		
				for(AUTO_VAR(it, netherFiles->begin()); it != netherFiles->end(); ++it)
				{
					m_saveFile->deleteFile(*it);
				}
				delete netherFiles;
			}
#endif
			resetNetherPlayerPositions();
		}

		return new McRegionChunkStorage(m_saveFile, LevelStorage::NETHER_FOLDER);
    }
	
	if (dynamic_cast<TheEndDimension *>(dimension))
	{
		//File dir2 = new File(folder, LevelStorage.ENDER_FOLDER);
		//dir2.mkdirs();
		//return new ThreadedMcRegionChunkStorage(dir2);
		
		// 4J-PB - save version 0 at this point means it's a create new world
		int iSaveVersion=m_saveFile->getSaveVersion();

		if((iSaveVersion!=0) && (iSaveVersion < SAVE_FILE_VERSION_NEW_END))
		{
			// For versions before TU9 (TU7 and 8) we generate a part of The End, but we want to scrap it if it exists so that it is replaced with the TU9+ version
			app.DebugPrintf("Loaded save version number is: %d, required to keep The End is: %d\n",m_saveFile->getSaveVersion(), SAVE_FILE_VERSION_NEW_END);

			vector<FileEntry *> *endFiles = m_saveFile->getFilesWithPrefix(LevelStorage::ENDER_FOLDER);

			// 4J-PB - There will be no End in early saves
			if(endFiles!=NULL)
			{			
				for(AUTO_VAR(it, endFiles->begin()); it != endFiles->end(); ++it)
				{
					m_saveFile->deleteFile(*it);
				}
				delete endFiles;
			}
		}
		return new McRegionChunkStorage(m_saveFile, LevelStorage::ENDER_FOLDER);
	}

    return new McRegionChunkStorage(m_saveFile, L"");
}

void McRegionLevelStorage::saveLevelData(LevelData *levelData, vector<shared_ptr<Player> > *players) 
{
    levelData->setVersion(MCREGION_VERSION_ID);
	MemSect(38);
    DirectoryLevelStorage::saveLevelData(levelData, players);
	MemSect(0);
}

void McRegionLevelStorage::closeAll() 
{
    RegionFileCache::clear();
}