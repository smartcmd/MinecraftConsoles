#include "stdafx.h"
#include "ByteBuffer.h"
#include "ZoneIo.h"

ZoneIo::ZoneIo(HANDLE channel, int64_t pos)
{
	this->channel = channel;
	this->pos = pos;
}

void ZoneIo::write(byteArray bb, int size)
{
    ByteBuffer *buff = ByteBuffer::wrap(bb);
//    if (bb.length != size) throw new IllegalArgumentException("Expected " + size + " bytes, got " + bb.length);	// 4J - TODO
    buff->order(ZonedChunkStorage::BYTEORDER);
    buff->position(bb.length);
    buff->flip();
    write(buff, size);
	delete buff;
}

void ZoneIo::write(ByteBuffer *bb, int size)
{
	DWORD numberOfBytesWritten;
	SetFilePointer(channel,static_cast<int>(pos),nullptr,nullptr);
	WriteFile(channel,bb->getBuffer(), bb->getSize(),&numberOfBytesWritten,nullptr);
    pos += size;
}

ByteBuffer *ZoneIo::read(int size)
{
	DWORD numberOfBytesRead;
    byteArray bb = byteArray(size);
	SetFilePointer(channel,static_cast<int>(pos),nullptr,nullptr);
    ByteBuffer *buff = ByteBuffer::wrap(bb);
	// 4J - to investigate - why is this buffer flipped before anything goes in it?
    buff->order(ZonedChunkStorage::BYTEORDER);
    buff->position(size);
    buff->flip();
	ReadFile(channel, buff->getBuffer(), buff->getSize(), &numberOfBytesRead, nullptr);
    pos += size;
    return buff;
}

void ZoneIo::flush()
{
	// 4J - was channel.force(false);
}