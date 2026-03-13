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
    int64_t key = xZone + (zZone << 20l);
	// 4J - was !zoneFiles.containsKey(key)
    if (zoneFiles.find(key) == zoneFiles.end())
	{
		wchar_t xRadix36[64] {};
		wchar_t zRadix36[64] {};
		_itow_s(x,xRadix36,36);
		_itow_s(z,zRadix36,36);
		File file = File(dir, wstring( L"zone_") + std::wstring( xRadix36 ) + L"_" + std::wstring( zRadix36 ) + L".dat" );

		if ( !file.exists() )
		{
            if (!create) return nullptr;
			HANDLE ch = CreateFile(wstringtofilename(file.getPath()), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			CloseHandle(ch);
        }

		File entityFile = File(dir, wstring( L"entities_") + std::wstring( xRadix36 ) + L"_" + std::wstring( zRadix36 ) + L".dat" );

        zoneFiles[key] = new ZoneFile(key, file, entityFile);
    }

    ZoneFile *zoneFile = zoneFiles[key];
    zoneFile->lastUse = tickCount;
    if (!zoneFile->containsSlot(slot))
	{
        if (!create) return nullptr;
    }
    return zoneFile;

}

ZoneIo *ZonedChunkStorage::getBuffer(int x, int z, bool create)
{
    ZoneFile *zoneFile = getZoneFile(x, z, create);
    if (zoneFile == nullptr) return nullptr;
    return zoneFile->getZoneIo(getSlot(x, z));
}

LevelChunk *ZonedChunkStorage::load(Level *level, int x, int z)
{
    ZoneIo *zoneIo = getBuffer(x, z, false);
    if (zoneIo == nullptr) return nullptr;

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
    int64_t time = header->getLong();
    int64_t flags = header->getLong();

    lc->terrainPopulated = (flags & BIT_TERRAIN_POPULATED) != 0;

    loadEntities(level, lc);

    lc->fixBlocks();
    return lc;

}

void ZonedChunkStorage::save(Level *level, LevelChunk *lc)
{
    int64_t flags = 0;
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
		std::vector<int64_t> toClose;

		for ( auto& it : zoneFiles )
		{
			ZoneFile *zoneFile = it.second;
            if ( zoneFile && tickCount - zoneFile->lastUse > 20 * 60)
			{
                toClose.push_back(zoneFile->key);
            }
		}

		for ( int64_t key : toClose )
		{
			char buf[256];
			sprintf(buf,"Closing zone %I64d\n",key);
			app.DebugPrintf(buf);
            zoneFiles[key]->close();
			zoneFiles.erase(zoneFiles.find(key));
		}
    }
}


void ZonedChunkStorage::flush()
{
    auto itEnd = zoneFiles.end();
	for ( auto& it : zoneFiles )
	{
		ZoneFile *zoneFile = it.second;
        if ( zoneFile )
            zoneFile->close();
	}
	zoneFiles.clear();
}

void ZonedChunkStorage::loadEntities(Level *level, LevelChunk *lc)
{
    int slot = getSlot(lc->x, lc->z);
    ZoneFile *zoneFile = getZoneFile(lc->x, lc->z, true);
    vector<CompoundTag *> *tags = zoneFile->entityFile->readAll(slot);

	for ( CompoundTag *tag : *tags )
	{
        int type = tag->getInt(L"_TYPE");
        if (type == 0)
		{
            shared_ptr<Entity> e = EntityIO::loadStatic(tag, level);
            if (e != nullptr)
            {
                lc->addEntity(e);
                lc->addRidingEntities(e, tag);
            }
        }
		else if (type == 1)
		{
            shared_ptr<TileEntity> te = TileEntity::loadStatic(tag);
            if (te != nullptr) lc->addTileEntity(te);
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

		for ( auto& e : *entities )
		{
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
