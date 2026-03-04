#include "stdafx.h"
#include "CritParticle.h"
#include "..\Minecraft.World\net.minecraft.world.entity.h"
#include "..\Minecraft.World\Random.h"
#include "..\Minecraft.World\net.minecraft.world.phys.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"

void CritParticle::_init(Level *level, shared_ptr<Entity> entity, ePARTICLE_TYPE type)
{
	life = 0;
	this->entity = entity;
	lifeTime = 3;
	particleName = type;
	// 4J-PB - can't use a shared_from_this in the constructor
	//tick();
}

CritParticle::CritParticle(Level *level, shared_ptr<Entity> entity) : Particle(level, entity->x, entity->bb->y0 + entity->bbHeight / 2, entity->z, entity->xd, entity->yd, entity->zd)
{
	_init(level,entity,eParticleType_crit);
}

CritParticle::CritParticle(Level *level, shared_ptr<Entity> entity, ePARTICLE_TYPE type) : Particle(level, entity->x, entity->bb->y0 + entity->bbHeight / 2, entity->z, entity->xd, entity->yd, entity->zd)
{
	_init(level, entity, type);
}

// 4J - Added this so that we can use some shared_ptr functions that were needed in the ctor
void CritParticle::CritParticlePostConstructor(void)
{
	tick();
}

void CritParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
}

void CritParticle::tick()
{
	for (int i=0; i<16; i++)
	{
		double xa = random->nextFloat()*2-1;
		double ya = random->nextFloat()*2-1;
		double za = random->nextFloat()*2-1;
		if (xa*xa+ya*ya+za*za>1) continue;
		double x = entity->x+xa*entity->bbWidth/4;
		double y = entity->bb->y0+entity->bbHeight/2+ya*entity->bbHeight/4;
		double z = entity->z+za*entity->bbWidth/4;
		level->addParticle(particleName, x, y, z, xa, ya+0.2, za);
	}
	life++;
	if (life >= lifeTime) 
	{
		remove();
	}
}

int CritParticle::getParticleTexture()
{
	return ParticleEngine::ENTITY_PARTICLE_TEXTURE;
}