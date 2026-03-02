#pragma once
class StructurePiece;
class BoundingBox;

#include "StructureFeatureIO.h"

class StructureStart
{
public:
	list<StructurePiece *> pieces;

protected:
	BoundingBox *boundingBox;

private:
	int chunkX, chunkZ;

public:
	StructureStart();
	StructureStart(int x, int z);
	~StructureStart();
	BoundingBox *getBoundingBox();
	list<StructurePiece *> *getPieces();
	void postProcess(Level *level, Random *random, BoundingBox *chunkBB);
protected:
	void calculateBoundingBox();

public:
	virtual CompoundTag *createTag(int chunkX, int chunkZ);
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void load(Level *level, CompoundTag *tag);
	virtual void readAdditonalSaveData(CompoundTag *tag);

protected:
	void moveBelowSeaLevel(Level *level, Random *random, int offset);
	void moveInsideHeights(Level *level, Random *random, int lowestAllowed, int highestAllowed);
public:
	bool isValid();
	int getChunkX();
	int getChunkZ();

	virtual EStructureStart GetType() = 0;
};
