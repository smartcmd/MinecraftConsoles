#include "stdafx.h"
#include "File.h"
#include "InputOutputStream.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.chunk.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.level.storage.h"
#include "FileHeader.h"
#include "OldChunkStorage.h"
DWORD OldChunkStorage::tlsIdx = 0;
OldChunkStorage::ThreadStorage *OldChunkStorage::tlsDefault = NULL;

OldChunkStorage::ThreadStorage::ThreadStorage()
{
	blockData = byteArray(Level::CHUNK_TILE_COUNT);
	dataData = byteArray(Level::HALF_CHUNK_TILE_COUNT);
	skyLightData = byteArray(Level::HALF_CHUNK_TILE_COUNT);
	blockLightData = byteArray(Level::HALF_CHUNK_TILE_COUNT);
}

OldChunkStorage::ThreadStorage::~ThreadStorage()
{
	delete [] blockData.data;
	delete [] dataData.data;
	delete [] skyLightData.data;
	delete [] blockLightData.data;
}

void OldChunkStorage::CreateNewThreadStorage()
{
	ThreadStorage *tls = new ThreadStorage();
	if(tlsDefault == NULL )
	{
		tlsIdx = TlsAlloc();
		tlsDefault = tls;
	}
	TlsSetValue(tlsIdx, tls);
}

void OldChunkStorage::UseDefaultThreadStorage()
{
	TlsSetValue(tlsIdx, tlsDefault);
}

void OldChunkStorage::ReleaseThreadStorage()
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(tlsIdx);
	if( tls == tlsDefault ) return;

	delete tls;
}

OldChunkStorage::OldChunkStorage(File dir, bool create)
{
	this->dir = dir;
	this->create = create;
}

File OldChunkStorage::getFile(int x, int z)
{
	wchar_t name[MAX_PATH_SIZE];
	wchar_t path1[MAX_PATH_SIZE];
	wchar_t path2[MAX_PATH_SIZE];

	wchar_t xRadix36[64];
	wchar_t zRadix36[64];
#if ( defined __PS3__ || defined __ORBIS__ || defined __PSVITA__ )
	assert(0);	// need a gcc verison of _itow ?
#else
	_itow(x,xRadix36,36);
	_itow(z,zRadix36,36);
	swprintf(name,MAX_PATH_SIZE,L"c.%ls.%ls.dat",xRadix36,zRadix36);
	_itow(x & 63,path1,36);
	_itow(z & 63,path2,36);
#endif
	//sprintf(file,"%s\\%s",dir,path1);
	File file( dir, wstring( path1 ) );
	if( !file.exists() )
	{
		if(create) file.mkdir();
		else
		{
			return File(L"");
		}
	}

	//strcat(file,"\\");
	//strcat(file,path2);
	file = File( file, wstring( path2 ) );
	if( !file.exists() )
	{
		if(create) file.mkdir();
		else
		{
			return File(L"");
		}
	}

	//strcat(file,"\\");
	//strcat(file,name);
	//sprintf(file,"%s\\%s",file,name);
	file = File( file, wstring( name ) );
	if ( !file.exists() )
	{
		if (!create)
		{
			return File(L"");
		}
	}
	return file;
}

LevelChunk *OldChunkStorage::load(Level *level, int x, int z)
{
	File file = getFile(x, z);
	if (!file.getPath().empty() && file.exists())
	{
		// 4J - removed try/catch
		//		try {
		//                System.out.println("Loading chunk "+x+", "+z);
		FileInputStream fis = FileInputStream(file);
		CompoundTag *tag = NbtIo::readCompressed(&fis);
		if (!tag->contains(L"Level"))
		{
			char buf[256];
			sprintf(buf,"Chunk file at %d, %d is missing level data, skipping\n",x,z);
			app.DebugPrintf(buf);
			return NULL;
		}
		if (!tag->getCompound(L"Level")->contains(L"Blocks"))
		{
			char buf[256];
			sprintf(buf,"Chunk file at %d, %d is missing block data, skipping\n",x,z);
			app.DebugPrintf(buf);
			return NULL;
		}
		LevelChunk *levelChunk = OldChunkStorage::load(level, tag->getCompound(L"Level"));
		if (!levelChunk->isAt(x, z))
		{
			char buf[256];
			sprintf(buf,"Chunk fileat %d, %d is in the wrong location; relocating. Expected %d, %d, got %d, %d\n",
				x, z, x, z, levelChunk->x, levelChunk->z);
			app.DebugPrintf(buf);
			tag->putInt(L"xPos", x);
			tag->putInt(L"zPos", z);
			levelChunk = OldChunkStorage::load(level, tag->getCompound(L"Level"));
		}

		return levelChunk;
		//		} catch (Exception e) {
		//			e.printStackTrace();
		//		}
	}
	return NULL;
}

void OldChunkStorage::save(Level *level, LevelChunk *levelChunk)
{
	level->checkSession();
	File file = getFile(levelChunk->x, levelChunk->z);
	if (file.exists())
	{
		LevelData *levelData = level->getLevelData();
		levelData->setSizeOnDisk( levelData->getSizeOnDisk() - file.length() );
	}

	// 4J - removed try/catch
	//    try {
	//char tmpFileName[MAX_PATH_SIZE];
	//sprintf(tmpFileName,"%s\\%s",dir,"tmp_chunk.dat");
	File tmpFile( dir, L"tmp_chunk.dat" );
	//            System.out.println("Saving chunk "+levelChunk.x+", "+levelChunk.z);

	FileOutputStream fos = FileOutputStream(tmpFile);
	CompoundTag *tag = new CompoundTag();
	CompoundTag *levelData = new CompoundTag();
	tag->put(L"Level", levelData);
	OldChunkStorage::save(levelChunk, level, levelData);
	NbtIo::writeCompressed(tag, &fos);
	fos.close();

	if (file.exists())
	{
		//DeleteFile(file);
		file._delete();
	}
	//MoveFile(tmpFile,file);
	tmpFile.renameTo( file );

	LevelData *levelInfo = level->getLevelData();
	levelInfo->setSizeOnDisk(levelInfo->getSizeOnDisk() + file.length() );
	//    } catch (Exception e) {
	//        e.printStackTrace();
	//    }
}

bool OldChunkStorage::saveEntities(LevelChunk *lc, Level *level, CompoundTag *tag)
{
	// If we saved and it had no entities, and nothing has been added since skip this one
	if(!lc->lastSaveHadEntities) return false;

	lc->lastSaveHadEntities = false;
	ListTag<CompoundTag> *entityTags = new ListTag<CompoundTag>();

#ifdef _ENTITIES_RW_SECTION
	EnterCriticalRWSection(&lc->m_csEntities, true);
#else
	EnterCriticalSection(&lc->m_csEntities);
#endif
	for (int i = 0; i < lc->ENTITY_BLOCKS_LENGTH; i++)
	{
		for( auto& e : *lc->entityBlocks[i] )
		{
			lc->lastSaveHadEntities = true;
			CompoundTag *teTag = new CompoundTag();
			if ( e && e->save(teTag))
			{
				entityTags->add(teTag);
			}
		}
	}
#ifdef _ENTITIES_RW_SECTION
	LeaveCriticalRWSection(&lc->m_csEntities, true);
#else
	LeaveCriticalSection(&lc->m_csEntities);
#endif

	tag->put(L"Entities", entityTags);

	return lc->lastSaveHadEntities;
}

void OldChunkStorage::save(LevelChunk *lc, Level *level, DataOutputStream *dos)
{
	dos->writeShort(SAVE_FILE_VERSION_NUMBER);
	dos->writeInt(lc->x);
	dos->writeInt(lc->z);
	dos->writeLong(level->getGameTime());
	dos->writeLong(lc->inhabitedTime);

	PIXBeginNamedEvent(0,"Getting block data");
	lc->writeCompressedBlockData(dos);
	PIXEndNamedEvent();

	PIXBeginNamedEvent(0,"Getting data data");
	lc->writeCompressedDataData(dos);
	PIXEndNamedEvent();

	PIXBeginNamedEvent(0,"Getting sky and block light data");
	lc->writeCompressedSkyLightData(dos);
	lc->writeCompressedBlockLightData(dos);
	PIXEndNamedEvent();

	dos->write(lc->heightmap);
	dos->writeShort(lc->terrainPopulated);
	dos->write(lc->getBiomes());

	PIXBeginNamedEvent(0,"Saving entities");
	CompoundTag *tag = new CompoundTag();
#ifndef SPLIT_SAVES
	saveEntities(lc, level, tag);
#endif

	PIXBeginNamedEvent(0,"Saving tile entities");
	ListTag<CompoundTag> *tileEntityTags = new ListTag<CompoundTag>();

	for(auto& it : lc->tileEntities)
	{
		shared_ptr<TileEntity> te = it.second;
		CompoundTag *teTag = new CompoundTag();
		te->save(teTag);
		tileEntityTags->add(teTag);
	}
	tag->put(L"TileEntities", tileEntityTags);
	PIXEndNamedEvent();

	PIXBeginNamedEvent(0,"Saving tile tick data");
	vector<TickNextTickData > *ticksInChunk = level->fetchTicksInChunk(lc, false);
	if (ticksInChunk != NULL)
	{
		__int64 levelTime = level->getGameTime();

		ListTag<CompoundTag> *tickTags = new ListTag<CompoundTag>();
		for( int i = 0; i < ticksInChunk->size(); i++ )
		{
			TickNextTickData td = ticksInChunk->at(i);
			CompoundTag *teTag = new CompoundTag();
			teTag->putInt(L"i", td.tileId);
			teTag->putInt(L"x", td.x);
			teTag->putInt(L"y", td.y);
			teTag->putInt(L"z", td.z);
			teTag->putInt(L"t", (int) (td.m_delay - levelTime));

			tickTags->add(teTag);
		}
		tag->put(L"TileTicks", tickTags);
	}
	delete ticksInChunk;
	PIXEndNamedEvent();

	NbtIo::write(tag,dos);
	delete tag;
	PIXEndNamedEvent();
}

void OldChunkStorage::save(LevelChunk *lc, Level *level, CompoundTag *tag)
{
	level->checkSession();
	tag->putInt(L"xPos", lc->x);
	tag->putInt(L"zPos", lc->z);
	tag->putLong(L"LastUpdate", level->getGameTime());
	tag->putLong(L"InhabitedTime", lc->inhabitedTime);
	// 4J - changes here for new storage. Now have static storage for getting lighting data for block, data, and sky & block lighting. This
	// wasn't required in the original version as we could just reference the information in the level itself, but with our new storage system
	// the full  data doesn't normally exist & so getSkyLightData/getBlockLightData etc. need somewhere to output this data. Making this static so
	// that we aren't dynamically allocating memory in the server thread when writing chunks as this causes serious stalling on the main thread.
	// Will be fine so long as we only actually create tags for once chunk at a time.

	// 4J Stu - As we now save on multiple threads, the static data has been moved to TLS
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(tlsIdx);

	PIXBeginNamedEvent(0,"Getting block data");
	//static byteArray blockData = byteArray(32768);
	lc->getBlockData(tls->blockData);
	tag->putByteArray(L"Blocks", tls->blockData);
	PIXEndNamedEvent();

	PIXBeginNamedEvent(0,"Getting data data");
	//static byteArray dataData = byteArray(16384);
	lc->getDataData(tls->dataData);
	tag->putByteArray(L"Data", tls->dataData);
	PIXEndNamedEvent();

	PIXBeginNamedEvent(0,"Getting sky and block light data");
	//static byteArray skyLightData = byteArray(16384);
	//static byteArray blockLightData = byteArray(16384);
	lc->getSkyLightData(tls->skyLightData);
	lc->getBlockLightData(tls->blockLightData);
	tag->putByteArray(L"SkyLight", tls->skyLightData);
	tag->putByteArray(L"BlockLight", tls->blockLightData);
	PIXEndNamedEvent();

	tag->putByteArray(L"HeightMap", lc->heightmap);
	tag->putShort(L"TerrainPopulatedFlags", lc->terrainPopulated);		// 4J - changed from "TerrainPopulated" to "TerrainPopulatedFlags" as now stores a bitfield, java stores a bool
	tag->putByteArray(L"Biomes", lc->getBiomes());

	PIXBeginNamedEvent(0,"Saving entities");
#ifndef SPLIT_SAVES
	saveEntities(lc, level, tag);
#endif

	PIXBeginNamedEvent(0,"Saving tile entities");
	ListTag<CompoundTag> *tileEntityTags = new ListTag<CompoundTag>();

	for(auto& it : lc->tileEntities)
	{
		shared_ptr<TileEntity> te = it.second;
		CompoundTag *teTag = new CompoundTag();
		te->save(teTag);
		tileEntityTags->add(teTag);
	}
	tag->put(L"TileEntities", tileEntityTags);
	PIXEndNamedEvent();

	PIXBeginNamedEvent(0,"Saving tile tick data");
	vector<TickNextTickData > *ticksInChunk = level->fetchTicksInChunk(lc, false);
	if (ticksInChunk != NULL)
	{
		__int64 levelTime = level->getGameTime();

		ListTag<CompoundTag> *tickTags = new ListTag<CompoundTag>();
		for( int i = 0; i < ticksInChunk->size(); i++ )
		{
			TickNextTickData td = ticksInChunk->at(i);
			CompoundTag *teTag = new CompoundTag();
			teTag->putInt(L"i", td.tileId);
			teTag->putInt(L"x", td.x);
			teTag->putInt(L"y", td.y);
			teTag->putInt(L"z", td.z);
			teTag->putInt(L"t", (int) (td.m_delay - levelTime));
			teTag->putInt(L"p", td.priorityTilt);

			tickTags->add(teTag);
		}
		tag->put(L"TileTicks", tickTags);
	}
	delete ticksInChunk;
	PIXEndNamedEvent();
	PIXEndNamedEvent();
}

void OldChunkStorage::loadEntities(LevelChunk *lc, Level *level, CompoundTag *tag)
{
	ListTag<CompoundTag> *entityTags = (ListTag<CompoundTag> *) tag->getList(L"Entities");
	if (entityTags != NULL)
	{
		for (int i = 0; i < entityTags->size(); i++)
		{
			CompoundTag *teTag = entityTags->get(i);
			shared_ptr<Entity> te = EntityIO::loadStatic(teTag, level);
			lc->lastSaveHadEntities = true;
			if (te != NULL)
			{
				lc->addEntity(te);
			}
		}
	}

	ListTag<CompoundTag> *tileEntityTags = (ListTag<CompoundTag> *) tag->getList(L"TileEntities");
	if (tileEntityTags != NULL)
	{
		for (int i = 0; i < tileEntityTags->size(); i++)
		{
			CompoundTag *teTag = tileEntityTags->get(i);
			shared_ptr<TileEntity> te = TileEntity::loadStatic(teTag);
			if (te != NULL)
			{
				lc->addTileEntity(te);
			}
		}
	}
}

LevelChunk *OldChunkStorage::load(Level *level, DataInputStream *dis)
{
	PIXBeginNamedEvent(0,"Loading chunk");
	short version = dis->readShort();
	int x = dis->readInt();
	int z = dis->readInt();
	int time = dis->readLong();

	LevelChunk *levelChunk = new LevelChunk(level, x, z);

	if (version >= SAVE_FILE_VERSION_CHUNK_INHABITED_TIME)
	{
		levelChunk->inhabitedTime = dis->readLong();
	}

	levelChunk->readCompressedBlockData(dis);
	levelChunk->readCompressedDataData(dis);
	levelChunk->readCompressedSkyLightData(dis);
	levelChunk->readCompressedBlockLightData(dis);

	dis->readFully(levelChunk->heightmap);

	levelChunk->terrainPopulated = dis->readShort();
	// If all neighbours have been post-processed, then we should have done the post-post-processing now. Check that this is set as if it isn't then we won't be able
	// to send network data for chunks, and we won't ever try and set it again as all the directional flags are now already set - should only be an issue for old maps
	// before this flag was added.
	if( ( levelChunk->terrainPopulated & LevelChunk::sTerrainPopulatedAllNeighbours ) == LevelChunk::sTerrainPopulatedAllNeighbours )
	{
		levelChunk->terrainPopulated |= LevelChunk::sTerrainPostPostProcessed;
	}

#ifndef _CONTENT_PACKAGE
	if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_EnableBiomeOverride))
	{
		// Read the biome data from the stream, but don't use it
		byteArray dummyBiomes(levelChunk->biomes.length);
		dis->readFully(dummyBiomes);
		delete [] dummyBiomes.data;
	}
	else
#endif
	{
		dis->readFully(levelChunk->biomes);
	}

	CompoundTag *tag = NbtIo::read(dis);

	loadEntities(levelChunk, level, tag);

	if (tag->contains(L"TileTicks"))
	{
		PIXBeginNamedEvent(0,"Loading TileTicks");
		ListTag<CompoundTag> *tileTicks = (ListTag<CompoundTag> *) tag->getList(L"TileTicks");

		if (tileTicks != NULL)
		{
			for (int i = 0; i < tileTicks->size(); i++)
			{
				CompoundTag *teTag = tileTicks->get(i);

				level->forceAddTileTick(teTag->getInt(L"x"), teTag->getInt(L"y"), teTag->getInt(L"z"), teTag->getInt(L"i"), teTag->getInt(L"t"), teTag->getInt(L"p"));
			}
		}
		PIXEndNamedEvent();
	}

	delete tag;

	PIXEndNamedEvent();

	return levelChunk;
}

LevelChunk *OldChunkStorage::load(Level *level, CompoundTag *tag)
{
	int x = tag->getInt(L"xPos");
	int z = tag->getInt(L"zPos");

	LevelChunk *levelChunk = new LevelChunk(level, x, z);
	// 4J - the original code uses the data in the tag directly, but this is now just used as a source when creating the compressed data, so
	// we need to free up the data in the tag once we are done
	levelChunk->setBlockData(tag->getByteArray(L"Blocks"));
	delete [] tag->getByteArray(L"Blocks").data;
	//	levelChunk->blocks = tag->getByteArray(L"Blocks");

	// 4J - the original code uses the data in the tag directly, but this is now just used as a source when creating the compressed data, so
	// we need to free up the data in the tag once we are done
	levelChunk->setDataData(tag->getByteArray(L"Data"));
	delete [] tag->getByteArray(L"Data").data;

	// 4J - changed to use our new methods for accessing lighting
	levelChunk->setSkyLightData(tag->getByteArray(L"SkyLight"));
	levelChunk->setBlockLightData(tag->getByteArray(L"BlockLight"));

	// In the original code (commented out below) constructing DataLayers from these arrays uses the data directly and so it doesn't need deleted. The new
	// setSkyLightData/setBlockLightData take a copy of the data so we need to delete the local one now
	delete [] tag->getByteArray(L"SkyLight").data;
	delete [] tag->getByteArray(L"BlockLight").data;

	//	levelChunk->skyLight = new DataLayer(tag->getByteArray(L"SkyLight"), level->depthBits);
	//	levelChunk->blockLight = new DataLayer(tag->getByteArray(L"BlockLight"), level->depthBits);

	delete [] levelChunk->heightmap.data;
	levelChunk->heightmap = tag->getByteArray(L"HeightMap");
	// 4J - TerrainPopulated was a bool (java), then changed to be a byte bitfield, then replaced with TerrainPopulatedShort to store a wider bitfield
	if( tag->get(L"TerrainPopulated") )
	{
		// Java bool type or byte bitfield
		levelChunk->terrainPopulated = tag->getByte(L"TerrainPopulated");
		if( levelChunk->terrainPopulated >= 1 ) levelChunk->terrainPopulated = LevelChunk::sTerrainPopulatedAllNeighbours | LevelChunk::sTerrainPostPostProcessed;	// Convert from old bool type to new bitfield
	}
	else
	{
		// New style short
		levelChunk->terrainPopulated = tag->getShort(L"TerrainPopulatedFlags");
		// If all neighbours have been post-processed, then we should have done the post-post-processing now. Check that this is set as if it isn't then we won't be able
		// to send network data for chunks, and we won't ever try and set it again as all the directional flags are now already set - should only be an issue for old maps
		// before this flag was added.
		if( ( levelChunk->terrainPopulated & LevelChunk::sTerrainPopulatedAllNeighbours ) == LevelChunk::sTerrainPopulatedAllNeighbours )
		{
			levelChunk->terrainPopulated |= LevelChunk::sTerrainPostPostProcessed;
		}
	}

#if 0
	// 4J - removed - we shouldn't need this any more
	if (!levelChunk->data->isValid())
	{
		levelChunk->data = new DataLayer(LevelChunk::BLOCKS_LENGTH, level->depthBits);		// 4J - BLOCKS_LENGTH was levelChunk->blocks.length
	}
#endif

	// 4J removed - we shouldn't need this any more
#if 0
	if (levelChunk->heightmap.data == NULL || !levelChunk->skyLight->isValid())
	{
		static int chunksUpdated = 0;
		delete [] levelChunk->heightmap.data;
		levelChunk->heightmap = byteArray(16 * 16);
		delete levelChunk->skyLight;
		levelChunk->skyLight = new DataLayer(levelChunk->blocks.length, level->depthBits);
		levelChunk->recalcHeightmap();
	}

	if (!levelChunk->blockLight->isValid())
	{
		delete levelChunk->blockLight;
		levelChunk->blockLight = new DataLayer(levelChunk->blocks.length, level->depthBits);
		levelChunk->recalcBlockLights();
	}
#endif

#ifndef _CONTENT_PACKAGE
	if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_EnableBiomeOverride))
	{
		// Do nothing
	}
	else
#endif
	{
		if (tag->contains(L"Biomes"))
		{
			levelChunk->setBiomes(tag->getByteArray(L"Biomes"));
		}
	}

	loadEntities(levelChunk, level, tag);

	if (tag->contains(L"TileTicks"))
	{
		ListTag<CompoundTag> *tileTicks = (ListTag<CompoundTag> *) tag->getList(L"TileTicks");

		if (tileTicks != NULL)
		{
			for (int i = 0; i < tileTicks->size(); i++)
			{
				CompoundTag *teTag = tileTicks->get(i);

				level->forceAddTileTick(teTag->getInt(L"x"), teTag->getInt(L"y"), teTag->getInt(L"z"), teTag->getInt(L"i"), teTag->getInt(L"t"), teTag->getInt(L"p"));
			}
		}
	}

	return levelChunk;
}

void OldChunkStorage::tick()
{
}

void OldChunkStorage::flush()
{
}

void OldChunkStorage::saveEntities(Level *level, LevelChunk *levelChunk)
{
}
