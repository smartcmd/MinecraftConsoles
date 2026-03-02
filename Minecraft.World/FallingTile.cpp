#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.damagesource.h"
#include "com.mojang.nbt.h"
#include "FallingTile.h"



// 4J - added for common ctor code
void FallingTile::_init()
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	tile = 0;
	data = 0;
	time = 0;
	dropItem = true;
	
	cancelDrop = false;
	hurtEntities = false;
	fallDamageMax = 40;
	fallDamageAmount = 2;

	// 4J Added so that client-side falling tiles can fall through blocks
	// This fixes a bug on the host where the tile update from the server comes in before the client-side falling tile
	// has reached that level, causing it to stop at one block higher.
	m_ignoreVerticalCollisions = level->isClientSide;
}

FallingTile::FallingTile(Level *level) :
	Entity( level )
{
	_init();
}

FallingTile::FallingTile(Level *level, double x, double y, double z, int tile, int data) : Entity( level )
{
	_init();

	this->tile = tile;
	this->data = data;
	blocksBuilding = true;
	setSize(0.98f, 0.98f);
	heightOffset = bbHeight / 2.0f;
	setPos(x, y, z);

	xd = 0;
	yd = 0;
	zd = 0;

	xo = x;
	yo = y;
	zo = z;

	// 4J added - without this newly created falling tiles weren't interpolating their render positions correctly
	xOld = x;
	yOld = y;
	zOld = z;
}

bool FallingTile::makeStepSound()
{
	return false;
}

void FallingTile::defineSynchedData()
{
}

bool FallingTile::isPickable()
{
	return !removed;
}

void FallingTile::tick()
{
	if (tile == 0)
	{
		remove();
		return;
	}

	xo = x;
	yo = y;
	zo = z;
	time++;

	yd -= 0.04f;
	move(xd, yd, zd);
	xd *= 0.98f;
	yd *= 0.98f;
	zd *= 0.98f;

	if(!level->isClientSide)
	{

		int xt = Mth::floor(x);
		int yt = Mth::floor(y);
		int zt = Mth::floor(z);
		if(time == 1)
		{
			if (level->getTile(xt, yt, zt) == tile)
			{
				level->setTile(xt, yt, zt, 0);
			}
			else
			{
				remove();
				return;
			}
		}

		if (onGround)
		{
			xd *= 0.7f;
			zd *= 0.7f;
			yd *= -0.5f;

			// if (HeavyTile.isFree(level, xt, yt, zt)) {
			if (level->getTile(xt, yt, zt) != Tile::pistonMovingPiece_Id)
			{
				remove();
				if (!cancelDrop && level->mayPlace(tile, xt, yt, zt, true, 1, nullptr) && !HeavyTile::isFree(level, xt, yt - 1, zt) && level->setTileAndData(xt, yt, zt, tile, data))
				{
					HeavyTile *hv = dynamic_cast<HeavyTile *>(Tile::tiles[tile]);
					if (hv)
					{
						hv->onLand(level, xt, yt, zt, data);
					}
				}
				else
				{
					if(dropItem && !cancelDrop) spawnAtLocation( shared_ptr<ItemInstance>(new ItemInstance(tile, 1, Tile::tiles[tile]->getSpawnResourcesAuxValue(data))), 0);
				}
			}
		}
		else if ( (time > 20 * 5 && !level->isClientSide && (yt < 1 || yt > Level::maxBuildHeight)) || (time > 20 * 30))
		{
			if(dropItem) spawnAtLocation(tile, 1);
			remove();
		}
	}
}

void FallingTile::causeFallDamage(float distance)
{
	if (hurtEntities)
	{
		int dmg = Mth::ceil(distance - 1);
		if (dmg > 0)
		{
			vector<shared_ptr<Entity> > *entities = level->getEntities(shared_from_this(), bb);
			DamageSource *source = tile == Tile::anvil_Id ? DamageSource::anvil : DamageSource::fallingBlock;

			//for (Entity entity : entities)
			for(AUTO_VAR(it,entities->begin()); it != entities->end(); ++it)
			{
				(*it)->hurt(source, min(Mth::floor(dmg * fallDamageAmount), fallDamageMax));
			}
			if (tile == Tile::anvil_Id && random->nextFloat() < 0.05f + (dmg * 0.05))
			{
				int damage = data >> 2;
				int dir = data & 3;

				if (++damage > 2)
				{
					cancelDrop = true;
				}
				else
				{
					data = dir | (damage << 2);
				}
			}
		}
	}
}

void FallingTile::addAdditonalSaveData(CompoundTag *tag)
{
	tag->putByte(L"Tile", (byte) tile);
	tag->putByte(L"Data", (byte) data);
	tag->putByte(L"Time", (byte) time);
	tag->putBoolean(L"DropItem", dropItem);
	tag->putBoolean(L"HurtEntities", hurtEntities);
	tag->putFloat(L"FallHurtAmount", fallDamageAmount);
	tag->putInt(L"FallHurtMax", fallDamageMax);
}

void FallingTile::readAdditionalSaveData(CompoundTag *tag)
{
	tile = tag->getByte(L"Tile") & 0xff;
	data = tag->getByte(L"Data") & 0xff;
	time = tag->getByte(L"Time") & 0xff;

	if (tag->contains(L"HurtEntities"))
	{
		hurtEntities = tag->getBoolean(L"HurtEntities");
		fallDamageAmount = tag->getFloat(L"FallHurtAmount");
		fallDamageMax = tag->getInt(L"FallHurtMax");
	}
	else if (tile == Tile::anvil_Id)
	{
		hurtEntities = true;
	}

	if (tag->contains(L"DropItem"))
	{
		dropItem = tag->getBoolean(L"DropItem");
	}

	if (tile == 0)
	{
		tile = Tile::sand_Id;
	}
}


float FallingTile::getShadowHeightOffs()
{
	return 0;
}

Level *FallingTile::getLevel()
{
	return level;
}

void FallingTile::setHurtsEntities(bool value)
{
	this->hurtEntities = value;
}

bool FallingTile::displayFireAnimation()
{
	return false;
}