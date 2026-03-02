#pragma once

class Level;

class Feature
{
private:
	bool doUpdate;
public:
	Feature();
	Feature(bool doUpdate);
	virtual ~Feature() {};

	virtual bool place(Level *level, Random *random, int x, int y, int z) = 0;
	virtual bool placeWithIndex(Level *level, Random *random, int x, int y, int z,int iIndex, int iRadius) { return false;}
	virtual void init(double V1, double V2, double V3) {};
	virtual void applyFeature(Level *level, Random *random, int xChunk, int zChunk);

protected:
	virtual void placeBlock(Level *level, int x, int y, int z, int tile);
	virtual void placeBlock(Level *level, int x, int y, int z, int tile, int data);
};
