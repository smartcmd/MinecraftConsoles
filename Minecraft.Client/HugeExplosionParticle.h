#pragma once

#include "Particle.h"

class HugeExplosionParticle : public Particle
{
private:
	static ResourceLocation EXPLOSION_LOCATION;
	int life;
	int lifeTime;
	Textures *textures;
	float size;

public:
	virtual eINSTANCEOF GetType() { return eType_HUGEEXPLOSIONPARTICLE; }
	HugeExplosionParticle(Textures *textures, Level *level, double x, double y, double z, double xa, double ya, double za);
	void render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2);
	int getLightColor(float a);
	void tick();
	int getParticleTexture();
};