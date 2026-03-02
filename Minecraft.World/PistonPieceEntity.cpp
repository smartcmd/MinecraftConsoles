#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "PistonPieceEntity.h"
#include "PistonMovingPiece.h"
#include "net.minecraft.world.level.h"
#include "Facing.h"
#include "Tile.h"



PistonPieceEntity::PistonPieceEntity()
{
	// for the tile entity loader

	// 4J - added initialisers
	this->id = 0;
	this->data = 0;
	this->facing = 0;
	this->extending = 0;
	this->_isSourcePiston = 0;
	progress = 0.0f;
	progressO = 0.0f;
}

PistonPieceEntity::PistonPieceEntity(int id, int data, int facing, bool extending, bool isSourcePiston) : TileEntity()
{
	// 4J - added initialisers
	progress = 0.0f;
	progressO = 0.0f;

	this->id = id;
	this->data = data;
	this->facing = facing;
	this->extending = extending;
	this->_isSourcePiston = isSourcePiston;
}

int PistonPieceEntity::getId()
{
	return id;
}

int PistonPieceEntity::getData()
{
	return data;
}

bool PistonPieceEntity::isExtending()
{
	return extending;
}

int PistonPieceEntity::getFacing()
{
	return facing;
}

bool PistonPieceEntity::isSourcePiston()
{
	return _isSourcePiston;
}

float PistonPieceEntity::getProgress(float a)
{
	if (a > 1)
	{
		a = 1;
	}
	return progressO + (progress - progressO) * a;
}

float PistonPieceEntity::getXOff(float a)
{
	if (extending)
	{
		return (getProgress(a) - 1.0f) * Facing::STEP_X[facing];
	}
	else
	{
		return (1.0f - getProgress(a)) * Facing::STEP_X[facing];
	}
}

float PistonPieceEntity::getYOff(float a)
{
	if (extending)
	{
		return (getProgress(a) - 1.0f) * Facing::STEP_Y[facing];
	}
	else
	{
		return (1.0f - getProgress(a)) * Facing::STEP_Y[facing];
	}
}

float PistonPieceEntity::getZOff(float a)
{
	if (extending)
	{
		return (getProgress(a) - 1.0f) * Facing::STEP_Z[facing];
	}
	else
	{
		return (1.0f - getProgress(a)) * Facing::STEP_Z[facing];
	}
}

void PistonPieceEntity::moveCollidedEntities(float progress, float amount)
{
	if (extending)
	{
		progress = 1.0f - progress;
	}
	else
	{
		progress = progress - 1.0f;
	}

	AABB *aabb = Tile::pistonMovingPiece->getAABB(level, x, y, z, id, progress, facing);
	if (aabb != NULL)
	{
		vector<shared_ptr<Entity> > *entities = level->getEntities(nullptr, aabb);
		if (!entities->empty())
		{
			vector< shared_ptr<Entity> > collisionHolder;
			for( AUTO_VAR(it, entities->begin()); it != entities->end(); it++ )
			{
				collisionHolder.push_back(*it);
			}

			for( AUTO_VAR(it, collisionHolder.begin()); it != collisionHolder.end(); it++ )
			{
				(*it)->move(amount * Facing::STEP_X[facing],
							amount * Facing::STEP_Y[facing],
							amount * Facing::STEP_Z[facing]);
			}
		}
	}
}

void PistonPieceEntity::finalTick()
{
	if (progressO < 1 && level != NULL)
	{
		progressO = progress = 1;
		level->removeTileEntity(x, y, z);
		setRemoved();
		if (level->getTile(x, y, z) == Tile::pistonMovingPiece_Id)
			level->setTileAndData(x, y, z, id, data);
	}
}

void PistonPieceEntity::tick()
{
	progressO = progress;

	if (progressO >= 1)
	{
		moveCollidedEntities(1, 4 / 16.f);
		level->removeTileEntity(x, y, z);
		setRemoved();
		if (level->getTile(x, y, z) == Tile::pistonMovingPiece_Id)
			level->setTileAndData(x, y, z, id, data);
		return;
	}

	progress += .5f;
	if (progress >= 1)
	{
		progress = 1;
	}

	if (extending)
	{
		moveCollidedEntities(progress, (progress - progressO) + 1.0f / 16.0f);
	}
}

void PistonPieceEntity::load(CompoundTag *tag)
{
	TileEntity::load(tag);

	id = tag->getInt(L"blockId");
	data = tag->getInt(L"blockData");
	facing = tag->getInt(L"facing");
	progressO = progress = tag->getFloat(L"progress");
	extending = tag->getBoolean(L"extending");
}

void PistonPieceEntity::save(CompoundTag *tag)
{
	TileEntity::save(tag);

	tag->putInt(L"blockId", id);
	tag->putInt(L"blockData", data);
	tag->putInt(L"facing", facing);
	tag->putFloat(L"progress", progressO);
	tag->putBoolean(L"extending", extending);
}

// 4J Added
shared_ptr<TileEntity> PistonPieceEntity::clone()
{
	shared_ptr<PistonPieceEntity> result = shared_ptr<PistonPieceEntity>( new PistonPieceEntity() );
	TileEntity::clone(result);

	result->id = id;
	result->data = data;
	result->facing = facing;
	result->extending = extending;
	result->_isSourcePiston = _isSourcePiston;
	result->progress = progress;
	result->progressO = progressO;
	return result;
}
