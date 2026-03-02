#pragma once
#include "Particle.h"
class Entity;

class TakeAnimationParticle : public Particle
{
public:
	virtual eINSTANCEOF GetType() { return eType_TAKEANIMATIONPARTICLE; }
private:
    shared_ptr<Entity> item;
    shared_ptr<Entity> target;
    int life;
    int lifeTime;
    float yOffs;

public:
	TakeAnimationParticle(Level *level, shared_ptr<Entity> item, shared_ptr<Entity> target, float yOffs);
	~TakeAnimationParticle();
    virtual void render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2);
    virtual void tick();
    virtual int getParticleTexture();
};