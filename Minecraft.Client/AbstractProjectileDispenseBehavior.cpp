#include "stdafx.h"
#include "..\Minecraft.World\net.minecraft.core.h"
#include "..\Minecraft.World\net.minecraft.world.entity.projectile.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "AbstractProjectileDispenseBehavior.h"

shared_ptr<ItemInstance> AbstractProjectileDispenseBehavior::execute(BlockSource *source, shared_ptr<ItemInstance> dispensed)
{
	Level *world = source->getWorld();
	Position position = DispenserTile::getDispensePosition(source);
	FacingEnum *facing = DispenserTile::getFacing(source->getData());

	shared_ptr<Projectile> arrow = getProjectile(world, position);
	arrow->shoot(facing->getStepX(), facing->getStepY() + .1f, facing->getStepZ(), getPower(), getUncertainty());
	world->addEntity(arrow);

	dispensed->remove(1);

	return dispensed;
}

void AbstractProjectileDispenseBehavior::playSound(BlockSource *source)
{
	source->getWorld()->levelEvent(LevelEvent::SOUND_LAUNCH, source->getBlockX(), source->getBlockY(), source->getBlockZ(), 0);
}


float AbstractProjectileDispenseBehavior::getUncertainty()
{
	return 6;
}

float AbstractProjectileDispenseBehavior::getPower()
{
	return 1.1f;
}
