#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "LightningBolt.h"
#include "SoundTypes.h"
#include "..\Minecraft.Client\MinecraftServer.h"
#include "..\Minecraft.Client\PlayerList.h"
#include "net.minecraft.world.level.dimension.h"


LightningBolt::LightningBolt(Level *level, double x, double y, double z) : 
	life( 0 ),
	seed( 0 ),
	flashes( 0 ),
	GlobalEntity( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	moveTo(x, y, z, 0, 0);
	life = START_LIFE;
	seed = random->nextLong();
	// 4J-PB - Microsoft request due to photosensitivity issue with multiple flashes of lightning
	//flashes = random->nextInt(3) + 1;
	flashes = 1;

	// 4J - added clientside check
	if( !level->isClientSide )
	{
		if (level->difficulty >= 2 && level->hasChunksAt( Mth::floor(x), Mth::floor(y), Mth::floor(z), 10))
		{
			{
				int xt = Mth::floor(x);
				int yt = Mth::floor(y);
				int zt = Mth::floor(z);
				// 4J added - don't go setting tiles if we aren't tracking them for network synchronisation
				if( MinecraftServer::getInstance()->getPlayers()->isTrackingTile(xt, yt, zt, level->dimension->id) )
				{
					if (level->getTile(xt, yt, zt) == 0 && Tile::fire->mayPlace(level, xt, yt, zt)) level->setTile(xt, yt, zt, Tile::fire_Id);
				}
			}

			for (int i = 0; i < 4; i++)
			{
				int xt = Mth::floor(x) + random->nextInt(3) - 1;
				int yt = Mth::floor(y) + random->nextInt(3) - 1;
				int zt = Mth::floor(z) + random->nextInt(3) - 1;
				// 4J added - don't go setting tiles if we aren't tracking them for network synchronisation
				if( MinecraftServer::getInstance()->getPlayers()->isTrackingTile(xt, yt, zt, level->dimension->id) )
				{
					if (level->getTile(xt, yt, zt) == 0 && Tile::fire->mayPlace(level, xt, yt, zt)) level->setTile(xt, yt, zt, Tile::fire_Id);
				}
			}
		}
	}
}

void LightningBolt::tick()
{
	GlobalEntity::tick();

	if (life == START_LIFE) {
		// 4J-PB - this volume seems off the scale! But the volume is used to check the distance from the camera player - (volume*32) squared
		// so we'll limit the sound in the sound engine
		level->playSound(x, y, z, eSoundType_AMBIENT_WEATHER_THUNDER, 10000, 0.8f + random->nextFloat() * 0.2f);
		level->playSound(x, y, z, eSoundType_RANDOM_EXPLODE, 2, 0.5f + random->nextFloat() * 0.2f);
	}

	life--;
	if (life < 0)
	{
		if (flashes == 0)
		{
			remove();
		}
		else if (life < -random->nextInt(10))
		{
			flashes--;
			life = 1;
			// 4J - added clientside check
			if( !level->isClientSide )
			{
				seed = random->nextLong();
				if (level->hasChunksAt( (int) floor(x), (int) floor(y), (int) floor(z), 10))
				{
					int xt = (int) floor(x);
					int yt = (int) floor(y);
					int zt = (int) floor(z);

					// 4J added - don't go setting tiles if we aren't tracking them for network synchronisation
					if( MinecraftServer::getInstance()->getPlayers()->isTrackingTile(xt, yt, zt, level->dimension->id) )
					{
						if (level->getTile(xt, yt, zt) == 0 && Tile::fire->mayPlace(level, xt, yt, zt)) level->setTile(xt, yt, zt, Tile::fire_Id);
					}
				}
			}
		}
	}

	if (life >= 0)
	{
		double r = 3;
		// 4J - added clientside check
		if( !level->isClientSide )
		{
			vector<shared_ptr<Entity> > *entities = level->getEntities(shared_from_this(), AABB::newTemp(x - r, y - r, z - r, x + r, y + 6 + r, z + r));
			AUTO_VAR(itEnd, entities->end());
			for (AUTO_VAR(it, entities->begin()); it != itEnd; it++)
			{
				shared_ptr<Entity> e = (*it); //entities->at(i);
				e->thunderHit(this);
			}
		}

		level->lightningBoltTime = 2;
	}
}


void LightningBolt::defineSynchedData()
{
}

void LightningBolt::readAdditionalSaveData(CompoundTag *tag)
{
}

void LightningBolt::addAdditonalSaveData(CompoundTag *tag)
{
}


bool LightningBolt::shouldAlwaysRender()
{
	return true;
}

bool LightningBolt::shouldRender(Vec3 *c)
{
	return life >= 0;
}
