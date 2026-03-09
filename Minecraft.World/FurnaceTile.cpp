#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.inventory.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.h"
#include "FurnaceTile.h"
#include "Mob.h"
#include "Facing.h"

bool FurnaceTile::noDrop = false;

FurnaceTile::FurnaceTile(int id, bool lit) : BaseEntityTile(id, Material::stone)
{
	random = new Random();
	this->lit = lit;
	
	iconTop = nullptr;
	iconFront = nullptr;
}

int FurnaceTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Tile::furnace_Id;
}

void FurnaceTile::onPlace(Level *level, int x, int y, int z)
{
	BaseEntityTile::onPlace(level, x, y, z);
	recalcLockDir(level, x, y, z);
}

void FurnaceTile::recalcLockDir(Level *level, int x, int y, int z)
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

Icon *FurnaceTile::getTexture(int face, int data)
{
	if (face == Facing::UP) return iconTop;
	if (face == Facing::DOWN) return iconTop;

	if (face != data) return icon;
	return iconFront;
}

void FurnaceTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(L"furnace_side");
	iconFront = iconRegister->registerIcon(lit ? L"furnace_front_lit" : L"furnace_front");
	iconTop = iconRegister->registerIcon(L"furnace_top");
}

void FurnaceTile::animateTick(Level *level, int xt, int yt, int zt, Random *random)
{
	if (!lit) return;

	int dir = level->getData(xt, yt, zt);

	float x = xt + 0.5f;
	float y = yt + 0.0f + random->nextFloat() * 6 / 16.0f;
	float z = zt + 0.5f;
	float r = 0.52f;
	float ss = random->nextFloat() * 0.6f - 0.3f;

	if (dir == 4)
	{
		level->addParticle(eParticleType_smoke, x - r, y, z + ss, 0, 0, 0);
		level->addParticle(eParticleType_flame, x - r, y, z + ss, 0, 0, 0);
	}
	else if (dir == 5)
	{
		level->addParticle(eParticleType_smoke, x + r, y, z + ss, 0, 0, 0);
		level->addParticle(eParticleType_flame, x + r, y, z + ss, 0, 0, 0);
	}
	else if (dir == 2)
	{
		level->addParticle(eParticleType_smoke, x + ss, y, z - r, 0, 0, 0);
		level->addParticle(eParticleType_flame, x + ss, y, z - r, 0, 0, 0);
	}
	else if (dir == 3)
	{
		level->addParticle(eParticleType_smoke, x + ss, y, z + r, 0, 0, 0);
		level->addParticle(eParticleType_flame, x + ss, y, z + r, 0, 0, 0);
	}
}

// 4J-PB - Adding a TestUse for tooltip display
bool FurnaceTile::TestUse()
{
	return true;
}

bool FurnaceTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if( soundOnly) return false;

	if (level->isClientSide)
	{
		return true;
	}
	shared_ptr<FurnaceTileEntity> furnace = dynamic_pointer_cast<FurnaceTileEntity>( level->getTileEntity(x, y, z) );
	if (furnace != nullptr ) player->openFurnace(furnace);
	return true;
}

void FurnaceTile::setLit(bool lit, Level *level, int x, int y, int z)
{
	int data = level->getData(x, y, z);
	shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);

	noDrop = true;
	if (lit) level->setTileAndUpdate(x, y, z, Tile::furnace_lit_Id);
	else level->setTileAndUpdate(x, y, z, Tile::furnace_Id);
	noDrop = false;

	level->setData(x, y, z, data, Tile::UPDATE_CLIENTS);
	if( te != nullptr )
	{
		te->clearRemoved();
		level->setTileEntity(x, y, z, te);
	}
}

shared_ptr<TileEntity> FurnaceTile::newTileEntity(Level *level)
{
	return std::make_shared<FurnaceTileEntity>();
}

void FurnaceTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	int dir = (Mth::floor(by->yRot * 4 / (360) + 0.5)) & 3;

	if (dir == 0) level->setData(x, y, z, Facing::NORTH, Tile::UPDATE_CLIENTS);
	if (dir == 1) level->setData(x, y, z, Facing::EAST, Tile::UPDATE_CLIENTS);
	if (dir == 2) level->setData(x, y, z, Facing::SOUTH, Tile::UPDATE_CLIENTS);
	if (dir == 3) level->setData(x, y, z, Facing::WEST, Tile::UPDATE_CLIENTS);

	if (itemInstance->hasCustomHoverName())
	{
		dynamic_pointer_cast<FurnaceTileEntity>( level->getTileEntity(x, y, z))->setCustomName(itemInstance->getHoverName());
	}
}

void FurnaceTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	if (!noDrop)
	{
		shared_ptr<Container> container = dynamic_pointer_cast<FurnaceTileEntity>( level->getTileEntity(x, y, z) );
		if( container != nullptr )
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

#ifndef _CONTENT_PACKAGE
						if(level->isClientSide)
						{
							printf("Client furnace dropping %d of %d/%d\n", count, item->id, item->getAuxValue() );
						}
						else
						{
							printf("Server furnace dropping %d of %d/%d\n", count, item->id, item->getAuxValue() );
						}
#endif
						
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
	}
	BaseEntityTile::onRemove(level, x, y, z, id, data);

}

bool FurnaceTile::hasAnalogOutputSignal()
{
	return true;
}

int FurnaceTile::getAnalogOutputSignal(Level *level, int x, int y, int z, int dir)
{
	return AbstractContainerMenu::getRedstoneSignalFromContainer(dynamic_pointer_cast<Container>( level->getTileEntity(x, y, z)) );
}

int FurnaceTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Tile::furnace_Id;
}