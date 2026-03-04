#include "stdafx.h"
#include "ExplodeParticle.h"
#include "..\Minecraft.World\JavaMath.h"
#include "..\Minecraft.World\Random.h"

ExplodeParticle::ExplodeParticle(Level *level, double x, double y, double z, double xa, double ya, double za) : Particle(level, x, y, z, xa, ya, za)
{
	xd = xa+(float)(Math::random()*2-1)*0.05f;
	yd = ya+(float)(Math::random()*2-1)*0.05f;
	zd = za+(float)(Math::random()*2-1)*0.05f;        
        
	//rCol = gCol = bCol = random->nextFloat()*.3f+.7;
	
	unsigned int clr = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Particle_Explode ); //0xFFFFFF
	double r = ( (clr>>16)&0xFF )/255.0f, g = ( (clr>>8)&0xFF )/255.0, b = ( clr&0xFF )/255.0;

	float br = random->nextFloat() * 0.3f + 0.7f;
	rCol = r * br;
	gCol = g * br;
	bCol = b * br;

	size = random->nextFloat()*random->nextFloat()*6+1;
        
	lifetime = (int)(16/(random->nextFloat()*0.8+0.2))+2;
//        noPhysics = true;
}

void ExplodeParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
	// 4J - don't render explosion particles that are less than 3 metres away, to try and avoid large particles that are causing us problems with photosensitivity testing
	float x = (float) (xo + (this->x - xo) * a - xOff);
	float y = (float) (yo + (this->y - yo) * a - yOff);
	float z = (float) (zo + (this->z - zo) * a - zOff);

	float distSq = (x*x + y*y + z*z);
	if( distSq < (3.0f * 3.0f) ) return;

	Particle::render(t, a, xa, ya, za, xa2, za2);
}

void ExplodeParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime) remove();
        
    setMiscTex(7-age*8/lifetime);

    yd += 0.004;
    move(xd, yd, zd);
    xd *= 0.90f;
    yd *= 0.90f;
    zd *= 0.90f;

    if (onGround)
    {
        xd *= 0.7f;
        zd *= 0.7f;
    }
}