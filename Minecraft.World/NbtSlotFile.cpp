#include "stdafx.h"
#include "File.h"
#include "NbtSlotFile.h"


byteArray NbtSlotFile::READ_BUFFER(1024*1024);
int64_t NbtSlotFile::largest = 0;

NbtSlotFile::NbtSlotFile(File file)
{
	totalFileSlots = 0;
	fileSlotMapLength = ZonedChunkStorage::CHUNKS_PER_ZONE * ZonedChunkStorage::CHUNKS_PER_ZONE;
	fileSlotMap = new vector<int> *[fileSlotMapLength];

	if ( !file.exists() || file.length() )
	{
		raf = CreateFile(wstringtofilename(file.getPath()), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        writeHeader();
    }
	else
	{
		raf = CreateFile(wstringtofilename(file.getPath()), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    readHeader();

    for (int i = 0; i < fileSlotMapLength; i++)
	{
        fileSlotMap[i] = new vector<int>;
    }

	DWORD numberofBytesRead;
    for (int fileSlot = 0; fileSlot < totalFileSlots; fileSlot++)
	{
        seekSlotHeader(fileSlot);
		short slot;
		ReadFile(raf,&slot,2,&numberofBytesRead,NULL);
        if (slot == 0)
		{
			freeFileSlots.push_back(fileSlot);
        } else if (slot < 0)
		{
			fileSlotMap[(-slot) - 1]->push_back(fileSlot);
        } else {
            fileSlotMap[slot - 1]->push_back(fileSlot);
        }
    }
}

void NbtSlotFile::readHeader()
{
	DWORD numberOfBytesRead;
	SetFilePointer(raf,0,0,FILE_BEGIN);
    int magic;
	ReadFile(raf,&magic,4,&numberOfBytesRead,NULL);
//    if (magic != MAGIC_NUMBER) throw new IOException("Bad magic number: " + magic);		// 4J - TODO
    short version;
	ReadFile(raf,&version,2,&numberOfBytesRead,NULL);
//    if (version != 0) throw new IOException("Bad version number: " + version);		// 4J - TODO
	ReadFile(raf,&totalFileSlots,4,&numberOfBytesRead,NULL);
}

void NbtSlotFile::writeHeader()
{
	DWORD numberOfBytesWritten;
	short version = 0;
	SetFilePointer(raf,0,0,FILE_BEGIN);
	WriteFile(raf,&MAGIC_NUMBER,4,&numberOfBytesWritten,NULL);
	WriteFile(raf,&version,2,&numberOfBytesWritten,NULL);
	WriteFile(raf,&totalFileSlots,4,&numberOfBytesWritten,NULL);
}

void NbtSlotFile::seekSlotHeader(int fileSlot)
{
    int target = FILE_HEADER_SIZE + fileSlot * (FILE_SLOT_SIZE + FILE_SLOT_HEADER_SIZE);
	SetFilePointer(raf,target,0,FILE_BEGIN);
}

void NbtSlotFile::seekSlot(int fileSlot)
{
    int target = FILE_HEADER_SIZE + fileSlot * (FILE_SLOT_SIZE + FILE_SLOT_HEADER_SIZE);
	SetFilePointer(raf,target+FILE_SLOT_HEADER_SIZE,0,FILE_BEGIN);
}

vector<CompoundTag *> *NbtSlotFile::readAll(int slot)
{
	DWORD numberOfBytesRead;
    vector<CompoundTag *> *tags = new vector<CompoundTag *>;
    vector<int> *fileSlots = fileSlotMap[slot];
    int skipped = 0;

	for (int c : *fileSlots)
	{
        int pos = 0;
        int continuesAt = -1;
        int expectedSlot = slot + 1;
        do
		{
            seekSlotHeader(c);
            short oldSlot;
			ReadFile(raf,&oldSlot,2,&numberOfBytesRead,NULL);
            short size;
			ReadFile(raf,&size,2,&numberOfBytesRead,NULL);
			ReadFile(raf,&continuesAt,4,&numberOfBytesRead,NULL);
			int lastSlot;
			ReadFile(raf,&lastSlot,4,&numberOfBytesRead,NULL);

            seekSlot(c);
            if (expectedSlot > 0 && oldSlot == -expectedSlot)
			{
                skipped++;
                goto fileSlotLoop;	// 4J - used to be continue fileSlotLoop, with for loop labelled as fileSlotLoop
            }

			ReadFile(raf,READ_BUFFER.data + pos,size,&numberOfBytesRead,NULL);

            if (continuesAt >= 0)
			{
                pos += size;
                c = continuesAt;
                expectedSlot = -slot - 1;
            }
        } while (continuesAt >= 0);
        tags->push_back(NbtIo::decompress(READ_BUFFER));
fileSlotLoop:
		continue;
    }

    return tags;

}

int NbtSlotFile::getFreeSlot()
{    int fileSlot;

// 4J - removed - don't see how toReplace can ever have anything in here, and might not be initialised
//    if (toReplace->size() > 0)
//	{
//		fileSlot = toReplace->back();
//		toReplace->pop_back();
//    } else

	if (freeFileSlots.size() > 0)
	{
        fileSlot = freeFileSlots.back();
		freeFileSlots.pop_back();
    }
	else
	{
        fileSlot = totalFileSlots++;
        writeHeader();
    }

    return fileSlot;

}
void NbtSlotFile::replaceSlot(int slot, vector<CompoundTag *> *tags)
{
	DWORD numberOfBytesWritten;
	toReplace = fileSlotMap[slot];
    fileSlotMap[slot] = new vector<int>();

	for (auto tag : *tags)
	{
        byteArray compressed = NbtIo::compress(tag);
        if (compressed.length > largest)
		{
			wchar_t buf[256];
            largest = compressed.length;
#ifndef _CONTENT_PACKAGE
			swprintf(buf, 256, L"New largest: %I64d (%ls)\n",largest,tag->getString(L"id").c_str() );
			OutputDebugStringW(buf);
#endif
        }

        int pos = 0;
        int remaining = compressed.length;
        if (remaining == 0) continue;

        int nextFileSlot = getFreeSlot();
        short currentSlot = slot + 1;
        int lastFileSlot = -1;

        while (remaining > 0)
		{
            int fileSlot = nextFileSlot;
            fileSlotMap[slot]->push_back(fileSlot);

            short toWrite = remaining;
            if (toWrite > FILE_SLOT_SIZE)
			{
                toWrite = FILE_SLOT_SIZE;
            }

            remaining -= toWrite;
            if (remaining > 0)
			{
                nextFileSlot = getFreeSlot();
            }
			else
			{
                nextFileSlot = -1;
            }

            seekSlotHeader(fileSlot);
			WriteFile(raf,&currentSlot,2,&numberOfBytesWritten,NULL);
			WriteFile(raf,&toWrite,2,&numberOfBytesWritten,NULL);
			WriteFile(raf,&nextFileSlot,4,&numberOfBytesWritten,NULL);
			WriteFile(raf,&lastFileSlot,4,&numberOfBytesWritten,NULL);

            seekSlot(fileSlot);
			WriteFile(raf,compressed.data+pos,toWrite,&numberOfBytesWritten,NULL);

            if (remaining > 0)
			{
                lastFileSlot = fileSlot;
                pos += toWrite;
                currentSlot = -slot - 1;
            }
        }
		delete[] compressed.data;
    }

	for (int c : *toReplace)
	{
		freeFileSlots.push_back(c);

        seekSlotHeader(c);
		short zero = 0;
		WriteFile(raf,&zero,2,&numberOfBytesWritten,NULL);
    }

    toReplace->clear();

}

void NbtSlotFile::close()
{
	CloseHandle(raf);
}
