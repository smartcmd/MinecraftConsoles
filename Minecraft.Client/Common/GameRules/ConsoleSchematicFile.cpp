#include "stdafx.h"
#include <vector>
#include "..\..\..\Minecraft.World\com.mojang.nbt.h"
#include "..\..\..\Minecraft.World\System.h"
#include "ConsoleSchematicFile.h"
#include "..\..\..\Minecraft.World\InputOutputStream.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.chunk.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.phys.h"
#include "..\..\..\Minecraft.World\compression.h"

ConsoleSchematicFile::ConsoleSchematicFile()
{
	m_xSize = m_ySize = m_zSize = 0;
	m_refCount = 1;
	m_data.data = NULL;
}

ConsoleSchematicFile::~ConsoleSchematicFile()
{
	app.DebugPrintf("Deleting schematic file\n");
	if(m_data.data != NULL) delete [] m_data.data;
}

void ConsoleSchematicFile::save(DataOutputStream *dos)
{
	if(dos != NULL)
	{
		dos->writeInt(XBOX_SCHEMATIC_CURRENT_VERSION);

		dos->writeByte(APPROPRIATE_COMPRESSION_TYPE);

		dos->writeInt(m_xSize);
		dos->writeInt(m_ySize);
		dos->writeInt(m_zSize);

		byteArray ba(new BYTE[ m_data.length ], m_data.length);
		Compression::getCompression()->CompressLZXRLE(	ba.data, &ba.length,
													m_data.data, m_data.length);

		dos->writeInt(ba.length);
		dos->write(ba);

		save_tags(dos);

		delete [] ba.data;
	}
}

void ConsoleSchematicFile::load(DataInputStream *dis)
{
	if(dis != NULL)
	{
		// VERSION CHECK //
		int version = dis->readInt();

		Compression::ECompressionTypes compressionType = Compression::eCompressionType_LZXRLE;

		if (version > XBOX_SCHEMATIC_ORIGINAL_VERSION) // Or later versions
		{
			compressionType = (Compression::ECompressionTypes)dis->readByte();
		}

		if (version > XBOX_SCHEMATIC_CURRENT_VERSION)
			assert(false && "Unrecognised schematic version!!");

		m_xSize = dis->readInt();
		m_ySize = dis->readInt();
		m_zSize = dis->readInt();

		int compressedSize = dis->readInt();
		byteArray compressedBuffer(compressedSize);
		dis->readFully(compressedBuffer);

		if(m_data.data != NULL)
		{
			delete [] m_data.data;
			m_data.data = NULL;
		}

		if(compressionType == Compression::eCompressionType_None)
		{
			m_data = compressedBuffer;
		}
		else
		{
			unsigned int outputSize = m_xSize * m_ySize * m_zSize * 3/2;
			m_data = byteArray(outputSize);

			switch(compressionType)
			{
			case Compression::eCompressionType_RLE:
				Compression::getCompression()->DecompressRLE( m_data.data, &m_data.length, compressedBuffer.data, compressedSize);
				break;
			case APPROPRIATE_COMPRESSION_TYPE:
				Compression::getCompression()->DecompressLZXRLE( m_data.data, &m_data.length, compressedBuffer.data, compressedSize);
				break;
			default:
				app.DebugPrintf("Unrecognized compression type for Schematic file (%d)\n", (int)compressionType);
				Compression::getCompression()->SetDecompressionType( (Compression::ECompressionTypes)compressionType );
				Compression::getCompression()->DecompressLZXRLE( m_data.data, &m_data.length, compressedBuffer.data, compressedSize);
				Compression::getCompression()->SetDecompressionType( APPROPRIATE_COMPRESSION_TYPE );
			};

			delete [] compressedBuffer.data;
		}

		// READ TAGS //
		CompoundTag *tag = NbtIo::read(dis);
		ListTag<CompoundTag> *tileEntityTags = (ListTag<CompoundTag> *) tag->getList(L"TileEntities");
		if (tileEntityTags != NULL)
		{
			for (int i = 0; i < tileEntityTags->size(); i++)
			{
				CompoundTag *teTag = tileEntityTags->get(i);
				shared_ptr<TileEntity> te = TileEntity::loadStatic(teTag);

				if(te == NULL)
				{
#ifndef _CONTENT_PACKAGE
					app.DebugPrintf("ConsoleSchematicFile has read a NULL tile entity\n");
					__debugbreak();
#endif
				}
				else
				{
					m_tileEntities.push_back(te);
				}
			}
		}
		ListTag<CompoundTag> *entityTags = (ListTag<CompoundTag> *) tag->getList(L"Entities");
		if (entityTags != NULL)
		{
			for (int i = 0; i < entityTags->size(); i++)
			{
				CompoundTag *eTag = entityTags->get(i);
				eINSTANCEOF type = EntityIO::getType(eTag->getString(L"id"));
				ListTag<DoubleTag> *pos = (ListTag<DoubleTag> *) eTag->getList(L"Pos");

				double x = pos->get(0)->data;
				double y = pos->get(1)->data;
				double z = pos->get(2)->data;

				if( type == eTYPE_PAINTING || type == eTYPE_ITEM_FRAME )
				{
					x = ((IntTag *) eTag->get(L"TileX") )->data;
					y = ((IntTag *) eTag->get(L"TileY") )->data;
					z = ((IntTag *) eTag->get(L"TileZ") )->data;
				}
#ifdef _DEBUG
				//app.DebugPrintf(1,"Loaded entity type %d at (%f,%f,%f)\n",(int)type,x,y,z);
#endif
				m_entities.push_back( pair<Vec3 *, CompoundTag *>(Vec3::newPermanent(x,y,z),(CompoundTag *)eTag->copy()));
			}
		}
		delete tag;
	}
}

void ConsoleSchematicFile::save_tags(DataOutputStream *dos)
{
	CompoundTag *tag = new CompoundTag();

	ListTag<CompoundTag> *tileEntityTags = new ListTag<CompoundTag>();
	tag->put(L"TileEntities", tileEntityTags);

	for ( auto& it : m_tileEntities )
	{
		CompoundTag *cTag = new CompoundTag();
		it->save(cTag);
		tileEntityTags->add(cTag);
	}

	ListTag<CompoundTag> *entityTags = new ListTag<CompoundTag>();
	tag->put(L"Entities", entityTags);

	for (auto& it : m_entities )
		entityTags->add( (CompoundTag *)(it).second->copy() );

	NbtIo::write(tag,dos);
	delete tag;
}

int64_t ConsoleSchematicFile::applyBlocksAndData(LevelChunk *chunk, AABB *chunkBox, AABB *destinationBox, ESchematicRotation rot)
{
	int xStart = static_cast<int>(std::fmax<double>(destinationBox->x0, (double)chunk->x*16));
	int xEnd = static_cast<int>(std::fmin<double>(destinationBox->x1, (double)((xStart >> 4) << 4) + 16));

	int yStart = destinationBox->y0;
	int yEnd = destinationBox->y1;
	if(yEnd > Level::maxBuildHeight) yEnd = Level::maxBuildHeight;

	int zStart = static_cast<int>(std::fmax<double>(destinationBox->z0, (double)chunk->z * 16));
	int zEnd = static_cast<int>(std::fmin<double>(destinationBox->z1, (double)((zStart >> 4) << 4) + 16));

#ifdef _DEBUG
	app.DebugPrintf("Range is (%d,%d,%d) to (%d,%d,%d)\n",xStart,yStart,zStart,xEnd-1,yEnd-1,zEnd-1);
#endif

	int rowBlocksIncluded = (yEnd-yStart)*(zEnd-zStart);
	int blocksIncluded = (xEnd-xStart)*rowBlocksIncluded;

	int rowBlockCount = getYSize() * getZSize();
	int totalBlockCount = getXSize() * rowBlockCount;

	byteArray blockData = byteArray(Level::CHUNK_TILE_COUNT);
	PIXBeginNamedEvent(0,"Getting block data");
	chunk->getBlockData(blockData);
	PIXEndNamedEvent();
	byteArray dataData = byteArray(Level::HALF_CHUNK_TILE_COUNT);
	PIXBeginNamedEvent(0,"Getting Data data");
	chunk->getDataData(dataData);
	PIXEndNamedEvent();

	// Ignore light data
	int blockLightP = -1;
	int skyLightP = -1;
	if( rot == eSchematicRot_90 || rot == eSchematicRot_180 || rot == eSchematicRot_270 )
	{
		int schematicXRow = 0;
		int schematicZRow = 0;
		int blocksP = 0;
		int dataP = 0;

		for(int x = xStart; x < xEnd; ++x)
		{
			int x0 = x - chunk->x*16;
			int x1 = x0 + 1;

			for(int z = zStart; z < zEnd; ++z)
			{
				int z0 = z - chunk->z*16;
				int z1 = z0 + 1;

				chunkCoordToSchematicCoord(destinationBox, x, z, rot, schematicXRow, schematicZRow);
				blocksP = (schematicXRow*rowBlockCount) + (schematicZRow*getYSize());
				dataP = totalBlockCount + (blocksP)/2;

				ConsoleSchematicFile::setBlocksAndData(chunk,blockData,dataData,m_data, x0, yStart, z0, x1, yEnd, z1, blocksP, dataP, blockLightP, skyLightP);
			}
		}
	}
	else if( rot == eSchematicRot_0 )
	{
		// The initial pointer offsets for the different data types
		int schematicXRow = xStart - destinationBox->x0;
		int schematicZRow = zStart - destinationBox->z0;
		int blocksP = (schematicXRow*rowBlockCount) + (schematicZRow*getYSize());
		int dataP = totalBlockCount + (schematicXRow*rowBlockCount + (schematicZRow*getYSize()))/2;

		for(int x = xStart; x < xEnd; ++x)
		{
			int x0 = x - chunk->x*16;
			int x1 = x0 + 1;

			int z0 = zStart - chunk->z*16;
			int z1 = zEnd - chunk->z*16;

			ConsoleSchematicFile::setBlocksAndData(chunk,blockData,dataData,m_data, x0, yStart, z0, x1, yEnd, z1, blocksP, dataP, blockLightP, skyLightP);
			// update all pointer positions
			// For z start to z end
			// Set blocks and data
			// increment z by the right amount
			blocksP += (rowBlockCount-rowBlocksIncluded);
			dataP += (rowBlockCount-rowBlocksIncluded)/2;
		}
	}
	else
	{
		app.DebugPrintf("ERROR: Rotation of block and data not implemented!!\n");
	}

	// 4J Stu - Hack for ME pack to replace sand with end stone in schematics
	//for(int i = 0; i < blockData.length; ++i)
	//{
	//	if(blockData[i] == Tile::sand_Id || blockData[i] == Tile::sandStone_Id)
	//	{
	//		blockData[i] = Tile::endStone_Id;
	//	}
	//}

	PIXBeginNamedEvent(0,"Setting Block data");
	chunk->setBlockData(blockData);
	PIXEndNamedEvent();
	delete blockData.data;
	chunk->recalcHeightmapOnly();
	PIXBeginNamedEvent(0,"Setting Data data");
	chunk->setDataData(dataData);
	PIXEndNamedEvent();
	delete dataData.data;

	// A basic pass through to roughly do the lighting. At this point of post-processing, we don't have all the neighbouring chunks loaded in,
	// so any lighting here should be things that won't propagate out of this chunk.
	for( int xx = xStart ; xx < xEnd; xx++ )
		for( int y = yStart ; y < yEnd; y++ )
			for( int zz = zStart ; zz < zEnd; zz++ )
			{
				int x = xx - chunk->x * 16;
				int z = zz - chunk->z * 16;
				chunk->setBrightness(LightLayer::Block,x,y,z,0);
				if( chunk->getTile(x,y,z) )
				{
					chunk->setBrightness(LightLayer::Sky,x,y,z,0);
				}
				else
				{
					if( chunk->isSkyLit(x,y,z) )
					{
						chunk->setBrightness(LightLayer::Sky,x,y,z,15);
					}
					else
					{
						chunk->setBrightness(LightLayer::Sky,x,y,z,0);
					}
				}
			}

	return blocksIncluded;
}

// At the point that this is called, we have all the neighbouring chunks loaded in (and generally post-processed, apart from this lighting pass), so
// we can do the sort of lighting that might propagate out of the chunk.
int64_t ConsoleSchematicFile::applyLighting(LevelChunk *chunk, AABB *chunkBox, AABB *destinationBox, ESchematicRotation rot)
{
	int xStart = max(destinationBox->x0, (double)chunk->x*16);
	int xEnd = min(destinationBox->x1, (double)((xStart>>4)<<4) + 16);

	int yStart = destinationBox->y0;
	int yEnd = destinationBox->y1;
	if(yEnd > Level::maxBuildHeight) yEnd = Level::maxBuildHeight;

	int zStart = max(destinationBox->z0, (double)chunk->z*16);
	int zEnd = min(destinationBox->z1, (double)((zStart>>4)<<4) + 16);

	int rowBlocksIncluded = (yEnd-yStart)*(zEnd-zStart);
	int blocksIncluded = (xEnd-xStart)*rowBlocksIncluded;

	// Now actually do a checkLight on blocks that might need it, which should more accurately put everything in place
	for( int xx = xStart ; xx < xEnd; xx++ )
		for( int y = yStart ; y < yEnd; y++ )
			for( int zz = zStart ; zz < zEnd; zz++ )
			{
				int x = xx - chunk->x * 16;
				int z = zz - chunk->z * 16;

				if( y <= chunk->getHeightmap( x, z ) )
				{
					chunk->level->checkLight(LightLayer::Sky, xx, y, zz, true);
				}
				if( Tile::lightEmission[chunk->getTile(x,y,z)] )
				{
					// Note that this lighting passes a rootOnlyEmissive flag of true, which means that only the location xx/y/zz is considered
					// as possibly being a source of emissive light, not other tiles that we might encounter whilst propagating the light from
					// the start location. If we don't do this, and Do encounter another emissive source in the radius of influence that the first
					// light source had, then we'll start also lighting from that tile but won't actually be able to progatate that second light
					// fully since checkLight only has a finite radius of 17 from the start position that it can light. Then when we do a checkLight
					// on the second light later, it won't bother doing anything because the light level at the location of the tile itself will be correct.
					chunk->level->checkLight(LightLayer::Block, xx, y, zz, true, true);
				}
			}

	return blocksIncluded;
}

void ConsoleSchematicFile::chunkCoordToSchematicCoord(AABB *destinationBox, int chunkX, int chunkZ, ESchematicRotation rot, int &schematicX, int &schematicZ)
{
	switch(rot)
	{
	case eSchematicRot_90:
		// schematicX decreases as chunkZ increases
		// schematicZ increases as chunkX increases
		schematicX = chunkZ - destinationBox->z0;
		schematicZ = (destinationBox->x1 - 1 - destinationBox->x0) - (chunkX - destinationBox->x0);
		break;
	case eSchematicRot_180:
		// schematicX decreases as chunkX increases
		// schematicZ decreases as chunkZ increases
		schematicX = (destinationBox->x1 - 1 - destinationBox->x0) - (chunkX - destinationBox->x0);
		schematicZ = (destinationBox->z1 - 1 - destinationBox->z0) - (chunkZ - destinationBox->z0);
		break;
	case eSchematicRot_270:
		// schematicX increases as chunkZ increases
		// shcematicZ decreases as chunkX increases
		schematicX = (destinationBox->z1 - 1 - destinationBox->z0) - (chunkZ - destinationBox->z0);
		schematicZ = chunkX - destinationBox->x0;
		break;
	case eSchematicRot_0:
	default:
		// schematicX increases as chunkX increases
		// schematicZ increases as chunkZ increases
		schematicX = chunkX - destinationBox->x0;
		schematicZ = chunkZ - destinationBox->z0;
		break;
	};
}

void ConsoleSchematicFile::schematicCoordToChunkCoord(AABB *destinationBox, double schematicX, double schematicZ, ESchematicRotation rot, double &chunkX, double &chunkZ)
{
	switch(rot)
	{
	case eSchematicRot_90:
		// schematicX decreases as chunkZ increases
		// schematicZ increases as chunkX increases
		chunkX = (destinationBox->x1 - 1 - schematicZ);
		chunkZ = schematicX + destinationBox->z0;
		break;
	case eSchematicRot_180:
		// schematicX decreases as chunkX increases
		// schematicZ decreases as chunkZ increases
		chunkX = (destinationBox->x1 - 1 - schematicX);
		chunkZ = (destinationBox->z1 - 1 - schematicZ);
		break;
	case eSchematicRot_270:
		// schematicX increases as chunkZ increases
		// shcematicZ decreases as chunkX increases
		chunkX = schematicZ + destinationBox->x0;
		chunkZ = (destinationBox->z1 - 1 - schematicX);
		break;
	case eSchematicRot_0:
	default:
		// schematicX increases as chunkX increases
		// schematicZ increases as chunkZ increases
		chunkX = schematicX + destinationBox->x0;
		chunkZ = schematicZ + destinationBox->z0;
		break;
	};
}

void ConsoleSchematicFile::applyTileEntities(LevelChunk *chunk, AABB *chunkBox, AABB *destinationBox, ESchematicRotation rot)
{
	for (auto& te : m_tileEntities )
	{
		double targetX = te->x;
		double targetY = te->y + destinationBox->y0;
		double targetZ = te->z;

		schematicCoordToChunkCoord(destinationBox, te->x, te->z, rot, targetX, targetZ);

		Vec3 *pos = Vec3::newTemp(targetX,targetY,targetZ);
		if( chunkBox->containsIncludingLowerBound(pos) )
		{
			shared_ptr<TileEntity> teCopy = chunk->getTileEntity( (int)targetX & 15, (int)targetY & 15, (int)targetZ & 15 );

			if ( teCopy != NULL )
			{
				CompoundTag *teData = new CompoundTag();
				te->save(teData);

				teCopy->load(teData);

				delete teData;

				// Adjust the tileEntity position to world coords from schematic co-ords
				teCopy->x = targetX;
				teCopy->y = targetY;
				teCopy->z = targetZ;

				// Remove the current tile entity
				//chunk->removeTileEntity( (int)targetX & 15, (int)targetY & 15, (int)targetZ & 15 );
			}
			else
			{
				teCopy = te->clone();

				// Adjust the tileEntity position to world coords from schematic co-ords
				teCopy->x = targetX;
				teCopy->y = targetY;
				teCopy->z = targetZ;
				chunk->addTileEntity(teCopy);
			}

			teCopy->setChanged();
		}
	}
	for (auto it = m_entities.begin(); it != m_entities.end();)
	{
		Vec3 *source = it->first;

		double targetX = source->x;
		double targetY = source->y + destinationBox->y0;
		double targetZ = source->z;
		schematicCoordToChunkCoord(destinationBox, source->x, source->z, rot, targetX, targetZ);

		// Add 0.01 as the AABB::contains function returns false if a value is <= the lower bound
		Vec3 *pos = Vec3::newTemp(targetX+0.01,targetY+0.01,targetZ+0.01);
		if( !chunkBox->containsIncludingLowerBound(pos) )
		{
			++it;
			continue;
		}

		CompoundTag *eTag = it->second;
		shared_ptr<Entity> e = EntityIO::loadStatic(eTag, NULL);

		if( e->GetType() == eTYPE_PAINTING )
		{
			shared_ptr<Painting> painting = dynamic_pointer_cast<Painting>(e);

			double tileX = painting->xTile;
			double tileZ = painting->zTile;
			schematicCoordToChunkCoord(destinationBox, painting->xTile, painting->zTile, rot, tileX, tileZ);

			painting->yTile += destinationBox->y0;
			painting->xTile = tileX;
			painting->zTile = tileZ;
			painting->setDir(painting->dir);
		}
		else if( e->GetType() == eTYPE_ITEM_FRAME )
		{
			shared_ptr<ItemFrame> frame = dynamic_pointer_cast<ItemFrame>(e);

			double tileX = frame->xTile;
			double tileZ = frame->zTile;
			schematicCoordToChunkCoord(destinationBox, frame->xTile, frame->zTile, rot, tileX, tileZ);

			frame->yTile += destinationBox->y0;
			frame->xTile = tileX;
			frame->zTile = tileZ;
			frame->setDir(frame->dir);
		}
		else
		{
			e->absMoveTo(targetX, targetY, targetZ,e->yRot,e->xRot);
		}
#ifdef _DEBUG
		app.DebugPrintf("Adding entity type %d at (%f,%f,%f)\n",e->GetType(),e->x,e->y,e->z);
#endif
		e->setLevel(chunk->level);
		e->resetSmallId();
		e->setDespawnProtected();		// default to being protected against despawning
		chunk->level->addEntity(e);

		// 4J Stu - Until we can copy every type of entity, remove them from this vector
		// This means that the entities will only exist in the first use of the schematic that is processed
		//it = m_entities.erase(it);
		++it;
	}
}

void ConsoleSchematicFile::generateSchematicFile(DataOutputStream *dos, Level *level, int xStart, int yStart, int zStart, int xEnd, int yEnd, int zEnd, bool bSaveMobs, Compression::ECompressionTypes compressionType)
{
	assert(xEnd > xStart);
	assert(yEnd > yStart);
	assert(zEnd > zStart);
	// 4J Stu - Enforce even numbered positions to start with to avoid problems with half-bytes in data

	// We want the start to be even
	if(xStart > 0 && xStart%2 != 0)
		xStart-=1;
	else if(xStart < 0 && xStart%2 !=0)
		xStart-=1;
	if(yStart < 0) yStart = 0;
	else if(yStart > 0 && yStart%2 != 0)
		yStart-=1;
	if(zStart > 0 && zStart%2 != 0)
		zStart-=1;
	else if(zStart < 0 && zStart%2 !=0)
		zStart-=1;

	// We want the end to be odd to have a total size that is even
	if(xEnd > 0 && xEnd%2 == 0)
		xEnd+=1;
	else if(xEnd < 0 && xEnd%2 ==0)
		xEnd+=1;
	if(yEnd > Level::maxBuildHeight)
		yEnd = Level::maxBuildHeight;
	else if(yEnd > 0 && yEnd%2 == 0)
		yEnd+=1;
	else if(yEnd < 0 && yEnd%2 ==0)
		yEnd+=1;
	if(zEnd > 0 && zEnd%2 == 0)
		zEnd+=1;
	else if(zEnd < 0 && zEnd%2 ==0)
		zEnd+=1;

	int xSize = xEnd - xStart + 1;
	int ySize = yEnd - yStart + 1;
	int zSize = zEnd - zStart + 1;

	app.DebugPrintf("Generating schematic file for area (%d,%d,%d) to (%d,%d,%d), %dx%dx%d\n",xStart,yStart,zStart,xEnd,yEnd,zEnd,xSize,ySize,zSize);

	if(dos != NULL) dos->writeInt(XBOX_SCHEMATIC_CURRENT_VERSION);

	if(dos != NULL) dos->writeByte(compressionType);

	//Write xSize
	if(dos != NULL) dos->writeInt(xSize);

	//Write ySize
	if(dos != NULL) dos->writeInt(ySize);

	//Write zSize
	if(dos != NULL) dos->writeInt(zSize);

	//byteArray rawBuffer = level->getBlocksAndData(xStart, yStart, zStart, xSize, ySize, zSize, false);
	int xRowSize = ySize * zSize;
	int blockCount = xSize * xRowSize;
	byteArray result( blockCount * 3 / 2 );

	// Position pointers into the data when not ordered by chunk
	int p = 0;
	int dataP = blockCount;
	int blockLightP = -1;
	int skyLightP = -1;

	int y0 = yStart;
	int y1 = yStart + ySize;
	if (y0 < 0) y0 = 0;
	if (y1 > Level::maxBuildHeight) y1 = Level::maxBuildHeight;

	// Every x is a whole row
	for(int xPos = xStart; xPos < xStart + xSize; ++xPos)
	{
		int xc = xPos >> 4;

		int x0 = xPos - xc * 16;
		if (x0 < 0) x0 = 0;
		int x1 = x0 + 1;
		if (x1 > 16) x1 = 16;

		for(int zPos = zStart; zPos < zStart + zSize;)
		{
			int zc = zPos >> 4;

			int z0 = zStart - zc * 16;
			int z1 = zStart + zSize - zc * 16;
			if (z0 < 0) z0 = 0;
			if (z1 > 16) z1 = 16;
			getBlocksAndData(level->getChunk(xc, zc), &result, x0, y0, z0, x1, y1, z1, p, dataP, blockLightP, skyLightP);
			zPos += (z1-z0);
		}
	}

#ifndef _CONTENT_PACKAGE
	if(p!=blockCount) __debugbreak();
#endif

	// We don't know how this will compress - just make a fixed length buffer to initially decompress into
	// Some small sets of blocks can end up compressing into something bigger than their source
	unsigned int inputSize = blockCount * 3 / 2;
	unsigned char *ucTemp = new unsigned char[inputSize];

	switch(compressionType)
	{
	case Compression::eCompressionType_LZXRLE:
		Compression::getCompression()->CompressLZXRLE( ucTemp, &inputSize, result.data, (unsigned int) result.length );
		break;
	case Compression::eCompressionType_RLE:
		Compression::getCompression()->CompressRLE( ucTemp, &inputSize, result.data, (unsigned int) result.length );
		break;
	case Compression::eCompressionType_None:
	default:
		memcpy( ucTemp, result.data, inputSize );
		break;
	};

	delete [] result.data;
	byteArray buffer = byteArray(ucTemp,inputSize);

	if(dos != NULL) dos->writeInt(inputSize);
	if(dos != NULL) dos->write(buffer);
	delete [] buffer.data;

	CompoundTag tag;
	ListTag<CompoundTag> *tileEntitiesTag = new ListTag<CompoundTag>(L"tileEntities");

	int xc0 = xStart >> 4;
	int zc0 = zStart >> 4;
	int xc1 = (xStart + xSize - 1) >> 4;
	int zc1 = (zStart + zSize - 1) >> 4;

	for (int xc = xc0; xc <= xc1; xc++)
	{
		for (int zc = zc0; zc <= zc1; zc++)
		{
			vector<shared_ptr<TileEntity> > *tileEntities = getTileEntitiesInRegion(level->getChunk(xc, zc), xStart, yStart, zStart, xStart + xSize, yStart + ySize, zStart + zSize);
			for( auto& te : *tileEntities )
			{
				CompoundTag *teTag = new CompoundTag();
				shared_ptr<TileEntity> teCopy = te->clone();

				// Adjust the tileEntity position to schematic coords from world co-ords
				teCopy->x -= xStart;
				teCopy->y -= yStart;
				teCopy->z -= zStart;
				teCopy->save(teTag);
				tileEntitiesTag->add(teTag);
			}
			delete tileEntities;
		}
	}
	tag.put(L"TileEntities", tileEntitiesTag);

	AABB *bb = AABB::newTemp(xStart,yStart,zStart,xEnd,yEnd,zEnd);
	vector<shared_ptr<Entity> > *entities = level->getEntities(nullptr, bb);
	ListTag<CompoundTag> *entitiesTag = new ListTag<CompoundTag>(L"entities");

	for (auto& e : *entities )
	{
		bool mobCanBeSaved = false;
		if (bSaveMobs)
		{
			if ( e->instanceof(eTYPE_MONSTER) || e->instanceof(eTYPE_WATERANIMAL) || e->instanceof(eTYPE_ANIMAL) || (e->GetType() == eTYPE_VILLAGER) )

				// 4J-JEV: All these are derived from eTYPE_ANIMAL and true implicitly.
				//||	( e->GetType() == eTYPE_CHICKEN ) || ( e->GetType() == eTYPE_WOLF ) || ( e->GetType() == eTYPE_MUSHROOMCOW ) )
			{
				mobCanBeSaved = true;
			}
		}

		// 4J-JEV: Changed to check for instances of minecarts and hangingEntities instead of just eTYPE_PAINTING, eTYPE_ITEM_FRAME and eTYPE_MINECART
		if (mobCanBeSaved || e->instanceof(eTYPE_MINECART)  || e->GetType() == eTYPE_BOAT || e->instanceof(eTYPE_HANGING_ENTITY))
		{
			CompoundTag *eTag = new CompoundTag();
			if( e->save(eTag) )
			{
				ListTag<DoubleTag> *pos = (ListTag<DoubleTag> *) eTag->getList(L"Pos");

				pos->get(0)->data -= xStart;
				pos->get(1)->data -= yStart;
				pos->get(2)->data -= zStart;

				if( e->instanceof(eTYPE_HANGING_ENTITY) )
				{
					((IntTag *) eTag->get(L"TileX") )->data -= xStart;
					((IntTag *) eTag->get(L"TileY") )->data -= yStart;
					((IntTag *) eTag->get(L"TileZ") )->data -= zStart;
				}

				entitiesTag->add(eTag);
			}
		}
	}

	tag.put(L"Entities", entitiesTag);

	if(dos != NULL) NbtIo::write(&tag,dos);
}

void ConsoleSchematicFile::getBlocksAndData(LevelChunk *chunk, byteArray *data, int x0, int y0, int z0, int x1, int y1, int z1, int &blocksP, int &dataP, int &blockLightP, int &skyLightP)
{
	// 4J Stu - Needs updated to work with higher worlds, should still work with non-optimised version below
	//int xs = x1 - x0;
	//int ys = y1 - y0;
	//int zs = z1 - z0;
	//if (xs * ys * zs == LevelChunk::BLOCKS_LENGTH)
	//{
	//	byteArray blockData = byteArray(data->data + blocksP, Level::CHUNK_TILE_COUNT);
	//	chunk->getBlockData(blockData);
	//	blocksP  += blockData.length;

	//	byteArray dataData = byteArray(data->data + dataP, 16384);
	//	chunk->getBlockLightData(dataData);
	//	dataP += dataData.length;

	//	byteArray blockLightData = byteArray(data->data + blockLightP, 16384);
	//	chunk->getBlockLightData(blockLightData);
	//	blockLightP += blockLightData.length;

	//	byteArray skyLightData = byteArray(data->data + skyLightP, 16384);
	//	chunk->getSkyLightData(skyLightData);
	//	skyLightP += skyLightData.length;
	//	return;
	//}

	bool bHasLower, bHasUpper;
	bHasLower = bHasUpper = false;
	int lowerY0, lowerY1, upperY0, upperY1;
	lowerY0 = upperY0 = y0;
	lowerY1 = upperY1 = y1;

	int compressedHeight = Level::COMPRESSED_CHUNK_SECTION_HEIGHT;
	if(y0 < Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
	{
		lowerY0 = y0;
		lowerY1 = min(y1, compressedHeight);
		bHasLower = true;
	}
	if(y1 >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
	{
		upperY0 = max(y0, compressedHeight) - Level::COMPRESSED_CHUNK_SECTION_HEIGHT;
		upperY1 = y1 - Level::COMPRESSED_CHUNK_SECTION_HEIGHT;
		bHasUpper = true;
	}

	byteArray blockData = byteArray(Level::CHUNK_TILE_COUNT);
	chunk->getBlockData(blockData);
	for (int x = x0; x < x1; x++)
		for (int z = z0; z < z1; z++)
		{
			if(bHasLower)
			{
				int slot = x << Level::genDepthBitsPlusFour | z << Level::genDepthBits | lowerY0;
				int len = lowerY1 - lowerY0;
				System::arraycopy(blockData, slot, data, blocksP, len);
				blocksP += len;
			}
			if(bHasUpper)
			{
				int slot = (x << Level::genDepthBitsPlusFour | z << Level::genDepthBits | upperY0) + Level::COMPRESSED_CHUNK_SECTION_TILES;
				int len = upperY1 - upperY0;
				System::arraycopy(blockData, slot, data, blocksP, len);
				blocksP += len;
			}
		}
	delete blockData.data;

	byteArray dataData = byteArray(Level::CHUNK_TILE_COUNT);
	chunk->getDataData(dataData);
	for (int x = x0; x < x1; x++)
		for (int z = z0; z < z1; z++)
		{
			if(bHasLower)
			{
				int slot = (x << Level::genDepthBitsPlusFour | z << Level::genDepthBits | lowerY0) >> 1;
				int len = (lowerY1 - lowerY0) / 2;
				System::arraycopy(dataData, slot, data, dataP, len);
				dataP += len;
			}
			if(bHasUpper)
			{
				int slot = ((x << Level::genDepthBitsPlusFour | z << Level::genDepthBits | upperY0) + Level::COMPRESSED_CHUNK_SECTION_TILES) >> 1;
				int len = (upperY1 - upperY0) / 2;
				System::arraycopy(dataData, slot, data, dataP, len);
				dataP += len;
			}
		}
	delete dataData.data;

	// 4J Stu - Allow ignoring light data
	if(blockLightP > -1)
	{
		byteArray blockLightData = byteArray(Level::HALF_CHUNK_TILE_COUNT);
		chunk->getBlockLightData(blockLightData);
		for (int x = x0; x < x1; x++)
			for (int z = z0; z < z1; z++)
			{
				if(bHasLower)
				{
					int slot = (x << Level::genDepthBitsPlusFour | z << Level::genDepthBits | lowerY0) >> 1;
					int len = (lowerY1 - lowerY0) / 2;
					System::arraycopy(blockLightData, slot, data, blockLightP, len);
					blockLightP += len;
				}
				if(bHasUpper)
				{
					int slot = ((x << Level::genDepthBitsPlusFour | z << Level::genDepthBits | upperY0) >> 1) + (Level::COMPRESSED_CHUNK_SECTION_TILES/2);
					int len = (upperY1 - upperY0) / 2;
					System::arraycopy(blockLightData, slot, data, blockLightP, len);
					blockLightP += len;
				}
			}
			delete blockLightData.data;
	}


	// 4J Stu - Allow ignoring light data
	if(skyLightP > -1)
	{
		byteArray skyLightData = byteArray(Level::HALF_CHUNK_TILE_COUNT);
		chunk->getSkyLightData(skyLightData);
		for (int x = x0; x < x1; x++)
			for (int z = z0; z < z1; z++)
			{
				if(bHasLower)
				{
					int slot = (x << Level::genDepthBitsPlusFour | z << Level::genDepthBits | lowerY0) >> 1;
					int len = (lowerY1 - lowerY0) / 2;
					System::arraycopy(skyLightData, slot, data, skyLightP, len);
					skyLightP += len;
				}
				if(bHasUpper)
				{
					int slot = ((x << Level::genDepthBitsPlusFour | z << Level::genDepthBits | upperY0) >> 1) + (Level::COMPRESSED_CHUNK_SECTION_TILES/2);
					int len = (upperY1 - upperY0) / 2;
					System::arraycopy(skyLightData, slot, data, skyLightP, len);
					skyLightP += len;
				}
			}
			delete skyLightData.data;
	}

	return;
}

void ConsoleSchematicFile::setBlocksAndData(LevelChunk *chunk, byteArray blockData, byteArray dataData, byteArray inputData, int x0, int y0, int z0, int x1, int y1, int z1, int &blocksP, int &dataP, int &blockLightP, int &skyLightP)
{
	bool bHasLower, bHasUpper;
	bHasLower = bHasUpper = false;
	int lowerY0, lowerY1, upperY0, upperY1;
	lowerY0 = upperY0 = y0;
	lowerY1 = upperY1 = y1;

	int compressedHeight = Level::COMPRESSED_CHUNK_SECTION_HEIGHT;
	if(y0 < Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
	{
		lowerY0 = y0;
		lowerY1 = min(y1, compressedHeight);
		bHasLower = true;
	}
	if(y1 >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
	{
		upperY0 = max(y0, compressedHeight) - Level::COMPRESSED_CHUNK_SECTION_HEIGHT;
		upperY1 = y1 - Level::COMPRESSED_CHUNK_SECTION_HEIGHT;
		bHasUpper = true;
	}
	PIXBeginNamedEvent(0,"Applying block data");
	for (int x = x0; x < x1; x++)
		for (int z = z0; z < z1; z++)
		{
			if(bHasLower)
			{
				int slot = x << Level::genDepthBitsPlusFour | z << Level::genDepthBits | lowerY0;
				int len = lowerY1 - lowerY0;
				System::arraycopy(inputData, blocksP, &blockData, slot, len);
				blocksP += len;
			}
			if(bHasUpper)
			{
				int slot = (x << Level::genDepthBitsPlusFour | z << Level::genDepthBits | upperY0) + Level::COMPRESSED_CHUNK_SECTION_TILES;
				int len = upperY1 - upperY0;
				System::arraycopy(inputData, blocksP, &blockData, slot, len);
				blocksP += len;
			}
		}
	PIXEndNamedEvent();

	PIXBeginNamedEvent(0,"Applying Data data");
	for (int x = x0; x < x1; x++)
		for (int z = z0; z < z1; z++)
		{
			if(bHasLower)
			{
				int slot = (x << Level::genDepthBitsPlusFour | z << Level::genDepthBits | lowerY0) >> 1;
				int len = (lowerY1 - lowerY0) / 2;
				System::arraycopy(inputData, dataP, &dataData, slot, len);
				dataP += len;
			}
			if(bHasUpper)
			{
				int slot = ((x << Level::genDepthBitsPlusFour | z << Level::genDepthBits | upperY0) + Level::COMPRESSED_CHUNK_SECTION_TILES) >> 1;
				int len = (upperY1 - upperY0) / 2;
				System::arraycopy(inputData, dataP, &dataData, slot, len);
				dataP += len;
			}
		}
	PIXEndNamedEvent();
	// 4J Stu - Allow ignoring light data
	if(blockLightP > -1)
	{
		byteArray blockLightData = byteArray(Level::HALF_CHUNK_TILE_COUNT);
		chunk->getBlockLightData(blockLightData);
		for (int x = x0; x < x1; x++)
			for (int z = z0; z < z1; z++)
			{
				if(bHasLower)
				{
					int slot = (x << Level::genDepthBitsPlusFour | z << Level::genDepthBits | lowerY0) >> 1;
					int len = (lowerY1 - lowerY0) / 2;
					System::arraycopy(inputData, blockLightP, &blockLightData, slot, len);
					blockLightP += len;
				}
				if(bHasUpper)
				{
					int slot = ( (x << Level::genDepthBitsPlusFour | z << Level::genDepthBits | upperY0) >> 1) + (Level::COMPRESSED_CHUNK_SECTION_TILES/2);
					int len = (upperY1 - upperY0) / 2;
					System::arraycopy(inputData, blockLightP, &blockLightData, slot, len);
					blockLightP += len;
				}
			}
		chunk->setBlockLightData(blockLightData);
		delete blockLightData.data;
	}

	// 4J Stu - Allow ignoring light data
	if(skyLightP > -1)
	{
		byteArray skyLightData = byteArray(Level::HALF_CHUNK_TILE_COUNT);
		chunk->getSkyLightData(skyLightData);
		for (int x = x0; x < x1; x++)
			for (int z = z0; z < z1; z++)
			{
				if(bHasLower)
				{
					int slot = (x << Level::genDepthBitsPlusFour | z << Level::genDepthBits | lowerY0) >> 1;
					int len = (lowerY1 - lowerY0) / 2;
					System::arraycopy(inputData, skyLightP, &skyLightData, slot, len);
					skyLightP += len;
				}
				if(bHasUpper)
				{
					int slot = (x << Level::genDepthBitsPlusFour | z << Level::genDepthBits | upperY0) + (Level::COMPRESSED_CHUNK_SECTION_TILES/2);
					int len = (upperY1 - upperY0) / 2;
					System::arraycopy(inputData, skyLightP, &skyLightData, slot, len);
					skyLightP += len;
				}
			}
			chunk->setSkyLightData(skyLightData);
			delete skyLightData.data;
	}
}

vector<shared_ptr<TileEntity> > *ConsoleSchematicFile::getTileEntitiesInRegion(LevelChunk *chunk, int x0, int y0, int z0, int x1, int y1, int z1)
{
	vector<shared_ptr<TileEntity> > *result = new vector<shared_ptr<TileEntity> >;
	if ( result )
	{
		for ( auto& it : chunk->tileEntities )
		{
			shared_ptr<TileEntity> te = it.second;
			if (te->x >= x0 && te->y >= y0 && te->z >= z0 && te->x < x1 && te->y < y1 && te->z < z1)
			{
				result->push_back(te);
			}
		}
	}
	return result;
}
