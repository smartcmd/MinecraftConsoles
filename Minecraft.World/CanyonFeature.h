#pragma once
#include "LargeFeature.h"

class Level;

class CanyonFeature : public LargeFeature
{
private:
	float rs[1024];

protected:
    void addTunnel(__int64 seed, int xOffs, int zOffs, byteArray blocks, double xCave, double yCave, double zCave, float thickness, float yRot, float xRot, int step, int dist, double yScale);
    virtual void addFeature(Level *level, int x, int z, int xOffs, int zOffs, byteArray blocks);
};
