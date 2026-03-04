#include "stdafx.h"
#include "..\Minecraft.World\JavaMath.h"
#include "EchantmentTableParticle.h"

EchantmentTableParticle::EchantmentTableParticle(Level *level, double x, double y, double z, double xd, double yd, double zd) : Particle(level, x, y, z, xd, yd, zd)
{
	this->xd = xd;
	this->yd = yd;
	this->zd = zd;
	this->xStart = this->x = x;
	this->yStart = this->y = y;
	this->zStart = this->z = z;

	unsigned int clr = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Particle_EnchantmentTable ); //0xE5E5FF
	double r = ( (clr>>16)&0xFF )/255.0f, g = ( (clr>>8)&0xFF )/255.0, b = ( clr&0xFF )/255.0;

	float br = random->nextFloat() * 0.6f + 0.4f;
	rCol = r * br;
	gCol = g * br;
	bCol = b * br;

	oSize = size = random->nextFloat() * 0.5f + 0.2f;

	lifetime = (int) (Math::random() * 10) + 30;
	noPhysics = true;
	setMiscTex( (int) (Math::random() * 26 + 1 + 14 * 16) );
}

int EchantmentTableParticle::getLightColor(float a)
{
	int br = Particle::getLightColor(a);

	float pos = age / (float) lifetime;
	pos = pos * pos;
	pos = pos * pos;

	int br1 = (br) & 0xff;
	int br2 = (br >> 16) & 0xff;
	br2 += (int) (pos * 15 * 16);
	if (br2 > 15 * 16) br2 = 15 * 16;
	return br1 | br2 << 16;
}

float EchantmentTableParticle::getBrightness(float a)
{
	float br = Particle::getBrightness(a);
	float pos = age / (float) lifetime;
	pos = pos * pos;
	pos = pos * pos;
	return br * (1 - pos) + pos;
}

void EchantmentTableParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;

	float pos = age / (float) lifetime;

	pos = 1 - pos;

	float pp = 1 - pos;
	pp = pp * pp;
	pp = pp * pp;
	x = xStart + xd * pos;
	y = yStart + yd * pos - pp * 1.2f;
	z = zStart + zd * pos;

	if (age++ >= lifetime) remove();
}