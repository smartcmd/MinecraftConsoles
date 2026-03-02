#pragma once

#include "Particle.h"

class Entity;

class CritParticle : public Particle
{
private:
	shared_ptr<Entity> entity;
	int life;
	int lifeTime;
	ePARTICLE_TYPE particleName;

	void _init(Level *level, shared_ptr<Entity> entity, ePARTICLE_TYPE type);

public:
	virtual eINSTANCEOF GetType() { return eType_CRITPARTICLE; }
	CritParticle(Level *level, shared_ptr<Entity> entity);
	CritParticle(Level *level, shared_ptr<Entity> entity, ePARTICLE_TYPE type);
	void CritParticlePostConstructor(void);
	void render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2);
	void tick();
	int getParticleTexture();
};