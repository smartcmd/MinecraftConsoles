#include "stdafx.h"
#include "JavaMath.h"
#include "net.minecraft.stats.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.damagesource.h"
#include "com.mojang.nbt.h"
#include "ItemEntity.h"
#include "SoundTypes.h"



void ItemEntity::_init()
{
	age = 0;
	throwTime = 0;
	health = 5;
	bobOffs = (float) (Math::random() * PI * 2);

	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	setSize(0.25f, 0.25f);
	heightOffset = bbHeight / 2.0f;
}

void ItemEntity::_init(Level *level, double x, double y, double z)
{
	_init();

	setPos(x, y, z);

	yRot = (float) (Math::random() * 360);

	xd = (float) (Math::random() * 0.2f - 0.1f);
	yd = +0.2f;
	zd = (float) (Math::random() * 0.2f - 0.1f);
}

ItemEntity::ItemEntity(Level *level, double x, double y, double z) : Entity(level)
{
	_init(level,x,y,z);
}

ItemEntity::ItemEntity(Level *level, double x, double y, double z, shared_ptr<ItemInstance> item) : Entity( level )
{
	_init(level,x,y,z);
	setItem(item);
}

bool ItemEntity::makeStepSound()
{
	return false;
}

ItemEntity::ItemEntity(Level *level) : Entity( level )
{
	_init();
}

void ItemEntity::defineSynchedData()
{
	getEntityData()->defineNULL(DATA_ITEM, NULL);
}

void ItemEntity::tick()
{
	Entity::tick();
	
	if (throwTime > 0) throwTime--;
	xo = x;
	yo = y;
	zo = z;

	yd -= 0.04f;
	noPhysics = checkInTile(x, (bb->y0 + bb->y1) / 2, z);
	
	// 4J - added parameter here so that these don't care about colliding with other entities
	move(xd, yd, zd, true);

	bool moved = (int) xo != (int) x || (int) yo != (int) y || (int) zo != (int) z;

	if (moved || tickCount % 25 == 0)
	{
		if (level->getMaterial( Mth::floor(x),  Mth::floor(y),  Mth::floor(z)) == Material::lava)
		{
			yd = 0.2f;
			xd = (random->nextFloat() - random->nextFloat()) * 0.2f;
			zd = (random->nextFloat() - random->nextFloat()) * 0.2f;
			MemSect(31);
			level->playSound(shared_from_this(), eSoundType_RANDOM_FIZZ, 0.4f, 2.0f + random->nextFloat() * 0.4f);
			MemSect(0);
		}

		if (!level->isClientSide)
		{
			mergeWithNeighbours();
		}
	}

	float friction = 0.98f;
	if (onGround)
	{
		friction = 0.6f * 0.98f;
		int t = level->getTile( Mth::floor(x),  Mth::floor(bb->y0) - 1,  Mth::floor(z) );
		if (t > 0)
		{
			friction = Tile::tiles[t]->friction * 0.98f;
		}
	}

	xd *= friction;
	yd *= 0.98f;
	zd *= friction;

	if (onGround)
	{
		yd *= -0.5f;
	}

	tickCount++;
	age++;
	if (!level->isClientSide && age >= LIFETIME)
	{
		remove();
	}
}

void ItemEntity::mergeWithNeighbours()
{	
	vector<shared_ptr<Entity> > *neighbours = level->getEntitiesOfClass(typeid(*this), bb->grow(0.5, 0, 0.5));
	for(AUTO_VAR(it, neighbours->begin()); it != neighbours->end(); ++it)
	{
		shared_ptr<ItemEntity> entity = dynamic_pointer_cast<ItemEntity>(*it);
		merge(entity);
	}
	delete neighbours;
}

bool ItemEntity::merge(shared_ptr<ItemEntity> target)
{
	if (target == shared_from_this()) return false;
	if (!target->isAlive() || !this->isAlive()) return false;
	shared_ptr<ItemInstance> myItem = this->getItem();
	shared_ptr<ItemInstance> targetItem = target->getItem();

	if (targetItem->getItem() != myItem->getItem()) return false;
	if (targetItem->hasTag() ^ myItem->hasTag()) return false;
	if (targetItem->hasTag() && !targetItem->getTag()->equals(myItem->getTag())) return false;
	if (targetItem->getItem()->isStackedByData() && targetItem->getAuxValue() != myItem->getAuxValue()) return false;
	if (targetItem->count < myItem->count) return target->merge(dynamic_pointer_cast<ItemEntity>(shared_from_this()));
	if (targetItem->count + myItem->count > targetItem->getMaxStackSize()) return false;

	targetItem->count += myItem->count;
	target->throwTime = max(target->throwTime, this->throwTime);
	target->age = min(target->age, this->age);
	target->setItem(targetItem);
	remove();

	return true;
}

void ItemEntity::setShortLifeTime()
{
	// reduce lifetime to one minute
	age = LIFETIME - (60 * SharedConstants::TICKS_PER_SECOND);
}

bool ItemEntity::updateInWaterState()
{
	return level->checkAndHandleWater(bb, Material::water, shared_from_this());
}


void ItemEntity::burn(int dmg)
{
	hurt(DamageSource::inFire, dmg);
}


bool ItemEntity::hurt(DamageSource *source, int damage)
{
	// 4J - added next line: found whilst debugging an issue with item entities getting into a bad state when being created by a cactus, since entities insides cactuses get hurt
	// and therefore depending on the timing of things they could get removed from the client when they weren't supposed to be. Are there really any cases were we would want
	// an itemEntity to be locally hurt?
	if (level->isClientSide ) return false;		

	markHurt();
	health -= damage;
	if (health <= 0)
	{
		remove();
	}
	return false;
}

void ItemEntity::addAdditonalSaveData(CompoundTag *entityTag)
{
	entityTag->putShort(L"Health", (byte) health);
	entityTag->putShort(L"Age", (short) age);
	if (getItem() != NULL) entityTag->putCompound(L"Item", getItem()->save(new CompoundTag()));
}

void ItemEntity::readAdditionalSaveData(CompoundTag *tag)
{
	health = tag->getShort(L"Health") & 0xff;
	age = tag->getShort(L"Age");
	CompoundTag *itemTag = tag->getCompound(L"Item");
	setItem(ItemInstance::fromTag(itemTag));
	if (getItem() == NULL) remove();
}

void ItemEntity::playerTouch(shared_ptr<Player> player)
{
	if (level->isClientSide) return;

	shared_ptr<ItemInstance> item = getItem();

	// 4J Stu - Fix for duplication glitch
	if(item->count <= 0)
	{
		remove();
		return;
	}

	int orgCount = item->count;
	if (throwTime == 0 && player->inventory->add(item))
	{
		//if (item.id == Tile.treeTrunk.id) player.awardStat(Achievements.mineWood);
		//if (item.id == Item.leather.id) player.awardStat(Achievements.killCow);
		//if (item.id == Item.diamond.id) player.awardStat(Achievements.diamonds);
		//if (item.id == Item.blazeRod.id) player.awardStat(Achievements.blazeRod);
		if (item->id == Item::diamond_Id)
		{
			player->awardStat(GenericStats::diamonds(), GenericStats::param_diamonds());

#ifdef _EXTENDED_ACHIEVEMENTS
			if ( getItem()->getItem()->id )
			{
				shared_ptr<Player> pThrower = level->getPlayerByName(getThrower());
				if ( (pThrower != nullptr) && (pThrower != player) )
				{
					pThrower->awardStat(GenericStats::diamondsToYou(), GenericStats::param_diamondsToYou());
				}
			}
#endif
		}
		if (item->id == Item::blazeRod_Id) 
			player->awardStat(GenericStats::blazeRod(), GenericStats::param_blazeRod());

		level->playSound(shared_from_this(), eSoundType_RANDOM_POP, 0.2f, ((random->nextFloat() - random->nextFloat()) * 0.7f + 1.0f) * 2.0f);
		player->take(shared_from_this(), orgCount);
		//            System.out.println(item.count + ", " + orgCount);
		if (item->count <= 0) remove();
	}
}

wstring ItemEntity::getAName()
{
	return L"";//L"item." + getItem()->getDescriptionId();
	//return I18n.get("item." + item.getDescriptionId());
}

shared_ptr<ItemInstance> ItemEntity::getItem()
{
	shared_ptr<ItemInstance> result = getEntityData()->getItemInstance(DATA_ITEM);

	if (result == NULL)
	{
		if (level != NULL)
		{
			app.DebugPrintf("Item entity %d has no item?!\n", entityId);
			//level.getLogger().severe("Item entity " + entityId + " has no item?!");
		}
		return shared_ptr<ItemInstance>(new ItemInstance(Tile::rock));
	}

	return result;
}

void ItemEntity::setItem(shared_ptr<ItemInstance> item)
{
	getEntityData()->set(DATA_ITEM, item);
	getEntityData()->markDirty(DATA_ITEM);
}

bool ItemEntity::isAttackable()
{
	return false;
}

void ItemEntity::setThrower(const wstring &thrower)
{
	this->thrower = thrower;
}

wstring ItemEntity::getThrower()
{
	return this->thrower;
}
