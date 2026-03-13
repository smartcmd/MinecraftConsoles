#include "stdafx.h"
#include "WaterDropParticle.h"
#include "../Minecraft.World/JavaMath.h"
#include "../Minecraft.World/net.minecraft.world.level.h"
#include "../Minecraft.World/net.minecraft.world.level.material.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"

WaterDropParticle::WaterDropParticle(Level *level, double x, double y, double z) : Particle(level, x, y, z, 0, 0, 0)
{
    xd *= 0.3f;
    yd = static_cast<float>(Math::random()) * 0.2f + 0.1f;
    zd *= 0.3f;

    rCol = 1.0f;
    gCol = 1.0f;
    bCol = 1.0f;
    setMiscTex(16+3+random->nextInt(4));
    this->setSize(0.01f, 0.01f);
    gravity = 0.06f;

	noPhysics = true;	// 4J - optimisation - do we really need collision on these? its really slow...
    lifetime = static_cast<int>(8 / (Math::random() * 0.8 + 0.2));
}

void WaterDropParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    yd -= gravity;
    move(xd, yd, zd);
    xd *= 0.98f;
    yd *= 0.98f;
    zd *= 0.98f;

    if (lifetime-- <= 0) remove();

    if (onGround)
	{
        if (Math::random() < 0.5) remove();
        xd *= 0.7f;
        zd *= 0.7f;
    }

    Material *m = level->getMaterial(Mth::floor(x), Mth::floor(y), Mth::floor(z));
    if (m->isLiquid() || m->isSolid())
	{
        double y0 = Mth::floor(y) + 1 - LiquidTile::getHeight(level->getData(Mth::floor(x), Mth::floor(y), Mth::floor(z)));
        if (y < y0)
		{
            remove();
        }
    }
}
