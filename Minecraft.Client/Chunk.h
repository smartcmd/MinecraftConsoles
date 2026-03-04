#pragma once
#include "AllowAllCuller.h"
#include "Tesselator.h"
#include "..\Minecraft.World\ArrayWithLength.h"
#include "LevelRenderer.h"

class Level;
class TileEntity;
class Entity;
using namespace std;

class ClipChunk
{
public:
	Chunk *chunk;
	int globalIdx;
	bool visible;
	float aabb[6];
	int xm, ym, zm;
};

class Chunk
{
private:
	static const int XZSIZE = LevelRenderer::CHUNK_XZSIZE;
	static const int SIZE = LevelRenderer::CHUNK_SIZE;

public:
	Level *level;
	static LevelRenderer *levelRenderer;
private:
#ifndef _LARGE_WORLDS
    static Tesselator *t;
#else
	static DWORD tlsIdx;
public:
	static void CreateNewThreadStorage();
	static void ReleaseThreadStorage();
	static unsigned char *GetTileIdsStorage();
#endif
	
public:
	static int updates;

    int x, y, z;
    int xRender, yRender, zRender;
    int xRenderOffs, yRenderOffs, zRenderOffs;
 
    int xm, ym, zm;
    AABB *bb;
	ClipChunk *clipChunk;

    int id;
//public:
//	vector<shared_ptr<TileEntity> > renderableTileEntities;		// 4J - removed
    
private:
	LevelRenderer::rteMap			*globalRenderableTileEntities;
	CRITICAL_SECTION				*globalRenderableTileEntities_cs;
	bool							assigned;
public:
	Chunk(Level *level, LevelRenderer::rteMap &globalRenderableTileEntities, CRITICAL_SECTION &globalRenderableTileEntities_cs, int x, int y, int z, ClipChunk *clipChunk);
	Chunk();

    void setPos(int x, int y, int z);
private:
	void translateToPos();
public:
	void makeCopyForRebuild(Chunk *source);
	void rebuild();
#ifdef __PS3__
	void rebuild_SPU();
#endif // __PS3__
	float distanceToSqr(shared_ptr<Entity> player) const;
    float squishedDistanceToSqr(shared_ptr<Entity> player);
    void reset();
    void _delete();

    int getList(int layer);
    void cull(Culler *culler);
    void renderBB() ;
    bool isEmpty();
    void setDirty();
	void clearDirty();	// 4J added
	bool emptyFlagSet(int layer);
	~Chunk();
};
