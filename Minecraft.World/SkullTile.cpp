#include "stdafx.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "WitherBoss.h"
#include "net.minecraft.h"
#include "SkullTile.h"

SkullTile::SkullTile(int id) : BaseEntityTile(id, Material::decoration, isSolidRender() )
{
	setShape(4.0f / 16.0f, 0, 4.0f / 16.0f, 12.0f / 16.0f, .5f, 12.0f / 16.0f);
}

int SkullTile::getRenderShape()
{
	return SHAPE_INVISIBLE;
}

bool SkullTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool SkullTile::isCubeShaped()
{
	return false;
}

void SkullTile::updateShape(LevelSource *level, int x, int y, int z, int forceData , shared_ptr<TileEntity> forceEntity)
{
	int data = level->getData(x, y, z) & PLACEMENT_MASK;

	switch (data)
	{
	default:
	case Facing::UP:
		setShape(4.0f / 16.0f, 0, 4.0f / 16.0f, 12.0f / 16.0f, .5f, 12.0f / 16.0f);
		break;
	case Facing::NORTH:
		setShape(4.0f / 16.0f, 4.0f / 16.0f, .5f, 12.0f / 16.0f, 12.0f / 16.0f, 1);
		break;
	case Facing::SOUTH:
		setShape(4.0f / 16.0f, 4.0f / 16.0f, 0, 12.0f / 16.0f, 12.0f / 16.0f, .5f);
		break;
	case Facing::WEST:
		setShape(.5f, 4.0f / 16.0f, 4.0f / 16.0f, 1, 12.0f / 16.0f, 12.0f / 16.0f);
		break;
	case Facing::EAST:
		setShape(0, 4.0f / 16.0f, 4.0f / 16.0f, .5f, 12.0f / 16.0f, 12.0f / 16.0f);
		break;
	}
}

AABB *SkullTile::getAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	return BaseEntityTile::getAABB(level, x, y, z);
}

void SkullTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by)
{
	int dir = Mth::floor(by->yRot * 4 / (360) + 2.5) & 3;
	level->setData(x, y, z, dir, Tile::UPDATE_CLIENTS);
}

shared_ptr<TileEntity> SkullTile::newTileEntity(Level *level)
{
	return std::make_shared<SkullTileEntity>();
}

int SkullTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Item::skull_Id;
}

int SkullTile::cloneTileData(Level *level, int x, int y, int z)
{
	shared_ptr<TileEntity> tileEntity = level->getTileEntity(x, y, z);
	shared_ptr<SkullTileEntity> skull = dynamic_pointer_cast<SkullTileEntity>(tileEntity);
	if (skull != nullptr)
	{
		return skull->getSkullType();
	}
	return BaseEntityTile::cloneTileData(level, x, y, z);
}

int SkullTile::getSpawnResourcesAuxValue(int data)
{
	return data;
}

void SkullTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel)
{
	// do nothing, resource is popped by onRemove
	// ... because the tile entity is removed prior to spawnResources
}

void SkullTile::playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player)
{
	if (player->abilities.instabuild)
	{
		// prevent resource drop
		data |= NO_DROP_BIT;
		level->setData(x, y, z, data, Tile::UPDATE_NONE);
	}
	BaseEntityTile::playerWillDestroy(level, x, y, z, data, player);
}

void SkullTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	if (level->isClientSide) return;
	if ((data & NO_DROP_BIT) == 0)
	{
		shared_ptr<ItemInstance> item = std::make_shared<ItemInstance>(Item::skull_Id, 1, cloneTileData(level, x, y, z));
		shared_ptr<SkullTileEntity> entity = dynamic_pointer_cast<SkullTileEntity>(level->getTileEntity(x, y, z));

		if (entity->getSkullType() == SkullTileEntity::TYPE_CHAR && !entity->getExtraType().empty())
		{
			item->setTag(new CompoundTag());
			item->getTag()->putString(L"SkullOwner", entity->getExtraType());
		}

		popResource(level, x, y, z, item);
	}
	BaseEntityTile::onRemove(level, x, y, z, id, data);
}

int SkullTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::skull_Id;
}

void SkullTile::checkMobSpawn(Level *level, int x, int y, int z, shared_ptr<SkullTileEntity> placedSkull)
{
	if (placedSkull->getSkullType() == SkullTileEntity::TYPE_WITHER && y >= 2 && level->difficulty > Difficulty::PEACEFUL && !level->isClientSide)
	{
		// Check wither boss spawn
		int ss = Tile::soulsand_Id;

		// North-south alignment
		for (int zo = -2; zo <= 0; zo++)
		{
			if ( //
				level->getTile(x, y - 1, z + zo) == ss && //
				level->getTile(x, y - 1, z + zo + 1) == ss && //
				level->getTile(x, y - 2, z + zo + 1) == ss && //
				level->getTile(x, y - 1, z + zo + 2) == ss && //
				isSkullAt(level, x, y, z + zo, SkullTileEntity::TYPE_WITHER) && //
				isSkullAt(level, x, y, z + zo + 1, SkullTileEntity::TYPE_WITHER) && //
				isSkullAt(level, x, y, z + zo + 2, SkullTileEntity::TYPE_WITHER))
			{
				level->setData(x, y, z + zo, NO_DROP_BIT, Tile::UPDATE_CLIENTS);
				level->setData(x, y, z + zo + 1, NO_DROP_BIT, Tile::UPDATE_CLIENTS);
				level->setData(x, y, z + zo + 2, NO_DROP_BIT, Tile::UPDATE_CLIENTS);
				level->setTileAndData(x, y, z + zo, 0, 0, Tile::UPDATE_CLIENTS);
				level->setTileAndData(x, y, z + zo + 1, 0, 0, Tile::UPDATE_CLIENTS);
				level->setTileAndData(x, y, z + zo + 2, 0, 0, Tile::UPDATE_CLIENTS);
				level->setTileAndData(x, y - 1, z + zo, 0, 0, Tile::UPDATE_CLIENTS);
				level->setTileAndData(x, y - 1, z + zo + 1, 0, 0, Tile::UPDATE_CLIENTS);
				level->setTileAndData(x, y - 1, z + zo + 2, 0, 0, Tile::UPDATE_CLIENTS);
				level->setTileAndData(x, y - 2, z + zo + 1, 0, 0, Tile::UPDATE_CLIENTS);

				// 4J: Check that we can spawn a Wither
				if (level->canCreateMore(eTYPE_WITHERBOSS, Level::eSpawnType_Egg))
				{
					// 4J: Removed !isClientSide check because there's one earlier on
					shared_ptr<WitherBoss> witherBoss = std::make_shared<WitherBoss>(level);
					witherBoss->moveTo(x + 0.5, y - 1.45, z + zo + 1.5, 90, 0);
					witherBoss->yBodyRot = 90;
					witherBoss->makeInvulnerable();
					level->addEntity(witherBoss);
				}
				else
				{
					// 4J: Can't spawn, drop resource instead
					Tile::tiles[Tile::soulsand_Id]->spawnResources(level, x, y - 1, z + zo, 0, 0);
					Tile::tiles[Tile::soulsand_Id]->spawnResources(level, x, y - 1, z + zo + 1, 0, 0);
					Tile::tiles[Tile::soulsand_Id]->spawnResources(level, x, y - 2, z + zo + 1, 0, 0);
					Tile::tiles[Tile::soulsand_Id]->spawnResources(level, x, y - 1, z + zo + 2, 0, 0);
					
					shared_ptr<ItemInstance> itemInstance = std::make_shared<ItemInstance>(Item::skull_Id, 3, SkullTileEntity::TYPE_WITHER);
					shared_ptr<ItemEntity> itemEntity = std::make_shared<ItemEntity>(level, x, y, z + zo + 1, itemInstance);
					level->addEntity(itemEntity);
				}

				for (int i = 0; i < 120; i++)
				{
					level->addParticle(eParticleType_snowballpoof, x + level->random->nextDouble(), y - 2 + level->random->nextDouble() * 3.9, z + zo + 1 + level->random->nextDouble(), 0, 0, 0);
				}

				level->tileUpdated(x, y, z + zo, 0);
				level->tileUpdated(x, y, z + zo + 1, 0);
				level->tileUpdated(x, y, z + zo + 2, 0);
				level->tileUpdated(x, y - 1, z + zo, 0);
				level->tileUpdated(x, y - 1, z + zo + 1, 0);
				level->tileUpdated(x, y - 1, z + zo + 2, 0);
				level->tileUpdated(x, y - 2, z + zo + 1, 0);

				return;
			}
		}
		// West-east alignment
		for (int xo = -2; xo <= 0; xo++)
		{
			if ( //
				level->getTile(x + xo, y - 1, z) == ss && //
				level->getTile(x + xo + 1, y - 1, z) == ss && //
				level->getTile(x + xo + 1, y - 2, z) == ss && //
				level->getTile(x + xo + 2, y - 1, z) == ss && //
				isSkullAt(level, x + xo, y, z, SkullTileEntity::TYPE_WITHER) && //
				isSkullAt(level, x + xo + 1, y, z, SkullTileEntity::TYPE_WITHER) && //
				isSkullAt(level, x + xo + 2, y, z, SkullTileEntity::TYPE_WITHER))
			{

				level->setData(x + xo, y, z, NO_DROP_BIT, Tile::UPDATE_CLIENTS);
				level->setData(x + xo + 1, y, z, NO_DROP_BIT, Tile::UPDATE_CLIENTS);
				level->setData(x + xo + 2, y, z, NO_DROP_BIT, Tile::UPDATE_CLIENTS);
				level->setTileAndData(x + xo, y, z, 0, 0, Tile::UPDATE_CLIENTS);
				level->setTileAndData(x + xo + 1, y, z, 0, 0, Tile::UPDATE_CLIENTS);
				level->setTileAndData(x + xo + 2, y, z, 0, 0, Tile::UPDATE_CLIENTS);
				level->setTileAndData(x + xo, y - 1, z, 0, 0, Tile::UPDATE_CLIENTS);
				level->setTileAndData(x + xo + 1, y - 1, z, 0, 0, Tile::UPDATE_CLIENTS);
				level->setTileAndData(x + xo + 2, y - 1, z, 0, 0, Tile::UPDATE_CLIENTS);
				level->setTileAndData(x + xo + 1, y - 2, z, 0, 0, Tile::UPDATE_CLIENTS);

				// 4J: Check that we can spawn a Wither
				if (level->canCreateMore(eTYPE_WITHERBOSS, Level::eSpawnType_Egg))
				{
					// 4J: Removed !isClientSide check because there's one earlier on
					shared_ptr<WitherBoss> witherBoss = std::make_shared<WitherBoss>(level);
					witherBoss->moveTo(x + xo + 1.5, y - 1.45, z + .5, 0, 0);
					witherBoss->makeInvulnerable();
					level->addEntity(witherBoss);
				}
				else
				{
					// 4J: Can't spawn, drop resource instead
					Tile::tiles[Tile::soulsand_Id]->spawnResources(level, x + xo, y - 1, z, 0, 0);
					Tile::tiles[Tile::soulsand_Id]->spawnResources(level, x + xo + 1, y - 1, z, 0, 0);
					Tile::tiles[Tile::soulsand_Id]->spawnResources(level, x + xo + 1, y - 2, z, 0, 0);
					Tile::tiles[Tile::soulsand_Id]->spawnResources(level, x + xo + 2, y - 1, z, 0, 0);

					shared_ptr<ItemInstance> itemInstance = std::make_shared<ItemInstance>(Item::skull_Id, 3, SkullTileEntity::TYPE_WITHER);
					shared_ptr<ItemEntity> itemEntity = std::make_shared<ItemEntity>(level, x + xo + 1, y, z, itemInstance);
					level->addEntity(itemEntity);
				}

				for (int i = 0; i < 120; i++)
				{
					level->addParticle(eParticleType_snowballpoof, x + xo + 1 + level->random->nextDouble(), y - 2 + level->random->nextDouble() * 3.9, z + level->random->nextDouble(), 0, 0, 0);
				}

				level->tileUpdated(x + xo, y, z, 0);
				level->tileUpdated(x + xo + 1, y, z, 0);
				level->tileUpdated(x + xo + 2, y, z, 0);
				level->tileUpdated(x + xo, y - 1, z, 0);
				level->tileUpdated(x + xo + 1, y - 1, z, 0);
				level->tileUpdated(x + xo + 2, y - 1, z, 0);
				level->tileUpdated(x + xo + 1, y - 2, z, 0);

				return;
			}
		}
	}
}

bool SkullTile::isSkullAt(Level *level, int x, int y, int z, int skullType)
{
	if (level->getTile(x, y, z) != id)
	{
		return false;
	}
	shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
	shared_ptr<SkullTileEntity> skull = dynamic_pointer_cast<SkullTileEntity>(te);
	if (skull == nullptr)
	{
		return false;
	}
	return skull->getSkullType() == skullType;
}

void SkullTile::registerIcons(IconRegister *iconRegister)
{
	// None
}

Icon *SkullTile::getTexture(int face, int data)
{
	return Tile::soulsand->getTexture(face);
}

wstring SkullTile::getTileItemIconName()
{
	return getIconName() + L"_" + SkullItem::ICON_NAMES[0];
}