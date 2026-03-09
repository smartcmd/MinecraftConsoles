#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.level.tile.h"
#include "FireTile.h"
#include "SoundTypes.h"
#include "..\Minecraft.Client\MinecraftServer.h"
#include "..\Minecraft.Client\PlayerList.h"

// AP - added for Vita to set Alpha Cut out
#include "IntBuffer.h"
#include "..\Minecraft.Client\Tesselator.h"


const wstring FireTile::TEXTURE_FIRST = L"fire_0";
const wstring FireTile::TEXTURE_SECOND = L"fire_1";

FireTile::FireTile(int id) : Tile(id, Material::fire,isSolidRender())
{
	flameOdds = new int[256];
	memset( flameOdds,0,sizeof(int)*256);

	burnOdds = new int[256];
	memset( burnOdds,0,sizeof(int)*256);

	icons = nullptr;

	setTicking(true);
}

FireTile::~FireTile()
{
	delete [] flameOdds;
	delete [] burnOdds;
}

void FireTile::init()
{
	setFlammable(Tile::wood_Id, FLAME_HARD, BURN_MEDIUM);
	setFlammable(Tile::woodSlab_Id, FLAME_HARD, BURN_MEDIUM);
	setFlammable(Tile::woodSlabHalf_Id, FLAME_HARD, BURN_MEDIUM);
	setFlammable(Tile::fence_Id, FLAME_HARD, BURN_MEDIUM);
	setFlammable(Tile::stairs_wood_Id, FLAME_HARD, BURN_MEDIUM);
	setFlammable(Tile::stairs_birchwood_Id, FLAME_HARD, BURN_MEDIUM);
	setFlammable(Tile::stairs_sprucewood_Id, FLAME_HARD, BURN_MEDIUM);
	setFlammable(Tile::stairs_junglewood_Id, FLAME_HARD, BURN_MEDIUM);
	setFlammable(Tile::treeTrunk_Id, FLAME_HARD, BURN_HARD);
	setFlammable(Tile::leaves_Id, FLAME_EASY, BURN_EASY);
	setFlammable(Tile::bookshelf_Id, FLAME_EASY, BURN_MEDIUM);
	setFlammable(Tile::tnt_Id, FLAME_MEDIUM, BURN_INSTANT);
	setFlammable(Tile::tallgrass_Id, FLAME_INSTANT, BURN_INSTANT);
	setFlammable(Tile::wool_Id, FLAME_EASY, BURN_EASY);
	setFlammable(Tile::vine_Id, FLAME_MEDIUM, BURN_INSTANT);
	setFlammable(Tile::coalBlock_Id, FLAME_HARD, BURN_HARD);
	setFlammable(Tile::hayBlock_Id, FLAME_INSTANT, BURN_MEDIUM);
}

void FireTile::setFlammable(int id, int flame, int burn)
{
	flameOdds[id] = flame;
	burnOdds[id] = burn;
}

AABB *FireTile::getAABB(Level *level, int x, int y, int z)
{
	return nullptr;
}

bool FireTile::blocksLight()
{
	return false;
}

bool FireTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool FireTile::isCubeShaped()
{
	return false;
}

int FireTile::getRenderShape()
{
	return Tile::SHAPE_FIRE;
}

int FireTile::getResourceCount(Random *random)
{
	return 0;
}

int FireTile::getTickDelay(Level *level)
{
	return 30;
}

void FireTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (!level->getGameRules()->getBoolean(GameRules::RULE_DOFIRETICK))
	{
		return;
	}

	// 4J added - we don't want fire to do anything that might create new fire, or destroy this fire, if we aren't actually tracking (for network) the chunk this is in in the player
	// chunk map. If we did change something in that case, then the change wouldn't get sent to any player that had already received that full chunk, and so we'd just become desynchronised.
	// Seems safest just to do an addToTickNextTick here instead with a decent delay, to make sure that we will get ticked again in the future, when we might again be in a chunk
	// that is being tracked.
	if( !level->isClientSide )		// Note - should only be being ticked on the server
	{
		if( !MinecraftServer::getInstance()->getPlayers()->isTrackingTile(x, y, z, level->dimension->id) )
		{
			level->addToTickNextTick(x, y, z, id, getTickDelay(level) * 5);
			return;
		}
	}


	bool infiniBurn = level->getTile(x, y - 1, z) == Tile::netherRack_Id;
	if (level->dimension->id == 1)		// 4J - was == instanceof TheEndDimension
	{
		if (level->getTile(x, y - 1, z) == Tile::unbreakable_Id) infiniBurn = true;
	}

	if (!mayPlace(level, x, y, z))
	{
		level->removeTile(x, y, z);
	}

	if (!infiniBurn && level->isRaining())
	{
		if (level->isRainingAt(x, y, z) || level->isRainingAt(x - 1, y, z) || level->isRainingAt(x + 1, y, z) || level->isRainingAt(x, y, z - 1) || level->isRainingAt(x, y, z + 1)) {

			level->removeTile(x, y, z);
			return;
		}
	}

	int age = level->getData(x, y, z);
	if (age < 15)
	{
		level->setData(x, y, z, age + random->nextInt(3) / 2, Tile::UPDATE_NONE);
	}
	level->addToTickNextTick(x, y, z, id, getTickDelay(level) + random->nextInt(10));

	if (!infiniBurn && !isValidFireLocation(level, x, y, z))
	{
		if (!level->isTopSolidBlocking(x, y - 1, z) || age > 3) level->removeTile(x, y, z);
		return;
	}

	if (!infiniBurn && !canBurn(level, x, y - 1, z))
	{
		if (age == 15 && random->nextInt(4) == 0)
		{
			level->removeTile(x, y, z);
			return;
		}
	}

	bool isHumid = level->isHumidAt(x, y, z);
	int extra = 0;
	if (isHumid)
	{
		extra = -50;
	}
	checkBurnOut(level, x + 1, y, z, 300 + extra, random, age);
	checkBurnOut(level, x - 1, y, z, 300 + extra, random, age);
	checkBurnOut(level, x, y - 1, z, 250 + extra, random, age);
	checkBurnOut(level, x, y + 1, z, 250 + extra, random, age);
	checkBurnOut(level, x, y, z - 1, 300 + extra, random, age);
	checkBurnOut(level, x, y, z + 1, 300 + extra, random, age);
	if( app.GetGameHostOption(eGameHostOption_FireSpreads) )
	{
		for (int xx = x - 1; xx <= x + 1; xx++)
		{
			for (int zz = z - 1; zz <= z + 1; zz++)
			{
				for (int yy = y - 1; yy <= y + 4; yy++)
				{
					if (xx == x && yy == y && zz == z) continue;

					int rate = 100;
					if (yy > y + 1)
					{
						rate += ((yy - (y + 1)) * 100);
					}

					int fodds = getFireOdds(level, xx, yy, zz);
					if (fodds > 0) {
						int odds = (fodds + 40 + (level->difficulty * 7)) / (age + 30);
						if (isHumid)
						{
							odds /= 2;
						}
						if (odds > 0 && random->nextInt(rate) <= odds)
						{
							if (!(level->isRaining() && level->isRainingAt(xx, yy, zz) || level->isRainingAt(xx - 1, yy, z) || level->isRainingAt(xx + 1, yy, zz) || level->isRainingAt(xx, yy, zz - 1) || level->isRainingAt(xx, yy, zz + 1)))
							{
								int tAge = age + random->nextInt(5) / 4;
								if (tAge > 15) tAge = 15;
								level->setTileAndData(xx, yy, zz, id, tAge, Tile::UPDATE_ALL);
							}
						}
					}
				}
			}
		}
	}
}

bool FireTile::canInstantlyTick()
{
	return false;
}

void FireTile::checkBurnOut(Level *level, int x, int y, int z, int chance, Random *random, int age)
{
	int odds = burnOdds[level->getTile(x, y, z)];
	if (random->nextInt(chance) < odds)
	{
		bool wasTnt = level->getTile(x, y, z) == Tile::tnt_Id;
		if (random->nextInt(age + 10) < 5 && !level->isRainingAt(x, y, z) && app.GetGameHostOption(eGameHostOption_FireSpreads))
		{
			int tAge = age + random->nextInt(5) / 4;
			if (tAge > 15) tAge = 15;
			level->setTileAndData(x, y, z, id, tAge, Tile::UPDATE_ALL);
		} else
		{
			level->removeTile(x, y, z);
		}
		if (wasTnt)
		{
			Tile::tnt->destroy(level, x, y, z, TntTile::EXPLODE_BIT);
		}
	}
}

bool FireTile::isValidFireLocation(Level *level, int x, int y, int z)
{
	if (canBurn(level, x + 1, y, z)) return true;
	if (canBurn(level, x - 1, y, z)) return true;
	if (canBurn(level, x, y - 1, z)) return true;
	if (canBurn(level, x, y + 1, z)) return true;
	if (canBurn(level, x, y, z - 1)) return true;
	if (canBurn(level, x, y, z + 1)) return true;

	return false;
}

int FireTile::getFireOdds(Level *level, int x, int y, int z)
{
	int odds = 0;
	if (!level->isEmptyTile(x, y, z)) return 0;

	odds = getFlammability(level, x + 1, y, z, odds);
	odds = getFlammability(level, x - 1, y, z, odds);
	odds = getFlammability(level, x, y - 1, z, odds);
	odds = getFlammability(level, x, y + 1, z, odds);
	odds = getFlammability(level, x, y, z - 1, odds);
	odds = getFlammability(level, x, y, z + 1, odds);

	return odds;
}

bool FireTile::mayPick()
{
	return false;
}

bool FireTile::canBurn(LevelSource *level, int x, int y, int z)
{
	return flameOdds[level->getTile(x, y, z)] > 0;
}

int FireTile::getFlammability(Level *level, int x, int y, int z, int odds)
{
	int f = flameOdds[level->getTile(x, y, z)];
	if (f > odds) return f;
	return odds;
}

bool FireTile::mayPlace(Level *level, int x, int y, int z)
{
	return level->isTopSolidBlocking(x, y - 1, z) || isValidFireLocation(level, x, y, z);
}

void FireTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (!level->isTopSolidBlocking(x, y - 1, z) && !isValidFireLocation(level, x, y, z))
	{
		level->removeTile(x, y, z);
		return;
	}
}

void FireTile::onPlace(Level *level, int x, int y, int z)
{
	if (level->dimension->id <= 0 && level->getTile(x, y - 1, z) == Tile::obsidian_Id)
	{
		if (Tile::portalTile->trySpawnPortal(level, x, y, z, true))
		{
			return;
		}
	}
	if (!level->isTopSolidBlocking(x, y - 1, z) && !isValidFireLocation(level, x, y, z))
	{
		level->removeTile(x, y, z);
		return;
	}
	level->addToTickNextTick(x, y, z, id, getTickDelay(level) + level->random->nextInt(10));
}

bool FireTile::isFlammable(int tile)
{
	return flameOdds[tile] > 0;
}

void FireTile::animateTick(Level *level, int x, int y, int z, Random *random)
{
	if (random->nextInt(24) == 0)
	{
		level->playLocalSound(x + 0.5f, y + 0.5f, z + 0.5f,eSoundType_FIRE_FIRE, 1 + random->nextFloat(), random->nextFloat() * 0.7f + 0.3f, false);
	}

	if (level->isTopSolidBlocking(x, y - 1, z) || Tile::fire->canBurn(level, x, y - 1, z))
	{
		for (int i = 0; i < 3; i++)
		{
			float xx = x + random->nextFloat();
			float yy = y + random->nextFloat() * 0.5f + 0.5f;
			float zz = z + random->nextFloat();
			level->addParticle(eParticleType_largesmoke, xx, yy, zz, 0, 0, 0);
		}
	}
	else
	{
		if (Tile::fire->canBurn(level, x - 1, y, z))
		{
			for (int i = 0; i < 2; i++) 
			{
				float xx = x + random->nextFloat() * 0.1f;
				float yy = y + random->nextFloat();
				float zz = z + random->nextFloat();
				level->addParticle(eParticleType_largesmoke, xx, yy, zz, 0, 0, 0);
			}
		}
		if (Tile::fire->canBurn(level, x + 1, y, z))
		{
			for (int i = 0; i < 2; i++)
			{
				float xx = x + 1 - random->nextFloat() * 0.1f;
				float yy = y + random->nextFloat();
				float zz = z + random->nextFloat();
				level->addParticle(eParticleType_largesmoke, xx, yy, zz, 0, 0, 0);
			}
		}
		if (Tile::fire->canBurn(level, x, y, z - 1))
		{
			for (int i = 0; i < 2; i++)
			{
				float xx = x + random->nextFloat();
				float yy = y + random->nextFloat();
				float zz = z + random->nextFloat() * 0.1f;
				level->addParticle(eParticleType_largesmoke, xx, yy, zz, 0, 0, 0);
			}
		}
		if (Tile::fire->canBurn(level, x, y, z + 1))
		{
			for (int i = 0; i < 2; i++)
			{
				float xx = x + random->nextFloat();
				float yy = y + random->nextFloat();
				float zz = z + 1 - random->nextFloat() * 0.1f;
				level->addParticle(eParticleType_largesmoke, xx, yy, zz, 0, 0, 0);
			}
		}
		if (Tile::fire->canBurn(level, x, y + 1, z))
		{
			for (int i = 0; i < 2; i++)
			{
				float xx = x + random->nextFloat();
				float yy = y + 1 - random->nextFloat() * 0.1f;
				float zz = z + random->nextFloat();
				level->addParticle(eParticleType_largesmoke, xx, yy, zz, 0, 0, 0);
			}
		}
	}
}

void FireTile::registerIcons(IconRegister *iconRegister)
{
	icons = new Icon*[2];
	icons[0] = iconRegister->registerIcon(TEXTURE_FIRST);
	icons[1] = iconRegister->registerIcon(TEXTURE_SECOND);
}

Icon *FireTile::getTextureLayer(int layer)
{
#ifdef __PSVITA__
	// AP - alpha cut out is expensive on vita. Set the Alpha Cut out flag
	Tesselator* t = Tesselator::getInstance();
	t->setAlphaCutOut( true );
#endif

	return icons[layer];
}

Icon *FireTile::getTexture(int face, int data)
{
	return icons[0];
}
