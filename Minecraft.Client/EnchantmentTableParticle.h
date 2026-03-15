#pragma once

#include "Particle.h"

class Level;

class EnchantmentTableParticle : public Particle
{
private:
	float oSize;
	double xStart, yStart, zStart;

public:
	virtual eINSTANCEOF GetType() { return eTYPE_ENCHANTMENTTABLEPARTICLE; }

	EnchantmentTableParticle(Level *level, double x, double y, double z, double xd, double yd, double zd);

	virtual int getLightColor(float a);
	virtual float getBrightness(float a);
	virtual void tick();
};
