#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.core.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.item.h"
#include "DefaultDispenseItemBehavior.h"

shared_ptr<ItemInstance> DefaultDispenseItemBehavior::dispense(BlockSource *source, shared_ptr<ItemInstance> dispensed)
{
	eOUTCOME outcome = DISPENCED_ITEM;
	shared_ptr<ItemInstance> result = execute(source, dispensed, outcome);

	playSound(source, outcome);
	playAnimation(source, DispenserTile::getFacing(source->getData()), outcome);

	return result;
}

shared_ptr<ItemInstance> DefaultDispenseItemBehavior::execute(BlockSource *source, shared_ptr<ItemInstance> dispensed, eOUTCOME &outcome)
{
	FacingEnum *facing = DispenserTile::getFacing(source->getData());
	Position *position = DispenserTile::getDispensePosition(source);

	shared_ptr<ItemInstance> itemInstance = dispensed->remove(1);

	spawnItem(source->getWorld(), itemInstance, 6, facing, position);

	delete position;

	outcome = DISPENCED_ITEM;
	return dispensed;
}

void DefaultDispenseItemBehavior::spawnItem(Level *world, shared_ptr<ItemInstance> item, int accuracy, FacingEnum *facing, Position *position)
{
	double spawnX = position->getX();
	double spawnY = position->getY();
	double spawnZ = position->getZ();

	shared_ptr<ItemEntity> itemEntity = shared_ptr<ItemEntity>(new ItemEntity(world, spawnX, spawnY - 0.3, spawnZ, item));

	double pow = world->random->nextDouble() * 0.1 + 0.2;
	itemEntity->xd = facing->getStepX() * pow;
	itemEntity->yd = .2f;
	itemEntity->zd = facing->getStepZ() * pow;

	itemEntity->xd += world->random->nextGaussian() * 0.0075f * accuracy;
	itemEntity->yd += world->random->nextGaussian() * 0.0075f * accuracy;
	itemEntity->zd += world->random->nextGaussian() * 0.0075f * accuracy;

	world->addEntity(itemEntity);
}

void DefaultDispenseItemBehavior::playSound(BlockSource *source, eOUTCOME outcome)
{
	if (outcome != LEFT_ITEM)
	{
		source->getWorld()->levelEvent(LevelEvent::SOUND_CLICK, source->getBlockX(), source->getBlockY(), source->getBlockZ(), 0);
	}
	else
	{
		// some negative sound effect?
		source->getWorld()->levelEvent(LevelEvent::SOUND_CLICK_FAIL, source->getBlockX(), source->getBlockY(), source->getBlockZ(), 0);
	}
}

void DefaultDispenseItemBehavior::playAnimation(BlockSource *source, FacingEnum *facing, eOUTCOME outcome)
{
	if (outcome != LEFT_ITEM)
	{
		source->getWorld()->levelEvent(LevelEvent::PARTICLES_SHOOT, source->getBlockX(), source->getBlockY(), source->getBlockZ(), getLevelEventDataFrom(facing));
	}
	else
	{
	}
}

int DefaultDispenseItemBehavior::getLevelEventDataFrom(FacingEnum *facing)
{
	return facing->getStepX() + 1 + (facing->getStepZ() + 1) * 3;
}