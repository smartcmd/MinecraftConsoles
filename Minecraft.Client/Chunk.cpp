#include "stdafx.h"
#include "Chunk.h"
#include "TileRenderer.h"
#include "TileEntityRenderDispatcher.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.chunk.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "LevelRenderer.h"

#ifdef __PS3__
#include "PS3\SPU_Tasks\ChunkUpdate\ChunkRebuildData.h"
#include "PS3\SPU_Tasks\ChunkUpdate\TileRenderer_SPU.h"
#include "PS3\SPU_Tasks\CompressedTile\CompressedTileStorage_SPU.h"

#include "C4JThread_SPU.h"
#include "C4JSpursJob.h"
//#define DISABLE_SPU_CODE

#endif

int Chunk::updates = 0;

#ifdef _LARGE_WORLDS
DWORD Chunk::tlsIdx = TlsAlloc();

void Chunk::CreateNewThreadStorage()
{
	unsigned char *tileIds = new unsigned char[16 * 16 * Level::maxBuildHeight];
	TlsSetValue(tlsIdx, tileIds);
}

void Chunk::ReleaseThreadStorage()
{
	unsigned char *tileIds = static_cast<unsigned char *>(TlsGetValue(tlsIdx));
	delete tileIds;
}

unsigned char *Chunk::GetTileIdsStorage()
{
	unsigned char *tileIds = static_cast<unsigned char *>(TlsGetValue(tlsIdx));
	return tileIds;
}
#else
// 4J Stu - Don't want this when multi-threaded
Tesselator *Chunk::t = Tesselator::getInstance();
#endif
LevelRenderer *Chunk::levelRenderer;

// TODO - 4J see how input entity vector is set up and decide what way is best to pass this to the function
Chunk::Chunk(Level *level, LevelRenderer::rteMap &globalRenderableTileEntities, CRITICAL_SECTION& globalRenderableTileEntities_cs, int x, int y, int z, ClipChunk *clipChunk)
	: globalRenderableTileEntities( &globalRenderableTileEntities ), globalRenderableTileEntities_cs(&globalRenderableTileEntities_cs)
{
	clipChunk->visible = false;
	bb = nullptr;
	id = 0;

	this->level = level;
	//this->globalRenderableTileEntities = globalRenderableTileEntities;

	assigned = false;
	this->clipChunk = clipChunk;
	setPos(x, y, z);
}

void Chunk::setPos(int x, int y, int z)
{
	if(assigned && (x == this->x && y == this->y && z == this->z)) return;

	reset();

	this->x = x;
	this->y = y;
	this->z = z;
	xm = x + XZSIZE / 2;
	ym = y + SIZE / 2;
	zm = z + XZSIZE / 2;
	clipChunk->xm = xm;
	clipChunk->ym = ym;
	clipChunk->zm = zm;

	clipChunk->globalIdx = LevelRenderer::getGlobalIndexForChunk(x, y, z, level);

#if 1
	// 4J - we're not using offsetted renderlists anymore, so just set the full position of this chunk into x/y/zRenderOffs where
	// it will be used directly in the renderlist of this chunk
	xRenderOffs = x;
	yRenderOffs = y;
	zRenderOffs = z;
	xRender = 0;
	yRender = 0;
	zRender = 0;
#else
	xRenderOffs = x & 1023;
	yRenderOffs = y;
	zRenderOffs = z & 1023;
	xRender = x - xRenderOffs;
	yRender = y - yRenderOffs;
	zRender = z - zRenderOffs;
#endif

	float g = 6.0f;
	// 4J - changed to just set the value rather than make a new one, if we've already created storage
	if( !bb )
	{
		bb = shared_ptr<AABB>(AABB::newPermanent(-g, -g, -g, XZSIZE+g, SIZE+g, XZSIZE+g));
	}
	else
	{
		// 4J MGH - bounds are relative to the position now, so the AABB will be setup already, either above, or from the tesselator bounds.
// 		bb->set(-g, -g, -g, SIZE+g, SIZE+g, SIZE+g);
	}
	clipChunk->aabb[0] = bb->x0 + x;
	clipChunk->aabb[1] = bb->y0 + y;
	clipChunk->aabb[2] = bb->z0 + z;
	clipChunk->aabb[3] = bb->x1 + x;
	clipChunk->aabb[4] = bb->y1 + y;
	clipChunk->aabb[5] = bb->z1 + z;

	assigned = true;

	EnterCriticalSection(&levelRenderer->m_csDirtyChunks);
	unsigned char refCount = levelRenderer->incGlobalChunkRefCount(x, y, z, level);
//	printf("\t\t [inc] refcount %d at %d, %d, %d\n",refCount,x,y,z);

//	int idx = levelRenderer->getGlobalIndexForChunk(x, y, z, level);

	// If we're the first thing to be referencing this, mark it up as dirty to get rebuilt
	if( refCount == 1 )
	{
//		printf("Setting %d %d %d dirty [%d]\n",x,y,z, idx);
		// Chunks being made dirty in this way can be very numerous (eg the full visible area of the world at start up, or a whole edge of the world when moving).
		// On account of this, don't want to stick them into our lock free queue that we would normally use for letting the render update thread know about this chunk.
		// Instead, just set the flag to say this is dirty, and then pass a special value of 1 through to the lock free stack which lets that thread know that at least
		// one chunk other than the ones in the stack itself have been made dirty.
		levelRenderer->setGlobalChunkFlag(x, y, z, level, LevelRenderer::CHUNK_FLAG_DIRTY );
#ifdef _XBOX
		PIXSetMarker(0,"Non-stack event pushed");
#else
		PIXSetMarkerDeprecated(0,"Non-stack event pushed");
#endif
	}

	LeaveCriticalSection(&levelRenderer->m_csDirtyChunks);


}

void Chunk::translateToPos()
{
	glTranslatef(static_cast<float>(xRenderOffs), static_cast<float>(yRenderOffs), static_cast<float>(zRenderOffs));
}


Chunk::Chunk()
{
	bb = nullptr;
}

void Chunk::makeCopyForRebuild(Chunk *source)
{
	this->level = source->level;
	this->x = source->x;
	this->y = source->y;
	this->z = source->z;
	this->xRender = source->xRender;
	this->yRender = source->yRender;
	this->zRender = source->zRender;
	this->xRenderOffs = source->xRenderOffs;
	this->yRenderOffs = source->yRenderOffs;
	this->zRenderOffs = source->zRenderOffs;
	this->xm = source->xm;
	this->ym = source->ym;
	this->zm = source->zm;
	this->bb = source->bb;
	this->clipChunk = nullptr;
	this->id = source->id;
	this->globalRenderableTileEntities = source->globalRenderableTileEntities;
	this->globalRenderableTileEntities_cs = source->globalRenderableTileEntities_cs;
}

void Chunk::rebuild()
{
	PIXBeginNamedEvent(0,"Rebuilding chunk %d, %d, %d", x, y, z);
#if defined __PS3__ && !defined DISABLE_SPU_CODE
	rebuild_SPU();
  		return;
#endif // __PS3__

//	if (!dirty) return;
	PIXBeginNamedEvent(0,"Rebuild section A");

#ifdef _LARGE_WORLDS
	Tesselator *t = Tesselator::getInstance();
#else
	Chunk::t = Tesselator::getInstance(); // 4J - added - static initialiser being set at the wrong time
#endif

	updates++;

	int x0 = x;
	int y0 = y;
	int z0 = z;
	int x1 = x + XZSIZE;
	int y1 = y + SIZE;
	int z1 = z + XZSIZE;

	LevelChunk::touchedSky = false;

//	unordered_set<shared_ptr<TileEntity> > oldTileEntities(renderableTileEntities.begin(),renderableTileEntities.end());		// 4J removed this & next line
//	renderableTileEntities.clear();

	vector<shared_ptr<TileEntity> > renderableTileEntities;	// 4J - added

	int r = 1;

	int lists = levelRenderer->getGlobalIndexForChunk(this->x,this->y,this->z,level) * 2;
	lists += levelRenderer->chunkLists;

	PIXEndNamedEvent();

	PIXBeginNamedEvent(0,"Rebuild section B");
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 4J - optimisation begins.

	// Get the data for the level chunk that this render chunk is it (level chunk is 16 x 16 x 128,
	// render chunk is 16 x 16 x 16. We wouldn't have to actually get all of it if the data was ordered differently, but currently
	// it is ordered by x then z then y so just getting a small range of y out of it would involve getting the whole thing into
	// the cache anyway.

#ifdef _LARGE_WORLDS
	unsigned char *tileIds = GetTileIdsStorage();
#else
	static unsigned char tileIds[16 * 16 * Level::maxBuildHeight];
#endif
	byteArray tileArray = byteArray(tileIds, 16 * 16 * Level::maxBuildHeight);
	level->getChunkAt(x,z)->getBlockData(tileArray);		// 4J - TODO - now our data has been re-arranged, we could just extra the vertical slice of this chunk rather than the whole thing

	LevelSource *region = new Region(level, x0 - r, y0 - r, z0 - r, x1 + r, y1 + r, z1 + r, r);
	TileRenderer *tileRenderer = new TileRenderer(region, this->x, this->y, this->z, tileIds);

	// AP - added a caching system for Chunk::rebuild to take advantage of
	// Basically we're storing of copy of the tileIDs array inside the region so that calls to Region::getTile can grab data
	// more quickly from this array rather than calling CompressedTileStorage. On the Vita the total thread time spent in
	// Region::getTile went from 20% to 4%.
#ifdef __PSVITA__
	int xc = x >> 4;
	int zc = z >> 4;
	((Region*)region)->setCachedTiles(tileIds, xc, zc);
#endif

	// We now go through the vertical section of this level chunk that we are interested in and try and establish
	// (1) if it is completely empty
	// (2) if any of the tiles can be quickly determined to not need rendering because they are in the middle of other tiles and
	//     so can't be seen. A large amount (> 60% in tests) of tiles that call tesselateInWorld in the unoptimised version
	//     of this function fall into this category. By far the largest category of these are tiles in solid regions of rock.
	bool empty = true;
	for( int yy = y0; yy < y1; yy++ )
	{
		for( int zz = 0; zz < 16; zz++ )
		{
			for( int xx = 0; xx < 16; xx++ )
			{
				// 4J Stu - tile data is ordered in 128 blocks of full width, lower 128 then upper 128
				int indexY = yy;
				int offset = 0;
				if(indexY >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
				{
					indexY -= Level::COMPRESSED_CHUNK_SECTION_HEIGHT;
					offset = Level::COMPRESSED_CHUNK_SECTION_TILES;
				}

				unsigned char tileId = tileIds[ offset + ( ( ( xx + 0 ) << 11 ) | ( ( zz + 0 ) << 7 ) | ( indexY + 0 ) ) ];
				if( tileId > 0 ) empty = false;

				// Don't bother trying to work out neighbours for this tile if we are at the edge of the chunk - apart from the very
				// bottom of the world where we shouldn't ever be able to see
				if( yy == (Level::maxBuildHeight - 1) ) continue;
				if(( xx == 0 ) || ( xx == 15 )) continue;
				if(( zz == 0 ) || ( zz == 15 )) continue;

				// Establish whether this tile and its neighbours are all made of rock, dirt, unbreakable tiles, or have already
				// been determined to meet this criteria themselves and have a tile of 255 set.
				if( !( ( tileId == Tile::stone_Id ) || ( tileId == Tile::dirt_Id ) || ( tileId == Tile::unbreakable_Id ) || ( tileId == 255) ) ) continue;
				tileId = tileIds[ offset + ( ( ( xx - 1 ) << 11 ) | ( ( zz + 0 ) << 7 ) | ( indexY + 0 )) ];
				if( !( ( tileId == Tile::stone_Id ) || ( tileId == Tile::dirt_Id ) || ( tileId == Tile::unbreakable_Id ) || ( tileId == 255) ) ) continue;
				tileId = tileIds[ offset + ( ( ( xx + 1 ) << 11 ) | ( ( zz + 0 ) << 7 ) | ( indexY + 0 )) ];
				if( !( ( tileId == Tile::stone_Id ) || ( tileId == Tile::dirt_Id ) || ( tileId == Tile::unbreakable_Id ) || ( tileId == 255) ) ) continue;
				tileId = tileIds[ offset + ( ( ( xx + 0 ) << 11 ) | ( ( zz - 1 ) << 7 ) | ( indexY + 0 )) ];
				if( !( ( tileId == Tile::stone_Id ) || ( tileId == Tile::dirt_Id ) || ( tileId == Tile::unbreakable_Id ) || ( tileId == 255) ) ) continue;
				tileId = tileIds[ offset + ( ( ( xx + 0 ) << 11 ) | ( ( zz + 1 ) << 7 ) | ( indexY + 0 )) ];
				if( !( ( tileId == Tile::stone_Id ) || ( tileId == Tile::dirt_Id ) || ( tileId == Tile::unbreakable_Id ) || ( tileId == 255) ) ) continue;
				// Treat the bottom of the world differently - we shouldn't ever be able to look up at this, so consider tiles as invisible
				// if they are surrounded on sides other than the bottom
				if( yy > 0 )
				{
					int indexYMinusOne = yy - 1;
					int yMinusOneOffset = 0;
					if(indexYMinusOne >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
					{
						indexYMinusOne -= Level::COMPRESSED_CHUNK_SECTION_HEIGHT;
						yMinusOneOffset = Level::COMPRESSED_CHUNK_SECTION_TILES;
					}
					tileId = tileIds[ yMinusOneOffset + ( ( ( xx + 0 ) << 11 ) | ( ( zz + 0 ) << 7 ) | indexYMinusOne ) ];
					if( !( ( tileId == Tile::stone_Id ) || ( tileId == Tile::dirt_Id ) || ( tileId == Tile::unbreakable_Id ) || ( tileId == 255) ) ) continue;
				}
				int indexYPlusOne = yy + 1;
				int yPlusOneOffset = 0;
				if(indexYPlusOne >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
				{
					indexYPlusOne -= Level::COMPRESSED_CHUNK_SECTION_HEIGHT;
					yPlusOneOffset = Level::COMPRESSED_CHUNK_SECTION_TILES;
				}
				tileId = tileIds[ yPlusOneOffset + ( ( ( xx + 0 ) << 11 ) | ( ( zz + 0 ) << 7 ) | indexYPlusOne ) ];
				if( !( ( tileId == Tile::stone_Id ) || ( tileId == Tile::dirt_Id ) || ( tileId == Tile::unbreakable_Id ) || ( tileId == 255) ) ) continue;

				// This tile is surrounded. Flag it as not requiring to be rendered by setting its id to 255.
				tileIds[ offset + ( ( ( xx + 0 ) << 11 ) | ( ( zz + 0 ) << 7 ) | ( indexY + 0 ) ) ] = 0xff;
			}
		}
	}
	PIXEndNamedEvent();
	// Nothing at all to do for this chunk?
	if( empty )
	{
		// 4J - added - clear any renderer data associated with this
		for (int currentLayer = 0; currentLayer < 2; currentLayer++)
		{
			levelRenderer->setGlobalChunkFlag(this->x, this->y, this->z, level, LevelRenderer::CHUNK_FLAG_EMPTY0, currentLayer);
			RenderManager.CBuffClear(lists + currentLayer);
		}

		delete region;
		delete tileRenderer;
		return;
	}
	// 4J - optimisation ends
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PIXBeginNamedEvent(0,"Rebuild section C");
	Tesselator::Bounds bounds;	// 4J MGH - added
	{
		// this was the old default clip bounds for the chunk, set in Chunk::setPos.
		float g = 6.0f;
		bounds.boundingBox[0] = -g;
		bounds.boundingBox[1] = -g;
		bounds.boundingBox[2] = -g;
		bounds.boundingBox[3] = XZSIZE+g;
		bounds.boundingBox[4] = SIZE+g;
		bounds.boundingBox[5] = XZSIZE+g;
	}
	for (int currentLayer = 0; currentLayer < 2; currentLayer++)
	{
		bool renderNextLayer = false;
		bool rendered = false;

		bool started = false;

		// 4J - changed loop order here to leave y as the innermost loop for better cache performance
		for (int z = z0; z < z1; z++)
		{
			for (int x = x0; x < x1; x++)
			{
				for (int y = y0; y < y1; y++)
				{
					// 4J Stu - tile data is ordered in 128 blocks of full width, lower 128 then upper 128
					int indexY = y;
					int offset = 0;
					if(indexY >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
					{
						indexY -= Level::COMPRESSED_CHUNK_SECTION_HEIGHT;
						offset = Level::COMPRESSED_CHUNK_SECTION_TILES;
					}

					// 4J - get tile from those copied into our local array in earlier optimisation
					unsigned char tileId = tileIds[ offset + ( ( ( x - x0 ) << 11 ) | ( ( z - z0 ) << 7 ) | indexY) ];
					// If flagged as not visible, drop out straight away
					if( tileId == 0xff ) continue;
//					int tileId = region->getTile(x,y,z);
					if (tileId > 0)
					{
						if (!started)
						{
							started = true;

							MemSect(31);
							glNewList(lists + currentLayer, GL_COMPILE);
							MemSect(0);
							glPushMatrix();
							glDepthMask(true);	// 4J added
							t->useCompactVertices(true);	 // 4J added
							translateToPos();
							float ss = 1.000001f;
							// 4J - have removed this scale as I don't think we should need it, and have now optimised the vertex
							// shader so it doesn't do anything other than translate with this matrix anyway
#if 0
							glTranslatef(-zs / 2.0f, -ys / 2.0f, -zs / 2.0f);
							glScalef(ss, ss, ss);
							glTranslatef(zs / 2.0f, ys / 2.0f, zs / 2.0f);
#endif
							t->begin();
							t->offset(static_cast<float>(-this->x), static_cast<float>(-this->y), static_cast<float>(-this->z));
						}

						Tile *tile = Tile::tiles[tileId];
						if (currentLayer == 0 && tile->isEntityTile())
						{
							shared_ptr<TileEntity> et = region->getTileEntity(x, y, z);
							if (TileEntityRenderDispatcher::instance->hasRenderer(et))
							{
								renderableTileEntities.push_back(et);
							}
						}
						int renderLayer = tile->getRenderLayer();

						if (renderLayer != currentLayer)
						{
							renderNextLayer = true;
						}
						else if (renderLayer == currentLayer)
						{
							rendered |= tileRenderer->tesselateInWorld(tile, x, y, z);
						}
					}
				}
			}
		}

#ifdef __PSVITA__
		if( currentLayer==0 )
		{
			levelRenderer->clearGlobalChunkFlag(this->x, this->y, this->z, level, LevelRenderer::CHUNK_FLAG_CUT_OUT);
		}
#endif

		if (started)
		{
#ifdef __PSVITA__
			// AP - make sure we don't attempt to render chunks without cutout geometry
			if( t->getCutOutFound() )
			{
				levelRenderer->setGlobalChunkFlag(this->x, this->y, this->z, level, LevelRenderer::CHUNK_FLAG_CUT_OUT);
			}
#endif
			t->end();
			bounds.addBounds(t->bounds);		// 4J MGH - added
			glPopMatrix();
			glEndList();
			t->useCompactVertices(false);	 // 4J added
			t->offset(0, 0, 0);
		}
		else
		{
			rendered = false;
		}

		if (rendered)
		{
			levelRenderer->clearGlobalChunkFlag(this->x, this->y, this->z, level, LevelRenderer::CHUNK_FLAG_EMPTY0, currentLayer);
		}
		else
		{
			// 4J - added - clear any renderer data associated with this unused list
			levelRenderer->setGlobalChunkFlag(this->x, this->y, this->z, level, LevelRenderer::CHUNK_FLAG_EMPTY0, currentLayer);
			RenderManager.CBuffClear(lists + currentLayer);
		}
		if((currentLayer==0)&&(!renderNextLayer))
		{
			levelRenderer->setGlobalChunkFlag(this->x, this->y, this->z, level, LevelRenderer::CHUNK_FLAG_EMPTY1);
			RenderManager.CBuffClear(lists + 1);
			break;
		}
	}

	// 4J MGH - added this to take the bound from the value calc'd in the tesselator
	if( bb )
	{
		bb->set(bounds.boundingBox[0], bounds.boundingBox[1], bounds.boundingBox[2],
			   bounds.boundingBox[3], bounds.boundingBox[4], bounds.boundingBox[5]);
	}

	delete tileRenderer;
	delete region;

	PIXEndNamedEvent();
	PIXBeginNamedEvent(0,"Rebuild section D");
	// 4J - have rewritten the way that tile entities are stored globally to make it work more easily with split screen. Chunks are now
	// stored globally in the levelrenderer, in a hashmap with a special key made up from the dimension and chunk position (using same index
	// as is used for global flags)
#if 1
	int key = levelRenderer->getGlobalIndexForChunk(this->x,this->y,this->z,level);
	EnterCriticalSection(globalRenderableTileEntities_cs);
	if( renderableTileEntities.size() )
	{
        auto it = globalRenderableTileEntities->find(key);
        if( it != globalRenderableTileEntities->end() )
		{
			// We've got some renderable tile entities that we want associated with this chunk, and an existing list of things that used to be.
			// We need to flag any that we don't need any more to be removed, keep those that we do, and add any new ones

			// First pass - flag everything already existing to be removed
			for(auto& it2 : it->second)
			{
				it2->setRenderRemoveStage(TileEntity::e_RenderRemoveStageFlaggedAtChunk);
			}

			// Now go through the current list. If these are already in the list, then unflag the remove flag. If they aren't, then add
			for(const auto& it3 : renderableTileEntities)
			{
                auto it2 = find(it->second.begin(), it->second.end(), it3);
                if( it2 == it->second.end() )
				{
					(*globalRenderableTileEntities)[key].push_back(it3);
				}
				else
				{
					(*it2)->setRenderRemoveStage(TileEntity::e_RenderRemoveStageKeep);
				}
			}
		}
		else
		{
			// Easy case - nothing already existing for this chunk. Add them all in.
			for( size_t i = 0; i < renderableTileEntities.size(); i++ )
			{
				(*globalRenderableTileEntities)[key].push_back(renderableTileEntities[i]);
			}
		}
	}
	else
	{
		// Another easy case - we don't want any renderable tile entities associated with this chunk. Flag all to be removed.
        auto it = globalRenderableTileEntities->find(key);
        if( it != globalRenderableTileEntities->end() )
		{
			for(auto& it2 : it->second)
			{
				it2->setRenderRemoveStage(TileEntity::e_RenderRemoveStageFlaggedAtChunk);
			}
		}
	}
	LeaveCriticalSection(globalRenderableTileEntities_cs);
	PIXEndNamedEvent();
#else
	// Find the removed ones:

	// 4J - original code for this section:
	/*
		Set<TileEntity> newTileEntities = new HashSet<TileEntity>();
		newTileEntities.addAll(renderableTileEntities);
		newTileEntities.removeAll(oldTileEntities);
		globalRenderableTileEntities.addAll(newTileEntities);

		oldTileEntities.removeAll(renderableTileEntities);
		globalRenderableTileEntities.removeAll(oldTileEntities);
		*/


    unordered_set<shared_ptr<TileEntity> > newTileEntities(renderableTileEntities.begin(),renderableTileEntities.end());

	auto endIt = oldTileEntities.end();
	for( unordered_set<shared_ptr<TileEntity> >::iterator it = oldTileEntities.begin(); it != endIt; it++ )
	{
		newTileEntities.erase(*it);
	}

	// 4J - newTileEntities is now renderableTileEntities with any old ones from oldTileEntitesRemoved (so just new things added)

	EnterCriticalSection(globalRenderableTileEntities_cs);
	endIt = newTileEntities.end();
	for( unordered_set<shared_ptr<TileEntity> >::iterator it = newTileEntities.begin(); it != endIt; it++ )
	{
		globalRenderableTileEntities->push_back(*it);
	}

	// 4J - All these new things added to globalRenderableTileEntities

	auto endItRTE = renderableTileEntities.end();
	for( vector<shared_ptr<TileEntity> >::iterator it = renderableTileEntities.begin(); it != endItRTE; it++ )
	{
		oldTileEntities.erase(*it);
	}
	// 4J - oldTileEntities is now the removed items
	vector<shared_ptr<TileEntity> >::iterator it = globalRenderableTileEntities->begin();
	while(  it != globalRenderableTileEntities->end() )
	{
		if( oldTileEntities.find(*it) != oldTileEntities.end() )
		{
			it = globalRenderableTileEntities->erase(it);
		}
		else
		{
			++it;
		}
	}

	LeaveCriticalSection(globalRenderableTileEntities_cs);
#endif

	// 4J - These removed items are now also removed from globalRenderableTileEntities

	if( LevelChunk::touchedSky )
	{
		levelRenderer->clearGlobalChunkFlag(x, y, z, level, LevelRenderer::CHUNK_FLAG_NOTSKYLIT);
	}
	else
	{
		levelRenderer->setGlobalChunkFlag(x, y, z, level, LevelRenderer::CHUNK_FLAG_NOTSKYLIT);
	}
	levelRenderer->setGlobalChunkFlag(x, y, z, level, LevelRenderer::CHUNK_FLAG_COMPILED);
	PIXEndNamedEvent();
	return;

}


#ifdef __PS3__
ChunkRebuildData g_rebuildDataIn __attribute__((__aligned__(16)));
ChunkRebuildData g_rebuildDataOut __attribute__((__aligned__(16)));
TileCompressData_SPU g_tileCompressDataIn __attribute__((__aligned__(16)));
unsigned char* g_tileCompressDataOut = (unsigned char*)&g_rebuildDataIn.m_tileIds;


void RunSPURebuild()
{

	static C4JSpursJobQueue::Port p("C4JSpursJob_ChunkUpdate");
	C4JSpursJob_CompressedTile tileJob(&g_tileCompressDataIn,g_tileCompressDataOut);
	C4JSpursJob_ChunkUpdate chunkJob(&g_rebuildDataIn, &g_rebuildDataOut);

	if(g_rebuildDataIn.m_currentLayer == 0)		// only need to create the tiles on the first layer
	{
		p.submitJob(&tileJob);
 		p.submitSync();
	}

	p.submitJob(&chunkJob);
	p.waitForCompletion();

	assert(g_rebuildDataIn.m_x0 == g_rebuildDataOut.m_x0);
}

void Chunk::rebuild_SPU()
{

//	if (!dirty) return;
	Chunk::t = Tesselator::getInstance(); // 4J - added - static initialiser being set at the wrong time
	updates++;

	int x0 = x;
	int y0 = y;
	int z0 = z;
	int x1 = x + SIZE;
	int y1 = y + SIZE;
	int z1 = z + SIZE;

	LevelChunk::touchedSky = false;

//	unordered_set<shared_ptr<TileEntity> > oldTileEntities(renderableTileEntities.begin(),renderableTileEntities.end());		// 4J removed this & next line
//	renderableTileEntities.clear();

	vector<shared_ptr<TileEntity> > renderableTileEntities;	// 4J - added

//        List<TileEntity> newTileEntities = new ArrayList<TileEntity>();
//        newTileEntities.clear();
//        renderableTileEntities.clear();

	int r = 1;

	Region region(level, x0 - r, y0 - r, z0 - r, x1 + r, y1 + r, z1 + r, r);
	TileRenderer tileRenderer(&region);

	int lists = levelRenderer->getGlobalIndexForChunk(this->x,this->y,this->z,level) * 2;
	lists += levelRenderer->chunkLists;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 4J - optimisation begins.

	// Get the data for the level chunk that this render chunk is it (level chunk is 16 x 16 x 128,
	// render chunk is 16 x 16 x 16. We wouldn't have to actually get all of it if the data was ordered differently, but currently
	// it is ordered by x then z then y so just getting a small range of y out of it would involve getting the whole thing into
	// the cache anyway.
	ChunkRebuildData* pOutData = nullptr;
	g_rebuildDataIn.buildForChunk(&region, level, x0, y0, z0);

 	Tesselator::Bounds bounds;
	{
		// this was the old default clip bounds for the chunk, set in Chunk::setPos.
		float g = 6.0f;
		bounds.boundingBox[0] = -g;
		bounds.boundingBox[1] = -g;
		bounds.boundingBox[2] = -g;
		bounds.boundingBox[3] = SIZE+g;
		bounds.boundingBox[4] = SIZE+g;
		bounds.boundingBox[5] = SIZE+g;
	}

	for (int currentLayer = 0; currentLayer < 2; currentLayer++)
	{
		bool rendered = false;

		{
			glNewList(lists + currentLayer, GL_COMPILE);
			MemSect(0);
			glPushMatrix();
			glDepthMask(true);	// 4J added
			t->useCompactVertices(true);	 // 4J added
			translateToPos();
			float ss = 1.000001f;
			// 4J - have removed this scale as I don't think we should need it, and have now optimised the vertex
			// shader so it doesn't do anything other than translate with this matrix anyway
	#if 0
			glTranslatef(-zs / 2.0f, -ys / 2.0f, -zs / 2.0f);
			glScalef(ss, ss, ss);
			glTranslatef(zs / 2.0f, ys / 2.0f, zs / 2.0f);
	#endif
			t->begin();
			t->offset((float)(-this->x), (float)(-this->y), (float)(-this->z));
		}

		g_rebuildDataIn.copyFromTesselator();
		intArray_SPU tesselatorArray((unsigned int*)g_rebuildDataIn.m_tesselator.m_PPUArray);
		g_rebuildDataIn.m_tesselator._array = &tesselatorArray;
		g_rebuildDataIn.m_currentLayer = currentLayer;
		g_tileCompressDataIn.setForChunk(&region, x0, y0, z0);
		RunSPURebuild();
 		g_rebuildDataOut.storeInTesselator();
		pOutData = &g_rebuildDataOut;

		if(pOutData->m_flags & ChunkRebuildData::e_flag_Rendered)
			rendered = true;

				// 4J - changed loop order here to leave y as the innermost loop for better cache performance
		for (int z = z0; z < z1; z++)
		{
			for (int x = x0; x < x1; x++)
			{
				for (int y = y0; y < y1; y++)
				{
					// 4J - get tile from those copied into our local array in earlier optimisation
					unsigned char tileId = pOutData->getTile(x,y,z);
					if (tileId > 0)
					{
						if (currentLayer == 0 && Tile::tiles[tileId]->isEntityTile())
						{
							shared_ptr<TileEntity> et = region.getTileEntity(x, y, z);
							if (TileEntityRenderDispatcher::instance->hasRenderer(et))
							{
								renderableTileEntities.push_back(et);
							}
						}
						int flags = pOutData->getFlags(x,y,z);
						if(flags & ChunkRebuildData::e_flag_SPURenderCodeMissing)
						{

							Tile *tile = Tile::tiles[tileId];
							int renderLayer = tile->getRenderLayer();

							if (renderLayer != currentLayer)
							{
	//							renderNextLayer = true;
							}
							else if (renderLayer == currentLayer)
							{
								//if(currentLayer == 0)
								//	numRenderedLayer0++;
 								rendered |= tileRenderer.tesselateInWorld(tile, x, y, z);
							}
						}
					}
				}
			}
		}


		{
			t->end();
			bounds.addBounds(t->bounds);
			glPopMatrix();
			glEndList();
			t->useCompactVertices(false);	 // 4J added
			t->offset(0, 0, 0);
		}
		if (rendered)
		{
			levelRenderer->clearGlobalChunkFlag(this->x, this->y, this->z, level, LevelRenderer::CHUNK_FLAG_EMPTY0, currentLayer);
		}
		else
		{
			// 4J - added - clear any renderer data associated with this unused list
			levelRenderer->setGlobalChunkFlag(this->x, this->y, this->z, level, LevelRenderer::CHUNK_FLAG_EMPTY0, currentLayer);
			RenderManager.CBuffClear(lists + currentLayer);
		}

	}

	if( bb )
	{
		bb->set(bounds.boundingBox[0], bounds.boundingBox[1], bounds.boundingBox[2],
			bounds.boundingBox[3], bounds.boundingBox[4], bounds.boundingBox[5]);
	}


	if(pOutData->m_flags & ChunkRebuildData::e_flag_TouchedSky)
		LevelChunk::touchedSky = true;


	// 4J - have rewritten the way that tile entities are stored globally to make it work more easily with split screen. Chunks are now
	// stored globally in the levelrenderer, in a hashmap with a special key made up from the dimension and chunk position (using same index
	// as is used for global flags)
#if 1
	int key = levelRenderer->getGlobalIndexForChunk(this->x,this->y,this->z,level);
	EnterCriticalSection(globalRenderableTileEntities_cs);
	if( renderableTileEntities.size() )
	{
		auto it = globalRenderableTileEntities->find(key);
		if( it != globalRenderableTileEntities->end() )
		{
			// We've got some renderable tile entities that we want associated with this chunk, and an existing list of things that used to be.
			// We need to flag any that we don't need any more to be removed, keep those that we do, and add any new ones

			// First pass - flag everything already existing to be removed
			for( auto it2 = it->second.begin(); it2 != it->second.end(); it2++ )
			{
				(*it2)->setRenderRemoveStage(TileEntity::e_RenderRemoveStageFlaggedAtChunk);
			}

			// Now go through the current list. If these are already in the list, then unflag the remove flag. If they aren't, then add
			for( size_t i = 0; i < renderableTileEntities.size(); i++ )
			{
				auto it2 = find( it->second.begin(), it->second.end(), renderableTileEntities[i] );
				if( it2 == it->second.end() )
				{
					(*globalRenderableTileEntities)[key].push_back(renderableTileEntities[i]);
				}
				else
				{
					(*it2)->setRenderRemoveStage(TileEntity::e_RenderRemoveStageKeep);
				}
			}
		}
		else
		{
			// Easy case - nothing already existing for this chunk. Add them all in.
			for( size_t i = 0; i < renderableTileEntities.size(); i++ )
			{
				(*globalRenderableTileEntities)[key].push_back(renderableTileEntities[i]);
			}
		}
	}
	else
	{
		// Another easy case - we don't want any renderable tile entities associated with this chunk. Flag all to be removed.
		auto it = globalRenderableTileEntities->find(key);
		if( it != globalRenderableTileEntities->end() )
		{
			for( auto it2 = it->second.begin(); it2 != it->second.end(); it2++ )
			{
				(*it2)->setRenderRemoveStage(TileEntity::e_RenderRemoveStageFlaggedAtChunk);
			}
		}
	}
	LeaveCriticalSection(globalRenderableTileEntities_cs);
#else
	// Find the removed ones:

	// 4J - original code for this section:
	/*
		Set<TileEntity> newTileEntities = new HashSet<TileEntity>();
		newTileEntities.addAll(renderableTileEntities);
		newTileEntities.removeAll(oldTileEntities);
		globalRenderableTileEntities.addAll(newTileEntities);

		oldTileEntities.removeAll(renderableTileEntities);
		globalRenderableTileEntities.removeAll(oldTileEntities);
		*/


    unordered_set<shared_ptr<TileEntity> > newTileEntities(renderableTileEntities.begin(),renderableTileEntities.end());

	auto endIt = oldTileEntities.end();
	for( unordered_set<shared_ptr<TileEntity> >::iterator it = oldTileEntities.begin(); it != endIt; it++ )
	{
		newTileEntities.erase(*it);
	}

	// 4J - newTileEntities is now renderableTileEntities with any old ones from oldTileEntitesRemoved (so just new things added)

	EnterCriticalSection(globalRenderableTileEntities_cs);
	endIt = newTileEntities.end();
	for( unordered_set<shared_ptr<TileEntity> >::iterator it = newTileEntities.begin(); it != endIt; it++ )
	{
		globalRenderableTileEntities.push_back(*it);
	}

	// 4J - All these new things added to globalRenderableTileEntities

	auto endItRTE = renderableTileEntities.end();
	for( vector<shared_ptr<TileEntity> >::iterator it = renderableTileEntities.begin(); it != endItRTE; it++ )
	{
		oldTileEntities.erase(*it);
	}
	// 4J - oldTileEntities is now the removed items
	vector<shared_ptr<TileEntity> >::iterator it = globalRenderableTileEntities->begin();
	while(  it != globalRenderableTileEntities->end() )
	{
		if( oldTileEntities.find(*it) != oldTileEntities.end() )
		{
			it = globalRenderableTileEntities->erase(it);
		}
		else
		{
			++it;
		}
	}

	LeaveCriticalSection(globalRenderableTileEntities_cs);
#endif

	// 4J - These removed items are now also removed from globalRenderableTileEntities

	if( LevelChunk::touchedSky )
	{
		levelRenderer->clearGlobalChunkFlag(x, y, z, level, LevelRenderer::CHUNK_FLAG_NOTSKYLIT);
	}
	else
	{
		levelRenderer->setGlobalChunkFlag(x, y, z, level, LevelRenderer::CHUNK_FLAG_NOTSKYLIT);
	}
	levelRenderer->setGlobalChunkFlag(x, y, z, level, LevelRenderer::CHUNK_FLAG_COMPILED);
	return;

}
#endif // _PS3_


float Chunk::distanceToSqr(shared_ptr<Entity> player) const
{
	float xd = static_cast<float>(player->x - xm);
	float yd = static_cast<float>(player->y - ym);
	float zd = static_cast<float>(player->z - zm);
	return xd * xd + yd * yd + zd * zd;
}

float Chunk::squishedDistanceToSqr(shared_ptr<Entity> player)
{
	float xd = static_cast<float>(player->x - xm);
	float yd = static_cast<float>(player->y - ym) * 2;
	float zd = static_cast<float>(player->z - zm);
	return xd * xd + yd * yd + zd * zd;
}

void Chunk::reset()
{
	if( assigned )
	{
		EnterCriticalSection(&levelRenderer->m_csDirtyChunks);
		unsigned char refCount = levelRenderer->decGlobalChunkRefCount(x, y, z, level);
		assigned = false;
//		printf("\t\t [dec] refcount %d at %d, %d, %d\n",refCount,x,y,z);
		if( refCount == 0 )
		{
			int lists = levelRenderer->getGlobalIndexForChunk(x, y, z, level) * 2;
			if(lists >= 0)
			{
				lists += levelRenderer->chunkLists;
				for (int i = 0; i < 2; i++)
				{
					// 4J - added - clear any renderer data associated with this unused list
					RenderManager.CBuffClear(lists + i);
				}
				levelRenderer->setGlobalChunkFlags(x, y, z, level, 0);
			}
		}
		LeaveCriticalSection(&levelRenderer->m_csDirtyChunks);
	}

	clipChunk->visible = false;
}

void Chunk::_delete()
{
	reset();
	level = nullptr;
}

int Chunk::getList(int layer)
{
	if (!clipChunk->visible) return -1;

	int lists = levelRenderer->getGlobalIndexForChunk(x, y, z,level) * 2;
	lists += levelRenderer->chunkLists;

	bool empty =  levelRenderer->getGlobalChunkFlag(x, y, z, level, LevelRenderer::CHUNK_FLAG_EMPTY0, layer);
	if (!empty) return lists + layer;
	return -1;
}

void Chunk::cull(Culler *culler)
{
	clipChunk->visible = culler->isVisible(bb.get());
}

void Chunk::renderBB()
{
//	glCallList(lists + 2);	// 4J - removed - TODO put back in
}

bool Chunk::isEmpty()
{
	if (!levelRenderer->getGlobalChunkFlag(x, y, z, level, LevelRenderer::CHUNK_FLAG_COMPILED)) return false;
	return levelRenderer->getGlobalChunkFlag(x, y, z, level, LevelRenderer::CHUNK_FLAG_EMPTYBOTH);
}

void Chunk::setDirty()
{
	// 4J - not used, but if this starts being used again then we'll need to investigate how best to handle it.
	__debugbreak();
	levelRenderer->setGlobalChunkFlag(x, y, z, level, LevelRenderer::CHUNK_FLAG_DIRTY);
}

void Chunk::clearDirty()
{
	levelRenderer->clearGlobalChunkFlag(x, y, z, level, LevelRenderer::CHUNK_FLAG_DIRTY);
#ifdef _CRITICAL_CHUNKS
	levelRenderer->clearGlobalChunkFlag(x, y, z, level, LevelRenderer::CHUNK_FLAG_CRITICAL);
#endif
}

Chunk::~Chunk() = default;

bool Chunk::emptyFlagSet(int layer)
{
	return levelRenderer->getGlobalChunkFlag(x, y, z, level, LevelRenderer::CHUNK_FLAG_EMPTY0, layer);
}
