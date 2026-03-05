#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.chunk.h"
#include "PacketListener.h"
#include "ChunkTilesUpdatePacket.h"
#include "Dimension.h"



ChunkTilesUpdatePacket::~ChunkTilesUpdatePacket() 
{
	delete [] blocks.data;
	delete [] data.data;
	delete [] positions.data;
}

ChunkTilesUpdatePacket::ChunkTilesUpdatePacket() 
{
	shouldDelay = true;
	xc = 0;
	zc = 0;
	count = 0;
}

ChunkTilesUpdatePacket::ChunkTilesUpdatePacket(int xc, int zc, shortArray positions, byte count, Level *level)
{
	shouldDelay = true;
	this->xc = xc;
	this->zc = zc;
	this->count = count;
	this->positions = shortArray(count);

	this->blocks = byteArray(count);
	this->data = byteArray(count);
	LevelChunk *levelChunk = level->getChunk(xc, zc);
	for (int i = 0; i < count; i++) 
	{
		int x = (positions[i] >> 12) & 15;
		int z = (positions[i] >> 8) & 15;
		int y = (positions[i]) & 255;

		this->positions[i] = positions[i];
		blocks[i] = static_cast<byte>(levelChunk->getTile(x, y, z));
		data[i] = static_cast<byte>(levelChunk->getData(x, y, z));
	}
	levelIdx = ( ( level->dimension->id == 0 ) ? 0 : ( (level->dimension->id == -1) ? 1 : 2 ) );
}

void ChunkTilesUpdatePacket::read(DataInputStream *dis) //throws IOException 
{
	// 4J - changed format. See comments in write method.
#ifdef _LARGE_WORLDS
	xc = dis->readShort();
	zc = dis->readShort();
	xc = ( xc << 16 ) >> 16;
	zc = ( zc << 16 ) >> 16;
#else
	xc = dis->read();
	zc = dis->read();
	xc = ( xc << 24 ) >> 24;
	zc = ( zc << 24 ) >> 24;
#endif

	int countAndFlags = dis->readByte();
	bool dataAllZero = (( countAndFlags & 0x80 ) == 0x80 );
	levelIdx = ( countAndFlags >> 5 ) & 3;
	count = countAndFlags & 0x1f;

	positions = shortArray(count);
	blocks = byteArray(count);
	data = byteArray(count);

	int currentBlockType = -1;
	for( int i = 0; i < count; i++ )
	{
		int xzAndFlag = dis->readShort();
		int y = dis->readByte();
		positions[i] = (xzAndFlag & 0xff00) | (y & 0xff);
		if( ( xzAndFlag & 0x0080 ) == 0x0080 )
		{
			currentBlockType = dis->read();
		}
		blocks[i] = currentBlockType;
		if( !dataAllZero)
		{
			data[i] = dis->read();
		}
		else
		{
			data[i] = 0;
		}
	}
}

void ChunkTilesUpdatePacket::write(DataOutputStream *dos) //throws IOException 
{
	// 4J - changed format to reduce size of these packets.
#ifdef _LARGE_WORLDS
	dos->writeShort(xc);
	dos->writeShort(zc);
#else
	dos->write(xc);
	dos->write(zc);
#endif
	// Determine if we've got any data elements that are non-zero - a large % of these packets set all data to zero, so we don't
	// bother sending all those zeros in that case.
	bool dataAllZero = true;
	for( int i = 0; i < count; i++ )
	{
		if( data[i] ) dataAllZero = false;
	}
	int countAndFlags = count;
	if( dataAllZero ) countAndFlags |= 0x80;
	countAndFlags |= ( levelIdx << 5 );
	dos->write(countAndFlags);
	int lastBlockType = -1;
	// Each block is represented by 15 bits of position, a flag to say whether the current block type is to change, and a possible data value.
	// A large % of these packets set the same block type to a several positions, so no point resending the block type when not necessary.
	for( int i = 0; i < count; i++ )
	{
		int xzAndFlag = positions[i] &0xff00;
		int y = positions[i] & 0xff;
		int thisBlockType = blocks[i];
		if( thisBlockType != lastBlockType )
		{
			xzAndFlag |= 0x0080;	// Use top bit of y as a flag, we only need 7 bits for that
			dos->writeShort(xzAndFlag);
			dos->write(y);
			dos->write(thisBlockType);
			lastBlockType = thisBlockType;
		}
		else
		{
			dos->writeShort(xzAndFlag);
			dos->write(y);
		}
		if( !dataAllZero )
		{
			dos->write(data[i]);
		}
	}
}

void ChunkTilesUpdatePacket::handle(PacketListener *listener) 
{
	listener->handleChunkTilesUpdate(shared_from_this());
}

int ChunkTilesUpdatePacket::getEstimatedSize() 
{
	bool dataAllZero = true;
	int lastBlockType = -1;
	int blockTypeChanges = 0;
	for( int i = 0; i < count; i++ )
	{
		if( data[i] ) dataAllZero = false;
		int thisBlockType = blocks[i];
		if( thisBlockType != lastBlockType )
		{
			blockTypeChanges++;
			lastBlockType = thisBlockType;
		}
	}
	int byteCount = 3 + 2 * count + blockTypeChanges;
	if( !dataAllZero )
	{
		byteCount += count;
	}

	return byteCount;
}
