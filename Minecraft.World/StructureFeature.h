#pragma once
#include "LargeFeature.h"
#include "StructureFeatureSavedData.h"

class StructureStart;

//#define ENABLE_STRUCTURE_SAVING

class StructureFeature : public LargeFeature
{
public:
	// 4J added - Maps to values in the game rules xml
	enum EFeatureTypes
	{
		eFeature_Mineshaft,
		eFeature_NetherBridge,
		eFeature_Temples,
		eFeature_Stronghold,
		eFeature_Village,
	};

#ifdef ENABLE_STRUCTURE_SAVING
private:
	shared_ptr<StructureFeatureSavedData> savedData;
#endif


protected:
	unordered_map<int64_t, StructureStart *> cachedStructures;

public:
	StructureFeature();
	~StructureFeature();

	virtual wstring getFeatureName() = 0;

	virtual void addFeature(Level *level, int x, int z, int xOffs, int zOffs, byteArray blocks);

	bool postProcess(Level *level, Random *random, int chunkX, int chunkZ);
	bool isIntersection(int cellX, int cellZ);

	bool isInsideFeature(int cellX, int cellY, int cellZ);

protected:
	StructureStart *getStructureAt(int cellX, int cellY, int cellZ);

public:
	bool isInsideBoundingFeature(int cellX, int cellY, int cellZ);
	TilePos *getNearestGeneratedFeature(Level *level, int cellX, int cellY, int cellZ);

protected:
	vector<TilePos> *getGuesstimatedFeaturePositions();

private:
	virtual void restoreSavedData(Level *level);
	virtual void saveFeature(int chunkX, int chunkZ, StructureStart *feature);

	/**
	* Returns true if the given chunk coordinates should hold a structure
	* source.
	*
	* @param x
	*            chunk x
	* @param z
	*            chunk z
	* @return
	*/
protected:
	virtual bool isFeatureChunk(int x, int z, bool bIsSuperflat=false) = 0;

	/**
	* Creates a new instance of a structure source at the given chunk
	* coordinates.
	*
	* @param x
	*            chunk x
	* @param z
	*            chunk z
	* @return
	*/
	virtual StructureStart *createStructureStart(int x, int z) = 0;
};
