#include "stdafx.h"
#include "PistonMovingPiece.h"
#include "PistonPieceEntity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.h"
#include "Facing.h"
#include "AABB.h"

PistonMovingPiece::PistonMovingPiece(int id) : BaseEntityTile(id, Material::piston, isSolidRender() )
{
	setDestroyTime(INDESTRUCTIBLE_DESTROY_TIME);
}

shared_ptr<TileEntity> PistonMovingPiece::newTileEntity(Level *level)
{
	return nullptr;
}

void PistonMovingPiece::onPlace(Level *level, int x, int y, int z)
{
}

void PistonMovingPiece::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	shared_ptr<TileEntity> tileEntity = level->getTileEntity(x, y, z);
	if (tileEntity != nullptr && dynamic_pointer_cast<PistonPieceEntity>(tileEntity) != nullptr)
	{
		dynamic_pointer_cast<PistonPieceEntity>(tileEntity)->finalTick();
	}
	else
	{
		BaseEntityTile::onRemove(level, x, y, z, id, data);
	}
}

bool PistonMovingPiece::mayPlace(Level *level, int x, int y, int z)
{
	return false;
}

bool PistonMovingPiece::mayPlace(Level *level, int x, int y, int z, int face)
{
	return false;
}

int PistonMovingPiece::getRenderShape()
{
	return SHAPE_INVISIBLE;
}

bool PistonMovingPiece::isSolidRender(bool isServerLevel)
{
	return false;
}

bool PistonMovingPiece::isCubeShaped()
{
	return false;
}

bool PistonMovingPiece::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if( soundOnly) return false;
	// this is a special case in order to help removing invisible, unbreakable, blocks in the world
	if (!level->isClientSide && level->getTileEntity(x, y, z) == nullptr)
	{
		// this block is no longer valid
		level->removeTile(x, y, z);
		return true;
	}
	return false;
}

int PistonMovingPiece::getResource(int data, Random *random, int playerBonusLevel)
{
	return 0;
}

void PistonMovingPiece::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonus)
{
	if (level->isClientSide) return;

	shared_ptr<PistonPieceEntity> entity = getEntity(level, x, y, z);
	if (entity == nullptr)
	{
		return;
	}

	Tile::tiles[entity->getId()]->spawnResources(level, x, y, z, entity->getData(), 0);
}

void PistonMovingPiece::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (!level->isClientSide)
	{
		level->getTileEntity(x, y, z) == nullptr;
	}
}

shared_ptr<TileEntity> PistonMovingPiece::newMovingPieceEntity(int block, int data, int facing, bool extending, bool isSourcePiston)
{
	return std::make_shared<PistonPieceEntity>(block, data, facing, extending, isSourcePiston);
}

AABB *PistonMovingPiece::getAABB(Level *level, int x, int y, int z)
{
	shared_ptr<PistonPieceEntity> entity = getEntity(level, x, y, z);
	if (entity == nullptr)
	{
		return nullptr;
	}

	// move the aabb depending on the animation
	float progress = entity->getProgress(0);
	if (entity->isExtending())
	{
		progress = 1.0f - progress;
	}
	return getAABB(level, x, y, z, entity->getId(), progress, entity->getFacing());
}

void PistonMovingPiece::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	shared_ptr<PistonPieceEntity> entity = dynamic_pointer_cast<PistonPieceEntity>(forceEntity);
	if( entity == nullptr ) entity = getEntity(level, x, y, z);
	if (entity != nullptr)
	{
		Tile *tile = Tile::tiles[entity->getId()];
		if (tile == nullptr || tile == this)
		{
			return;
		}
		tile->updateShape(level, x, y, z);

		float progress = entity->getProgress(0);
		if (entity->isExtending())
		{
			progress = 1.0f - progress;
		}
		int facing = entity->getFacing();
		ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
		tls->xx0 = tile->getShapeX0() - Facing::STEP_X[facing] * progress;
		tls->yy0 = tile->getShapeY0() - Facing::STEP_Y[facing] * progress;
		tls->zz0 = tile->getShapeZ0() - Facing::STEP_Z[facing] * progress;
		tls->xx1 = tile->getShapeX1() - Facing::STEP_X[facing] * progress;
		tls->yy1 = tile->getShapeY1() - Facing::STEP_Y[facing] * progress;
		tls->zz1 = tile->getShapeZ1() - Facing::STEP_Z[facing] * progress;
	}
}

AABB *PistonMovingPiece::getAABB(Level *level, int x, int y, int z, int tile, float progress, int facing)
{
	if (tile == 0 || tile == id)
	{
		return nullptr;
	}
	AABB *aabb = Tile::tiles[tile]->getAABB(level, x, y, z);

	if (aabb == nullptr)
	{
		return nullptr;
	}

	// move the aabb depending on the animation
	if (Facing::STEP_X[facing] < 0)
	{
		aabb->x0 -= Facing::STEP_X[facing] * progress;
	}
	else
	{
		aabb->x1 -= Facing::STEP_X[facing] * progress;
	}
	if (Facing::STEP_Y[facing] < 0)
	{
		aabb->y0 -= Facing::STEP_Y[facing] * progress;
	}
	else
	{
		aabb->y1 -= Facing::STEP_Y[facing] * progress;
	}
	if (Facing::STEP_Z[facing] < 0)
	{
		aabb->z0 -= Facing::STEP_Z[facing] * progress;
	}
	else
	{
		aabb->z1 -= Facing::STEP_Z[facing] * progress;
	}
	return aabb;
}

shared_ptr<PistonPieceEntity> PistonMovingPiece::getEntity(LevelSource *level, int x, int y, int z)
{
	shared_ptr<TileEntity> tileEntity = level->getTileEntity(x, y, z);
	if (tileEntity != nullptr && dynamic_pointer_cast<PistonPieceEntity>(tileEntity) != nullptr)
	{
		return  dynamic_pointer_cast<PistonPieceEntity>(tileEntity);
	}
	return nullptr;
}

void PistonMovingPiece::registerIcons(IconRegister *iconRegister)
{
	// don't register null, register piston top instead (to get proper
	// particle effect)
	icon = iconRegister->registerIcon(L"piston_top");
}

int PistonMovingPiece::cloneTileId(Level *level, int x, int y, int z)
{
	return 0;
}
