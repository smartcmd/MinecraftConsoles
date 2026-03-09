#pragma once

#include "LargeFeature.h"

class LargeHellCaveFeature : public LargeFeature
{
protected:
    void addRoom(int64_t seed, int xOffs, int zOffs, byteArray blocks, double xRoom, double yRoom, double zRoom);
    void addTunnel(int64_t seed, int xOffs, int zOffs, byteArray blocks, double xCave, double yCave, double zCave, float thickness, float yRot, float xRot, int step, int dist, double yScale);
    virtual void addFeature(Level *level, int x, int z, int xOffs, int zOffs, byteArray blocks);
};
