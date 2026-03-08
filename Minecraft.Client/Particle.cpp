#include "stdafx.h"
#include "Particle.h"
#include "Tesselator.h"
#include "../Minecraft.World/Random.h"
#include "../Minecraft.World/Mth.h"
#include "../Minecraft.World/JavaMath.h"
#include "../Minecraft.World/net.minecraft.world.h"

/*
    protected int tex;
    protected float gravity;
	*/

double Particle::xOff = 0;
double Particle::yOff = 0;
double Particle::zOff = 0;

void Particle::_init(Level *level, double x, double y, double z)
{
	// 4J - added these initialisers
	alpha = 1.0f;
    tex = NULL;
	gravity = 0.0f;

	setSize(0.2f, 0.2f);
	heightOffset = bbHeight / 2.0f;
	setPos(x, y, z);
	xo = xOld = x;
	yo = yOld = y;
	zo = zOld = z;
	rCol = gCol = bCol = 1.0f;

	uo = random->nextFloat() * 3;
	vo = random->nextFloat() * 3;

	size = (random->nextFloat() * 0.5f + 0.5f) * 2;

	lifetime = (int) (4 / (random->nextFloat() * 0.9f + 0.1f));
	age = 0;

	texX = 0;
	texY = 0;
}

Particle::Particle(Level *level, double x, double y, double z) : Entity(level, false)
{
	_init(level,x,y,z);
}

Particle::Particle(Level *level, double x, double y, double z, double xa, double ya, double za) : Entity(level, false)
{
	_init(level,x,y,z);

    xd = xa + (float) (Math::random() * 2 - 1) * 0.4f;
    yd = ya + (float) (Math::random() * 2 - 1) * 0.4f;
    zd = za + (float) (Math::random() * 2 - 1) * 0.4f;
    float speed = (float) (Math::random() + Math::random() + 1) * 0.15f;

    float dd = (float) (Mth::sqrt(xd * xd + yd * yd + zd * zd));
    xd = xd / dd * speed * 0.4f;
    yd = yd / dd * speed * 0.4f + 0.1f;
    zd = zd / dd * speed * 0.4f;}

shared_ptr<Particle> Particle::setPower(float power)
{
    xd *= power;
    yd = (yd - 0.1f) * power + 0.1f;
    zd *= power;
    return dynamic_pointer_cast<Particle>( shared_from_this() );
}

shared_ptr<Particle> Particle::scale(float scale)
{
    setSize(0.2f * scale, 0.2f * scale);
    size *= scale;
    return dynamic_pointer_cast<Particle>( shared_from_this() );
}

void Particle::setColor(float r, float g, float b)
{
	this->rCol = r;
	this->gCol = g;
	this->bCol = b;
}

void Particle::setAlpha(float alpha)
{
	// 4J - brought forward from Java 1.8
    if (this->alpha == 1.0f && alpha < 1.0f)
	{
		Minecraft::GetInstance()->particleEngine->markTranslucent(dynamic_pointer_cast<Particle>(shared_from_this()));
    }
	else if (this->alpha < 1.0f && alpha == 1.0f)
	{
		Minecraft::GetInstance()->particleEngine->markOpaque(dynamic_pointer_cast<Particle>(shared_from_this()));
    }
	this->alpha = alpha;
}

float Particle::getRedCol()
{
	return rCol;
}

float Particle::getGreenCol()
{
	return gCol;
}

float Particle::getBlueCol()
{
	return bCol;
}

float Particle::getAlpha()
{
	return alpha;
}

bool Particle::makeStepSound()
{
	return false;
}

void Particle::defineSynchedData()
{
}

void Particle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime) remove();

    yd -= 0.04 * gravity;
    move(xd, yd, zd);
    xd *= 0.98f;
    yd *= 0.98f;
    zd *= 0.98f;

    if (onGround)
	{
        xd *= 0.7f;
        zd *= 0.7f;
    }

}

void Particle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
    float u0 = texX / 16.0f;
    float u1 = u0 + 0.999f / 16.0f;
    float v0 = texY / 16.0f;
    float v1 = v0 + 0.999f / 16.0f;
    float r = 0.1f * size;

	if (tex != NULL)
	{
		u0 = tex->getU0();
		u1 = tex->getU1();
		v0 = tex->getV0();
		v1 = tex->getV1();
	}

    float x = (float) (xo + (this->x - xo) * a - xOff);
    float y = (float) (yo + (this->y - yo) * a - yOff);
    float z = (float) (zo + (this->z - zo) * a - zOff);

	float br = 1.0f;							// 4J - change brought forward from 1.8.2
	if( !SharedConstants::TEXTURE_LIGHTING )
	{
	    br = getBrightness(a);
	}

#ifdef __PSVITA__
	// AP - this will set up the 4 vertices in half the time.
	t->tileParticleQuad((float)(x - xa * r - xa2 * r), (float)( y - ya * r), (float)( z - za * r - za2 * r), (float)( u1), (float)( v1),
					(float)(x - xa * r + xa2 * r), (float)( y + ya * r), (float)( z - za * r + za2 * r), (float)( u1), (float)( v0), 
					(float)(x + xa * r + xa2 * r), (float)( y + ya * r), (float)( z + za * r + za2 * r), (float)( u0), (float)( v0), 
					(float)(x + xa * r - xa2 * r), (float)( y - ya * r), (float)( z + za * r - za2 * r), (float)( u0), (float)( v1), 
					rCol * br, gCol * br, bCol * br, alpha);
#else
    t->color(rCol * br, gCol * br, bCol * br, alpha);

    t->vertexUV((float)(x - xa * r - xa2 * r), (float)( y - ya * r), (float)( z - za * r - za2 * r), (float)( u1), (float)( v1));
    t->vertexUV((float)(x - xa * r + xa2 * r), (float)( y + ya * r), (float)( z - za * r + za2 * r), (float)( u1), (float)( v0));
    t->vertexUV((float)(x + xa * r + xa2 * r), (float)( y + ya * r), (float)( z + za * r + za2 * r), (float)( u0), (float)( v0));
    t->vertexUV((float)(x + xa * r - xa2 * r), (float)( y - ya * r), (float)( z + za * r - za2 * r), (float)( u0), (float)( v1));
#endif
}

int Particle::getParticleTexture()
{
	return ParticleEngine::MISC_TEXTURE;
}

void Particle::addAdditonalSaveData(CompoundTag *entityTag)
{
}

void Particle::readAdditionalSaveData(CompoundTag *tag)
{
}

void Particle::setTex(Textures *textures, Icon *icon)
{
	if (getParticleTexture() == ParticleEngine::TERRAIN_TEXTURE)
	{
		tex = icon;
	}
	else if (getParticleTexture() == ParticleEngine::ITEM_TEXTURE)
	{
		tex = icon;
	}
	else
	{
#ifndef _CONTENT_PACKAGE
		printf("Invalid call to Particle.setTex, use coordinate methods\n");
		__debugbreak();
#endif
		//throw new RuntimeException("Invalid call to Particle.setTex, use coordinate methods");
	}
}

void Particle::setMiscTex(int slotIndex)
{
	if (getParticleTexture() != ParticleEngine::MISC_TEXTURE && getParticleTexture() != ParticleEngine::DRAGON_BREATH_TEXTURE)
	{
#ifndef _CONTENT_PACKAGE
		printf("Invalid call to Particle.setMixTex\n");
		__debugbreak();
		//throw new RuntimeException("Invalid call to Particle.setMiscTex");
#endif
	}
	texX = slotIndex % 16;
	texY = slotIndex / 16;
}

void Particle::setNextMiscAnimTex()
{
	texX++;
}

bool Particle::isAttackable()
{
	return false;
}

//@Override
wstring Particle::toString()
{
	return L"A particle"; //getClass()->getSimpleName() + ", Pos (" + x + "," + y + "," + z + "), RGBA (" + rCol + "," + gCol + "," + bCol + "," + alpha + "), Age " + age;
}