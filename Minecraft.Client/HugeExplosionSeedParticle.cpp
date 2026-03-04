#include "stdafx.h"
#include "HugeExplosionSeedParticle.h"
#include "..\Minecraft.World\Random.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"

HugeExplosionSeedParticle::HugeExplosionSeedParticle(Level *level, double x, double y, double z, double xa, double ya, double za) : Particle(level,x,y,z,0,0,0)
{
	life = 0;

	lifeTime = 8;
}

void HugeExplosionSeedParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
}

void HugeExplosionSeedParticle::tick()
{
	// Horrible hack to communicate with the level renderer, which is just attached as a listener to this level. This let's the particle
	// rendering know to use this level (rather than try to work it out from the current player), and to not bother distance clipping particles
	// which would again be based on the current player.
	Minecraft::GetInstance()->animateTickLevel = level;
	for (int i = 0; i < 6; i++) {
		double xx = x + (random->nextDouble() - random->nextDouble()) * 4;
		double yy = y + (random->nextDouble() - random->nextDouble()) * 4;
		double zz = z + (random->nextDouble() - random->nextDouble()) * 4;
		level->addParticle(eParticleType_largeexplode, xx, yy, zz, life / (float) lifeTime, 0, 0);
	}
	Minecraft::GetInstance()->animateTickLevel = NULL;
	life++;
	if (life == lifeTime) remove();
}

int HugeExplosionSeedParticle::getParticleTexture()
{
	return ParticleEngine::TERRAIN_TEXTURE;
}