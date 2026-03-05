#include "stdafx.h"
#include "ParticleEngine.h"
#include "Particle.h"
#include "Textures.h"
#include "TextureAtlas.h"
#include "Tesselator.h"
#include "TerrainParticle.h"
#include "ResourceLocation.h"
#include "Camera.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"

ResourceLocation ParticleEngine::PARTICLES_LOCATION = ResourceLocation(TN_PARTICLES);

ParticleEngine::ParticleEngine(Level *level, Textures *textures)
{
//    if (level != NULL)	// 4J - removed - we want level to be initialised to *something*
	{
        this->level = level;
    }
    this->textures = textures;

	this->random = new Random();
}

ParticleEngine::~ParticleEngine()
{
	delete random;
}

void ParticleEngine::add(shared_ptr<Particle> p)
{
    int t = p->getParticleTexture();
	int l = p->level->dimension->id == 0 ? 0 : ( p->level->dimension->id == -1 ? 1 : 2);
	int maxParticles;
	switch(p->GetType())
	{
		case eTYPE_DRAGONBREATHPARTICLE:
			maxParticles = MAX_DRAGON_BREATH_PARTICLES;
			break;
		case eType_FIREWORKSSPARKPARTICLE:
			maxParticles = MAX_FIREWORK_SPARK_PARTICLES;
			break;
		default:
			maxParticles = MAX_PARTICLES_PER_LAYER;
			break;
	}
    int list = p->getAlpha() != 1.0f ? TRANSLUCENT_LIST : OPAQUE_LIST;		// 4J - Brought forward from Java 1.8

	if(	particles[l][t][list].size() >= maxParticles)
	{
		particles[l][t][list].pop_front();
	}
    particles[l][t][list].push_back(p);
}

void ParticleEngine::tick()
{
	for( int l = 0; l < 3; l++ )
	{
		for (int tt = 0; tt < TEXTURE_COUNT; tt++)
		{
			for( int list = 0; list < LIST_COUNT; list++ )		// 4J - Brought forward from Java 1.8
			{
				for (unsigned int i = 0; i < particles[l][tt][list].size(); i++)
				{
					shared_ptr<Particle> p = particles[l][tt][list][i];
					p->tick();
					if (p->removed)
					{
						particles[l][tt][list][i] = particles[l][tt][list].back();
						particles[l][tt][list].pop_back();
						i--;
					}
				}
			}
		}
	}
}

void ParticleEngine::render(shared_ptr<Entity> player, float a, int list)
{
	// 4J - change brought forward from 1.2.3
    float xa = Camera::xa;
    float za = Camera::za;

    float xa2 = Camera::xa2;
    float za2 = Camera::za2;
    float ya = Camera::ya;

    Particle::xOff = (player->xOld + (player->x - player->xOld) * a);
    Particle::yOff = (player->yOld + (player->y - player->yOld) * a);
    Particle::zOff = (player->zOld + (player->z - player->zOld) * a);
	int l = level->dimension->id == 0 ? 0 : ( level->dimension->id == -1 ? 1 : 2 );

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glAlphaFunc(GL_GREATER, 1.0f / 255.0f);

	for (int tt = 0; tt < TEXTURE_COUNT; tt++)
	{
		if(tt == ENTITY_PARTICLE_TEXTURE) continue;

		if (!particles[l][tt][list].empty())
		{
            switch (list)
			{
				case TRANSLUCENT_LIST:
					glDepthMask(false);
					break;
				case OPAQUE_LIST:
					 glDepthMask(true);
					break;
            }

			MemSect(31);
			if (tt == MISC_TEXTURE || tt == DRAGON_BREATH_TEXTURE) textures->bindTexture(&PARTICLES_LOCATION);
			if (tt == TERRAIN_TEXTURE)  textures->bindTexture(&TextureAtlas::LOCATION_BLOCKS);
			if (tt == ITEM_TEXTURE) textures->bindTexture(&TextureAtlas::LOCATION_ITEMS);
			MemSect(0);
			Tesselator *t = Tesselator::getInstance();
			glColor4f(1.0f, 1.0f, 1.0f, 1);

			t->begin();
			for (unsigned int i = 0; i < particles[l][tt][list].size(); i++)
			{
				if(t->hasMaxVertices())
				{
					t->end();
					t->begin();
				}
				shared_ptr<Particle> p = particles[l][tt][list][i];

				if (SharedConstants::TEXTURE_LIGHTING)		// 4J - change brought forward from 1.8.2
				{
					t->tex2(p->getLightColor(a));
				}
				p->render(t, a, xa, ya, za, xa2, za2);
			}
			t->end();
		}
    }

	glDisable(GL_BLEND);
	glDepthMask(true);
	glAlphaFunc(GL_GREATER, .1f);
}

void ParticleEngine::renderLit(shared_ptr<Entity> player, float a, int list)
{
	// 4J - added. We call this before ParticleEngine::render in the general render per player, so if we
	// don't set this here then the offsets will be from the previous player - a single frame lag for the
	// java game, or totally incorrect placement of things for split screen.
    Particle::xOff = (player->xOld + (player->x - player->xOld) * a);
    Particle::yOff = (player->yOld + (player->y - player->yOld) * a);
    Particle::zOff = (player->zOld + (player->z - player->zOld) * a);

	float RAD = PI / 180;
	float xa = (float) Mth::cos(player->yRot * RAD);
	float za = (float) Mth::sin(player->yRot * RAD);

	float xa2 = -za * (float) Mth::sin(player->xRot * RAD);
	float za2 = xa * (float) Mth::sin(player->xRot * RAD);
	float ya = (float) Mth::cos(player->xRot * RAD);

	int l = level->dimension->id == 0 ? 0 : ( level->dimension->id == -1 ? 1 : 2 );
    int tt = ENTITY_PARTICLE_TEXTURE;

	if( !particles[l][tt][list].empty() )
	{
		Tesselator *t = Tesselator::getInstance();
		for (unsigned int i = 0; i < particles[l][tt][list].size(); i++)
		{
			shared_ptr<Particle> p = particles[l][tt][list][i];

			if (SharedConstants::TEXTURE_LIGHTING)		// 4J - change brought forward from 1.8.2
			{
				t->tex2(p->getLightColor(a));
			}
			p->render(t, a, xa, ya, za, xa2, za2);
		}
	}
}

void ParticleEngine::setLevel(Level *level)
{
    this->level = level;
	// 4J - we've now got a set of particle vectors for each dimension, and only clearing them when its game over & the level is set to NULL
	if( level == NULL )
	{
		for( int l = 0; l < 3; l++ )
		{
			for (int tt = 0; tt < TEXTURE_COUNT; tt++)
			{
				for( int list = 0; list < LIST_COUNT; list++ )
				{
					particles[l][tt][list].clear();
				}
			}
		}
	}
}

void ParticleEngine::destroy(int x, int y, int z, int tid, int data)
{
    if (tid == 0) return;

    Tile *tile = Tile::tiles[tid];
    int SD = 4;
    for (int xx = 0; xx < SD; xx++)
        for (int yy = 0; yy < SD; yy++)
            for (int zz = 0; zz < SD; zz++)
			{
                double xp = x + (xx + 0.5) / SD;
                double yp = y + (yy + 0.5) / SD;
                double zp = z + (zz + 0.5) / SD;
				int face = random->nextInt(6);
                add(( shared_ptr<TerrainParticle>(new TerrainParticle(level, xp, yp, zp, xp - x - 0.5f, yp - y - 0.5f, zp - z - 0.5f, tile, face, data, textures) ) )->init(x, y, z, data));
            }
}

void ParticleEngine::crack(int x, int y, int z, int face)
{
    int tid = level->getTile(x, y, z);
    if (tid == 0) return;
    Tile *tile = Tile::tiles[tid];
    float r = 0.10f;
    double xp = x + random->nextDouble() * ((tile->getShapeX1() - tile->getShapeX0()) - r * 2) + r + tile->getShapeX0();
    double yp = y + random->nextDouble() * ((tile->getShapeY1() - tile->getShapeY0()) - r * 2) + r + tile->getShapeY0();
    double zp = z + random->nextDouble() * ((tile->getShapeZ1() - tile->getShapeZ0()) - r * 2) + r + tile->getShapeZ0();
    if (face == 0) yp = y + tile->getShapeY0() - r;
    if (face == 1) yp = y + tile->getShapeY1() + r;
    if (face == 2) zp = z + tile->getShapeZ0() - r;
    if (face == 3) zp = z + tile->getShapeZ1() + r;
    if (face == 4) xp = x + tile->getShapeX0() - r;
    if (face == 5) xp = x + tile->getShapeX1() + r;
    add(( shared_ptr<TerrainParticle>(new TerrainParticle(level, xp, yp, zp, 0, 0, 0, tile, face, level->getData(x, y, z), textures) ) )->init(x, y, z, level->getData(x, y, z))->setPower(0.2f)->scale(0.6f));

}

void ParticleEngine::markTranslucent(shared_ptr<Particle> particle)
{
    moveParticleInList(particle, OPAQUE_LIST, TRANSLUCENT_LIST);
}

void ParticleEngine::markOpaque(shared_ptr<Particle> particle)
{
	moveParticleInList(particle, TRANSLUCENT_LIST, OPAQUE_LIST);
}

void ParticleEngine::moveParticleInList(shared_ptr<Particle> particle, int source, int destination)
{
	int l = particle->level->dimension->id == 0 ? 0 : ( particle->level->dimension->id == -1 ? 1 : 2);
    for (int tt = 0; tt < TEXTURE_COUNT; tt++)
	{
        auto it = find(particles[l][tt][source].begin(), particles[l][tt][source].end(), particle);
        if(it != particles[l][tt][source].end() )
		{
			(*it) = particles[l][tt][source].back();
			particles[l][tt][source].pop_back();
			particles[l][tt][destination].push_back(particle);
		}
    }
}

wstring ParticleEngine::countParticles()
{
	int l = level->dimension->id == 0 ? 0 : (level->dimension->id == -1 ? 1 : 2 );
	int total = 0;
	for( int tt = 0; tt < TEXTURE_COUNT; tt++ )
	{
		for( int list = 0; list < LIST_COUNT; list++ )
		{
			total += particles[l][tt][list].size();
		}
	}
	return std::to_wstring(total);
}
