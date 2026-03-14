#include "stdafx.h"
#include "../Minecraft.World/Mth.h"
#include "NoteParticle.h"

void NoteParticle::init(Level *level, double x, double y, double z, double xa, double ya, double za, float scale)
{
    xd *= 0.01f;
    yd *= 0.01f;
    zd *= 0.01f;
    yd += 0.2;

	/*
	unsigned int cMin = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Particle_NoteMin );
	unsigned int cMax = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Particle_NoteMax );
	double rMin = ( (cMin>>16)&0xFF )/255.0f, gMin = ( (cMin>>8)&0xFF )/255.0, bMin = ( cMin&0xFF )/255.0;
	double rMax = ( (cMax>>16)&0xFF )/255.0f, gMax = ( (cMax>>8)&0xFF )/255.0, bMax = ( cMax&0xFF )/255.0;

    rCol = Mth::sin(((float) xa + 0.0f / 3) * PI * 2) * (rMax - rMin) + rMin;
    gCol = Mth::sin(((float) xa + 1.0f / 3) * PI * 2) * (gMax - gMin) + gMin;
    bCol = Mth::sin(((float) xa + 2.0f / 3) * PI * 2) * (bMax - bMin) + bMin;
	*/

	// 4J-JEV: Added,
	// There are 24 valid colours for this particle input through the 'xa' field (0.0-1.0).
	int note = static_cast<int>(floor(0.5 + (xa * 24.0))) + static_cast<int>(eMinecraftColour_Particle_Note_00);
	unsigned int col = Minecraft::GetInstance()->getColourTable()->getColor( static_cast<eMinecraftColour>(note) );
	
	rCol = ( (col>>16)&0xFF )/255.0;
	gCol = ( (col>>8)&0xFF )/255.0;
	bCol = ( col&0xFF )/255.0;

    size *= 0.75f;
    size *= scale;
    oSize = size;

    lifetime = 6;
    noPhysics = false;


    setMiscTex(16 * 4);
}

NoteParticle::NoteParticle(Level *level, double x, double y, double z, double xa, double ya, double za) : Particle(level, x, y, z, 0, 0, 0)
{
	init(level, x, y, z, xa, ya, za, 2);
}

NoteParticle::NoteParticle(Level *level, double x, double y, double z, double xa, double ya, double za, float scale) : Particle(level, x, y, z, 0, 0, 0)
{
	init(level, x, y, z, xa, ya, za, scale);
}

void NoteParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
    float l = ((age + a) / lifetime) * 32;
    if (l < 0) l = 0;
    if (l > 1) l = 1;

    size = oSize * l;
    Particle::render(t, a, xa, ya, za, xa2, za2);
}

void NoteParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime) remove();

    move(xd, yd, zd);
    if (y == yo)
	{
        xd *= 1.1;
        zd *= 1.1;
    }
    xd *= 0.66f;
    yd *= 0.66f;
    zd *= 0.66f;

    if (onGround)
	{
        xd *= 0.7f;
        zd *= 0.7f;
    }

}
