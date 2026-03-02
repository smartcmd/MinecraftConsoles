#pragma once
#include "ZonedChunkStorage.h"

class ByteBuffer;

class ZoneIo
{
private:
    HANDLE channel;
    __int64 pos;

public:
	ZoneIo(HANDLE channel, __int64 pos);
    void write(byteArray bb, int size);
    void write(ByteBuffer *bb, int size);
    ByteBuffer *read(int size);

    void flush();
};
