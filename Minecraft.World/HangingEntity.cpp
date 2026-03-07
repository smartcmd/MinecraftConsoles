#include "stdafx.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.damagesource.h"
#include "com.mojang.nbt.h"
#include "HangingEntity.h"



void HangingEntity::_init(Level *level)
{
	checkInterval = 0;
	dir = 0;
	xTile = yTile = zTile = 0;
	this->heightOffset = 0;
	this->setSize(0.5f, 0.5f);
}

HangingEntity::HangingEntity(Level *level) : Entity( level )
{
	_init(level);

}

HangingEntity::HangingEntity(Level *level, int xTile, int yTile, int zTile, int dir) : Entity( level )
{
	_init(level);
	this->xTile = xTile;
	this->yTile = yTile;
	this->zTile = zTile;
}

void HangingEntity::setDir(int dir)
{
	this->dir = dir;
	yRotO = yRot = (float)(dir * 90);

	float w = (float)getWidth();
	float h = (float)getHeight();
	float d = (float)getWidth();

	if (dir == Direction::NORTH || dir == Direction::SOUTH)
	{
		d = 0.5f;
		yRot = yRotO = (float)(Direction::DIRECTION_OPPOSITE[dir] * 90);
	}
	else
	{
		w = 0.5f;
	}

	w /= 32.0f;
	h /= 32.0f;
	d /= 32.0f;

	float x = xTile + 0.5f;
	float y = yTile + 0.5f;
	float z = zTile + 0.5f;

	float fOffs = 0.5f + 1.0f / 16.0f;

	if (dir == Direction::NORTH) z -= fOffs;
	if (dir == Direction::WEST) x -= fOffs;
	if (dir == Direction::SOUTH) z += fOffs;
	if (dir == Direction::EAST) x += fOffs;

	if (dir == Direction::NORTH) x -= offs(getWidth());
	if (dir == Direction::WEST) z += offs(getWidth());
	if (dir == Direction::SOUTH) x += offs(getWidth());
	if (dir == Direction::EAST) z -= offs(getWidth());
	y += offs(getHeight());

	setPos(x, y, z);

	float ss = -(0.5f / 16.0f);

	// 4J Stu - Due to rotations the bb couold be set with a lower bound x/z being higher than the higher bound
	float x0 = x - w - ss;
	float x1 = x + w + ss;
	float y0 = y - h - ss;
	float y1 = y + h + ss;
	float z0 = z - d - ss;
	float z1 = z + d + ss;
	bb->set(min(x0,x1), min(y0,y1), min(z0,z1), max(x0,x1), max(y0,y1), max(z0,z1));
}

float HangingEntity::offs(int w)
{
	if (w == 32) return 0.5f;
	if (w == 64) return 0.5f;
	return 0.0f;
}

void HangingEntity::tick()
{
	xo = x;
    yo = y;
    zo = z;
	if (checkInterval++ == 20 * 5 && !level->isClientSide)
	{
		checkInterval = 0;
		if (!removed && !survives())
		{
			remove();
			dropItem(nullptr);
		}
	}
}

bool HangingEntity::survives()
{
	if (level->getCubes(shared_from_this(), bb)->size()!=0)//isEmpty())
	{
		return false;
	}
	else
	{
		int ws = max(1, getWidth() / 16);
		int hs = max(1, getHeight() / 16);

		int xt = xTile;
		int yt = yTile;
		int zt = zTile;
		if (dir == Direction::NORTH) xt = Mth::floor(x - getWidth() / 32.0f);
		if (dir == Direction::WEST) zt =  Mth::floor(z - getWidth() / 32.0f);
		if (dir == Direction::SOUTH) xt = Mth::floor(x - getWidth() / 32.0f);
		if (dir == Direction::EAST) zt =  Mth::floor(z - getWidth() / 32.0f);
		yt = Mth::floor(y - getHeight() / 32.0f);

		for (int ss = 0; ss < ws; ss++)
		{
			for (int yy = 0; yy < hs; yy++)
			{
				Material *m;
				if (dir == Direction::NORTH || dir == Direction::SOUTH)
				{
					m = level->getMaterial(xt + ss, yt + yy, zTile);
				}
				else
				{
					m = level->getMaterial(xTile, yt + yy, zt + ss);
				}
				if (!m->isSolid())
				{
					return false;
				}
			}

			vector<shared_ptr<Entity> > *entities = level->getEntities(shared_from_this(), bb);

			if (entities != NULL && entities->size() > 0)
			{
				for (auto& e : *entities)
				{
					if( e && e->instanceof(eTYPE_HANGING_ENTITY) )
					{
						return false;
					}
				}
			}
		}
	}
	return true;
}

bool HangingEntity::isPickable()
{
	return true;
}

bool HangingEntity::skipAttackInteraction(shared_ptr<Entity> source)
{
	if(source->GetType()==eTYPE_PLAYER)
	{
		return hurt(DamageSource::playerAttack(dynamic_pointer_cast<Player>( source)), 0);
	}
	return false;
}

bool HangingEntity::hurt(DamageSource *source, float damage)
{
	if (isInvulnerable()) return false;
	if (!removed && !level->isClientSide)
	{
		if (dynamic_cast<EntityDamageSource *>(source) != NULL)
		{
			shared_ptr<Entity> sourceEntity = source->getDirectEntity();

			if ( (sourceEntity != NULL) && sourceEntity->instanceof(eTYPE_PLAYER) && !dynamic_pointer_cast<Player>(sourceEntity)->isAllowedToHurtEntity(shared_from_this()) )
			{
				return false;
			}
		}

		remove();
		markHurt();

		shared_ptr<Player> player = nullptr;
		shared_ptr<Entity> e = source->getEntity();
		if ( (e!=NULL) && e->instanceof(eTYPE_PLAYER) ) // check if it's serverplayer or player
		{
			player = dynamic_pointer_cast<Player>( e );
		}

		if (player != NULL && player->abilities.instabuild)
		{
			return true;
		}

		dropItem(nullptr);
	}
	return true;
}

// 4J - added noEntityCubes parameter
void HangingEntity::move(double xa, double ya, double za, bool noEntityCubes)
{
	if (!level->isClientSide && !removed && (xa * xa + ya * ya + za * za) > 0)
	{
		remove();
		dropItem(nullptr);
	}
}

void HangingEntity::push(double xa, double ya, double za)
{
	if (!level->isClientSide && !removed && (xa * xa + ya * ya + za * za) > 0)
	{
		remove();
		dropItem(nullptr);
	}
}

void HangingEntity::addAdditonalSaveData(CompoundTag *tag)
{
	tag->putByte(L"Direction", (byte) dir);
	tag->putInt(L"TileX", xTile);
	tag->putInt(L"TileY", yTile);
	tag->putInt(L"TileZ", zTile);

	// Back compat
	switch (dir)
	{
	case Direction::NORTH:
		tag->putByte(L"Dir", (byte) 0);
		break;
	case Direction::WEST:
		tag->putByte(L"Dir", (byte) 1);
		break;
	case Direction::SOUTH:
		tag->putByte(L"Dir", (byte) 2);
		break;
	case Direction::EAST:
		tag->putByte(L"Dir", (byte) 3);
		break;
	}
}

void HangingEntity::readAdditionalSaveData(CompoundTag *tag)
{
	if (tag->contains(L"Direction"))
	{
		dir = tag->getByte(L"Direction");
	}
	else
	{
		switch (tag->getByte(L"Dir"))
		{
		case 0:
			dir = Direction::NORTH;
			break;
		case 1:
			dir = Direction::WEST;
			break;
		case 2:
			dir = Direction::SOUTH;
			break;
		case 3:
			dir = Direction::EAST;
			break;
		}
	}
	xTile = tag->getInt(L"TileX");
	yTile = tag->getInt(L"TileY");
	zTile = tag->getInt(L"TileZ");
	setDir(dir);
}

bool HangingEntity::repositionEntityAfterLoad()
{
	return false;
}