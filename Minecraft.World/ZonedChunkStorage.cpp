#include "stdafx.h"
#include "File.h"
#include "ByteBuffer.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.level.chunk.h"
#include "ZonedChunkStorage.h"
#include "ZoneFile.h"

// 4J Stu - There are changes to this class for 1.8.2, but since we never use it anyway lets not worry about it

const int ZonedChunkStorage::BIT_TERRAIN_POPULATED = 0x0000001;
		   
const int ZonedChunkStorage::CHUNKS_PER_ZONE_BITS = 5; // = 32
const int ZonedChunkStorage::CHUNKS_PER_ZONE = 1 << ZonedChunkStorage::CHUNKS_PER_ZONE_BITS; // ^2

const int ZonedChunkStorage::CHUNK_WIDTH = 16;

const int ZonedChunkStorage::CHUNK_HEADER_SIZE = 256;
const int ZonedChunkStorage::CHUNK_SIZE = ZonedChunkStorage::CHUNK_WIDTH * ZonedChunkStorage::CHUNK_WIDTH * Level::DEPTH;
const int ZonedChunkStorage::CHUNK_LAYERS = 3;
const int ZonedChunkStorage::CHUNK_SIZE_BYTES = ZonedChunkStorage::CHUNK_SIZE * ZonedChunkStorage::CHUNK_LAYERS + ZonedChunkStorage::CHUNK_HEADER_SIZE;

const ByteOrder ZonedChunkStorage::BYTEORDER = BIGENDIAN;

ZonedChunkStorage::ZonedChunkStorage(File dir)
{
	tickCount = 0;

	//this->dir = dir;
	this->dir = File( dir, wstring( L"data" ) );
	if( !this->dir.exists() ) this->dir.mkdirs();
}


int ZonedChunkStorage::getSlot(int x, int z)
{
    int xZone = x >> CHUNKS_PER_ZONE_BITS;
    int zZone = z >> CHUNKS_PER_ZONE_BITS;
    int xOffs = x - (xZone << CHUNKS_PER_ZONE_BITS);
    int zOffs = z - (zZone << CHUNKS_PER_ZONE_BITS);
    int slot = xOffs + zOffs * CHUNKS_PER_ZONE;
    return slot;
}

ZoneFile *ZonedChunkStorage::getZoneFile(int x, int z, bool create)
{
    int slot = getSlot(x, z);

    int xZone = x >> CHUNKS_PER_ZONE_BITS;
    int zZone = z >> CHUNKS_PER_ZONE_BITS;
    __int64 key = xZone + (zZone << 20l);
	// 4J - was !zoneFiles.containsKey(key)
    if (zoneFiles.find(key) == zoneFiles.end())
	{
		wchar_t xRadix36[64];
		wchar_t zRadix36[64];
		_itow(x,xRadix36,36);
		_itow(z,zRadix36,36);
		File file = File(dir, wstring( L"zone_") +  _toString( xRadix36 ) + L"_" + _toString( zRadix36 ) + L".dat" );

		if ( !file.exists() )
		{
            if (!create) return NULL;
			HANDLE ch = CreateFile(wstringtofilename(file.getPath()), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			CloseHandle(ch);
        }

		File entityFile = File(dir, wstring( L"entities_") + _toString( xRadix36 ) + L"_" + _toString( zRadix36 ) + L".dat" );

        zoneFiles[key] = new ZoneFile(key, file, entityFile);
    }

    ZoneFile *zoneFile = zoneFiles[key];
    zoneFile->lastUse = tickCount;
    if (!zoneFile->containsSlot(slot))
	{
        if (!create) return NULL;
    }
    return zoneFile;

}

ZoneIo *ZonedChunkStorage::getBuffer(int x, int z, bool create)
{
    ZoneFile *zoneFile = getZoneFile(x, z, create);
    if (zoneFile == NULL) return NULL;
    return zoneFile->getZoneIo(getSlot(x, z));
}

LevelChunk *ZonedChunkStorage::load(Level *level, int x, int z)
{
    ZoneIo *zoneIo = getBuffer(x, z, false);
    if (zoneIo == NULL) return NULL;

    LevelChunk *lc = new LevelChunk(level, x, z);
    lc->unsaved = false;

    ByteBuffer *header = zoneIo->read(CHUNK_HEADER_SIZE);
    lc->blocks = zoneIo->read(CHUNK_SIZE)->array();
    lc->data = new DataLayer(zoneIo->read(CHUNK_SIZE / 2)->array());
    lc->skyLight = new DataLayer(zoneIo->read(CHUNK_SIZE / 2)->array());
    lc->blockLight = new DataLayer(zoneIo->read(CHUNK_SIZE / 2)->array());
    lc->heightmap = zoneIo->read(CHUNK_WIDTH * CHUNK_WIDTH)->array();

    header->flip();
    int xOrg = header->getInt();
    int zOrg = header->getInt();
    __int64 time = header->getLong();
    __int64 flags = header->getLong();

    lc->terrainPopulated = (flags & BIT_TERRAIN_POPULATED) != 0;

    loadEntities(level, lc);

    lc->fixBlocks();
    return lc;

}

void ZonedChunkStorage::save(Level *level, LevelChunk *lc)
{
    __int64 flags = 0;
    if (lc->terrainPopulated) flags |= BIT_TERRAIN_POPULATED;

    ByteBuffer *header = ByteBuffer::allocate(CHUNK_HEADER_SIZE);
    header->order(ZonedChunkStorage::BYTEORDER);
    header->putInt(lc->x);
    header->putInt(lc->z);
    header->putLong(level->getTime());
    header->putLong(flags);
    header->flip();

    ZoneIo *zoneIo = getBuffer(lc->x, lc->z, true);
    zoneIo->write(header, CHUNK_HEADER_SIZE);
    zoneIo->write(lc->blocks, CHUNK_SIZE);
    zoneIo->write(lc->data->data, CHUNK_SIZE / 2);
    zoneIo->write(lc->skyLight->data, CHUNK_SIZE / 2);
    zoneIo->write(lc->blockLight->data, CHUNK_SIZE / 2);
    zoneIo->write(lc->heightmap, CHUNK_WIDTH * CHUNK_WIDTH);
    zoneIo->flush();
}

void ZonedChunkStorage::tick()
{
    tickCount++;
    if (tickCount % (20 * 10) == 4)
	{
		vector<__int64> toClose;

		AUTO_VAR(itEndZF, zoneFiles.end());
		for( unordered_map<__int64, ZoneFile *>::iterator it = zoneFiles.begin(); it != itEndZF; it++ )
		{
			ZoneFile *zoneFile = it->second;
            if (tickCount - zoneFile->lastUse > 20 * 60)
			{
                toClose.push_back(zoneFile->key);
            }
		}
		
		AUTO_VAR(itEndTC, toClose.end());
		for (AUTO_VAR(it, toClose.begin()); it != itEndTC; it++)
		{
			__int64 key = *it ; //toClose[i];
			// 4J - removed try/catch
//            try {
			char buf[256];
			sprintf(buf,"Closing zone %I64d\n",key);
			app.DebugPrintf(buf);
            zoneFiles[key]->close();
			zoneFiles.erase(zoneFiles.find(key));
//           } catch (IOException e) {
//                e.printStackTrace();
//            }
		}
    }
}


void ZonedChunkStorage::flush()
{
	AUTO_VAR(itEnd, zoneFiles.end());
	for( unordered_map<__int64, ZoneFile *>::iterator it = zoneFiles.begin(); it != itEnd; it++ )
	{
		ZoneFile *zoneFile = it->second;
		// 4J - removed try/catch
//        try {
            zoneFile->close();
//        } catch (IOException e) {
//            e.printStackTrace();
//        }
	}
	zoneFiles.clear();
}

void ZonedChunkStorage::loadEntities(Level *level, LevelChunk *lc)
{
    int slot = getSlot(lc->x, lc->z);
    ZoneFile *zoneFile = getZoneFile(lc->x, lc->z, true);
    vector<CompoundTag *> *tags = zoneFile->entityFile->readAll(slot);

	AUTO_VAR(itEnd, tags->end());
	for (AUTO_VAR(it, tags->begin()); it != itEnd; it++)
	{
        CompoundTag *tag = *it; //tags->at(i);
        int type = tag->getInt(L"_TYPE");
        if (type == 0)
		{
            shared_ptr<Entity> e = EntityIO::loadStatic(tag, level);
            if (e != NULL) lc->addEntity(e);
        }
		else if (type == 1)
		{
            shared_ptr<TileEntity> te = TileEntity::loadStatic(tag);
            if (te != NULL) lc->addTileEntity(te);
        }
    }
}

void ZonedChunkStorage::saveEntities(Level *level, LevelChunk *lc)
{
    int slot = getSlot(lc->x, lc->z);
    ZoneFile *zoneFile = getZoneFile(lc->x, lc->z, true);

    vector<CompoundTag *> tags;

#ifdef _ENTITIES_RW_SECTION
	EnterCriticalRWSection(&lc->m_csEntities, true);
#else
	EnterCriticalSection(&lc->m_csEntities);
#endif
    for (int i = 0; i < LevelChunk::ENTITY_BLOCKS_LENGTH; i++)
	{
        vector<shared_ptr<Entity> > *entities = lc->entityBlocks[i];

		AUTO_VAR(itEndTags, entities->end());
		for (AUTO_VAR(it, entities->begin()); it != itEndTags; it++)
		{
            shared_ptr<Entity> e = *it; //entities->at(j);
            CompoundTag *cp = new CompoundTag();
            cp->putInt(L"_TYPE", 0);
            e->save(cp);
            tags.push_back(cp);
        }
    }
#ifdef _ENTITIES_RW_SECTION
	LeaveCriticalRWSection(&lc->m_csEntities, true);
#else
	LeaveCriticalSection(&lc->m_csEntities);
#endif

	for( unordered_map<TilePos, shared_ptr<TileEntity> , TilePosKeyHash, TilePosKeyEq>::iterator it = lc->tileEntities.begin();
		it != lc->tileEntities.end(); it++)
	{
		shared_ptr<TileEntity> te = it->second;
        CompoundTag *cp = new CompoundTag();
        cp->putInt(L"_TYPE", 1);
        te->save(cp);
        tags.push_back(cp);
	}

    zoneFile->entityFile->replaceSlot(slot, &tags);
}
