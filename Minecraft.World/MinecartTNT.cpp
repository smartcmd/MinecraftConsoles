#include "stdafx.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "MinecartTNT.h"

void MinecartTNT::_init()
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	fuse = -1;
}

MinecartTNT::MinecartTNT(Level *level) : Minecart(level)
{
	_init();
}

MinecartTNT::MinecartTNT(Level *level, double x, double y, double z) : Minecart(level, x, y, z)
{
	_init();
}

int MinecartTNT::getType()
{
	return TYPE_TNT;
}

Tile *MinecartTNT::getDefaultDisplayTile()
{
	return Tile::tnt;
}

void MinecartTNT::tick()
{
	Minecart::tick();

	if (fuse > 0)
	{
		fuse--;
		level->addParticle(eParticleType_smoke, x, y + 0.5f, z, 0, 0, 0);
	}
	else if (fuse == 0)
	{
		explode(xd * xd + zd * zd);
	}

	if (horizontalCollision)
	{
		double speedSqr = xd * xd + zd * zd;

		if (speedSqr >= 0.01f)
		{
			explode(speedSqr);
		}
	}
}

void MinecartTNT::destroy(DamageSource *source)
{
	Minecart::destroy(source);

	double speedSqr = xd * xd + zd * zd;

	if (!source->isExplosion())
	{
		spawnAtLocation( shared_ptr<ItemInstance>( new ItemInstance(Tile::tnt, 1) ), 0);
	}

	if (source->isFire() || source->isExplosion() || speedSqr >= 0.01f)
	{
		explode(speedSqr);
	}
}

void MinecartTNT::explode(double speedSqr)
{
	if (!level->isClientSide)
	{
		double speed = sqrt(speedSqr);
		if (speed > 5.0) speed = 5.0;
		if (app.GetGameHostOption(eGameHostOption_TNT))
		{
			level->explode(shared_from_this(), x, y, z, (float) (4 + random->nextDouble() * 1.5f * speed), true);
			remove();
		}
	}
}

void MinecartTNT::causeFallDamage(float distance)
{
	if (distance >= 3)
	{
		float power = distance / 10;
		explode(power * power);
	}

	Minecart::causeFallDamage(distance);
}

void MinecartTNT::activateMinecart(int xt, int yt, int zt, bool state)
{
	if (state && fuse < 0)
	{
		primeFuse();
	}
}

void MinecartTNT::handleEntityEvent(byte eventId)
{
	if (eventId == EVENT_PRIME)
	{
		primeFuse();
	}
	else
	{
		Minecart::handleEntityEvent(eventId);
	}
}

void MinecartTNT::primeFuse()
{
	fuse = 80;

	if (!level->isClientSide)
	{
		level->broadcastEntityEvent(shared_from_this(), EVENT_PRIME);
		level->playEntitySound(shared_from_this(), eSoundType_RANDOM_FUSE, 1, 1.0f);
	}
}

int MinecartTNT::getFuse()
{
	return fuse;
}

bool MinecartTNT::isPrimed()
{
	return fuse > -1;
}

float MinecartTNT::getTileExplosionResistance(Explosion *explosion, Level *level, int x, int y, int z, Tile *tile)
{
	if (isPrimed() && (BaseRailTile::isRail(tile->id) || BaseRailTile::isRail(level, x, y + 1, z)))
	{
		return 0;
	}

	return Minecart::getTileExplosionResistance(explosion, level, x, y, z, tile);
}

bool MinecartTNT::shouldTileExplode(Explosion *explosion, Level *level, int x, int y, int z, int id, float power)
{
	if (isPrimed() && (BaseRailTile::isRail(id) || BaseRailTile::isRail(level, x, y + 1, z))) return false;

	return Minecart::shouldTileExplode(explosion, level, x, y, z, id, power);
}

void MinecartTNT::readAdditionalSaveData(CompoundTag *tag)
{
	Minecart::readAdditionalSaveData(tag);
	if (tag->contains(L"TNTFuse")) fuse = tag->getInt(L"TNTFuse");
}

void MinecartTNT::addAdditonalSaveData(CompoundTag *tag)
{
	Minecart::addAdditonalSaveData(tag);
	tag->putInt(L"TNTFuse", fuse);
}