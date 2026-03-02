#pragma once
#include "WeighedRandom.h"
#include "BoundingBox.h"
#include "StructureFeatureIO.h"

class Level;
class Random;
class ChunkPos;
class BlockSelector;
class ChestTileEntity;
class TilePos;

/**
* 
* A structure piece is a construction or room, located somewhere in the world
* with a given orientatino (out of Direction.java). Structure pieces have a
* bounding box that says where the piece is located and its bounds, and the
* orientation is used to translate local coordinates into world coordinates.
* <p>
* The default orientation is Direction.UNDEFINED, in which case no translation
* will occur. If the orientation is Direction.NORTH, coordinate (0, 0, 0) will
* be at (boundingBox.x0, boundingBox.y0, boundingBox.z1). In other words, (1,
* 1, 1) will be translated to (boundingBox.x0 + 1, boundingBox.y0 + 1,
* boundingBox.z1 - 1).
* <p>
* When using Direction.SOUTH, the x coordinate will be the same, and the z
* coordinate will be flipped. In other words, the bounding box is NOT rotated!
* It is only flipped along the z axis. Also note that the bounding box is in
* world coordinates, so the local drawing must never reach outside of this.
* <p>
* When using east and west coordinates, the local z coordinate will be swapped
* with the local x coordinate. For example, (0, 0, 0) is (boundingBox.z1,
* boundingBox.y0, boundingBox.z0), and (1, 1, 1) becomes (boundingBox.x1 - 1,
* boundingBox.y0 + 1, boundingBox.z0 + 1) when using Direction.WEST.
* <p>
* When-ever a structure piece is placing blocks, it is VERY IMPORTANT to always
* make sure that all getTile and setTile calls are within the chunk's bounding
* box. Failing to check this will cause the level generator to create new
* chunks, leading to infinite loops and other errors.
*/
class StructurePiece
{
public:
	virtual EStructurePiece GetType() = 0;

public:
	class BlockSelector
	{

	protected:
		int nextId;
		int nextData;

	public:
		virtual void next(Random *random, int worldX, int worldY, int worldZ, bool isEdge) {}

		virtual int getNextId() { return nextId; }
		virtual int getNextData() { return nextData; }
	};

public:		// 4J is protected in java, but accessed from VillagePieces, not sure how
	BoundingBox *boundingBox;
protected:
	int orientation;
	int genDepth;

public:
	StructurePiece();

protected:
	StructurePiece(int genDepth);

public:
	virtual ~StructurePiece();

	virtual CompoundTag *createTag();

protected:
	virtual void addAdditonalSaveData(CompoundTag *tag) = 0;

public:
	virtual void load(Level *level, CompoundTag *tag);

protected:
	virtual void readAdditonalSaveData(CompoundTag *tag) = 0;

public:
	virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
	virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB) = 0;

	virtual BoundingBox *getBoundingBox();

	int getGenDepth();

public:
	bool isInChunk(ChunkPos *pos);
	static StructurePiece *findCollisionPiece(list<StructurePiece *> *pieces, BoundingBox *box);
	virtual TilePos *getLocatorPosition();
protected:
	bool edgesLiquid(Level *level, BoundingBox *chunkBB);
public:
	// 4J Stu - Made these public to use in game rules
	int getWorldX(int x, int z);
	int getWorldY(int y);
	int getWorldZ(int x, int z);
	int getOrientationData(int tile, int data);
	virtual void placeBlock(Level *level, int block, int data, int x, int y, int z, BoundingBox *chunkBB);

	/**
	* The purpose of this method is to wrap the getTile call on Level, in order
	* to prevent the level from generating chunks that shouldn't be loaded yet.
	* Returns 0 if the call is out of bounds.
	* 
	* @param level
	* @param x
	* @param y
	* @param z
	* @param chunkPosition
	* @return
	*/
	virtual int getBlock(Level *level, int x, int y, int z, BoundingBox *chunkBB);
	virtual void generateAirBox(Level *level, BoundingBox *chunkBB, int x0, int y0, int z0, int x1, int y1, int z1);
	virtual void generateBox(Level *level, BoundingBox *chunkBB, int x0, int y0, int z0, int x1, int y1, int z1, int edgeTile, int fillTile, bool skipAir);
	virtual void generateBox(Level *level, BoundingBox *chunkBB, int x0, int y0, int z0, int x1, int y1, int z1, int edgeTile, int edgeData, int fillTile, int fillData, bool skipAir);
	virtual void generateBox(Level *level, BoundingBox *chunkBB, BoundingBox *boxBB, int edgeTile, int fillTile, bool skipAir);
	virtual void generateBox(Level *level, BoundingBox *chunkBB, int x0, int y0, int z0, int x1, int y1, int z1, bool skipAir, Random *random, BlockSelector *selector);
	virtual void generateBox(Level *level, BoundingBox *chunkBB, BoundingBox *boxBB, bool skipAir, Random *random, BlockSelector *selector);
	virtual void generateMaybeBox(Level *level, BoundingBox *chunkBB, Random *random, float probability, int x0, int y0, int z0, int x1, int y1, int z1, int edgeTile, int fillTile, bool skipAir);
	virtual void maybeGenerateBlock(Level *level, BoundingBox *chunkBB, Random *random, float probability, int x, int y, int z, int tile, int data);
	virtual void generateUpperHalfSphere(Level *level, BoundingBox *chunkBB, int x0, int y0, int z0, int x1, int y1, int z1, int fillTile, bool skipAir);
	virtual void generateAirColumnUp(Level *level, int x, int startY, int z, BoundingBox *chunkBB);
	virtual void fillColumnDown(Level *level, int tile, int tileData, int x, int startY, int z, BoundingBox *chunkBB);
	virtual bool createChest(Level *level, BoundingBox *chunkBB, Random *random, int x, int y, int z, WeighedTreasureArray treasure, int numRolls);
	virtual bool createDispenser(Level *level, BoundingBox *chunkBB, Random *random, int x, int y, int z, int facing, WeighedTreasureArray items, int numRolls);

protected:
	void createDoor(Level *level, BoundingBox *chunkBB, Random *random, int x, int y, int z, int orientation);
};
