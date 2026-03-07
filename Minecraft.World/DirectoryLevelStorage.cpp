#include "stdafx.h"
#include "System.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.chunk.storage.h"
#include "net.minecraft.world.level.dimension.h"
#include "com.mojang.nbt.h"
#include "File.h"
#include "DataInputStream.h"
#include "FileInputStream.h"
#include "LevelData.h"
#include "DirectoryLevelStorage.h"
#include "ConsoleSaveFileIO.h"

const wstring DirectoryLevelStorage::sc_szPlayerDir(L"players/");

_MapDataMappings::_MapDataMappings()
{
#ifndef _DURANGO
	ZeroMemory(xuids,sizeof(PlayerUID)*MAXIMUM_MAP_SAVE_DATA);
#endif
	ZeroMemory(dimensions,sizeof(byte)*(MAXIMUM_MAP_SAVE_DATA/4));
}

int _MapDataMappings::getDimension(int id)
{
	const int offset = (2*(id%4));
	const int val = (dimensions[id>>2] & (3 << offset))>>offset;

	int returnVal=0;

	switch(val)
	{
	case 0:
		returnVal = 0; // Overworld
		break;
	case 1:
		returnVal = -1; // Nether
		break;
	case 2:
		returnVal = 1; // End
		break;
	default:
#ifndef _CONTENT_PACKAGE
		printf("Read invalid dimension from MapDataMapping\n");
		__debugbreak();
#endif
		break;
	}
	return returnVal;
}

void _MapDataMappings::setMapping(int id, PlayerUID xuid, int dimension)
{
	xuids[id] = xuid;

	const int offset = (2*(id%4));

	// Reset it first
	dimensions[id>>2] &= ~( 2 << offset );
	switch(dimension)
	{
	case 0: // Overworld
		//dimensions[id>>2] &= ~( 2 << offset );
		break;
	case -1: // Nether
		dimensions[id>>2] |= ( 1 << offset );
		break;
	case 1: // End
		dimensions[id>>2] |= ( 2 << offset );
		break;
	default:
#ifndef _CONTENT_PACKAGE
		printf("Trinyg to set a MapDataMapping for an invalid dimension.\n");
		__debugbreak();
#endif
		break;
	}
}

// Old version the only used 1 bit for dimension indexing
_MapDataMappings_old::_MapDataMappings_old()
{
#ifndef _DURANGO
	ZeroMemory(xuids,sizeof(PlayerUID)*MAXIMUM_MAP_SAVE_DATA);
#endif
	ZeroMemory(dimensions,sizeof(byte)*(MAXIMUM_MAP_SAVE_DATA/8));
}

int _MapDataMappings_old::getDimension(int id)
{
	return dimensions[id>>3] & (128 >> (id%8) ) ? -1 : 0;
}

void _MapDataMappings_old::setMapping(int id, PlayerUID xuid, int dimension)
{
	xuids[id] = xuid;
	if( dimension == 0 )
	{
		dimensions[id>>3] &= ~( 128 >> (id%8) );
	}
	else
	{
		dimensions[id>>3] |= ( 128 >> (id%8) );
	}
}

#ifdef _LARGE_WORLDS
void DirectoryLevelStorage::PlayerMappings::addMapping(int id, int centreX, int centreZ, int dimension, int scale)
{
	const int64_t index = ( static_cast<int64_t>(centreZ & 0x1FFFFFFF) << 34) | ( static_cast<int64_t>(centreX & 0x1FFFFFFF) << 5) | ( (scale & 0x7) << 2) | (dimension & 0x3);
	m_mappings[index] = id;
	//app.DebugPrintf("Adding mapping: %d - (%d,%d)/%d/%d [%I64d - 0x%016llx]\n", id, centreX, centreZ, dimension, scale, index, index);
}

bool DirectoryLevelStorage::PlayerMappings::getMapping(int &id, int centreX, int centreZ, int dimension, int scale)
{
	//int64_t zMasked = centreZ & 0x1FFFFFFF;
	//int64_t xMasked = centreX & 0x1FFFFFFF;
	//int64_t zShifted = zMasked << 34;
	//int64_t xShifted = xMasked << 5;
	//app.DebugPrintf("xShifted = %d (0x%016x), zShifted = %I64d (0x%016llx)\n", xShifted, xShifted, zShifted, zShifted);
	const int64_t index = ( static_cast<int64_t>(centreZ & 0x1FFFFFFF) << 34) | ( static_cast<int64_t>(centreX & 0x1FFFFFFF) << 5) | ( (scale & 0x7) << 2) | (dimension & 0x3);
	const auto it = m_mappings.find(index);
	if(it != m_mappings.end())
	{
		id = it->second;
		//app.DebugPrintf("Found mapping: %d - (%d,%d)/%d/%d [%I64d - 0x%016llx]\n", id, centreX, centreZ, dimension, scale, index, index);
		return true;
	}
	else
	{
		//app.DebugPrintf("Failed to find mapping: (%d,%d)/%d/%d [%I64d - 0x%016llx]\n", centreX, centreZ, dimension, scale, index, index);
		return false;
	}
}

void DirectoryLevelStorage::PlayerMappings::writeMappings(DataOutputStream *dos)
{
	dos->writeInt(m_mappings.size());
	for (const auto& it : m_mappings )
	{
		app.DebugPrintf("    -- %lld (0x%016llx) = %d\n", it.first, it.first, it.second);
		dos->writeLong(it.first);
		dos->writeInt(it.second);
	}
}

void DirectoryLevelStorage::PlayerMappings::readMappings(DataInputStream *dis)
{
	const int count = dis->readInt();
	for(unsigned int i = 0; i < count; ++i)
	{
		int64_t index = dis->readLong();
		const int id = dis->readInt();
		m_mappings[index] = id;
		app.DebugPrintf("    -- %lld (0x%016llx) = %d\n", index, index, id);
	}
}
#endif

DirectoryLevelStorage::DirectoryLevelStorage(ConsoleSaveFile *saveFile, const File dir, const wstring& levelId, bool createPlayerDir) : sessionId( System::currentTimeMillis() ),
	dir( L"" ), playerDir( sc_szPlayerDir ), dataDir( wstring(L"data/") ), levelId(levelId)
{
	m_saveFile = saveFile;
	m_bHasLoadedMapDataMappings = false;

#ifdef _LARGE_WORLDS
	m_usedMappings = byteArray(MAXIMUM_MAP_SAVE_DATA/8);
#endif
}

DirectoryLevelStorage::~DirectoryLevelStorage()
{
	delete m_saveFile;

	for(const auto& it : m_cachedSaveData )
	{
		delete it.second;
	}

#ifdef _LARGE_WORLDS
	delete m_usedMappings.data;
#endif
}

void DirectoryLevelStorage::initiateSession()
{
	// 4J Jev, removed try/catch.

	const File dataFile = File( dir, wstring(L"session.lock") );
	FileOutputStream fos = FileOutputStream(dataFile);
	DataOutputStream dos = DataOutputStream(&fos);
	dos.writeLong(sessionId);
	dos.close();

}

File DirectoryLevelStorage::getFolder()
{
	return dir;
}

void DirectoryLevelStorage::checkSession()
{
	// 4J-PB - Not in the Xbox game

	/*
	File dataFile = File( dir, wstring(L"session.lock"));
	FileInputStream fis = FileInputStream(dataFile);
	DataInputStream dis = DataInputStream(&fis);
	dis.close();
	*/
}

ChunkStorage *DirectoryLevelStorage::createChunkStorage(Dimension *dimension)
{
	// 4J Jev, removed try/catch.

	if (dynamic_cast<HellDimension *>(dimension) != nullptr)
	{
		const File dir2 = File(dir, LevelStorage::NETHER_FOLDER);
		//dir2.mkdirs(); // 4J Removed
		return new OldChunkStorage(dir2, true);
	}
	if (dynamic_cast<TheEndDimension *>(dimension) != nullptr)
	{
		const File dir2 = File(dir, LevelStorage::ENDER_FOLDER);
		//dir2.mkdirs(); // 4J Removed
		return new OldChunkStorage(dir2, true);
	}

	return new OldChunkStorage(dir, true);
}

LevelData *DirectoryLevelStorage::prepareLevel()
{
	// 4J Stu Added
#ifdef _LARGE_WORLDS
	const ConsoleSavePath mapFile = getDataFile(L"largeMapDataMappings");
#else
	ConsoleSavePath mapFile = getDataFile(L"mapDataMappings");
#endif
	if (!m_bHasLoadedMapDataMappings && !mapFile.getName().empty() && getSaveFile()->doesFileExist( mapFile ))
	{
		DWORD NumberOfBytesRead;
		FileEntry *fileEntry = getSaveFile()->createFile(mapFile);

#ifdef __PS3__
		// 4J Stu - This version changed happened before initial release
		if(getSaveFile()->getSaveVersion() < SAVE_FILE_VERSION_CHANGE_MAP_DATA_MAPPING_SIZE)
		{
			// Delete the old file
			if(fileEntry) getSaveFile()->deleteFile( fileEntry );

			// Save a new, blank version
			saveMapIdLookup();
		}
		else
#elif defined(_DURANGO)
		// 4J Stu - This version changed happened before initial release
		if(getSaveFile()->getSaveVersion() < SAVE_FILE_VERSION_DURANGO_CHANGE_MAP_DATA_MAPPING_SIZE)
		{
			// Delete the old file
			if(fileEntry) getSaveFile()->deleteFile( fileEntry );

			// Save a new, blank version
			saveMapIdLookup();
		}
		else
#endif
		{
			getSaveFile()->setFilePointer(fileEntry,0,nullptr, FILE_BEGIN);

#ifdef _LARGE_WORLDS
			const byteArray data(fileEntry->getFileSize());
			getSaveFile()->readFile( fileEntry, data.data, fileEntry->getFileSize(), &NumberOfBytesRead);
			assert( NumberOfBytesRead == fileEntry->getFileSize() );

			ByteArrayInputStream bais(data);
			DataInputStream dis(&bais);
			const int count = dis.readInt();
			app.DebugPrintf("Loading %d mappings\n", count);
			for(unsigned int i = 0; i < count; ++i)
			{
				PlayerUID playerUid = dis.readPlayerUID();
#ifdef _WINDOWS64
				app.DebugPrintf("  -- %d\n", playerUid);
#else
				app.DebugPrintf("  -- %ls\n", playerUid.toString().c_str());
#endif
				m_playerMappings[playerUid].readMappings(&dis);
			}
			dis.readFully(m_usedMappings);
#else

			if(getSaveFile()->getSaveVersion() < END_DIMENSION_MAP_MAPPINGS_SAVE_VERSION)
			{
				MapDataMappings_old oldMapDataMappings;
				getSaveFile()->readFile(	fileEntry,
					&oldMapDataMappings, // data buffer
					sizeof(MapDataMappings_old), // number of bytes to read
					&NumberOfBytesRead // number of bytes read
					);
				assert( NumberOfBytesRead == sizeof(MapDataMappings_old) );

				for(unsigned int i = 0; i < MAXIMUM_MAP_SAVE_DATA; ++i)
				{
					m_saveableMapDataMappings.setMapping(i,oldMapDataMappings.xuids[i],oldMapDataMappings.getDimension(i));
				}
			}
			else
			{
				getSaveFile()->readFile(	fileEntry,
					&m_saveableMapDataMappings, // data buffer
					sizeof(MapDataMappings), // number of bytes to read
					&NumberOfBytesRead // number of bytes read
					);
				assert( NumberOfBytesRead == sizeof(MapDataMappings) );
			}

			memcpy(&m_mapDataMappings,&m_saveableMapDataMappings,sizeof(MapDataMappings));
#endif


			// Write out our changes now
			if(getSaveFile()->getSaveVersion() < END_DIMENSION_MAP_MAPPINGS_SAVE_VERSION) saveMapIdLookup();
		}

		m_bHasLoadedMapDataMappings = true;
	}

	// 4J Jev, removed try/catch

	const ConsoleSavePath dataFile = ConsoleSavePath( wstring( L"level.dat" ) );

	if ( m_saveFile->doesFileExist( dataFile ) )
	{
		ConsoleSaveFileInputStream fis = ConsoleSaveFileInputStream(m_saveFile, dataFile);
		CompoundTag *root = NbtIo::readCompressed(&fis);
		CompoundTag *tag = root->getCompound(L"Data");
		LevelData *ret = new LevelData(tag);
		delete root;
		return ret;
	}

	return nullptr;
}

void DirectoryLevelStorage::saveLevelData(LevelData *levelData, vector<shared_ptr<Player> > *players)
{
	// 4J Jev, removed try/catch

	CompoundTag *dataTag = levelData->createTag(players);

	CompoundTag *root = new CompoundTag();
	root->put(L"Data", dataTag);

	const ConsoleSavePath currentFile = ConsoleSavePath( wstring( L"level.dat" ) );

	ConsoleSaveFileOutputStream fos = ConsoleSaveFileOutputStream( m_saveFile, currentFile );
	NbtIo::writeCompressed(root, &fos);

	delete root;
}

void DirectoryLevelStorage::saveLevelData(LevelData *levelData)
{
	// 4J Jev, removed try/catch

	CompoundTag *dataTag = levelData->createTag();

	CompoundTag *root = new CompoundTag();
	root->put(L"Data", dataTag);

	const ConsoleSavePath currentFile = ConsoleSavePath( wstring( L"level.dat" ) );

	ConsoleSaveFileOutputStream fos = ConsoleSaveFileOutputStream( m_saveFile, currentFile );
	NbtIo::writeCompressed(root, &fos);

	delete root;
}

void DirectoryLevelStorage::save(shared_ptr<Player> player)
{
	// 4J Jev, removed try/catch.
	PlayerUID playerXuid = player->getXuid();
#if defined(__PS3__) || defined(__ORBIS__)
	if( playerXuid != INVALID_XUID )
#else
	if( playerXuid != INVALID_XUID && !player->isGuest() )
#endif
	{
		CompoundTag *tag = new CompoundTag();
		player->saveWithoutId(tag);
#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
		ConsoleSavePath realFile = ConsoleSavePath( m_saveFile->getPlayerDataFilenameForSave(playerXuid).c_str() );
#elif defined(_DURANGO)
		ConsoleSavePath realFile = ConsoleSavePath( playerDir.getName() + player->getXuid().toString() + L".dat" );
#else
		const ConsoleSavePath realFile = ConsoleSavePath( playerDir.getName() + std::to_wstring( player->getXuid() ) + L".dat" );
#endif
		// If saves are disabled (e.g. because we are writing the save buffer to disk) then cache this player data
		if(StorageManager.GetSaveDisabled())
		{
			ByteArrayOutputStream *bos = new ByteArrayOutputStream();
			NbtIo::writeCompressed(tag,bos);

			const auto it = m_cachedSaveData.find(realFile.getName());
			if(it != m_cachedSaveData.end() )
			{
				delete it->second;
			}
			m_cachedSaveData[realFile.getName()] = bos;
			app.DebugPrintf("Cached saving of file %ls due to saves being disabled\n", realFile.getName().c_str() );
		}
		else
		{
			ConsoleSaveFileOutputStream fos = ConsoleSaveFileOutputStream( m_saveFile, realFile );
			NbtIo::writeCompressed(tag, &fos);
		}
		delete tag;
	}
	else if( playerXuid != INVALID_XUID )
	{
		app.DebugPrintf("Not saving player as their XUID is a guest\n");
		dontSaveMapMappingForPlayer(playerXuid);
	}
}

// 4J Changed return val to bool to check if new player or loaded player
CompoundTag *DirectoryLevelStorage::load(shared_ptr<Player> player)
{
	CompoundTag *tag = loadPlayerDataTag( player->getXuid() );
	if (tag != nullptr)
	{
		player->load(tag);
	}
	return tag;
}

CompoundTag *DirectoryLevelStorage::loadPlayerDataTag(PlayerUID xuid)
{
	// 4J Jev, removed try/catch.
#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	ConsoleSavePath realFile = ConsoleSavePath( m_saveFile->getPlayerDataFilenameForLoad(xuid).c_str() );
#elif defined(_DURANGO)
	ConsoleSavePath realFile = ConsoleSavePath( playerDir.getName() + xuid.toString() + L".dat" );
#else
	const ConsoleSavePath realFile = ConsoleSavePath( playerDir.getName() + std::to_wstring( xuid ) + L".dat" );
#endif
	const auto it = m_cachedSaveData.find(realFile.getName());
	if(it != m_cachedSaveData.end() )
	{
		ByteArrayOutputStream *bos = it->second;
		ByteArrayInputStream bis(bos->buf, 0, bos->size());
		CompoundTag *tag = NbtIo::readCompressed(&bis);
		bis.reset();
		app.DebugPrintf("Loaded player data from cached file %ls\n", realFile.getName().c_str() );
		return tag;
	}
	else if ( m_saveFile->doesFileExist( realFile ) )
	{
		ConsoleSaveFileInputStream fis = ConsoleSaveFileInputStream(m_saveFile, realFile);
		return NbtIo::readCompressed(&fis);
	}
	return nullptr;
}

// 4J Added function
void DirectoryLevelStorage::clearOldPlayerFiles()
{
	if(StorageManager.GetSaveDisabled() ) return;

#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	vector<FileEntry *> *playerFiles = m_saveFile->getValidPlayerDatFiles();
#else
	vector<FileEntry *> *playerFiles = m_saveFile->getFilesWithPrefix( playerDir.getName() );
#endif

	if( playerFiles != nullptr )
	{
#ifndef _FINAL_BUILD
		if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_DistributableSave))
		{
			for(unsigned int i = 0; i < playerFiles->size(); ++i )
			{
				const FileEntry *file = playerFiles->at(i);
				wstring xuidStr = replaceAll( replaceAll(file->data.filename,playerDir.getName(),L""),L".dat",L"");
#if defined(__PS3__) || defined(__ORBIS__) || defined(_DURANGO)
				PlayerUID xuid(xuidStr);
#else
				const PlayerUID xuid = _fromString<PlayerUID>(xuidStr);
#endif
				deleteMapFilesForPlayer(xuid);
				m_saveFile->deleteFile( playerFiles->at(i) );
			}
		}
		else
#endif
			if( playerFiles->size() > MAX_PLAYER_DATA_SAVES )
			{
				sort(playerFiles->begin(), playerFiles->end(), FileEntry::newestFirst );

				for(unsigned int i = MAX_PLAYER_DATA_SAVES; i < playerFiles->size(); ++i )
				{
					const FileEntry *file = playerFiles->at(i);
					wstring xuidStr = replaceAll( replaceAll(file->data.filename,playerDir.getName(),L""),L".dat",L"");
#if defined(__PS3__) || defined(__ORBIS__) || defined(_DURANGO)
					PlayerUID xuid(xuidStr);
#else
					const PlayerUID xuid = _fromString<PlayerUID>(xuidStr);
#endif
					deleteMapFilesForPlayer(xuid);
					m_saveFile->deleteFile( playerFiles->at(i) );
				}
			}

			delete playerFiles;
	}
}

PlayerIO *DirectoryLevelStorage::getPlayerIO()
{
	return this;
}

void DirectoryLevelStorage::closeAll()
{
}

ConsoleSavePath DirectoryLevelStorage::getDataFile(const wstring& id)
{
	return ConsoleSavePath( dataDir.getName() + id + L".dat" );
}

wstring DirectoryLevelStorage::getLevelId()
{
	return levelId;
}

void DirectoryLevelStorage::flushSaveFile(bool autosave)
{
#ifndef _CONTENT_PACKAGE
	if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_DistributableSave))
	{
		// Delete gamerules files if it exists
		const ConsoleSavePath gameRulesFiles(GAME_RULE_SAVENAME);
		if(m_saveFile->doesFileExist(gameRulesFiles))
		{
			FileEntry *fe = m_saveFile->createFile(gameRulesFiles);
			m_saveFile->deleteFile( fe );
		}
	}
#endif
	m_saveFile->Flush(autosave);
}

// 4J Added
void DirectoryLevelStorage::resetNetherPlayerPositions()
{
	if(app.GetResetNether())
	{
#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
		vector<FileEntry *> *playerFiles = m_saveFile->getValidPlayerDatFiles();
#else
		const vector<FileEntry *> *playerFiles = m_saveFile->getFilesWithPrefix( playerDir.getName() );
#endif

		if ( playerFiles )
		{
			for ( FileEntry * realFile : *playerFiles )
			{
				ConsoleSaveFileInputStream fis = ConsoleSaveFileInputStream(m_saveFile, realFile);
				CompoundTag *tag = NbtIo::readCompressed(&fis);
				if (tag != nullptr)
				{
					// If the player is in the nether, set their y position above the top of the nether
					// This will force the player to be spawned in a valid position in the overworld when they are loaded
					if(tag->contains(L"Dimension") && tag->getInt(L"Dimension") == LevelData::DIMENSION_NETHER && tag->contains(L"Pos"))
					{
						ListTag<DoubleTag> *pos = (ListTag<DoubleTag> *) tag->getList(L"Pos");
						pos->get(1)->data = DBL_MAX;

						ConsoleSaveFileOutputStream fos = ConsoleSaveFileOutputStream( m_saveFile, realFile );
						NbtIo::writeCompressed(tag, &fos);
					}
					delete tag;
				}
			}
			delete playerFiles;
		}
	}
}

int DirectoryLevelStorage::getAuxValueForMap(PlayerUID xuid, int dimension, int centreXC, int centreZC, int scale)
{
	int mapId = -1;
	bool foundMapping = false;

#ifdef _LARGE_WORLDS
	const auto it = m_playerMappings.find(xuid);
	if(it != m_playerMappings.end())
	{
		foundMapping = it->second.getMapping(mapId, centreXC, centreZC, dimension, scale);
	}

	if(!foundMapping)
	{
		for(unsigned int i = 0; i < m_usedMappings.length; ++i)
		{
			if(m_usedMappings[i] < 0xFF)
			{
				unsigned int offset = 0;
				for(; offset < 8; ++offset)
				{
					if( !(m_usedMappings[i] & (1<<offset)) )
					{
						break;
					}
				}
				mapId = (i*8) + offset;
				m_playerMappings[xuid].addMapping(mapId, centreXC, centreZC, dimension, scale);
				m_usedMappings[i] |= (1<<offset);
				break;
			}
		}
	}
#else
	for(unsigned int i = 0; i < MAXIMUM_MAP_SAVE_DATA; ++i)
	{
		if(m_mapDataMappings.xuids[i] == xuid && m_mapDataMappings.getDimension(i) == dimension)
		{
			foundMapping = true;
			mapId = i;
			break;
		}
		if( mapId < 0 && m_mapDataMappings.xuids[i] == INVALID_XUID )
		{
			mapId = i;
		}
	}
	if( !foundMapping && mapId >= 0 && mapId < MAXIMUM_MAP_SAVE_DATA )
	{
		m_mapDataMappings.setMapping(mapId, xuid, dimension);
		m_saveableMapDataMappings.setMapping(mapId, xuid, dimension);

		// If we had an old map file for a mapping that is no longer valid, delete it
		std::wstring id = wstring( L"map_" ) + std::to_wstring(mapId);
		ConsoleSavePath file = getDataFile(id);

		if(m_saveFile->doesFileExist(file) )
		{
			auto it = find(m_mapFilesToDelete.begin(), m_mapFilesToDelete.end(), mapId);
			if(it != m_mapFilesToDelete.end()) m_mapFilesToDelete.erase(it);

			m_saveFile->deleteFile( m_saveFile->createFile(file) );
		}
	}
#endif
	return mapId;
}

void DirectoryLevelStorage::saveMapIdLookup()
{
	if(StorageManager.GetSaveDisabled() ) return;

#ifdef _LARGE_WORLDS
	const ConsoleSavePath file = getDataFile(L"largeMapDataMappings");
#else
	ConsoleSavePath file = getDataFile(L"mapDataMappings");
#endif

	if (!file.getName().empty())
	{
		DWORD NumberOfBytesWritten;
		FileEntry *fileEntry = m_saveFile->createFile(file);
		m_saveFile->setFilePointer(fileEntry,0,nullptr, FILE_BEGIN);

#ifdef _LARGE_WORLDS
		ByteArrayOutputStream baos;
		DataOutputStream dos(&baos);
		dos.writeInt(m_playerMappings.size());
		app.DebugPrintf("Saving %d mappings\n", m_playerMappings.size());
		for ( auto& it : m_playerMappings )
		{
#ifdef _WINDOWS64
			app.DebugPrintf("  -- %d\n", it.first);
#else
			app.DebugPrintf("  -- %ls\n", it.first.toString().c_str());
#endif
			dos.writePlayerUID(it.first);
			it.second.writeMappings(&dos);
		}
		dos.write(m_usedMappings);
		m_saveFile->writeFile(	fileEntry,
			baos.buf.data, // data buffer
			baos.size(), // number of bytes to write
			&NumberOfBytesWritten // number of bytes written
			);
#else
		m_saveFile->writeFile(	fileEntry,
			&m_saveableMapDataMappings, // data buffer
			sizeof(MapDataMappings), // number of bytes to write
			&NumberOfBytesWritten // number of bytes written
			);
		assert( NumberOfBytesWritten == sizeof(MapDataMappings) );
#endif
	}
}

void DirectoryLevelStorage::dontSaveMapMappingForPlayer(PlayerUID xuid)
{
#ifdef _LARGE_WORLDS
	const auto it = m_playerMappings.find(xuid);
	if(it != m_playerMappings.end())
	{
		for (auto itMap = it->second.m_mappings.begin(); itMap != it->second.m_mappings.end(); ++itMap)
		{
			const int index = itMap->second / 8;
			const int offset = itMap->second % 8;
			m_usedMappings[index] &= ~(1<<offset);
		}
		m_playerMappings.erase(it);
	}
#else
	for(unsigned int i = 0; i < MAXIMUM_MAP_SAVE_DATA; ++i)
	{
		if(m_saveableMapDataMappings.xuids[i] == xuid)
		{
			m_saveableMapDataMappings.setMapping(i,INVALID_XUID,0);
		}
	}
#endif
}

void DirectoryLevelStorage::deleteMapFilesForPlayer(shared_ptr<Player> player)
{
	const PlayerUID playerXuid = player->getXuid();
	if(playerXuid != INVALID_XUID) deleteMapFilesForPlayer(playerXuid);
}

void DirectoryLevelStorage::deleteMapFilesForPlayer(PlayerUID xuid)
{
#ifdef _LARGE_WORLDS
	const auto it = m_playerMappings.find(xuid);
	if(it != m_playerMappings.end())
	{
		for (auto itMap = it->second.m_mappings.begin(); itMap != it->second.m_mappings.end(); ++itMap)
		{
			std::wstring id = wstring( L"map_" ) + std::to_wstring(itMap->second);
			ConsoleSavePath file = getDataFile(id);

			if(m_saveFile->doesFileExist(file) )
			{
				// If we can't actually delete this file, store the name so we can delete it later
				if(StorageManager.GetSaveDisabled()) m_mapFilesToDelete.push_back(itMap->second);
				else m_saveFile->deleteFile( m_saveFile->createFile(file) );
			}

			const int index = itMap->second / 8;
			const int offset = itMap->second % 8;
			m_usedMappings[index] &= ~(1<<offset);
		}
		m_playerMappings.erase(it);
	}
#else
	bool changed = false;
	for(unsigned int i = 0; i < MAXIMUM_MAP_SAVE_DATA; ++i)
	{
		if(m_mapDataMappings.xuids[i] == xuid)
		{
			changed = true;

			std::wstring id = wstring( L"map_" ) + std::to_wstring(i);
			ConsoleSavePath file = getDataFile(id);

			if(m_saveFile->doesFileExist(file) )
			{
				// If we can't actually delete this file, store the name so we can delete it later
				if(StorageManager.GetSaveDisabled()) m_mapFilesToDelete.push_back(i);
				else m_saveFile->deleteFile( m_saveFile->createFile(file) );
			}
			m_mapDataMappings.setMapping(i,INVALID_XUID,0);
			m_saveableMapDataMappings.setMapping(i,INVALID_XUID,0);
			break;
		}
	}
#endif
}

void DirectoryLevelStorage::saveAllCachedData()
{
	if(StorageManager.GetSaveDisabled() ) return;

	// Save any files that were saved while saving was disabled
	for ( auto& it : m_cachedSaveData )
	{
		ByteArrayOutputStream *bos = it.second;

		ConsoleSavePath realFile = ConsoleSavePath( it.first );
		ConsoleSaveFileOutputStream fos = ConsoleSaveFileOutputStream( m_saveFile, realFile );

		app.DebugPrintf("Actually writing cached file %ls\n",it.first.c_str() );
		fos.write(bos->buf, 0, bos->size() );
		delete bos;
	}
	m_cachedSaveData.clear();

	for (const auto& it : m_mapFilesToDelete )
	{
		std::wstring id = wstring( L"map_" ) + std::to_wstring(it);
		ConsoleSavePath file = getDataFile(id);
		if(m_saveFile->doesFileExist(file) )
		{
			m_saveFile->deleteFile( m_saveFile->createFile(file) );
		}
	}
	m_mapFilesToDelete.clear();
}
