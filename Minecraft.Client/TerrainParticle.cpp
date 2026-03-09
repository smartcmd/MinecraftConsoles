#include "stdafx.h"
#include "TerrainParticle.h"
#include "Tesselator.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.h"

TerrainParticle::TerrainParticle(Level *level, double x, double y, double z, double xa, double ya, double za, Tile *tile, int face, int data, Textures *textures) : Particle(level, x, y, z, xa, ya, za)
{
    this->tile = tile;
    this->setTex(textures, tile->getTexture(0, data));		// 4J - change brought forward from 1.8.2 to fix purple particles on door damage
    this->gravity = tile->gravity;
    rCol = gCol = bCol = 0.6f;
    size /= 2;
}

shared_ptr<TerrainParticle> TerrainParticle::init(int x, int y, int z, int data)	// 4J - added data parameter
{
    if (tile == Tile::grass) return dynamic_pointer_cast<TerrainParticle>( shared_from_this() );
    int col = tile->getColor(level, x, y, z, data);	// 4J - added data parameter
    rCol *= ((col >> 16) & 0xff) / 255.0f;
    gCol *= ((col >> 8) & 0xff) / 255.0f;
    bCol *= ((col) & 0xff) / 255.0f;
    return dynamic_pointer_cast<TerrainParticle>( shared_from_this() );
}

shared_ptr<TerrainParticle> TerrainParticle::init(int data)
{
	if (tile == Tile::grass) return dynamic_pointer_cast<TerrainParticle>( shared_from_this() );
	int col = tile->getColor(data);
	rCol *= ((col >> 16) & 0xff) / 255.0f;
	gCol *= ((col >> 8) & 0xff) / 255.0f;
	bCol *= ((col) & 0xff) / 255.0f;
	return dynamic_pointer_cast<TerrainParticle>( shared_from_this() );
}

int TerrainParticle::getParticleTexture()
{
	return ParticleEngine::TERRAIN_TEXTURE;
}

void TerrainParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
    float u0 = (texX + uo / 4.0f) / 16.0f;
    float u1 = u0 + 0.999f / 16.0f / 4;
    float v0 = (texY + vo / 4.0f) / 16.0f;
    float v1 = v0 + 0.999f / 16.0f / 4;
    float r = 0.1f * size;

	if (tex != nullptr)
	{
		u0 = tex->getU((uo / 4.0f) * SharedConstants::WORLD_RESOLUTION);
		u1 = tex->getU(((uo + 1) / 4.0f) * SharedConstants::WORLD_RESOLUTION);
		v0 = tex->getV((vo / 4.0f) * SharedConstants::WORLD_RESOLUTION);
		v1 = tex->getV(((vo + 1) / 4.0f) * SharedConstants::WORLD_RESOLUTION);
	}

    float x = static_cast<float>(xo + (this->x - xo) * a - xOff);
    float y = static_cast<float>(yo + (this->y - yo) * a - yOff);
    float z = static_cast<float>(zo + (this->z - zo) * a - zOff);

	// 4J - don't render terrain particles that are less than a metre away, to try and avoid large particles that are causing us problems with 
	// photosensitivity testing
	float distSq = (x*x + y*y + z*z);
	if( distSq < 1.0f ) return;

	float br = SharedConstants::TEXTURE_LIGHTING ? 1.0f : getBrightness(a);		// 4J - change brought forward from 1.8.2
    t->color(br * rCol, br * gCol, br * bCol);

    t->vertexUV((float)(x - xa * r - xa2 * r), (float)( y - ya * r), (float)( z - za * r - za2 * r), (float)( u0), (float)( v1));
    t->vertexUV((float)(x - xa * r + xa2 * r), (float)( y + ya * r), (float)( z - za * r + za2 * r), (float)( u0), (float)( v0));
    t->vertexUV((float)(x + xa * r + xa2 * r), (float)( y + ya * r), (float)( z + za * r + za2 * r), (float)( u1), (float)( v0));
    t->vertexUV((float)(x + xa * r - xa2 * r), (float)( y - ya * r), (float)( z + za * r - za2 * r), (float)( u1), (float)( v1));

}
