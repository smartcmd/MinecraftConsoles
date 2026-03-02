#pragma once
#include "LargeFeature.h"
class StructureStart;

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

protected:
	unordered_map<__int64, StructureStart *> cachedStructures;

public:
	~StructureFeature();

	virtual void addFeature(Level *level, int x, int z, int xOffs, int zOffs, byteArray blocks);

    bool postProcess(Level *level, Random *random, int chunkX, int chunkZ);
    bool isIntersection(int cellX, int cellZ);

	bool isInsideFeature(int cellX, int cellY, int cellZ);
	TilePos *getNearestGeneratedFeature(Level *level, int cellX, int cellY, int cellZ);
protected:
	vector<TilePos> *getGuesstimatedFeaturePositions(); 

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
