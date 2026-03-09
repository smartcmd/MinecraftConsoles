#include "stdafx.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.inventory.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.h"
#include "DispenserTile.h"
#include "net.minecraft.h"
#include "Mob.h"

BehaviorRegistry DispenserTile::REGISTRY = BehaviorRegistry(new DefaultDispenseItemBehavior());

DispenserTile::DispenserTile(int id) : BaseEntityTile(id, Material::stone)
{
	random = new Random();

	iconTop = nullptr;
	iconFront = nullptr;
	iconFrontVertical = nullptr;
}

int DispenserTile::getTickDelay(Level *level)
{
	return 4;
}

void DispenserTile::onPlace(Level *level, int x, int y, int z)
{
	BaseEntityTile::onPlace(level, x, y, z);
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
	level->setData(x, y, z, lockDir, Tile::UPDATE_CLIENTS);
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

void DispenserTile::dispenseFrom(Level *level, int x, int y, int z)
{
	BlockSourceImpl source(level, x, y, z);
	shared_ptr<DispenserTileEntity> trap = dynamic_pointer_cast<DispenserTileEntity>( source.getEntity() );
	if (trap == nullptr) return;

	int slot = trap->getRandomSlot();
	if (slot < 0)
	{
		level->levelEvent(LevelEvent::SOUND_CLICK_FAIL, x, y, z, 0);
	}
	else
	{
		shared_ptr<ItemInstance> item = trap->getItem(slot);
		DispenseItemBehavior *behavior = getDispenseMethod(item);

		if (behavior != DispenseItemBehavior::NOOP)
		{
			shared_ptr<ItemInstance> leftOver = behavior->dispense(&source, item);

			trap->setItem(slot, leftOver->count == 0 ? nullptr : leftOver);
		}
	}
}

DispenseItemBehavior *DispenserTile::getDispenseMethod(shared_ptr<ItemInstance> item)
{
	return REGISTRY.get(item->getItem());
}

void DispenserTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	bool signal = level->hasNeighborSignal(x, y, z) || level->hasNeighborSignal(x, y + 1, z);
	int data = level->getData(x, y, z);
	bool isTriggered = (data & TRIGGER_BIT) != 0;

	if (signal && !isTriggered)
	{
		level->addToTickNextTick(x, y, z, id, getTickDelay(level));
		level->setData(x, y, z, data | TRIGGER_BIT, UPDATE_NONE);
	}
	else if (!signal && isTriggered)
	{
		level->setData(x, y, z, data & ~TRIGGER_BIT, UPDATE_NONE);
	}
}

void DispenserTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (!level->isClientSide) // && (level.hasNeighborSignal(x, y, z) || level.hasNeighborSignal(x, y + 1, z)))
	{
		dispenseFrom(level, x, y, z);
	}
}

shared_ptr<TileEntity> DispenserTile::newTileEntity(Level *level)
{
	return std::make_shared<DispenserTileEntity>();
}

void DispenserTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	int dir = PistonBaseTile::getNewFacing(level, x, y, z, by);

	level->setData(x, y, z, dir, Tile::UPDATE_CLIENTS);

	if (itemInstance->hasCustomHoverName())
	{
		dynamic_pointer_cast<DispenserTileEntity>( level->getTileEntity(x, y, z))->setCustomName(itemInstance->getHoverName());
	}
}

void DispenserTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	shared_ptr<Container> container = dynamic_pointer_cast<DispenserTileEntity>( level->getTileEntity(x, y, z) );
	if (container != nullptr )
	{
		for (unsigned int i = 0; i < container->getContainerSize(); i++)
		{
			shared_ptr<ItemInstance> item = container->getItem(i);
			if (item != nullptr)
			{
				float xo = random->nextFloat() * 0.8f + 0.1f;
				float yo = random->nextFloat() * 0.8f + 0.1f;
				float zo = random->nextFloat() * 0.8f + 0.1f;

				while (item->count > 0)
				{
					int count = random->nextInt(21) + 10;
					if (count > item->count) count = item->count;
					item->count -= count;

					shared_ptr<ItemInstance> newItem = std::make_shared<ItemInstance>(item->id, count, item->getAuxValue());
					newItem->set4JData( item->get4JData() );
					shared_ptr<ItemEntity> itemEntity = std::make_shared<ItemEntity>(level, x + xo, y + yo, z + zo, newItem);
					float pow = 0.05f;
					itemEntity->xd = static_cast<float>(random->nextGaussian()) * pow;
					itemEntity->yd = static_cast<float>(random->nextGaussian()) * pow + 0.2f;
					itemEntity->zd = static_cast<float>(random->nextGaussian()) * pow;
					if (item->hasTag())
					{
						itemEntity->getItem()->setTag(static_cast<CompoundTag *>(item->getTag()->copy()));
					}
					level->addEntity(itemEntity);
				}

				// 4J Stu - Fix for duplication glitch
				container->setItem(i,nullptr);
			}
		}
		level->updateNeighbourForOutputSignal(x, y, z, id);
	}
	BaseEntityTile::onRemove(level, x, y, z, id, data);
}

Position *DispenserTile::getDispensePosition(BlockSource *source)
{
	FacingEnum *facing = getFacing(source->getData());

	double originX = source->getX() + 0.7 * facing->getStepX();
	double originY = source->getY() + 0.7 * facing->getStepY();
	double originZ = source->getZ() + 0.7 * facing->getStepZ();

	return new PositionImpl(originX, originY, originZ);
}

FacingEnum *DispenserTile::getFacing(int data)
{
	return FacingEnum::fromData(data & FACING_MASK);
}

bool DispenserTile::hasAnalogOutputSignal()
{
	return true;
}

int DispenserTile::getAnalogOutputSignal(Level *level, int x, int y, int z, int dir)
{
	return AbstractContainerMenu::getRedstoneSignalFromContainer(dynamic_pointer_cast<Container>( level->getTileEntity(x, y, z)) );
}