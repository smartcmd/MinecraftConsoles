#include "stdafx.h"
#include "EggTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.item.h"

EggTile::EggTile(int id) : Tile(id, Material::egg, isSolidRender())
{
}

void EggTile::onPlace(Level *level, int x, int y, int z)
{
	level->addToTickNextTick(x, y, z, id, getTickDelay(level));
}

void EggTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	level->addToTickNextTick(x, y, z, id, getTickDelay(level));
}

void EggTile::tick(Level *level, int x, int y, int z, Random *random)
{
	checkSlide(level, x, y, z);
}

void EggTile::checkSlide(Level *level, int x, int y, int z)
{
	if (HeavyTile::isFree(level, x, y - 1, z) && y >= 0)
	{
		int r = 32;
		if (HeavyTile::instaFall || !level->hasChunksAt(x - r, y - r, z - r, x + r, y + r, z + r))
		{
			level->removeTile(x, y, z);
			while (HeavyTile::isFree(level, x, y - 1, z) && y > 0)
				y--;
			if (y > 0)
			{
				level->setTileAndData(x, y, z, id, 0, Tile::UPDATE_CLIENTS);
			}
		}
		else
		{
			shared_ptr<FallingTile> e = shared_ptr<FallingTile>(new FallingTile(level, x + 0.5f, y + 0.5f, z + 0.5f, id));
			level->addEntity(e);
		}
	}
}

bool EggTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if(soundOnly) return false;

	teleport(level, x, y, z);
	return true;
}

void EggTile::attack(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	teleport(level, x, y, z);
}

void EggTile::teleport(Level *level, int x, int y, int z)
{
	if (level->getTile(x, y, z) != id) return;

	for (int i = 0; i < 1000; i++)
	{
		int xt = x + level->random->nextInt(16) - level->random->nextInt(16);
		int yt = y + level->random->nextInt(8) - level->random->nextInt(8);
		int zt = z + level->random->nextInt(16) - level->random->nextInt(16);
		if (level->getTile(xt, yt, zt) == 0)
		{
			// Fix for TU9: Content: Art: Dragon egg teleport particle effect isn't present.
			// Don't set tiles on client, and don't create particles on the server (matches later change in Java)
			if(!level->isClientSide)
			{
				level->setTileAndData(xt, yt, zt, id, level->getData(x, y, z), Tile::UPDATE_CLIENTS);
				level->removeTile(x, y, z);

				// 4J Stu - The PC version is wrong as the particles calculated on the client side will point towards a different
				// location to the one where the egg has actually moved. As the deltas are all small we can pack them into an int
				// See generateTeleportParticles for unpacking
				char deltaX = x-xt;
				char deltaY = y-yt;
				char deltaZ = z-zt;
				int deltas = 0|(deltaX&0xFF)|((deltaY&0xFF)<<8)|((deltaZ&0xFF)<<16);

				level->levelEvent(LevelEvent::END_EGG_TELEPORT,xt,yt,zt,deltas);
			}

			// 4J Stu - This code will not work correctly on the client as it will show the particles going in the wrong direction
			// and only for the player who attacks the egg
			//	else
			//	{
			//		int count = 128;
			//		for (int j = 0; j < count; j++)
			//		{
			//			double d = level->random->nextDouble(); // j < count / 2 ? 0 :
			//// 1;
			//			float xa = (level->random->nextFloat() - 0.5f) * 0.2f;
			//			float ya = (level->random->nextFloat() - 0.5f) * 0.2f;
			//			float za = (level->random->nextFloat() - 0.5f) * 0.2f;

			//			double _x = xt + (x - xt) * d + (level->random->nextDouble() - 0.5) * 1 + 0.5f;
			//			double _y = yt + (y - yt) * d + level->random->nextDouble() * 1 - 0.5f;
			//			double _z = zt + (z - zt) * d + (level->random->nextDouble() - 0.5) * 1 + 0.5f;
			//			level->addParticle(eParticleType_ender, _x, _y, _z, xa, ya, za);
			//		}
			//	}
			return;
		}
	}
}

int EggTile::getTickDelay(Level *level)
{
	return 5;
}

bool EggTile::blocksLight()
{
	return false;
}

bool EggTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool EggTile::isCubeShaped()
{
	return false;
}

bool EggTile::shouldRenderFace(LevelSource *level, int x, int y, int z, int face) 
{
	return true;
}

int EggTile::getRenderShape()
{
	return Tile::SHAPE_EGG;
}

int EggTile::cloneTileId(Level *level, int x, int y, int z)
{
	return 0;
}

// 4J Added for Fix for #77475 - TU9: Content: Art: Dragon egg teleport particle effect isn't present.
void EggTile::generateTeleportParticles(Level *level,int xt,int yt, int zt,int deltas)
{
	int count = 128;

	// See above for packing
	char deltaX = deltas&0xFF;
	char deltaY = (deltas>>8)&0xFF;
	char deltaZ = (deltas>>16)&0xFF;

	for (int j = 0; j < count; j++)
	{
		double d = level->random->nextDouble(); // j < count / 2 ? 0 :
		// 1;
		float xa = (level->random->nextFloat() - 0.5f) * 0.2f;
		float ya = (level->random->nextFloat() - 0.5f) * 0.2f;
		float za = (level->random->nextFloat() - 0.5f) * 0.2f;

		double _x = xt + deltaX * d + (level->random->nextDouble() - 0.5) * 1 + 0.5f;
		double _y = yt + deltaY * d + level->random->nextDouble() * 1 - 0.5f;
		double _z = zt + deltaZ * d + (level->random->nextDouble() - 0.5) * 1 + 0.5f;
		level->addParticle(eParticleType_ender, _x, _y, _z, xa, ya, za);
	}
}