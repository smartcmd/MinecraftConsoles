#include "stdafx.h"
#include "ByteBuffer.h"
#include "File.h"
#include "ZoneFile.h"


const int ZoneFile::slotsLength = ZonedChunkStorage::CHUNKS_PER_ZONE * ZonedChunkStorage::CHUNKS_PER_ZONE;

ZoneFile::ZoneFile(int64_t key, File file, File entityFile) : slots(slotsLength)
{
	lastUse = 0;

	this->key = key;
	this->file = file;

	// 4J - try/catch removed
//    try {
    this->entityFile = new NbtSlotFile(entityFile);
//    } catch (Exception e) {
//        System.out.println("Broken entity file: " + entityFile + " (" + e.toString() + "), replacing..");
//        entityFile.delete();
//        entityFile.createNewFile();
//        this.entityFile = new NbtSlotFile(entityFile);
//    }

    channel = CreateFile(wstringtofilename(file.getPath()), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	// 4J - try/catch removed
//    try {
        readHeader();
//    } catch (Exception e) {
//        e.printStackTrace();
//        throw new IOException("Broken zone file: " + file + ": " + e);
//    }
}

ZoneFile::~ZoneFile()
{
	delete [] slots.data;
}

void ZoneFile::readHeader()
{
    ZoneIo *zoneIo = new ZoneIo(channel, 0);
    ByteBuffer *bb = zoneIo->read(FILE_HEADER_SIZE);
    bb->flip();
    if (bb->remaining() < 5) return;
    int magic = bb->getInt();
//    if (magic != MAGIC_NUMBER) throw new IOException("Bad magic number: " + magic);		// 4J - TODO
    short version = bb->getShort();
//    if (version != 0) throw new IOException("Bad version number: " + version);	// 4J - TODO

    slotCount = bb->getShort();
    bb->getShortArray(slots);
    bb->position(bb->position() + slotsLength * 2);
}

void ZoneFile::writeHeader()
{
    ZoneIo *zoneIo = new ZoneIo(channel, 0);

    ByteBuffer *bb = ByteBuffer::allocate(FILE_HEADER_SIZE);
    bb->order(ZonedChunkStorage::BYTEORDER);
    bb->putInt(MAGIC_NUMBER);
    bb->putShort((short) 0);
    bb->putShort((short) slotCount);
    bb->putShortArray(slots);
    bb->position(bb->position() + slots.length * 2);
    bb->flip();
    zoneIo->write(bb, FILE_HEADER_SIZE);
}

void ZoneFile::close()
{
	CloseHandle(channel);
    entityFile->close();
}

ZoneIo *ZoneFile::getZoneIo(int slot)
{
    if (slots[slot] == 0)
	{
        slots[slot] = ++slotCount;
        writeHeader();
    }
    int byteOffs = (slots[slot] - 1) * ZonedChunkStorage::CHUNK_SIZE_BYTES + FILE_HEADER_SIZE;
    return new ZoneIo(channel, byteOffs);
}

bool ZoneFile::containsSlot(int slot)
{
	return slots[slot] > 0;
}