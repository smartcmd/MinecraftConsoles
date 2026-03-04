#pragma once
#include "../Minecraft.World/net.minecraft.world.item.h"
#include "../Minecraft.World/DispenserTile.h"
#include "../Minecraft.World/net.minecraft.core.h"
#include "../Minecraft.World/LevelEvent.h"

class DispenserBootstrap
{
public:
	static void bootStrap()
	{
		DispenserTile::REGISTRY.add(Item::arrow, new ArrowDispenseBehavior());
		DispenserTile::REGISTRY.add(Item::egg, new EggDispenseBehavior());	
		DispenserTile::REGISTRY.add(Item::snowBall, new SnowballDispenseBehavior());
		DispenserTile::REGISTRY.add(Item::expBottle, new ExpBottleDispenseBehavior());

		DispenserTile::REGISTRY.add(Item::potion, new PotionDispenseBehavior());
		DispenserTile::REGISTRY.add(Item::spawnEgg, new SpawnEggDispenseBehavior());
		DispenserTile::REGISTRY.add(Item::fireworks, new FireworksDispenseBehavior());
		DispenserTile::REGISTRY.add(Item::fireball, new FireballDispenseBehavior());
		DispenserTile::REGISTRY.add(Item::boat, new BoatDispenseBehavior());
		DispenserTile::REGISTRY.add(Item::bucket_lava, new FilledBucketDispenseBehavior());
		DispenserTile::REGISTRY.add(Item::bucket_water, new FilledBucketDispenseBehavior());
		DispenserTile::REGISTRY.add(Item::bucket_empty, new EmptyBucketDispenseBehavior());
		DispenserTile::REGISTRY.add(Item::flintAndSteel, new FlintAndSteelDispenseBehavior());
		DispenserTile::REGISTRY.add(Item::dye_powder, new DyeDispenseBehavior());
		DispenserTile::REGISTRY.add(Item::items[Tile::tnt_Id], new TntDispenseBehavior());
	}
};