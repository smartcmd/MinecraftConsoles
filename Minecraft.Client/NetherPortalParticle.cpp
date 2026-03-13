#include "stdafx.h"
#include "NetherPortalParticle.h"
#include "..\Minecraft.World\JavaMath.h"
#include "..\Minecraft.World\Random.h"
#include "Minecraft.h"

// 4J Stu - This class was originally "PortalParticle" but I have split the two uses of the particle
// Only the nether portal uses this particle

NetherPortalParticle::NetherPortalParticle(Level *level, double x, double y, double z, double xd, double yd, double zd) : Particle(level, x, y, z, xd, yd, zd)
{      
    this->xd = xd;
    this->yd = yd;
    this->zd = zd;
    this->xStart = this->x = x;
    this->yStart = this->y = y;
    this->zStart = this->z = z;

    float br = random->nextFloat()*0.6f+0.4f;
    oSize = size = random->nextFloat()*0.2f+0.5f;
    //rCol = gCol = bCol = 1.0f*br;
    //gCol *= 0.3f;
    //rCol *= 0.9f;

	// Default colour (0.9f, 0.3f, 1.0f)
	// 0xE64DFF

	unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Particle_NetherPortal );
	int r = (colour>>16)&0xFF;
	int g = (colour>>8)&0xFF;
	int b = colour&0xFF;
	rCol = (r/255.0f)*br;
	gCol = (g/255.0f)*br;
	bCol = (b/255.0f)*br;

    lifetime = static_cast<int>(Math::random() * 10) + 40;
    noPhysics = true;
    setMiscTex(static_cast<int>(Math::random() * 8));
}

void NetherPortalParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
    float s = (age + a) / static_cast<float>(lifetime);
    s = 1-s;
    s = s*s;
    s = 1-s;
    size = oSize * (s);
    Particle::render(t, a, xa, ya, za, xa2, za2);
}

// 4J - brought forward from 1.8.2
int NetherPortalParticle::getLightColor(float a)
{
    int br = Particle::getLightColor(a);

    float pos = age/static_cast<float>(lifetime);
    pos = pos*pos;
    pos = pos*pos;
        
    int br1 = (br) & 0xff;
    int br2 = (br >> 16) & 0xff;
    br2 += static_cast<int>(pos * 15 * 16);
    if (br2 > 15 * 16) br2 = 15 * 16;
    return br1 | br2 << 16;
}

float NetherPortalParticle::getBrightness(float a)
{
    float br = Particle::getBrightness(a);
    float pos = age/static_cast<float>(lifetime);
    pos = pos*pos;
    pos = pos*pos;
    return br*(1-pos)+pos;
}

void NetherPortalParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
        
    float pos = age/static_cast<float>(lifetime);
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
