#include "stdafx.h"
#include "../Minecraft.World/net.minecraft.world.level.h"
#include "../Minecraft.World/net.minecraft.world.level.material.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../Minecraft.World/JavaMath.h"
#include "../Minecraft.World/Mth.h"
#include "DripParticle.h"

DripParticle::DripParticle(Level *level, double x, double y, double z, Material *material) : Particle(level, x, y, z, 0, 0, 0)
{
	xd = yd = zd = 0;

	unsigned int clr;
	if (material == Material::water)
	{
		clr = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Particle_DripWater);
	}
	else
	{
		clr = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Particle_DripLavaStart );
	}

	rCol = ( (clr>>16)&0xFF )/255.0f;
	gCol = ( (clr>>8)&0xFF )/255.0;
	bCol = ( clr&0xFF )/255.0;

	setMiscTex(16 * 7 + 1);
	this->setSize(0.01f, 0.01f);
	gravity = 0.06f;
	this->material = material;
	stuckTime = 40;

	lifetime = static_cast<int>(64 / (Math::random() * 0.8 + 0.2));
	xd = yd = zd = 0;
}

int DripParticle::getLightColor(float a)
{
	if (material == Material::water) return Particle::getLightColor(a);
	
	// 4J-JEV: Looks like this value was never used on the java version,
	// but it is on ours, so I've changed this to be bright manualy.
	int s = 0x0f;
	int b = 0x0f;
	return s << 20 | b << 4; // MGH changed this to a proper value as PS3 wasn't clamping the values.
}

float DripParticle::getBrightness(float a)
{
	if (material == Material::water) return Particle::getBrightness(a);
	else return 1.0f;
}

void DripParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;

	if (material == Material::water)
	{
		//rCol = 0.2f;
		//gCol = 0.3f;
		//bCol = 1.0f;

		unsigned int clr = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Particle_DripWater);
		rCol = ( (clr>>16)&0xFF )/255.0f;
		gCol = ( (clr>>8)&0xFF )/255.0;
		bCol = ( clr&0xFF )/255.0;
	}
	else
	{
		//rCol = 1.0f;
		//gCol = 16.0f / (40 - stuckTime + 16);
		//bCol = 4.0f / (40 - stuckTime + 8);

		unsigned int cStart = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Particle_DripLavaStart );
		unsigned int cEnd = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Particle_DripLavaEnd );
		double rStart = ( (cStart>>16)&0xFF )/255.0f, gStart = ( (cStart>>8)&0xFF )/255.0, bStart = ( cStart&0xFF )/255.0;
		double rEnd = ( (cEnd>>16)&0xFF )/255.0f, gEnd = ( (cEnd>>8)&0xFF )/255.0, bEnd = ( cEnd&0xFF )/255.0;

		float variance = (40 - stuckTime);
		rCol = rStart - ((rStart - rEnd)/40) * variance;
		gCol = gStart - ((gStart - gEnd)/40) * variance;
		bCol = bStart - ((bStart - bEnd)/40) * variance;
	}

	yd -= gravity;
	if (stuckTime-- > 0)
	{
		xd *= 0.02;
		yd *= 0.02;
		zd *= 0.02;
		setMiscTex(16 * 7 + 1);
	}
	else
	{
		setMiscTex(16 * 7 + 0);
	}
	move(xd, yd, zd);
	xd *= 0.98f;
	yd *= 0.98f;
	zd *= 0.98f;

	if (lifetime-- <= 0) remove();

	if (onGround)
	{
		if (material == Material::water)
		{
			remove();
			level->addParticle(eParticleType_splash, x, y, z, 0, 0, 0);
		}
		else
		{
			setMiscTex(16 * 7 + 2);

		}
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
