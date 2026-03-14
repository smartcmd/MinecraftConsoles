#include "stdafx.h"
#include "HugeExplosionParticle.h"
#include "../Minecraft.World/Random.h"
#include "Textures.h"
#include "Tesselator.h"
#include "Lighting.h"
#include "ResourceLocation.h"

ResourceLocation HugeExplosionParticle::EXPLOSION_LOCATION = ResourceLocation(TN_MISC_EXPLOSION);

HugeExplosionParticle::HugeExplosionParticle(Textures *textures, Level *level, double x, double y, double z, double xa, double ya, double za) : Particle(level,x,y,z,0,0,0)
{
	life = 0;

	this->textures = textures;
	lifeTime = 6 + random->nextInt(4);

	// rCol = gCol = bCol = random->nextFloat() * 0.6f + 0.4f;

	unsigned int clr = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Particle_HugeExplosion );  //0x999999
	double r = ( (clr>>16)&0xFF )/255.0f, g = ( (clr>>8)&0xFF )/255.0, b = ( clr&0xFF )/255.0;

	double br = random->nextFloat() * 0.6 + 0.4;
	rCol = r * br;
	gCol = g * br;
	bCol = b * br;
	
	size = 1 - static_cast<float>(xa) * 0.5f;
}

void HugeExplosionParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
	int tex = static_cast<int>((life + a) * 15 / lifeTime);
	if (tex > 15) return;
	textures->bindTexture(&EXPLOSION_LOCATION);

	float u0 = (tex % 4) / 4.0f;
	float u1 = u0 + 0.999f / 4.0f;
	float v0 = (tex / 4) / 4.0f;
	float v1 = v0 + 0.999f / 4.0f;

	float r = 2.0f * size;

	float x = static_cast<float>(xo + (this->x - xo) * a - xOff);
	float y = static_cast<float>(yo + (this->y - yo) * a - yOff);
	float z = static_cast<float>(zo + (this->z - zo) * a - zOff);

	// 4J - don't render explosion particles that are less than 3 metres away, to try and avoid large particles that are causing us problems with photosensitivity testing
	float distSq = (x*x + y*y + z*z);
	if( distSq < ( 3.0f * 3.0f )) return;

	glColor4f(1, 1, 1, 1);
	glDisable(GL_LIGHTING);
	Lighting::turnOff();
	t->begin();
	t->color(rCol, gCol, bCol, 1.0f);
	t->normal(0, 1, 0);
	t->tex2(0x00f0);
	t->vertexUV(x - xa * r - xa2 * r, y - ya * r, z - za * r - za2 * r, u1, v1);
	t->vertexUV(x - xa * r + xa2 * r, y + ya * r, z - za * r + za2 * r, u1, v0);
	t->vertexUV(x + xa * r + xa2 * r, y + ya * r, z + za * r + za2 * r, u0, v0);
	t->vertexUV(x + xa * r - xa2 * r, y - ya * r, z + za * r - za2 * r, u0, v1);
	t->end();
	glPolygonOffset(0, 0.0f);
	glEnable(GL_LIGHTING);
}

int HugeExplosionParticle::getLightColor(float a)
{
	return 0xf0f0;
}

void HugeExplosionParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;
	life++;
	if (life == lifeTime) remove();
}

int HugeExplosionParticle::getParticleTexture()
{
	return ParticleEngine::ENTITY_PARTICLE_TEXTURE;
}