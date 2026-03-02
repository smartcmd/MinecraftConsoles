#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.boss.enderdragon.h"
#include "EnderCrystal.h"
#include "DamageSource.h"



void EnderCrystal::_init(Level *level)
{		
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	blocksBuilding = true;
	setSize(2.0f, 2.0f);
	heightOffset = bbHeight / 2.0f;
	life = MAX_LIFE;

	time = random->nextInt(100000);
}

EnderCrystal::EnderCrystal(Level *level) : Entity( level )
{
	_init(level);

}

EnderCrystal::EnderCrystal(Level *level, double x, double y, double z) : Entity( level )
{
	_init(level);
	setPos(x, y, z);

}

bool EnderCrystal::makeStepSound()
{
	return false;
}

void EnderCrystal::defineSynchedData()
{
	entityData->define(DATA_REMAINING_LIFE, life);
}

void EnderCrystal::tick()
{
	xo = x;
	yo = y;
	zo = z;
	time++;

	entityData->set(DATA_REMAINING_LIFE, life);

	// Don't set the tile directly on the client, as this can end up in the updatesToReset queue in the MultiPlayerLevel, and the perpetually end up removing/adding these fire tiles causing timing
	// glitches from the lighting changes requried.
	if (!level->isClientSide)
	{
		int xt = Mth::floor(x);
		int yt = Mth::floor(y);
		int zt = Mth::floor(z);
		if (level->getTile(xt, yt, zt) != Tile::fire_Id)
		{
			level->setTile(xt, yt, zt, Tile::fire_Id);
		}
	}
}


void EnderCrystal::addAdditonalSaveData(CompoundTag *tag)
{
}

void EnderCrystal::readAdditionalSaveData(CompoundTag *tag)
{
}


float EnderCrystal::getShadowHeightOffs()
{
	return 0;
}

bool EnderCrystal::isPickable()
{
	return true;
}

bool EnderCrystal::hurt(DamageSource *source, int damage)
{
	// 4J-PB - if the owner of the source is the enderdragon, then ignore it (where the dragon's fireball hits an endercrystal)
	shared_ptr<EnderDragon> sourceIsDragon = dynamic_pointer_cast<EnderDragon>(source->getEntity());

	if(sourceIsDragon!=NULL)
	{
		return false;
	}

	if (!removed && !level->isClientSide)
	{
		life = 0;
		if (life <= 0)
		{
			remove();
			if (!level->isClientSide)
			{
				level->explode(nullptr, x, y, z, 6, true);

				vector<shared_ptr<Entity> > entities = level->getAllEntities();
				shared_ptr<EnderDragon> dragon = nullptr;
				AUTO_VAR(itEnd, entities.end());
				for (AUTO_VAR(it, entities.begin()); it != itEnd; it++)
				{
					shared_ptr<Entity> e = *it; //entities->at(i);
					dragon = dynamic_pointer_cast<EnderDragon>(e);
					if(dragon != NULL)
					{
						dragon->handleCrystalDestroyed(source);
						break;
					}
				}
			}
		}
	}
	return true;
}