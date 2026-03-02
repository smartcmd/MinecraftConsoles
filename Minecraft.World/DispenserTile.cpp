#include "stdafx.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.h"
#include "DispenserTile.h"
#include "net.minecraft.h"
#include "Mob.h"

DispenserTile::DispenserTile(int id) : EntityTile(id, Material::stone)
{
	random = new Random();

	iconTop = NULL;
	iconFront = NULL;
	iconFrontVertical = NULL;
}

int DispenserTile::getTickDelay()
{
	return 4;
}

int DispenserTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Tile::dispenser_Id;
}

void DispenserTile::onPlace(Level *level, int x, int y, int z)
{
	EntityTile::onPlace(level, x, y, z);
	recalcLockDir(level, x, y, z);
}

void DispenserTile::recalcLockDir(Level *level, int x, int y, int z)
{
	if (level->isClientSide)
	{
		return;
	}

	int n = level->getTile(x, y, z - 1); // face = 2
	int s = level->getTile(x, y, z + 1); // face = 3
	int w = level->getTile(x - 1, y, z); // face = 4
	int e = level->getTile(x + 1, y, z); // face = 5

	int lockDir = 3;
	if (Tile::solid[n] && !Tile::solid[s]) lockDir = 3;
	if (Tile::solid[s] && !Tile::solid[n]) lockDir = 2;
	if (Tile::solid[w] && !Tile::solid[e]) lockDir = 5;
	if (Tile::solid[e] && !Tile::solid[w]) lockDir = 4;
	level->setData(x, y, z, lockDir);
}

Icon *DispenserTile::getTexture(int face, int data)
{
	int dir = data & FACING_MASK;

	if (face == dir)
	{
		if (dir == Facing::UP || dir == Facing::DOWN)
		{
			return iconFrontVertical;
		}
		else
		{
			return iconFront;
		}
	}

	if (dir == Facing::UP || dir == Facing::DOWN)
	{
		return iconTop;
	}
	else if (face == Facing::UP || face == Facing::DOWN)
	{
		return iconTop;
	}

	return icon;
}

void DispenserTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(L"furnace_side");
	iconTop = iconRegister->registerIcon(L"furnace_top");
	iconFront = iconRegister->registerIcon(L"dispenser_front");
	iconFrontVertical = iconRegister->registerIcon(L"dispenser_front_vertical");
}

// 4J-PB - Adding a TestUse for tooltip display
bool DispenserTile::TestUse()
{
	return true;
}

bool DispenserTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if( soundOnly) return false;

	if (level->isClientSide)
	{
		return true;
	}

	shared_ptr<DispenserTileEntity> trap = dynamic_pointer_cast<DispenserTileEntity>( level->getTileEntity(x, y, z) );
	player->openTrap(trap);

	return true;
}

void DispenserTile::fireArrow(Level *level, int x, int y, int z, Random *random)
{
	const int lockDir = level->getData(x, y, z);
	//const float power = 1.1f;
	const int accuracy = 6;
	//bool bLaunched=true;

	int xd = 0, zd = 0;
	if (lockDir == Facing::SOUTH)
	{
		zd = 1;
	}
	else if (lockDir == Facing::NORTH)
	{
		zd = -1;
	}
	else if (lockDir == Facing::EAST)
	{
		xd = 1;
	}
	else
	{
		xd = -1;
	}

	shared_ptr<DispenserTileEntity> trap = dynamic_pointer_cast<DispenserTileEntity>( level->getTileEntity(x, y, z) );
	if(trap != NULL)
	{
		int slot=trap->getRandomSlot();

		if (slot < 0)
		{
			level->levelEvent(LevelEvent::SOUND_CLICK_FAIL, x, y, z, 0);
		}
		else
		{
			double xp = x + xd * 0.6 + 0.5;
			double yp = y + 0.5;
			double zp = z + zd * 0.6 + 0.5;
			shared_ptr<ItemInstance> item=trap->getItem(slot);
			int result = dispenseItem(trap, level, item, random, x, y, z, xd, zd, xp, yp, zp);
			if (result == REMOVE_ITEM)
			{
				trap->removeItem(slot, 1);
			}
			else if (result == DISPENSE_ITEM)
			{
				item = trap->removeItem(slot, 1);
				throwItem(level, item, random, accuracy, xd, zd, xp, yp, zp);
				level->levelEvent(LevelEvent::SOUND_CLICK, x, y, z, 0);
			}

			level->levelEvent(LevelEvent::PARTICLES_SHOOT, x, y, z, (xd + 1) + (zd + 1) * 3);
		}
	}
}

void DispenserTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (type > 0 && Tile::tiles[type]->isSignalSource())
	{
		bool signal = level->hasNeighborSignal(x, y, z) || level->hasNeighborSignal(x, y + 1, z);
		if (signal)
		{
			level->addToTickNextTick(x, y, z, this->id, getTickDelay());
		}
	}
}

void DispenserTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (!level->isClientSide && ( level->hasNeighborSignal(x, y, z) || level->hasNeighborSignal(x, y + 1, z)))
	{
		fireArrow(level, x, y, z, random);
	}
}

shared_ptr<TileEntity> DispenserTile::newTileEntity(Level *level)
{
	return shared_ptr<DispenserTileEntity>( new DispenserTileEntity() );
}

void DispenserTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by)
{
	int dir = (Mth::floor(by->yRot * 4 / (360) + 0.5)) & 3;

	if (dir == 0) level->setData(x, y, z, Facing::NORTH);
	if (dir == 1) level->setData(x, y, z, Facing::EAST);
	if (dir == 2) level->setData(x, y, z, Facing::SOUTH);
	if (dir == 3) level->setData(x, y, z, Facing::WEST);
}

void DispenserTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	shared_ptr<Container> container = dynamic_pointer_cast<DispenserTileEntity>( level->getTileEntity(x, y, z) );
	if (container != NULL )
	{
		for (unsigned int i = 0; i < container->getContainerSize(); i++)
		{
			shared_ptr<ItemInstance> item = container->getItem(i);
			if (item != NULL)
			{
				float xo = random->nextFloat() * 0.8f + 0.1f;
				float yo = random->nextFloat() * 0.8f + 0.1f;
				float zo = random->nextFloat() * 0.8f + 0.1f;

				while (item->count > 0)
				{
					int count = random->nextInt(21) + 10;
					if (count > item->count) count = item->count;
					item->count -= count;

					shared_ptr<ItemInstance> newItem = shared_ptr<ItemInstance>( new ItemInstance(item->id, count, item->getAuxValue()) );
					newItem->set4JData( item->get4JData() );
					shared_ptr<ItemEntity> itemEntity = shared_ptr<ItemEntity>( new ItemEntity(level, x + xo, y + yo, z + zo, newItem ) );
					float pow = 0.05f;
					itemEntity->xd = (float) random->nextGaussian() * pow;
					itemEntity->yd = (float) random->nextGaussian() * pow + 0.2f;
					itemEntity->zd = (float) random->nextGaussian() * pow;
					if (item->hasTag())
					{
						itemEntity->getItem()->setTag((CompoundTag *) item->getTag()->copy());
					}
					level->addEntity(itemEntity);
				}

				// 4J Stu - Fix for duplication glitch
				container->setItem(i,nullptr);
			}
		}
	}
	EntityTile::onRemove(level, x, y, z, id, data);
}

void DispenserTile::throwItem(Level *level, shared_ptr<ItemInstance> item, Random *random, int accuracy, int xd, int zd, double xp, double yp, double zp)
{
	shared_ptr<ItemEntity> itemEntity = shared_ptr<ItemEntity>(new ItemEntity(level, xp, yp - 0.3, zp, item));

	double pow = random->nextDouble() * 0.1 + 0.2;
	itemEntity->xd = xd * pow;
	itemEntity->yd = .2f;
	itemEntity->zd = zd * pow;

	itemEntity->xd += (random->nextGaussian()) * 0.0075f * accuracy;
	itemEntity->yd += (random->nextGaussian()) * 0.0075f * accuracy;
	itemEntity->zd += (random->nextGaussian()) * 0.0075f * accuracy;

	level->addEntity(itemEntity);
}

int DispenserTile::dispenseItem(shared_ptr<DispenserTileEntity> trap, Level *level, shared_ptr<ItemInstance> item, Random *random, int x, int y, int z, int xd, int zd, double xp, double yp, double zp)
{
	float power = 1.1f;
	int accuracy = 6;

	// 4J-PB - moved to a switch
	switch(item->id)
	{
	case Item::arrow_Id:
		{
			int currentProjectiles = level->countInstanceOf(eTYPE_PROJECTILE,false);
			if(currentProjectiles < Level::MAX_DISPENSABLE_PROJECTILES)	// 4J - added limit
			{
				shared_ptr<Arrow> arrow = shared_ptr<Arrow>( new Arrow(level, xp, yp, zp) );
				arrow->shoot(xd, .1f, zd, power, (float) accuracy);
				arrow->pickup = Arrow::PICKUP_ALLOWED;
				level->addEntity(arrow);
				level->levelEvent(LevelEvent::SOUND_LAUNCH, x, y, z, 0);
				return REMOVE_ITEM;
			}
			else
			{
				// some negative sound effect?
				level->levelEvent(LevelEvent::SOUND_CLICK_FAIL, x, y, z, 0);

				// not sending a message here, since we will probably get flooded with them when people have automatic dispensers for spawn eggs
				return LEAVE_ITEM;
			}
		}					
		break;
	case Item::egg_Id:
		{
			int currentProjectiles = level->countInstanceOf(eTYPE_PROJECTILE,false);
			if(currentProjectiles < Level::MAX_DISPENSABLE_PROJECTILES)	// 4J - added limit
			{
				shared_ptr<ThrownEgg> egg = shared_ptr<ThrownEgg>( new ThrownEgg(level, xp, yp, zp) );
				egg->shoot(xd, .1f, zd, power, (float) accuracy);
				level->addEntity(egg);
				level->levelEvent(LevelEvent::SOUND_LAUNCH, x, y, z, 0);
				return REMOVE_ITEM;
			}
			else
			{
				// some negative sound effect?
				level->levelEvent(LevelEvent::SOUND_CLICK_FAIL, x, y, z, 0);

				// not sending a message here, since we will probably get flooded with them when people have automatic dispensers for spawn eggs
				return LEAVE_ITEM;
			}
		}
		break;
	case Item::snowBall_Id:
		{
			int currentProjectiles = level->countInstanceOf(eTYPE_PROJECTILE,false);
			if(currentProjectiles < Level::MAX_DISPENSABLE_PROJECTILES)	// 4J - added limit
			{
				shared_ptr<Snowball> snowball = shared_ptr<Snowball>( new Snowball(level, xp, yp, zp) );
				snowball->shoot(xd, .1f, zd, power, (float) accuracy);
				level->addEntity(snowball);
				level->levelEvent(LevelEvent::SOUND_LAUNCH, x, y, z, 0);
				return REMOVE_ITEM;
			}
			else
			{
				// some negative sound effect?
				level->levelEvent(LevelEvent::SOUND_CLICK_FAIL, x, y, z, 0);

				// not sending a message here, since we will probably get flooded with them when people have automatic dispensers for spawn eggs
				return LEAVE_ITEM;
			}
		}
		break;
	case Item::potion_Id:
		{
			int currentProjectiles = level->countInstanceOf(eTYPE_PROJECTILE,false);
			if(currentProjectiles < Level::MAX_DISPENSABLE_PROJECTILES)	// 4J - added limit
			{
				if(PotionItem::isThrowable(item->getAuxValue()))
				{
					shared_ptr<ThrownPotion> potion = shared_ptr<ThrownPotion>(new ThrownPotion(level, xp, yp, zp, item->getAuxValue()));
					potion->shoot(xd, .1f, zd, power * 1.25f, accuracy * .5f);
					level->addEntity(potion);
					level->levelEvent(LevelEvent::SOUND_LAUNCH, x, y, z, 0);
				}
				else
				{
					shared_ptr<ItemEntity> itemEntity = shared_ptr<ItemEntity>( new ItemEntity(level, xp, yp - 0.3, zp, item) );

					double pow = random->nextDouble() * 0.1 + 0.2;
					itemEntity->xd = xd * pow;
					itemEntity->yd = .2f;
					itemEntity->zd = zd * pow;

					itemEntity->xd += (random->nextGaussian()) * 0.0075f * accuracy;
					itemEntity->yd += (random->nextGaussian()) * 0.0075f * accuracy;
					itemEntity->zd += (random->nextGaussian()) * 0.0075f * accuracy;

					level->addEntity(itemEntity);
					level->levelEvent(LevelEvent::SOUND_CLICK, x, y, z, 0);
				}					
				return REMOVE_ITEM;
			}
			else
			{
				// some negative sound effect?
				level->levelEvent(LevelEvent::SOUND_CLICK_FAIL, x, y, z, 0);

				// not sending a message here, since we will probably get flooded with them when people have automatic dispensers for spawn eggs
				return LEAVE_ITEM;
			}
		}
		break;
	case Item::expBottle_Id:
		{
			int currentProjectiles = level->countInstanceOf(eTYPE_PROJECTILE,false);
			if(currentProjectiles < Level::MAX_DISPENSABLE_PROJECTILES)	// 4J - added limit
			{
				shared_ptr<ThrownExpBottle> expBottle = shared_ptr<ThrownExpBottle>( new ThrownExpBottle(level, xp, yp, zp) );
				expBottle->shoot(xd, .1f, zd, power * 1.25f, accuracy * .5f);
				level->addEntity(expBottle);
				level->levelEvent(LevelEvent::SOUND_LAUNCH, x, y, z, 0);
				return REMOVE_ITEM;
			}
			else
			{
				// some negative sound effect?
				level->levelEvent(LevelEvent::SOUND_CLICK_FAIL, x, y, z, 0);

				// not sending a message here, since we will probably get flooded with them when people have automatic dispensers for spawn eggs
				return LEAVE_ITEM;
			}
		}					
		break;
	case Item::fireball_Id: // TU9
		{
			int currentFireballs = level->countInstanceOf(eTYPE_SMALL_FIREBALL,true);
			if(currentFireballs < Level::MAX_DISPENSABLE_FIREBALLS)	// 4J - added limit
			{
				shared_ptr<SmallFireball> fireball = shared_ptr<SmallFireball>( new SmallFireball(level, xp + xd * .3, yp, zp + zd * .3, xd + random->nextGaussian() * .05, random->nextGaussian() * .05, zd + random->nextGaussian() * .05));
				level->addEntity(fireball);
				level->levelEvent(LevelEvent::SOUND_BLAZE_FIREBALL, x, y, z, 0);
				return REMOVE_ITEM;
			}
			else
			{
				// some negative sound effect?
				level->levelEvent(LevelEvent::SOUND_CLICK_FAIL, x, y, z, 0);

				// not sending a message here, since we will probably get flooded with them when people have automatic dispensers for spawn eggs
				return LEAVE_ITEM;
			}
		} 					
		break;
	case Item::monsterPlacer_Id:
		{
			int iResult=0;
			//MonsterPlacerItem *spawnEgg = (MonsterPlacerItem *)item->getItem();
			shared_ptr<Entity> newEntity = MonsterPlacerItem::canSpawn(item->getAuxValue(), level,&iResult);
			
			shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(newEntity);
			if (mob != NULL)
			{		
				// 4J-PB - Changed the line below slightly since mobs were sticking to the dispenser rather than dropping down when fired
				mob->moveTo(xp + xd * 0.4, yp - 0.3, zp + zd * 0.4, level->random->nextFloat() * 360, 0);
				mob->finalizeMobSpawn();
				level->addEntity(mob);		
				level->levelEvent(LevelEvent::SOUND_LAUNCH, x, y, z, 0);
				return REMOVE_ITEM;
			}
			else
			{
				// some negative sound effect?
				level->levelEvent(LevelEvent::SOUND_CLICK_FAIL, x, y, z, 0);

				// not sending a message here, since we will probably get flooded with them when people have automatic dispensers for spawn eggs
				return LEAVE_ITEM;
			}
		}
		break;
	case Item::bucket_lava_Id:
	case Item::bucket_water_Id:
		{
			BucketItem *pBucket = (BucketItem *) item->getItem();

			if (pBucket->emptyBucket(level, x, y, z, x + xd, y, z + zd)) 
			{
				item->id = Item::bucket_empty_Id;
				item->count = 1;
				return LEAVE_ITEM;
			}
			return DISPENSE_ITEM;
		}
		break;
	case Item::bucket_empty_Id:
		{
			int xt = x + xd;
			int zt = z + zd;
			Material *pMaterial=level->getMaterial(xt, y, zt);
			int data = level->getData(xt, y, zt);

			if (pMaterial == Material::water && data == 0)
			{
				level->setTile(xt, y, zt, 0);

				if (--item->count == 0)
				{
					item->id = Item::bucket_water_Id;
					item->count = 1;
				}
				else if (trap->addItem(shared_ptr<ItemInstance>(new ItemInstance(Item::bucket_water))) < 0)
				{
					throwItem(level, shared_ptr<ItemInstance>(new ItemInstance(Item::bucket_water)), random, 6, xd, zd, xp, yp, zp);
				}

				return LEAVE_ITEM;
			}
			else if (pMaterial == Material::lava && data == 0)
			{
				level->setTile(xt, y, zt, 0);

				if (--item->count == 0)
				{
					item->id = Item::bucket_lava_Id;
					item->count = 1;
				}
				else if (trap->addItem(shared_ptr<ItemInstance>(new ItemInstance(Item::bucket_lava))) < 0)
				{
					throwItem(level, shared_ptr<ItemInstance>(new ItemInstance(Item::bucket_lava)), random, 6, xd, zd, xp, yp, zp);
				}

				return LEAVE_ITEM;
			}
			return DISPENSE_ITEM;
		}

		break;
		// TU12
	case Item::minecart_Id:
	case Item::minecart_chest_Id:
	case Item::minecart_furnace_Id:
		{
			xp = x + (xd < 0 ? xd * 0.8 :  xd * 1.8f) + Mth::abs(zd) * 0.5f;
			zp = z + (zd < 0 ? zd * 0.8 :  zd * 1.8f) + Mth::abs(xd) * 0.5f;

			if (RailTile::isRail(level, x + xd, y, z + zd)) 
			{
				yp = y + 0.5f;
			} 
			else if (level->isEmptyTile(x + xd, y, z + zd) && RailTile::isRail(level, x + xd, y - 1, z + zd)) 				
			{
				yp = y - 0.5f;
			} 
			else 
			{
				return DISPENSE_ITEM;
			}

			if( level->countInstanceOf(eTYPE_MINECART, true) < Level::MAX_CONSOLE_MINECARTS )		// 4J - added limit
			{
				shared_ptr<Minecart> minecart = shared_ptr<Minecart>(new Minecart(level, xp, yp, zp, ((MinecartItem *) item->getItem())->type));
				level->addEntity(minecart);
				level->levelEvent(LevelEvent::SOUND_CLICK, x, y, z, 0);

				return REMOVE_ITEM;
			}
			else
			{
				return DISPENSE_ITEM;
			}
		}
		break;

	case Item::boat_Id:
		{
			bool bLaunchBoat=false;

			xp = x + (xd < 0 ? xd * 0.8 :  xd * 1.8f) + Mth::abs(zd) * 0.5f;
			zp = z + (zd < 0 ? zd * 0.8 :  zd * 1.8f) + Mth::abs(xd) * 0.5f;

			if (level->getMaterial(x + xd, y, z + zd) == Material::water) 
			{
				bLaunchBoat=true;
				yp = y + 1.0f;
			} 
			else if (level->isEmptyTile(x + xd, y, z + zd) && level->getMaterial(x + xd, y - 1, z + zd) == Material::water) 
			{
				bLaunchBoat=true;
				yp = y;
			} 			

			// check the limit on boats
			if( bLaunchBoat && level->countInstanceOf(eTYPE_BOAT, true) < Level::MAX_XBOX_BOATS )		// 4J - added limit
			{
				shared_ptr<Boat> boat = shared_ptr<Boat>(new Boat(level, xp, yp, zp));
				level->addEntity(boat);
				level->levelEvent(LevelEvent::SOUND_CLICK, x, y, z, 0);
				return REMOVE_ITEM;
			}
			else
			{
				return DISPENSE_ITEM;
			}
		}
		break;
	}

	return DISPENSE_ITEM;
}
