#include "stdafx.h"
#include "BreakingItemParticle.h"
#include "Tesselator.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\net.minecraft.world.h"

void BreakingItemParticle::_init(Item *item, Textures *textures, int data)
{	
    this->setTex(textures, item->getIcon(data));
    rCol = gCol = bCol = 1.0f;
    gravity = Tile::snow->gravity;
    size /= 2;
}

BreakingItemParticle::BreakingItemParticle(Level *level, double x, double y, double z, Item *item, Textures *textures, int data) : Particle(level, x, y, z, 0, 0, 0)
{
	_init(item, textures, data);
}

BreakingItemParticle::BreakingItemParticle(Level *level, double x, double y, double z, double xa, double ya, double za, Item *item, Textures *textures, int data) : Particle(level, x, y, z, 0, 0, 0)
{
	_init(item, textures, data);
	xd *= 0.1f;
	yd *= 0.1f;
	zd *= 0.1f;
	xd += xa;
	yd += ya;
	zd += za;
}

int BreakingItemParticle::getParticleTexture()
{
	return ParticleEngine::ITEM_TEXTURE;
}

void BreakingItemParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
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
    float br = SharedConstants::TEXTURE_LIGHTING ? 1 : getBrightness(a);	// 4J - change brought forward from 1.8.2
    t->color(br * rCol, br * gCol, br * bCol);

    t->vertexUV((float)(x - xa * r - xa2 * r), (float)( y - ya * r), (float)( z - za * r - za2 * r), (float)( u0), (float)( v1));
    t->vertexUV((float)(x - xa * r + xa2 * r), (float)( y + ya * r), (float)( z - za * r + za2 * r), (float)( u0), (float)( v0));
    t->vertexUV((float)(x + xa * r + xa2 * r), (float)( y + ya * r), (float)( z + za * r + za2 * r), (float)( u1), (float)( v0));
    t->vertexUV((float)(x + xa * r - xa2 * r), (float)( y - ya * r), (float)( z + za * r - za2 * r), (float)( u1), (float)( v1));

}