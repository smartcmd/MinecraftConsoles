#include "stdafx.h"
#include <iostream>
#include <exception>
#include "InputOutputStream.h"
#include "net.minecraft.world.level.h"
#include "compression.h"
#include "PacketListener.h"
#include "BlockRegionUpdatePacket.h"
#include "LevelChunk.h"
#include "DataLayer.h"
#include "Dimension.h"


#define BLOCK_REGION_UPDATE_FULLCHUNK 0x01
#define BLOCK_REGION_UPDATE_ZEROHEIGHT 0x02	// added so we can still send a byte for ys, which really needs the range 0-256

BlockRegionUpdatePacket::~BlockRegionUpdatePacket()
{
	delete [] buffer.data;
}

BlockRegionUpdatePacket::BlockRegionUpdatePacket()
{
	shouldDelay = true;
	x = 0;
	y = 0;
	z = 0;
	xs = 0;
	ys = 0;
	zs = 0;
	bIsFullChunk = false;
}

BlockRegionUpdatePacket::BlockRegionUpdatePacket(int x, int y, int z, int xs, int ys, int zs, Level *level)
{
	
	shouldDelay = true;
	this->x = x;
	this->y = y;
	this->z = z;
	this->xs = xs;
	this->ys = ys;
	this->zs = zs;
	bIsFullChunk = false;
	levelIdx = ( ( level->dimension->id == 0 ) ? 0 : ( (level->dimension->id == -1) ? 1 : 2 ) );

	// 4J - if we are compressing a full chunk, re-order the blocks so that they compress better
	// TODO - we should be using compressed data directly here rather than decompressing first and then recompressing...
	byteArray rawBuffer;

	if( xs == 16 && ys == Level::maxBuildHeight && zs == 16 && ( ( x & 15 ) == 0 ) && ( y == 0 ) && ( ( z & 15 ) == 0 ) ) 
	{
		bIsFullChunk = true;

		LevelChunk *lc = level->getChunkAt(x,z);
		rawBuffer = lc->getReorderedBlocksAndData(x&0xF, y, z&0xF, xs, this->ys, zs);
	}
	else
	{
		MemSect(50);
		rawBuffer = level->getBlocksAndData(x, y, z, xs, ys, zs, false);
		MemSect(0);
	}

	if(rawBuffer.length == 0)
	{
		size = 0;
		buffer = byteArray();
	}
	else
	{
		// We don't know how this will compress - just make a fixed length buffer to initially decompress into
		// Some small sets of blocks can end up compressing into something bigger than their source
		unsigned char *ucTemp = new unsigned char[(256 * 16 * 16 * 5)/2];
		unsigned int inputSize = (256 * 16 * 16 * 5)/2;
	
		Compression::getCompression()->CompressLZXRLE(ucTemp, &inputSize, rawBuffer.data, (unsigned int) rawBuffer.length);
		//app.DebugPrintf("Chunk (%d,%d) compressed from %d to size %d\n", x>>4, z>>4, rawBuffer.length, inputSize);
		unsigned char *ucTemp2 = new unsigned char[inputSize];
		memcpy(ucTemp2,ucTemp,inputSize);
		delete [] ucTemp;
		buffer = byteArray(ucTemp2,inputSize);
		delete [] rawBuffer.data;
		size = inputSize;
	}
}
 
void BlockRegionUpdatePacket::read(DataInputStream *dis) //throws IOException
{
	byte chunkFlags = dis->readByte();
	x = dis->readInt();
	y = dis->readShort();
	z = dis->readInt();
	xs = dis->read() + 1;
	ys = dis->read() + 1;
	zs = dis->read() + 1;

	bIsFullChunk = (chunkFlags & BLOCK_REGION_UPDATE_FULLCHUNK) ? true : false;
	if(chunkFlags & BLOCK_REGION_UPDATE_ZEROHEIGHT)	
		ys = 0;

	size = dis->readInt();
	levelIdx = ( size >> 30 ) & 3;
	size &= 0x3fffffff;

	if(size == 0)
	{
		buffer = byteArray();
	}
	else
	{
		byteArray compressedBuffer(size);
		bool success = dis->readFully(compressedBuffer);

		int bufferSize = xs * ys * zs * 5/2;
		// Add the size of the biome data if it's a full chunk
		if(bIsFullChunk) bufferSize += (16*16);
		buffer = byteArray(bufferSize);
		unsigned int outputSize = buffer.length;

		if( success )
		{
			Compression::getCompression()->DecompressLZXRLE( buffer.data, &outputSize, compressedBuffer.data, size);
		}
		else
		{
			app.DebugPrintf("Not decompressing packet that wasn't fully read\n");
		}

	//	printf("Block (%d %d %d), (%d %d %d) coming in decomp from %d to %d\n",x,y,z,xs,ys,zs,size,outputSize);
	

		delete [] compressedBuffer.data;
		assert(buffer.length == outputSize);
	}
}

void BlockRegionUpdatePacket::write(DataOutputStream *dos) // throws IOException
{
	byte chunkFlags = 0;
	if(bIsFullChunk) chunkFlags |= BLOCK_REGION_UPDATE_FULLCHUNK;
	if(ys == 0) chunkFlags |= BLOCK_REGION_UPDATE_ZEROHEIGHT;

	dos->writeByte(chunkFlags);
	dos->writeInt(x);
	dos->writeShort(y);
	dos->writeInt(z);
	dos->write(xs - 1);
	dos->write(ys - 1);
	dos->write(zs - 1);

	int sizeAndLevel = size;
	sizeAndLevel |= ( levelIdx << 30 );
	dos->writeInt(sizeAndLevel);
	dos->write(buffer, 0, size);
}

void BlockRegionUpdatePacket::handle(PacketListener *listener)
{
	listener->handleBlockRegionUpdate(shared_from_this());
}

int BlockRegionUpdatePacket::getEstimatedSize() 
{
	return 17 + size;
}

