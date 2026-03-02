#pragma once
#include "Particle.h"
class Tile;

class TerrainParticle : public Particle
{
public:
	virtual eINSTANCEOF GetType() { return eType_TERRAINPARTICLE; }
private:
	Tile *tile;

public:
	TerrainParticle(Level *level, double x, double y, double z, double xa, double ya, double za, Tile *tile, int face, int data, Textures *textures);
    shared_ptr<TerrainParticle> init(int x, int y, int z, int data);	// 4J - added data parameter
	shared_ptr<TerrainParticle> init(int data);
    virtual int getParticleTexture();
    virtual void render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2);
};