#include "stdafx.h"
#include "../Minecraft.World/JavaMath.h"
#include "DragonBreathParticle.h"

void DragonBreathParticle::init(Level *level, double x, double y, double z, double xa, double ya, double za, float scale)
{
    xd *= 0.1f;
    yd *= 0.1f;
    zd *= 0.1f;
    xd = xa; //+= xa;
    yd = ya; //+= ya;
    zd = za; //+= za;

	unsigned int cMin = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Particle_DragonBreathMin ); //0xb700d2
	unsigned int cMax = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Particle_DragonBreathMax ); //0xdf00f9
	double rMin = ( (cMin>>16)&0xFF )/255.0f, gMin = ( (cMin>>8)&0xFF )/255.0, bMin = ( cMin&0xFF )/255.0;
	double rMax = ( (cMax>>16)&0xFF )/255.0f, gMax = ( (cMax>>8)&0xFF )/255.0, bMax = ( cMax&0xFF )/255.0;

    rCol = (rMax - rMin) * Math::random() + rMin; // 184/255 -- 224/255
	gCol = (gMax - gMin) * Math::random() + gMin; // 0,0
	bCol = (bMax - bMin) * Math::random() + bMin; // 210/255 -- 250/255

    size *= 0.75f;
    size *= scale;
    oSize = size;

    lifetime = static_cast<int>(20 / (Math::random() * 0.8 + 0.2));
    lifetime = static_cast<int>(lifetime * scale);
    noPhysics = false;

	m_bHasHitGround = false;
}

DragonBreathParticle::DragonBreathParticle(Level *level, double x, double y, double z, double xa, double ya, double za) : Particle(level, x, y, z, 0, 0, 0)
{
	init(level, x, y, z, xa, ya, za, 1);
}

DragonBreathParticle::DragonBreathParticle(Level *level, double x, double y, double z, double xa, double ya, double za, float scale) : Particle(level, x, y, z, 0, 0, 0)
{
	init(level, x, y, z, xa, ya, za, scale);
}

void DragonBreathParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
    float l = ((age + a) / lifetime) * 32;
    if (l < 0) l = 0;
    if (l > 1) l = 1;

    size = oSize * l;
    Particle::render(t, a, xa, ya, za, xa2, za2);
}

void DragonBreathParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime) remove();

    setMiscTex( ( 3 * age / lifetime) + 5 );

	if(onGround)
	{
		yd = 0;
		m_bHasHitGround = true;
	}

	if(m_bHasHitGround) yd += 0.002; //0.004;

    move(xd, yd, zd);
    if (y == yo)
	{
        xd *= 1.1;
        zd *= 1.1;
    }
    xd *= 0.96f;
    zd *= 0.96f;

	if(m_bHasHitGround) yd *= 0.96f;

 //   if (onGround)
	//{
 //       xd *= 0.7f;
 //       zd *= 0.7f;
 //   }
}

int DragonBreathParticle::getParticleTexture()
{
	return ParticleEngine::DRAGON_BREATH_TEXTURE;
}

float DragonBreathParticle::getBrightness(float a)
{
	float l = ((age + a) / lifetime) * 32;
    if (l < 0) l = 0;
    if (l > 1) l = 1;

	float brightness = (0.5f / l) + 0.5f;

	return brightness;
}