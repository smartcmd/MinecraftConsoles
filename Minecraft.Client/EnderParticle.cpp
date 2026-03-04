#include "stdafx.h"
#include "EnderParticle.h"
#include "..\Minecraft.World\JavaMath.h"
#include "..\Minecraft.World\Random.h"

// 4J Stu - This class was originally "PortalParticle" but I have split the two uses of the particle
// End creatures/items (e.g. EnderMan, EyeOfEnder, etc) use this particle

EnderParticle::EnderParticle(Level *level, double x, double y, double z, double xd, double yd, double zd) : Particle(level, x, y, z, xd, yd, zd)
{      
    this->xd = xd;
    this->yd = yd;
    this->zd = zd;
    this->xStart = this->x = x;
    this->yStart = this->y = y;
    this->zStart = this->z = z;

	// 4J-JEV: Set particle colour from colour-table.
	unsigned int col = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Particle_Ender ); //0xE54CFF
	rCol = ( (col>>16)&0xFF )/255.0f, gCol = ( (col>>8)&0xFF )/255.0, bCol = ( col&0xFF )/255.0;

    float br = random->nextFloat() * 0.6f + 0.4f;
	rCol *= br; gCol *= br; bCol *= br;

    //rCol = gCol = bCol = 1.0f*br;
    //gCol *= 0.3f;
    //rCol *= 0.9f;

	oSize = size = random->nextFloat()*0.2f+0.5f;

    lifetime = (int) (Math::random()*10) + 40;
    noPhysics = true;
    setMiscTex((int)(Math::random()*8));
}

void EnderParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
    float s = (age + a) / (float) lifetime;
    s = 1-s;
    s = s*s;
    s = 1-s;
    size = oSize * (s);
    Particle::render(t, a, xa, ya, za, xa2, za2);
}

// 4J - brought forward from 1.8.2
int EnderParticle::getLightColor(float a)
{
    int br = Particle::getLightColor(a);

    float pos = age/(float)lifetime;
    pos = pos*pos;
    pos = pos*pos;
        
    int br1 = (br) & 0xff;
    int br2 = (br >> 16) & 0xff;
    br2 += (int) (pos * 15 * 16);
    if (br2 > 15 * 16) br2 = 15 * 16;
    return br1 | br2 << 16;
}

float EnderParticle::getBrightness(float a)
{
    float br = Particle::getBrightness(a);
    float pos = age/(float)lifetime;
    pos = pos*pos;
    pos = pos*pos;
    return br*(1-pos)+pos;
}

void EnderParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
        
    float pos = age/(float)lifetime;
    float a = pos;
    pos = -pos+pos*pos*2;
//        pos = pos*pos;
//        pos = pos*pos;
    pos = 1-pos;
        
    x = xStart+xd*pos;
    y = yStart+yd*pos+(1-a);
    z = zStart+zd*pos;
        
        
//        spd+=0.002/lifetime*age;

    if (age++ >= lifetime) remove();

//        move(xd*spd, yd*spd, zd*spd);
}
