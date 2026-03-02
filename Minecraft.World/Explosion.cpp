#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.damagesource.h"
#include "TilePos.h"
#include "Explosion.h"
#include "SoundTypes.h"

Explosion::Explosion(Level *level, shared_ptr<Entity> source, double x, double y, double z, float r)
{
	fire = false;
	random = new Random();

	this->level = level;
	this->source = source;
	this->r = r;
	this->x = x;
	this->y = y;
	this->z = z;

	destroyBlocks = true;
	size = 16;
}

Explosion::~Explosion()
{
	delete random;
	for(AUTO_VAR(it, hitPlayers.begin()); it != hitPlayers.end(); ++it)
	{
		delete it->second;
	}
}

void Explosion::explode()
{
	float oR = r;

	int size = 16;
	for (int xx = 0; xx < size; xx++)
	{
		for (int yy = 0; yy < size; yy++)
		{
			for (int zz = 0; zz < size; zz++)
			{
				if ((xx != 0 && xx != size - 1) && (yy != 0 && yy != size - 1) && (zz != 0 && zz != size - 1)) continue;

				double xd = xx / (size - 1.0f) * 2 - 1;
				double yd = yy / (size - 1.0f) * 2 - 1;
				double zd = zz / (size - 1.0f) * 2 - 1;
				double d = sqrt(xd * xd + yd * yd + zd * zd);

				xd /= d;
				yd /= d;
				zd /= d;

				float remainingPower = r * (0.7f + level->random->nextFloat() * 0.6f);
				double xp = x;
				double yp = y;
				double zp = z;

				float stepSize = 0.3f;
				while (remainingPower > 0)
				{
					int xt = Mth::floor(xp);
					int yt = Mth::floor(yp);
					int zt = Mth::floor(zp);
					int t = level->getTile(xt, yt, zt);
					if (t > 0)
					{
						remainingPower -= (Tile::tiles[t]->getExplosionResistance(source) + 0.3f) * stepSize;
					}
					if (remainingPower > 0)
					{
						toBlow.insert(TilePos(xt, yt, zt));
					}

					xp += xd * stepSize;
					yp += yd * stepSize;
					zp += zd * stepSize;
					remainingPower -= stepSize * 0.75f;
				}
				// if (xd*xd+yd*yd+zd*zd>1) continue;
			}
		}
	}

	r *= 2.0f;
	int x0 = Mth::floor(x - r - 1);
	int x1 = Mth::floor(x + r + 1);
	int y0 = Mth::floor(y - r - 1);
	int y1 = Mth::floor(y + r + 1);
	int z0 = Mth::floor(z - r - 1);
	int z1 = Mth::floor(z + r + 1);

	// Fix for 360 #123866 - [CRASH] TU13: Code: Compliance: Placing the TNT next to Ender Crystals will crash the title after a certain amount of time.
	// If we explode something next to an EnderCrystal then it creates a new explosion that overwrites the shared vector in the level
	// So copy it here instead of directly using the shared one
	vector<shared_ptr<Entity> > *levelEntities = level->getEntities(source, AABB::newTemp(x0, y0, z0, x1, y1, z1));
	vector<shared_ptr<Entity> > entities(levelEntities->begin(), levelEntities->end() );
	Vec3 *center = Vec3::newTemp(x, y, z);

	AUTO_VAR(itEnd, entities.end());
	for (AUTO_VAR(it, entities.begin()); it != itEnd; it++)
	{
		shared_ptr<Entity> e = *it; //entities->at(i);

		// 4J Stu - If the entity is not in a block that would be blown up, then they should not be damaged
		// Fix for #46606 - TU5: Content: Gameplay: The player can be damaged and killed by explosions behind obsidian walls
		bool canDamage = false;
		for(AUTO_VAR(it2, toBlow.begin()); it2 != toBlow.end(); ++it2)
		{
			if(e->bb->intersects(it2->x,it2->y,it2->z,it2->x + 1,it2->y + 1,it2->z + 1))
			{
				canDamage = true;
				break;
			}
		}

		double dist = e->distanceTo(x, y, z) / r;
		if (dist <= 1)
		{
			double xa = e->x - x;
			double ya = e->y + e->getHeadHeight() - y;
			double za = e->z - z;

			double da = sqrt(xa * xa + ya * ya + za * za);

			// 4J Stu - Added this check to remove divide by zero errors (or rather the issues caused by
			// the values being set to NaN and used in comparisons a bit later on e.g. fireball)
			if( da == 0 )
			{
				xa = ya = za = 0.0;
			}
			else
			{
				xa /= da;
				ya /= da;
				za /= da;
			}

			double sp = level->getSeenPercent(center, e->bb);
			double pow = (1 - dist) * sp;
			if(canDamage) e->hurt(DamageSource::explosion, (int) ((pow * pow + pow) / 2 * 8 * r + 1));

			double push = pow;
			e->xd += xa * push;
			e->yd += ya * push;
			e->zd += za * push;					

			shared_ptr<Player> player = dynamic_pointer_cast<Player>(e);
			if (player != NULL)
			{
				//app.DebugPrintf("Adding player knockback (%f,%f,%f)\n", xa * pow, ya * pow, za * pow);
				hitPlayers.insert( playerVec3Map::value_type( player, Vec3::newPermanent(xa * pow, ya * pow, za * pow)));
			}
		}
	}
	r = oR;
}


void Explosion::finalizeExplosion(bool generateParticles, vector<TilePos> *toBlowDirect/*=NULL*/)		// 4J - added toBlowDirect parameter
{
	level->playSound(x, y, z, eSoundType_RANDOM_EXPLODE, 4, (1 + (level->random->nextFloat() - level->random->nextFloat()) * 0.2f) * 0.7f);
	level->addParticle(eParticleType_hugeexplosion, x, y, z, 0, 0, 0);

	// 4J - use pointer to vector directly passed in if this is available - used to speed up calling this from an incoming packet
	vector<TilePos> *toBlowArray = toBlowDirect ? toBlowDirect : new vector<TilePos>( toBlow.begin(), toBlow.end() );
	//toBlowArray.addAll(toBlow);
	// TODO 4J Stu - Reverse iterator
	PIXBeginNamedEvent(0,"Finalizing explosion size %d",toBlow.size());
	app.DebugPrintf("Finalizing explosion size %d\n",toBlow.size());
	static const int MAX_EXPLODE_PARTICLES = 50;
	// 4J - try and make at most MAX_EXPLODE_PARTICLES pairs of particles
	int fraction = (int)toBlowArray->size() / MAX_EXPLODE_PARTICLES;
	if( fraction == 0 ) fraction = 1;
	size_t j = toBlowArray->size() - 1;
	//for (size_t j = toBlowArray->size() - 1; j >= 0; j--)
	for(AUTO_VAR(it,toBlowArray->rbegin()); it != toBlowArray->rend(); ++it)
	{
		TilePos *tp = &(*it); //&toBlowArray->at(j);
		int xt = tp->x;
		int yt = tp->y;
		int zt = tp->z;
		// if (xt >= 0 && yt >= 0 && zt >= 0 && xt < width && yt < depth &&
		// zt < height) {
		int t = level->getTile(xt, yt, zt);

		if (generateParticles)
		{
			if( ( j % fraction ) == 0  )
			{
				double xa = xt + level->random->nextFloat();
				double ya = yt + level->random->nextFloat();
				double za = zt + level->random->nextFloat();

				double xd = xa - x;
				double yd = ya - y;
				double zd = za - z;

				double dd = sqrt(xd * xd + yd * yd + zd * zd);

				xd /= dd;
				yd /= dd;
				zd /= dd;

				double speed = 0.5 / (dd / r + 0.1);
				speed *= (level->random->nextFloat() * level->random->nextFloat() + 0.3f);
				xd *= speed;
				yd *= speed;
				zd *= speed;

				level->addParticle(eParticleType_explode, (xa + x * 1) / 2, (ya + y * 1) / 2, (za + z * 1) / 2, xd, yd, zd);
				level->addParticle(eParticleType_smoke, xa, ya, za, xd, yd, zd);
			}
		}

		if (t > 0)
		{
			Tile::tiles[t]->spawnResources(level, xt, yt, zt, level->getData(xt, yt, zt), 0.3f, 0);
			level->setTile(xt, yt, zt, 0);
			Tile::tiles[t]->wasExploded(level, xt, yt, zt);
		}
		// }

		--j;
	}

	if (fire)
	{
		//for (size_t j = toBlowArray->size() - 1; j >= 0; j--)
		for(AUTO_VAR(it,toBlowArray->rbegin()); it != toBlowArray->rend(); ++it)
		{
			TilePos *tp = &(*it); //&toBlowArray->at(j);
			int xt = tp->x;
			int yt = tp->y;
			int zt = tp->z;
			int t = level->getTile(xt, yt, zt);
			int b = level->getTile(xt, yt - 1, zt);
			if (t == 0 && Tile::solid[b] && random->nextInt(3) == 0)
			{
				level->setTile(xt, yt, zt, Tile::fire_Id);
			}
		}
	}

	PIXEndNamedEvent();
	if( toBlowDirect == NULL )	delete toBlowArray;
}

Explosion::playerVec3Map *Explosion::getHitPlayers()
{
	return &hitPlayers;
}

Vec3 *Explosion::getHitPlayerKnockback( shared_ptr<Player> player )
{
	AUTO_VAR(it, hitPlayers.find(player));

	if(it == hitPlayers.end() ) return Vec3::newTemp(0.0,0.0,0.0);

	return it->second;
}